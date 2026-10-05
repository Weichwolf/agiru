#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-conversion.XXXXXX)
mkdir -p "$proof/source"
cp test/runtime/conversion/Fixture.Codeunit.al "$proof/source/Fixture.Codeunit.al"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$proof/apps.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate -Isrc/net
  "-I$proof/generated/fixture" "-I$proof/generated/absent" "-I$proof/generated/shared")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
gate="$B/gate_ConvertGate"
"$gate" > "$proof/current.log" 2>&1
if [ "${AGIRU_BASE64_REFERENCE+x}" = x ]; then
  reference_arguments=("$AGIRU_BASE64_REFERENCE")
  if [ "${AGIRU_CONVERT_REFERENCE+x}" = x ]; then
    reference_arguments+=("$AGIRU_CONVERT_REFERENCE")
    sha256sum "$AGIRU_CONVERT_REFERENCE" > "$proof/region-reference.sha256"
  fi
  "$gate" "${reference_arguments[@]}" > "$proof/reference.log" 2>&1
  sha256sum "$AGIRU_BASE64_REFERENCE" > "$proof/core-reference.sha256"
elif [ "${AGIRU_CONVERT_REFERENCE+x}" = x ]; then
  printf 'conversion: region reference requires the byte-core reference\n' >&2
  exit 2
fi
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" > "$proof/generation.log" 2>&1
rg -q '^absent    0 .NET type\(s\) with 0 member\(s\), 0 AL object\(s\) with 0$' "$proof/generation.log"
mapfile -t sources < <(rg --files --no-ignore "$proof/generated" -g '*.cpp' | LC_ALL=C sort)
[ "${#sources[@]}" -gt 0 ]
runner_source=test/runtime/conversion/Runner.cpp
command=("$CXX" "${flags[@]}" -c "$runner_source" -o "$proof/runner.o")
"${command[@]}"
"$CXX" "${flags[@]}" "$proof/runner.o" "${sources[@]}" "${links[@]}" -o "$proof/runner"
"$proof/runner" | tee "$proof/execution.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/$runner_source" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "${command[@]}" > "$B/fixture-commands/conversion.json"
for control in wrong-offset no-block-seam ignore-formatting; do
  awk -v control="$control" '
    control == "wrong-offset" && /detail::ReadByteBlock\(bytes, offset \+ consumed,/ {
      sub(/offset \+ consumed/, "consumed"); changed++
    }
    control == "no-block-seam" && /if \(lines && consumed != 0\) \{ result \+=/ {
      $0 = ""; changed++
    }
    control == "ignore-formatting" && /InsertLineBreaks\(\).Ordinal\(\)\) \{ return true;/ {
      sub(/return true/, "return false"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/net/Convert.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" -fPIC -shared "$proof/$control.cpp" "${links[@]}" \
    -o "$proof/$control.so"
  for fixture in "$gate" "$proof/runner"; do
    name=$(basename "$fixture")
    if LD_PRELOAD="$proof/$control.so" "$fixture" > "$proof/$control-$name.log" 2>&1; then
      printf 'conversion: %s escaped %s\n' "$control" "$name" >&2
      exit 1
    fi
    rg -q 'FAIL ' "$proof/$control-$name.log"
  done
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
awk '
  /if \(value < 0 \|\| value > std::numeric_limits<unsigned char>::max\(\)\)/ {
    $0 = "    if (false) {"; changed++
  }
  { print }
  END { if (changed != 1) exit 2 }
' src/net/ByteArray.cpp > "$proof/byte-narrow.cpp"
"$CXX" "${flags[@]}" -fPIC -shared "$proof/byte-narrow.cpp" "${links[@]}" -o "$proof/byte-narrow.so"
for fixture in "$gate" "$proof/runner"; do
  name=$(basename "$fixture")
  if LD_PRELOAD="$proof/byte-narrow.so" "$fixture" > "$proof/byte-narrow-$name.log" 2>&1; then
    printf 'conversion: byte narrowing escaped %s\n' "$name" >&2
    exit 1
  fi
  rg -q 'FAIL ' "$proof/byte-narrow-$name.log"
done
sha256sum "$proof/byte-narrow.cpp" "$proof/byte-narrow.so" >> "$proof/controls.sha256"
rm -- "$proof/byte-narrow.cpp" "$proof/byte-narrow.so"
sanitizers=(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
"$CXX" "${flags[@]}" "${sanitizers[@]}" test/gate/ConvertGate.cpp "${links[@]}" \
  -o "$proof/sanitized-gate"
"$CXX" "${flags[@]}" "${sanitizers[@]}" "$runner_source" "${sources[@]}" "${links[@]}" \
  -o "$proof/sanitized-runner"
"$CXX" "${flags[@]}" "${sanitizers[@]}" -fPIC -shared src/net/Convert.cpp src/net/ByteArray.cpp \
  "${links[@]}" -o "$proof/sanitized.so"
for fixture in sanitized-gate sanitized-runner; do
  env ASAN_OPTIONS=detect_stack_use_after_return=1:symbolize=0 \
    UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=0 LD_PRELOAD="$proof/sanitized.so" \
    "$proof/$fixture" > "$proof/$fixture.log" 2>&1
done
sha256sum "$proof/sanitized-gate" "$proof/sanitized-runner" "$proof/sanitized.so" \
  > "$proof/sanitizers.sha256"
rm -- "$proof/sanitized-gate" "$proof/sanitized-runner" "$proof/sanitized.so"
sha256sum src/net/Convert.cpp src/net/ByteArray.cpp src/net/ByteArray.h \
  include/dotnet/Convert.h include/dotnet/Base64FormattingOptions.h test/gate/ConvertGate.cpp \
  "$runner_source" test/runtime/conversion/Fixture.Codeunit.al src/gen/RuntimeSurface.cpp \
  "$B/libagiru_net.so" "$B/agirutc" "$gate" "$proof/runner" > "$proof/inputs.sha256"
rm -- "$proof/runner.o" "$proof/runner"
printf 'conversion: byte arrays and generated AL pass; four compiled controls reject both; %s\n' "$proof"

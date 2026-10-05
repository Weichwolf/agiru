#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-hashing.XXXXXX)
mkdir -p "$proof/source"
cp test/runtime/hashing/Fixture.Codeunit.al "$proof/source/Fixture.Codeunit.al"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$proof/apps.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate -Isrc/net
  "-I$proof/generated/fixture" "-I$proof/generated/absent" "-I$proof/generated/shared")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
gate="$B/gate_HashAlgorithmGate"
"$gate" > "$proof/current.log" 2>&1
if [ "${AGIRU_HASH_REFERENCE+x}" = x ]; then
  "$gate" "$AGIRU_HASH_REFERENCE" > "$proof/reference.log" 2>&1
  sha256sum "$AGIRU_HASH_REFERENCE" > "$proof/reference.sha256"
fi
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" > "$proof/generation.log" 2>&1
rg -q '^absent    0 .NET type\(s\) with 0 member\(s\), 0 AL object\(s\) with 0$' "$proof/generation.log"
mapfile -t sources < <(rg --files --no-ignore "$proof/generated" -g '*.cpp' | LC_ALL=C sort)
[ "${#sources[@]}" -gt 0 ]
runner_source=test/runtime/hashing/Runner.cpp
command=("$CXX" "${flags[@]}" -c "$runner_source" -o "$proof/runner.o")
"${command[@]}"
"$CXX" "${flags[@]}" "$proof/runner.o" "${sources[@]}" "${links[@]}" -o "$proof/runner"
"$proof/runner" | tee "$proof/execution.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/$runner_source" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "${command[@]}" > "$B/fixture-commands/hashing.json"
for control in wrong-algorithm wrong-offset shared-dispose; do
  awk -v control="$control" '
    control == "wrong-algorithm" && /if \(normalized == "md5"\) \{ return "MD5"; \}/ {
      sub(/return "MD5"/, "return \"SHA256\""); changed++
    }
    control == "wrong-offset" && /^  UpdateDigest\(\*context, bytes, offset, count\);$/ {
      sub(/bytes, offset, count/, "bytes, 0, count"); changed++
    }
    control == "shared-dispose" && /^  state_->digest.reset\(\);$/ {
      $0 = "  static_cast<void>(state_->digest);"; changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/net/HashAlgorithm.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" -fPIC -shared "$proof/$control.cpp" "${links[@]}" -lcrypto \
    -o "$proof/$control.so"
  for fixture in "$gate" "$proof/runner"; do
    name=$(basename "$fixture")
    if LD_PRELOAD="$proof/$control.so" "$fixture" > "$proof/$control-$name.log" 2>&1; then
      printf 'hashing: %s escaped %s\n' "$control" "$name" >&2
      exit 1
    fi
    rg -q 'FAIL ' "$proof/$control-$name.log"
  done
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done
sanitizers=(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
"$CXX" "${flags[@]}" "${sanitizers[@]}" test/gate/HashAlgorithmGate.cpp "${links[@]}" \
  -o "$proof/sanitized-gate"
"$CXX" "${flags[@]}" "${sanitizers[@]}" "$runner_source" "${sources[@]}" "${links[@]}" \
  -o "$proof/sanitized-runner"
"$CXX" "${flags[@]}" "${sanitizers[@]}" -fPIC -shared src/net/HashAlgorithm.cpp src/net/ByteArray.cpp \
  "${links[@]}" -lcrypto -o "$proof/sanitized.so"
for fixture in sanitized-gate sanitized-runner; do
  env ASAN_OPTIONS=detect_stack_use_after_return=1:symbolize=0 \
    UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=0 LD_PRELOAD="$proof/sanitized.so" \
    "$proof/$fixture" > "$proof/$fixture.log" 2>&1
done
sha256sum "$proof/sanitized-gate" "$proof/sanitized-runner" "$proof/sanitized.so" \
  > "$proof/sanitizers.sha256"
rm -- "$proof/sanitized-gate" "$proof/sanitized-runner" "$proof/sanitized.so"
sha256sum src/net/HashAlgorithm.cpp src/net/ByteArray.cpp src/net/ByteArray.h include/dotnet/HashAlgorithm.h \
  test/gate/HashAlgorithmGate.cpp "$runner_source" test/runtime/hashing/Fixture.Codeunit.al \
  src/gen/RuntimeSurface.cpp "$B/libagiru_net.so" "$B/agirutc" "$gate" "$proof/runner" \
  > "$proof/inputs.sha256"
rm -- "$proof/runner.o" "$proof/runner"
printf 'hashing: generic byte hashing and generated AL pass; three compiled controls reject both; %s\n' "$proof"

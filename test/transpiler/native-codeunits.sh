#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
input="$PWD/test/transpiler/native-codeunits"
proof=$(mktemp -d /tmp/agiru-native-codeunits.XXXXXX)
printf 'native-codeunits: receipts %s\n' "$proof"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate)
links=(--rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
sha256sum src/gen/{CodeunitWriter,NativeSource,Refused}.{cpp,h} src/tc/Main.cpp \
  "$B/agirutc" "$B/libagiru_gen.so" "$B/libagiru_al.so" \
  "$B/libagiru_rt.so" "$B/libagiru_net.so" > "$proof/inputs.sha256"
find "$input" -type f -exec sha256sum {} + >> "$proof/inputs.sha256"

generate() {
  local root=$1 output=$2 expected=$3 status=0
  "$B/agirutc" "$root" "$root/apps.json" "$output" > "$output.log" 2>&1 || status=$?
  [ "$status" -eq "$expected" ] || { printf 'native-codeunits: unexpected translation exit %s\n' "$status" >&2; exit 1; }
}

link_generated() {
  local output=$1 object=$2 runner=$3
  local sources=()
  mapfile -t sources < <(rg --files --no-ignore "$output" -g '*.cpp' | LC_ALL=C sort)
  [ "${#sources[@]}" -gt 0 ]
  "$CXX" "${flags[@]}" "-I$output/fixture" "-I$output/platform" "-I$output/shared" "-I$output/absent" \
    "-I$output" "$object" "${sources[@]}" "${links[@]}" -o "$runner"
}

record_command() {
  local name=$1 file=$2
  shift 2
  mkdir -p "$B/fixture-commands"
  jq -n --arg directory "$PWD" --arg file "$file" \
    --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- "$@" \
    > "$B/fixture-commands/$name.json"
}

generate "$input" "$proof/generated" 1
rg -q '^refused +7 ' "$proof/generated.log"
"$CXX" "${flags[@]}" "-I$proof/generated/fixture" -c "$input/Runner.cpp" -o "$proof/runner.o"
record_command native-codeunits "$input/Runner.cpp" \
  "$CXX" "${flags[@]}" "-I$proof/generated/fixture" -c "$input/Runner.cpp" -o "$proof/runner.o"
link_generated "$proof/generated" "$proof/runner.o" "$proof/runner"
"$proof/runner" | tee "$proof/execution.log"

cp -a "$input" "$proof/non-native"
sed -i '/\[Native\]/d; /\[nAtIvE\]/d' "$proof/non-native/source/NativeFixture.Codeunit.al"
! cmp -s "$input/source/NativeFixture.Codeunit.al" "$proof/non-native/source/NativeFixture.Codeunit.al"
generate "$proof/non-native" "$proof/non-native/generated" 0
link_generated "$proof/non-native/generated" "$proof/runner.o" "$proof/non-native/runner"
if "$proof/non-native/runner" > "$proof/non-native-execution.log" 2>&1; then
  printf 'native-codeunits: missing Native attributes escaped execution control\n' >&2
  exit 1
fi
rg -q 'unbound Native refuses with its original typed signature' "$proof/non-native-execution.log"

bound="$input/source-bound"
status=0
"$B/agirutc" "$bound" "$bound/apps.json" "$proof/source-bound" \
  --system-symbols "$bound/native" > "$proof/source-bound.log" 2>&1 || status=$?
[ "$status" -eq 1 ]
rg -q '2 codeunit sources indexed; 2 codeunit objects written into the platform app' "$proof/source-bound.log"
rg -q '2 codeunit declarations selected; 1 native methods unbound' "$proof/source-bound.log"
"$CXX" "${flags[@]}" "-I$proof/source-bound/fixture" "-I$proof/source-bound/platform" \
  -c "$input/SourceRunner.cpp" -o "$proof/source-runner.o"
record_command native-codeunits-source "$input/SourceRunner.cpp" \
  "$CXX" "${flags[@]}" "-I$proof/source-bound/fixture" "-I$proof/source-bound/platform" \
  -c "$input/SourceRunner.cpp" -o "$proof/source-runner.o"
link_generated "$proof/source-bound" "$proof/source-runner.o" "$proof/source-runner"
"$proof/source-runner" | tee "$proof/source-execution.log"

cp -a "$bound" "$proof/id-mutant"
sed -i 's/codeunit 50321/codeunit 50320/' "$proof/id-mutant/native/src/odd-source-name.aL"
status=0
"$B/agirutc" "$proof/id-mutant" "$proof/id-mutant/apps.json" "$proof/id-generated" \
  --system-symbols "$proof/id-mutant/native" > "$proof/id-mutant.log" 2>&1 || status=$?
[ "$status" -eq 1 ]
if "$CXX" "${flags[@]}" "-I$proof/id-generated/fixture" "-I$proof/id-generated/platform" \
  "-I$proof/id-generated/absent" "-I$proof/id-generated/shared" \
  -c "$input/SourceRunner.cpp" -o "$proof/id-runner.o" > "$proof/id-control.log" 2>&1; then
  printf 'native-codeunits: wrong source ID escaped compilation control\n' >&2
  exit 1
fi
rg -q 'static assertion failed' "$proof/id-control.log"

sources=()
mapfile -t sources < <(rg --files --no-ignore "$proof/source-bound" -g '*.cpp' | \
  LC_ALL=C sort | sed '\@/platform/system/fixture/codeunit/SourceNative.cpp$@d')
if "$CXX" "${flags[@]}" "-I$proof/source-bound/fixture" "-I$proof/source-bound/platform" \
  "$proof/source-runner.o" "${sources[@]}" "${links[@]}" -o "$proof/missing-definition" \
  > "$proof/missing-definition-control.log" 2>&1; then
  printf 'native-codeunits: missing native source definition escaped link control\n' >&2
  exit 1
fi
rg -q 'undefined symbol.*SourceNative_Codeunit' "$proof/missing-definition-control.log"

if [ -n "${AGIRU_SYSTEM_SYMBOLS:-}" ]; then
  python3 scripts/fetch_symbols.py --verify "$AGIRU_SYSTEM_SYMBOLS"
  original="$AGIRU_SYSTEM_SYMBOLS/src/System Codeunits/Runtime/Base64Convert.Codeunit.al"
  sha256sum "$original" "$AGIRU_SYSTEM_SYMBOLS/NavxManifest.xml" \
    test/transpiler/native-binding/Emit.cpp >> "$proof/inputs.sha256"
  emitter_flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Isrc/al -Isrc/gen)
  "$CXX" "${emitter_flags[@]}" -c test/transpiler/native-binding/Emit.cpp -o "$proof/emitter.o"
  record_command native-codeunit-emitter "$PWD/test/transpiler/native-binding/Emit.cpp" \
    "$CXX" "${emitter_flags[@]}" -c test/transpiler/native-binding/Emit.cpp -o "$proof/emitter.o"
  "$CXX" "${emitter_flags[@]}" "$proof/emitter.o" "${links[@]}" -lagiru_gen -lagiru_al -o "$proof/emitter"
  "$proof/emitter" --codeunit "$original" "$proof/original-generated"
  "$CXX" "${flags[@]}" "-I$proof/original-generated" -c "$input/OriginalRunner.cpp" -o "$proof/original-runner.o"
  record_command native-codeunits-original "$input/OriginalRunner.cpp" \
    "$CXX" "${flags[@]}" "-I$proof/original-generated" -c "$input/OriginalRunner.cpp" -o "$proof/original-runner.o"
  link_generated "$proof/original-generated" "$proof/original-runner.o" "$proof/original-runner"
  "$proof/original-runner" | tee "$proof/original-execution.log"
  sha256sum --check --status "$proof/inputs.sha256"
  rm -f "$proof/emitter.o" "$proof/emitter" "$proof/original-runner.o" "$proof/original-runner"
fi
rm -f "$proof/runner.o" "$proof/runner" "$proof/non-native/runner" \
  "$proof/source-runner.o" "$proof/source-runner"
printf 'native-codeunits: unbound declarations refuse before effects; missing-attribute control fails; %s\n' "$proof"

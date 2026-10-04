#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-native-enums.XXXXXX)
printf '%s\n' "$proof" > "$B/native-enums.latest"
input="$PWD/test/transpiler/native-enums"
"$B/agirutc" "$input" "$input/apps.json" "$proof/generated" \
  --system-symbols "$input/package" > "$proof/generation.log" 2>&1
rg -q 'native 1 enum sources bound; 1 enum objects written' "$proof/generation.log"
rg -q 'native 2 interface sources bound; 2 interface objects written' "$proof/generation.log"
rg -q '^codeunits[[:space:]]+1 of 1 parsed .*1 \[Test\] methods' "$proof/generation.log"
rg -q 'Native Enum Fixture' "$proof/generated/platform/PlatformModule.h"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate
  "-I$proof/generated/fixture" "-I$proof/generated/shared" "-I$proof/generated/platform"
  "-I$proof/generated/absent")
mapfile -t sources < <(rg --files --no-ignore "$proof/generated" -g '*.cpp' | LC_ALL=C sort)
[ "${#sources[@]}" -gt 0 ]
"$CXX" "${flags[@]}" -c test/transpiler/native-enums/Runner.cpp -o "$proof/runner.o"
"$CXX" "${flags[@]}" "$proof/runner.o" "${sources[@]}" --rtlib=compiler-rt \
  --unwindlib=libunwind -fuse-ld=lld-19 "-L$B" "-Wl,-rpath,$B" \
  -lagiru_rt -lagiru_net -lagiru_db -o "$proof/runner"
"$proof/runner" | tee "$proof/execution.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/transpiler/native-enums/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" -c test/transpiler/native-enums/Runner.cpp -o "$proof/runner.o" \
  > "$B/fixture-commands/native-enums.json"
"$B/agirutc" "$input" "$input/apps.json" --system-symbols "$input/package" \
  > "$proof/source-only.log" 2>&1
rg -q 'native 1 enum sources bound; 0 enum objects written' "$proof/source-only.log"

for control in wrong-ordinal wrong-caption; do
  cp -a "$input" "$proof/$control"
  changed="$proof/$control/package/src/unusual-name.aL"
  if [ "$control" = wrong-ordinal ]; then
    sed -i 's/value(10; Chosen)/value(11; Chosen)/' "$changed"
  else
    sed -i "s/Caption = 'Chosen value'/Caption = 'Wrong caption'/" "$changed"
  fi
  cmp -s "$input/package/src/unusual-name.aL" "$changed" && exit 1
  "$B/agirutc" "$proof/$control" "$proof/$control/apps.json" "$proof/$control/generated" \
    --system-symbols "$proof/$control/package" > "$proof/$control.log" 2>&1
  mutant_flags=(-Iinclude -Itest/gate "-I$proof/$control/generated/fixture"
    "-I$proof/$control/generated/shared" "-I$proof/$control/generated/platform"
    "-I$proof/$control/generated/absent")
  mapfile -t mutant_sources < <(rg --files --no-ignore "$proof/$control/generated" -g '*.cpp' | LC_ALL=C sort)
  "$CXX" -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror \
    "${mutant_flags[@]}" test/transpiler/native-enums/Runner.cpp "${mutant_sources[@]}" \
    --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 "-L$B" "-Wl,-rpath,$B" \
    -lagiru_rt -lagiru_net -lagiru_db -o "$proof/$control/runner"
  if "$proof/$control/runner" > "$proof/$control-execution.log" 2>&1; then
    printf 'native-enums: %s escaped the execution control\n' "$control" >&2
    exit 1
  fi
done

cp -a "$input" "$proof/wrong-interface-signature"
sed -i 's/procedure Echo(var Value/procedure Echo(Value/' \
  "$proof/wrong-interface-signature/package/src/contract-derived.aL"
"$B/agirutc" "$proof/wrong-interface-signature" \
  "$proof/wrong-interface-signature/apps.json" "$proof/wrong-interface-signature/generated" \
  --system-symbols "$proof/wrong-interface-signature/package" \
  > "$proof/wrong-interface-signature/generation.log" 2>&1
if "$CXX" -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror \
  -Iinclude -Itest/gate "-I$proof/wrong-interface-signature/generated/fixture" \
  "-I$proof/wrong-interface-signature/generated/shared" \
  "-I$proof/wrong-interface-signature/generated/platform" \
  "-I$proof/wrong-interface-signature/generated/absent" \
  -fsyntax-only test/transpiler/native-enums/Runner.cpp > "$proof/wrong-interface-signature/compile.log" 2>&1; then
  printf 'native-enums: changed native argument mode escaped compilation\n' >&2
  exit 1
fi
rg -q 'abstract|pure virtual' "$proof/wrong-interface-signature/compile.log"

cmake_source="$proof/cmake-source"
mkdir -p "$cmake_source"
cp CMakeLists.txt "$cmake_source/"
cp -a src include cmake test scripts "$cmake_source/"
cp -a "$proof/generated" "$cmake_source/apps"
cp "$input/apps.json" "$cmake_source/apps.json"
printf '%s\n' 'fixture/fixture/codeunit/NativeConsumerUT.cpp' \
  'fixture/fixture/table/NativeRecord.cpp' 'fixture/fixture/table/NativeRecord.def.cpp' \
  > "$cmake_source/test/slice"
for mode in app slice; do
  app_mode=OFF
  slice_mode=OFF
  if [ "$mode" = app ]; then app_mode=ON; else slice_mode=ON; fi
  cmake -S "$cmake_source" -B "$proof/cmake-$mode" -G Ninja \
    -DCMAKE_CXX_COMPILER="$CXX" -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DAGIRU_BUILD_APPS="$app_mode" -DAGIRU_BUILD_SLICE="$slice_mode" \
    > "$proof/cmake-$mode-configure.log" 2>&1
  if [ "$mode" = app ]; then selector='/apps/fixture/'; else selector='/Unity/unity_stable/'; fi
  jq -r --arg selector "$selector" \
    '.[] | select(.file | contains($selector)) | .command | capture(" -o (?<object>[^ ]+) ").object' \
    "$proof/cmake-$mode/compile_commands.json" > "$proof/cmake-$mode-objects.txt"
  mapfile -t objects < "$proof/cmake-$mode-objects.txt"
  [ "${#objects[@]}" -gt 0 ]
  cmake --build "$proof/cmake-$mode" -j "${JOBS:-2}" --target "${objects[@]}" \
    > "$proof/cmake-$mode-compile.log" 2>&1
done
mv "$cmake_source/apps/platform/PlatformModule.h" "$proof/module.saved"
if cmake -S "$cmake_source" -B "$proof/cmake-missing-module" -G Ninja \
  -DCMAKE_CXX_COMPILER="$CXX" -DAGIRU_BUILD_APPS=ON -DAGIRU_BUILD_SLICE=OFF \
  > "$proof/cmake-missing-module.log" 2>&1; then
  printf 'native-enums: missing header-only platform owner escaped configuration\n' >&2
  exit 1
fi
rg -q 'source-owned platform module' "$proof/cmake-missing-module.log"
mv "$proof/module.saved" "$cmake_source/apps/platform/PlatformModule.h"
printf 'native-enums: app/slice CMake consumers compile without a synthetic platform source; %s\n' "$proof"

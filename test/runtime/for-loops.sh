#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-for-loops.XXXXXX)
printf 'for-loops: receipts %s\n' "$proof"
mkdir -p "$proof/source"
cp test/runtime/for-loops/Fixture.Codeunit.al "$proof/source/Fixture.Codeunit.al"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$proof/apps.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate
  "-I$proof/generated/fixture" "-I$proof/generated/absent" "-I$proof/generated/shared")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
sha256sum test/runtime/for-loops.sh test/runtime/for-loops/Fixture.Codeunit.al \
  test/runtime/for-loops/Runner.cpp \
  src/gen/BodyWriter.cpp "$B/agirutc" "$B/libagiru_gen.so" "$B/libagiru_al.so" \
  "$B/libagiru_db.so" "$B/libagiru_net.so" "$B/libagiru_rt.so" > "$proof/inputs.sha256"
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" > "$proof/generation.log" 2>&1
rg -q '^absent    0 .NET type\(s\) with 0 member\(s\), 0 AL object\(s\) with 0$' "$proof/generation.log"
mapfile -t sources < <(rg --files --no-ignore "$proof/generated" -g '*.cpp' | LC_ALL=C sort)
[ "${#sources[@]}" -eq 1 ]
source=${sources[0]}
"$CXX" "${flags[@]}" -c test/runtime/for-loops/Runner.cpp -o "$proof/runner.o"
"$CXX" "${flags[@]}" "$proof/runner.o" "$source" "${links[@]}" -o "$proof/runner"
"$proof/runner" "$dsn" | tee "$proof/execution.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/runtime/for-loops/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" -c test/runtime/for-loops/Runner.cpp -o "$proof/runner.o" \
  > "$B/fixture-commands/for-loops.json"
for control in integer boolean borrowed queue; do
  case "$control" in
    integer)
      sed '/::OrderedBounds(/,/^}/ {
        s/const auto agiruForEnd_Block_/[[maybe_unused]] const auto agiruForEnd_Block_/
        s/I <= agiruForEnd_Block_2/I <= IntegerBound(Trace, Calls, 3)/
      }' "$source" > "$proof/$control.cpp"
      expected='integer bounds execute exactly once each' ;;
    boolean)
      sed '/::BooleanBounds(/,/^}/ {
        s/const auto agiruForEnd_Block_/[[maybe_unused]] const auto agiruForEnd_Block_/
        s/(agiruForEnd_Block_2 ? 1 : 0)/(BooleanBound(Calls, Bound) ? 1 : 0)/
      }' "$source" > "$proof/$control.cpp"
      expected='Boolean end expressions execute exactly once' ;;
    borrowed)
      sed '/::MutableBound(/,/^}/ s/const auto agiruForEnd_Block_/const auto \&agiruForEnd_Block_/' \
        "$source" > "$proof/$control.cpp"
      expected='body mutation does not change the captured integer bound' ;;
    queue)
      sed '/::QueuedValues(/,/^}/ {
        s/const auto agiruForEnd_Block_/[[maybe_unused]] const auto agiruForEnd_Block_/
        s/I <= agiruForEnd_Block_1/I <= DequeueInteger()/
      }' "$source" > "$proof/$control.cpp"
      expected='the Variant does not hold' ;;
  esac
  if cmp -s "$source" "$proof/$control.cpp"; then
    printf 'for-loops: %s control did not match\n' "$control" >&2
    exit 1
  fi
  "$CXX" "${flags[@]}" "-I$(dirname "$source")" "$proof/runner.o" \
    "$proof/$control.cpp" "${links[@]}" -o "$proof/$control"
  if "$proof/$control" "$dsn" > "$proof/$control.log" 2>&1; then
    printf 'for-loops: %s control escaped execution\n' "$control" >&2
    exit 1
  fi
  rg -q "$expected" "$proof/$control.log"
  rm -f "$proof/$control" "$proof/$control.cpp"
done
rm -f "$proof/runner.o" "$proof/runner"
printf 'for-loops: captured ordered bounds execute; four compiled controls fail; %s\n' "$proof"

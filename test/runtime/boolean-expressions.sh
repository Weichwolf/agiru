#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-boolean-expressions.XXXXXX)
printf 'boolean-expressions: receipts %s\n' "$proof"
mkdir -p "$proof/source"
cp test/runtime/boolean-expressions/Fixture.Codeunit.al "$proof/source/Fixture.Codeunit.al"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$proof/apps.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate
  "-I$proof/generated/fixture" "-I$proof/generated/absent" "-I$proof/generated/shared")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
sha256sum test/runtime/boolean-expressions/Fixture.Codeunit.al \
  test/runtime/boolean-expressions/Runner.cpp include/type/BooleanExpression.h \
  src/gen/BodyWriter.cpp src/gen/RuntimeSurface.cpp "$B/agirutc" \
  "$B/libagiru_gen.so" "$B/libagiru_al.so" "$B/libagiru_db.so" \
  "$B/libagiru_net.so" "$B/libagiru_rt.so" > "$proof/inputs.sha256"
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" > "$proof/generation.log" 2>&1
rg -q '^absent    0 .NET type\(s\) with 0 member\(s\), 0 AL object\(s\) with 0$' "$proof/generation.log"
mapfile -t sources < <(rg --files --no-ignore "$proof/generated" -g '*.cpp' | LC_ALL=C sort)
[ "${#sources[@]}" -gt 0 ]
"$CXX" "${flags[@]}" -c test/runtime/boolean-expressions/Runner.cpp -o "$proof/runner.o"
"$CXX" "${flags[@]}" "$proof/runner.o" "${sources[@]}" "${links[@]}" -o "$proof/runner"
"$proof/runner" "$dsn" | tee "$proof/execution.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/runtime/boolean-expressions/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" -c test/runtime/boolean-expressions/Runner.cpp -o "$proof/runner.o" \
  > "$B/fixture-commands/boolean-expressions.json"
cp "$proof/source/Fixture.Codeunit.al" "$proof/original.al"
sed 's/Push(Trace, 1, false) and Push(Trace, 2, true)/Push(Trace, 2, true) and Push(Trace, 1, false)/g' \
  "$proof/original.al" > "$proof/source/Fixture.Codeunit.al"
if cmp -s "$proof/original.al" "$proof/source/Fixture.Codeunit.al"; then
  printf 'boolean-expressions: operand-order control did not match\n' >&2
  exit 1
fi
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/mutant-generated" > "$proof/mutant.log" 2>&1
mapfile -t mutant_sources < <(rg --files --no-ignore "$proof/mutant-generated" -g '*.cpp' | LC_ALL=C sort)
[ "${#mutant_sources[@]}" -gt 0 ]
"$CXX" "-I$proof/mutant-generated/fixture" "-I$proof/mutant-generated/absent" \
  "-I$proof/mutant-generated/shared" "${flags[@]}" "$proof/runner.o" \
  "${mutant_sources[@]}" "${links[@]}" -o "$proof/mutant"
if "$proof/mutant" "$dsn" > "$proof/mutant-execution.log" 2>&1; then
  printf 'boolean-expressions: wrong operand order escaped execution control\n' >&2
  exit 1
fi
rg -q 'generated AL conjunction evaluates both operands left to right' "$proof/mutant-execution.log"
rm -f "$proof/runner.o" "$proof/runner" "$proof/mutant"
printf 'boolean-expressions: eager values/effects/errors execute; source-order control fails; %s\n' "$proof"

#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-test-contexts.XXXXXX)
mkdir -p "$proof/source"
cp test/runtime/test-contexts/Fixture.Codeunit.al "$proof/source/Fixture.Codeunit.al"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$proof/apps.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate
  "-I$proof/generated/fixture" "-I$proof/generated/absent" "-I$proof/generated/shared")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" > "$proof/generation.log" 2>&1
rg -q '^absent    0 .NET type\(s\) with 0 member\(s\), 0 AL object\(s\) with 0$' "$proof/generation.log"
mapfile -t sources < <(rg --files --no-ignore "$proof/generated" -g '*.cpp' | LC_ALL=C sort)
[ "${#sources[@]}" -gt 0 ]
"$CXX" "${flags[@]}" -Isrc/rt -c test/runtime/test-contexts/Runner.cpp -o "$proof/runner.o"
"$CXX" "${flags[@]}" "$proof/runner.o" "${sources[@]}" "${links[@]}" -o "$proof/runner"
"$proof/runner"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/runtime/test-contexts/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" -Isrc/rt -c test/runtime/test-contexts/Runner.cpp -o "$proof/runner.o" \
  > "$B/fixture-commands/test-contexts.json"
cp "$proof/source/Fixture.Codeunit.al" "$proof/original.al"
sed 's/exit(Context.CodeunitId);/exit(0);/' "$proof/original.al" > "$proof/source/Fixture.Codeunit.al"
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" > "$proof/mutant.log" 2>&1
"$CXX" "${flags[@]}" "$proof/runner.o" "${sources[@]}" "${links[@]}" -o "$proof/mutant"
if "$proof/mutant" > "$proof/mutant-execution.log" 2>&1; then
  printf 'test-contexts: altered provider identity escaped the execution control\n' >&2
  exit 1
fi
rg -q 'AL property syntax returns the provider ID' "$proof/mutant-execution.log"
printf 'test-contexts: generated getters/copy/skip execute; source-expression control fails; %s\n' "$proof"

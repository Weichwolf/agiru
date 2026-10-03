#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
root=$PWD
B=${B:-build}
B=$(realpath "$B")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-required-isolation.XXXXXX)
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -Iinclude -Itest/gate -Isrc/al -Isrc/gen -Isrc/rt -Isrc/net -Isrc/db
  "-DAGIRU_SOURCE_DIR=\"$root\"" "-DAGIRU_TEST_DSN=\"$dsn\"")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_gen -lagiru_al -lagiru_rt -lagiru_net -lagiru_db)

"$B/gate_RequiredTestIsolationGate" "$proof/generated"
"$CXX" "${flags[@]}" -I"$proof/generated" test/runtime/required-isolation/Runner.cpp \
  -c -o "$proof/runner.o"
"$CXX" "${flags[@]}" -I"$proof/generated" "$proof/runner.o" \
  "$proof/generated/GeneratedIsolationUT.cpp" "${links[@]}" -o "$proof/generated-runner"
"$proof/generated-runner" "$dsn"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/runtime/required-isolation/Runner.cpp" \
  --args '[{directory: $directory, file: $file, arguments: $ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" "-I$proof/generated" test/runtime/required-isolation/Runner.cpp \
  -c -o "$proof/runner.o" > "$proof/compile_commands.json"
cp "$proof/compile_commands.json" "$B/fixture-commands/required-isolation.json"

source="$proof/generated/GeneratedIsolationUT.cpp"
if [ "$(rg -c '\.requiredTestIsolation = "Disabled"' "$source")" != 1 ]; then
  printf 'required-isolation: source requirement is not unique\n' >&2
  exit 2
fi
sed 's/\.requiredTestIsolation = "Disabled"/.requiredTestIsolation = "None"/' \
  "$source" > "$proof/generated/RequirementRemoved.cpp"
"$CXX" "${flags[@]}" -I"$proof/generated" test/runtime/required-isolation/Runner.cpp \
  "$proof/generated/RequirementRemoved.cpp" "${links[@]}" -o "$proof/requirement-removed"
if "$proof/requirement-removed" "$dsn" > "$proof/requirement-removed.log" 2>&1; then
  printf 'required-isolation: removed metadata escaped the generated control\n' >&2
  exit 1
fi
rg -q 'Generated RequiredTestIsolation: .* [1-9][0-9]* red' "$proof/requirement-removed.log"

if [ "$(rg -c 'const std::string refusal = IsolationError\(catalogue, isolationPolicy\);' src/rt/TestRunner.cpp)" != 1 ]; then
  printf 'required-isolation: runner preflight is not unique\n' >&2
  exit 2
fi
sed 's/const std::string refusal = IsolationError(catalogue, isolationPolicy);/const std::string refusal = (IsolationError(catalogue, isolationPolicy), std::string{});/' \
  src/rt/TestRunner.cpp > "$proof/PolicyBypassed.cpp"
"$CXX" "${flags[@]}" test/gate/RequiredTestIsolationGate.cpp \
  "$proof/PolicyBypassed.cpp" "${links[@]}" -o "$proof/policy-bypassed"
if "$proof/policy-bypassed" > "$proof/policy-bypassed.log" 2>&1; then
  printf 'required-isolation: ignored policy escaped the runtime control\n' >&2
  exit 1
fi
rg -q 'RequiredTestIsolation: .* [1-9][0-9]* red' "$proof/policy-bypassed.log"
printf 'required-isolation: generated AL executes; removed metadata and bypassed policy fail\n'

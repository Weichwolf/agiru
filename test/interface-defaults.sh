#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d "$B/interface-defaults.XXXXXX")
"$B/gate_GenInterfaceGate" "$proof/input"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' \
  > "$proof/input/scope.json"
"$B/agirutc" "$proof/input" "$proof/input/apps.json" "$proof/generated" \
  > "$proof/generation.log" 2>&1
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -Iinclude -Itest/gate)
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)

compile() {
  local input=$1 output=$2
  local sources
  rg --files --no-ignore "$input" -g '*.cpp' | LC_ALL=C sort > "$output.sources"
  mapfile -t sources < "$output.sources"
  [ "${#sources[@]}" -gt 0 ] || { printf 'interface-defaults: no generated sources\n' >&2; return 2; }
  "$CXX" "${flags[@]}" "-I$input/fixture" "-I$input/shared" "-I$input/absent" \
    -c test/interface-defaults/Runner.cpp -o "$output.runner.o"
  "$CXX" "${flags[@]}" "-I$input/fixture" "-I$input/shared" "-I$input/absent" \
    "$output.runner.o" "${sources[@]}" "${links[@]}" -o "$output"
}

compile "$proof/generated" "$proof/runner"
"$proof/runner"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/interface-defaults/Runner.cpp" \
  --args '[{directory: $directory, file: $file, arguments: $ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" "-I$proof/generated/fixture" "-I$proof/generated/shared" \
  "-I$proof/generated/absent" -c test/interface-defaults/Runner.cpp -o "$proof/runner.runner.o" \
  > "$proof/compile_commands.json"
cp "$proof/compile_commands.json" "$B/fixture-commands/interface-defaults.json"

if "$CXX" "${flags[@]}" "-I$proof/generated/fixture" "-I$proof/generated/shared" \
  -x c++ -fsyntax-only test/interface-defaults/DirectCall.cpp.in > "$proof/direct-call.log" 2>&1; then
  printf 'interface-defaults: an inherited default escaped through the codeunit API\n' >&2
  exit 1
fi
rg -q 'private member' "$proof/direct-call.log"

cp -a "$proof/input" "$proof/required-input"
awk '
  /^    procedure Empty\(var Value: Integer\)$/ {
    print "    procedure Empty(var Value: Integer);"; removed++; cutting=1; next
  }
  cutting { if (/^    end;$/) cutting=0; next }
  { print }
  END { if (removed != 1 || cutting) exit 1 }
' "$proof/input/Fixture/DefaultContract.Interface.al" \
  > "$proof/required-input/Fixture/DefaultContract.Interface.al"
"$B/agirutc" "$proof/required-input" "$proof/required-input/apps.json" "$proof/required" \
  > "$proof/required-generation.log" 2>&1
if compile "$proof/required" "$proof/required-runner" > "$proof/required.log" 2>&1; then
  printf 'interface-defaults: removing the body did not make Empty required\n' >&2
  exit 1
fi
rg -q 'abstract' "$proof/required.log"
rg -q "unimplemented pure virtual method 'Empty'" "$proof/required.log"

for call in 'Empty(Value)' 'this.Empty(Value)' 'eMpTy(Value)' 'Add(Value, 4)' 'this.Add(Value, 4)'; do
  label=${call//[^a-zA-Z]/-}
  input="$proof/self-$label"
  cp -a "$proof/input" "$input"
  awk -v call="$call" '
    /^}$/ {
      print "    procedure Forbidden()";
      print "    var Value: Integer;";
      print "    begin " call "; end;";
      inserted++
    }
    { print }
    END { if (inserted != 1) exit 1 }
  ' "$proof/input/Fixture/DefaultConsumer.Codeunit.al" \
    > "$input/Fixture/DefaultConsumer.Codeunit.al"
  if "$B/agirutc" "$input" "$input/apps.json" "$input/generated" \
    > "$proof/self-$label.log" 2>&1; then
    if [ "$call" != 'Add(Value, 4)' ]; then
      printf 'interface-defaults: %s escaped interface-only dispatch\n' "$call" >&2
      exit 1
    fi
    if "$CXX" "${flags[@]}" "-I$input/generated/fixture" "-I$input/generated/shared" \
      -fsyntax-only "$input/generated/fixture/core/codeunit/DefaultConsumer.cpp" \
      > "$proof/self-$label-compile.log" 2>&1; then
      printf 'interface-defaults: the bare overload escaped through runtime lookup\n' >&2
      exit 1
    fi
    rg -q "error: no member named 'Add' in namespace 'agiru'" "$proof/self-$label-compile.log"
  else
    rg -qi 'interface default (Empty|Add) is not a declared codeunit method' "$proof/self-$label.log"
  fi
done

cp -a "$proof/input" "$proof/broken-input"
printf '%s\n' 'codeunit 50003 Broken { procedure Run() begin exit(1); }' \
  > "$proof/broken-input/Fixture/Broken.Codeunit.al"
cp -a "$proof/generated" "$proof/broken"
printf '%s\n' 'preserve on failure' > "$proof/broken/fixture/Previous.h"
if "$B/agirutc" "$proof/broken-input" "$proof/broken-input/apps.json" "$proof/broken" \
  > "$proof/broken-generation.log" 2>&1; then
  printf 'interface-defaults: a parse failure returned success\n' >&2
  exit 1
fi
rg -q 'Broken.Codeunit.al' "$proof/broken-generation.log"
rg -q '^failures  1 in 1 cluster' "$proof/broken-generation.log"
rg -q '^preserve on failure$' "$proof/broken/fixture/Previous.h"

if [ -n "${AGIRU_INTERFACE_PREVIOUS:-}" ]; then
  LD_LIBRARY_PATH="${AGIRU_INTERFACE_PREVIOUS_LIBS:-$(dirname "$AGIRU_INTERFACE_PREVIOUS")}" \
    "$AGIRU_INTERFACE_PREVIOUS" "$proof/input" "$proof/input/apps.json" "$proof/previous" \
    > "$proof/previous-generation.log" 2>&1
  if compile "$proof/previous" "$proof/previous-runner" > "$proof/previous.log" 2>&1; then
    printf 'interface-defaults: the actual previous emitter escaped the control\n' >&2
    exit 1
  fi
  rg -q 'abstract' "$proof/previous.log"
  cp -a "$proof/previous" "$proof/previous-broken"
  printf '%s\n' 'preserve on failure' > "$proof/previous-broken/fixture/Previous.h"
  LD_LIBRARY_PATH="${AGIRU_INTERFACE_PREVIOUS_LIBS:-$(dirname "$AGIRU_INTERFACE_PREVIOUS")}" \
    "$AGIRU_INTERFACE_PREVIOUS" "$proof/broken-input" "$proof/broken-input/apps.json" \
    "$proof/previous-broken" > "$proof/previous-broken.log" 2>&1
  rg -q '^failures  1 in 1 cluster' "$proof/previous-broken.log"
  [ ! -e "$proof/previous-broken/fixture/Previous.h" ]
fi
printf 'interface-defaults: generated dispatch executes; body/API/self-call controls refuse; %s\n' "$proof"

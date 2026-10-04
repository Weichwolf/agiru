#!/usr/bin/env bash
set -euo pipefail
ulimit -c 0
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-page-navigation.XXXXXX)
mkdir -p "$proof/source"
cp test/runtime/page-navigation/*.al "$proof/source/"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$proof/apps.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate
  "-I$proof/generated/fixture" "-I$proof/generated/absent" "-I$proof/generated/shared")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" > "$proof/generation.log" 2>&1
mapfile -t sources < <(rg --files --no-ignore "$proof/generated" -g '*.cpp' | LC_ALL=C sort)
[ "${#sources[@]}" -gt 0 ]
mkdir -p "$proof/objects"
objects=()
for source in "${sources[@]}"; do
  object="$proof/objects/${source##*/}.o"
  "$CXX" "${flags[@]}" -c "$source" -o "$object"
  objects+=("$object")
done
"$CXX" "${flags[@]}" -c test/runtime/page-navigation/Runner.cpp -o "$proof/runner.o"
"$CXX" "${flags[@]}" "$proof/runner.o" "${objects[@]}" "${links[@]}" -o "$proof/runner"
"$proof/runner" "$dsn" > "$proof/execution.log" 2>&1
cat "$proof/execution.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/runtime/page-navigation/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" -c test/runtime/page-navigation/Runner.cpp -o "$proof/runner.o" \
  > "$B/fixture-commands/page-navigation.json"
mkdir -p "$proof/mutant"
cp -a include "$proof/mutant/"
for control in no-card wrong-row no-policy; do
  awk -v control="$control" '
    control == "no-card" && /if \(edit && EditCard_\(\)\)/ {
      sub(/edit && EditCard_\(\)/, "false"); changed++
    }
    control == "wrong-row" && /detail::RunPageByNumber\(false, list.cardPageId.Value\(\), std::as_const\(Record_\(\)\)\)/ {
      sub(/, std::as_const\(Record_\(\)\)/, ""); changed++
    }
    control == "no-policy" && /if \(detail::SaysFalse\(entry->page->modifyAllowed\)\)/ {
      sub(/detail::SaysFalse\(entry->page->modifyAllowed\)/, "false"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' include/runtime/test/TestPage.h > "$proof/mutant/include/runtime/test/TestPage.h"
  "$CXX" "-I$proof/mutant/include" "${flags[@]}" test/runtime/page-navigation/Runner.cpp \
    "${objects[@]}" "${links[@]}" -o "$proof/$control"
  if "$proof/$control" "$dsn" > "$proof/$control-execution.log" 2>&1; then
    printf 'page-navigation: %s escaped execution controls\n' "$control" >&2
    exit 1
  fi
  case "$control" in
    no-card) rg -q 'not on a running page' "$proof/$control-execution.log" ;;
    wrong-row) rg -q 'system Edit opens the declared card on the selected row' "$proof/$control-execution.log" ;;
    no-policy) rg -q 'system Edit honors the card.s ModifyAllowed policy' "$proof/$control-execution.log" ;;
  esac
  rm -- "$proof/$control"
done
awk '
  /Integer TestField::AsInteger\(\)/ { method = 1 }
  method && /if \(core_ == nullptr\) \{ Unbound\(\); \}/ { changed++; next }
  method && /^}/ { method = 0 }
  { print }
  END { if (changed != 1) exit 2 }
' src/rt/TestPage.cpp > "$proof/unbound-integer.cpp"
"$CXX" "${flags[@]}" -Isrc/rt -fPIC -shared "$proof/unbound-integer.cpp" \
  "${links[@]}" -o "$proof/unbound-integer.so"
if LD_PRELOAD="$proof/unbound-integer.so" "$B/gate_PageSourceGate" \
  > "$proof/unbound-integer-execution.log" 2>&1; then
  printf 'page-navigation: unbound integer escaped the gate\n' >&2
  exit 1
fi
rm -- "$proof/unbound-integer.so" "$proof/runner" "$proof/runner.o"
rm -r -- "$proof/mutant"
rm -r -- "$proof/objects"
printf 'page-navigation: generated list/card selection, opening triggers, explicit action, card policy and empty selection execute; four compiled controls reject; %s\n' "$proof"

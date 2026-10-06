#!/usr/bin/env bash
set -euo pipefail
ulimit -c 0
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-page-navigation.XXXXXX)
sha256sum src/rt/PageDispatcher.cpp include/runtime/PageDispatcher.h include/runtime/PageCore.h \
  src/rt/PageCore.cpp src/rt/PageValue.cpp include/runtime/PageValue.h \
  src/rt/PageHtml.cpp src/rt/HtmlText.{h,cpp} include/runtime/PageHtml.h \
  src/rt/PageInstance.cpp include/runtime/PageInstance.h include/runtime/Catalogue.h \
  include/runtime/Page.h src/gen/BodyWriter.cpp \
  include/runtime/PageSession.h include/runtime/test/TestPage.h \
  include/runtime/Session.h include/runtime/SessionCommand.h src/rt/Session.cpp \
  src/rt/SessionCommand.cpp src/rt/Cursor.cpp src/rt/Transaction.cpp \
  include/runtime/TablePermissions.h src/rt/TablePermissions.cpp src/rt/{Table,Navigate,Query,RecordRef}.cpp \
  include/runtime/PageCommandHost.h src/rt/PageCommandHost.cpp test/ui/page-host/Runner.cpp \
  test/gate/PrivateAuthFile.h \
  test/gate/PageDispatcherGate.cpp test/runtime/page-navigation/Runner.cpp \
  test/runtime/page-navigation/*.al test/runtime/page-navigation.sh > "$proof/dispatcher-inputs.sha256"
mkdir -p "$proof/source"
cp test/runtime/page-navigation/*.al "$proof/source/"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$proof/apps.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate
  "-DAGIRU_TEST_DSN=\"$dsn\""
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
"$B/gate_PageDispatcherGate" > "$proof/dispatcher.log" 2>&1
cat "$proof/dispatcher.log"
host_target=${AGIRU_PAGE_HOST_BUILD:-$proof}
if [[ -n ${AGIRU_PAGE_HOST_BUILD:-} ]]; then
  [[ "$AGIRU_PAGE_HOST_BUILD" =~ ^/tmp/agiru-native-page-host\.[A-Za-z0-9]+$ ]]
  [[ -d "$AGIRU_PAGE_HOST_BUILD" ]]
fi
"$CXX" "${flags[@]}" -c test/ui/page-host/Runner.cpp -o "$host_target/host.o"
"$CXX" "$host_target/host.o" "${objects[@]}" "${links[@]}" -o "$host_target/host"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/ui/page-host/Runner.cpp" \
    --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
    "$CXX" "${flags[@]}" -c test/ui/page-host/Runner.cpp -o "$host_target/host.o" \
    > "$B/fixture-commands/page-host.json"
unlink "$host_target/host.o"
for control in no-authorization no-enabled no-editable no-visible unknown-control; do
  awk -v control="$control" '
    control == "no-authorization" && /authorization_\.Require\(declaration_\.id, command\)/ {
      sub(/authorization_\.Require\(declaration_\.id, command\)/, "static_cast<void>(authorization_)"); changed++
    }
    control == "no-enabled" && /!page_\.ControlEnabled\(control->name\)/ {
      sub(/!page_\.ControlEnabled\(control->name\)/, "false"); changed++
    }
    control == "no-editable" && /!page_\.ControlEditable\(control->name\)/ {
      sub(/!page_\.ControlEditable\(control->name\)/, "false"); changed++
    }
    control == "no-visible" && /!page_\.ControlVisible\(control->name\)/ {
      sub(/!page_\.ControlVisible\(control->name\)/, "false"); changed++
    }
    control == "unknown-control" && /if \(control == nullptr\).*unknown declared control/ {
      $0 = "  if (control == nullptr) { control = &declaration_.layout.front(); }"; changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/rt/PageDispatcher.cpp > "$proof/dispatcher-$control.cpp"
  "$CXX" "${flags[@]}" test/gate/PageDispatcherGate.cpp "$proof/dispatcher-$control.cpp" \
    "${links[@]}" -o "$proof/dispatcher-$control"
  status=0
  "$proof/dispatcher-$control" > "$proof/dispatcher-$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  rg -q 'invalid commands refuse with the expected diagnostic' "$proof/dispatcher-$control.log"
  rm -- "$proof/dispatcher-$control" "$proof/dispatcher-$control.cpp"
done
awk '
  /&instance->Declaration\(\) != entry->page/ {
    sub(/&instance->Declaration\(\) != entry->page/, "false"); changed++
  }
  { print }
  END { if (changed != 1) exit 2 }
' src/rt/PageInstance.cpp > "$proof/wrong-factory.cpp"
"$CXX" "${flags[@]}" test/gate/PageDispatcherGate.cpp "$proof/wrong-factory.cpp" \
  "${links[@]}" -o "$proof/wrong-factory"
status=0
"$proof/wrong-factory" > "$proof/wrong-factory.log" 2>&1 || status=$?
[[ "$status" = 1 ]]
rg -q 'absent or invalid factories refuse instead of headless success' "$proof/wrong-factory.log"
rm -- "$proof/wrong-factory" "$proof/wrong-factory.cpp"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/runtime/page-navigation/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" -c test/runtime/page-navigation/Runner.cpp -o "$proof/runner.o" \
  > "$B/fixture-commands/page-navigation.json"
mkdir -p "$proof/mutant"
cp -a include "$proof/mutant/"
awk '
  /PageSession\(\) = default;/ {
    print "  void Open() {}"; changed++
  }
  { print }
  END { if (changed != 1) exit 2 }
' include/runtime/PageSession.h > "$proof/mutant/include/runtime/PageSession.h"
if "$CXX" "-I$proof/mutant/include" "${flags[@]}" -c test/runtime/page-navigation/Runner.cpp \
  -o "$proof/shadowed-controls.o" > "$proof/shadowed-controls.log" 2>&1; then
  printf 'page-navigation: production API shadowed AL controls without refusal\n' >&2
  exit 1
fi
rg -q "member 'Open' found in multiple base classes" "$proof/shadowed-controls.log"
for control in no-card wrong-row no-policy collect-production-errors; do
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
    control == "collect-production-errors" && /static constexpr bool kCollectSaveErrors = false/ {
      sub(/kCollectSaveErrors = false/, "kCollectSaveErrors = true"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' include/runtime/PageSession.h > "$proof/mutant/include/runtime/PageSession.h"
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
    collect-production-errors) rg -q 'production row-save errors propagate instead of successful collection' "$proof/$control-execution.log" ;;
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
if [[ -z ${AGIRU_PAGE_HOST_BUILD:-} ]]; then unlink "$host_target/host"; fi
rm -r -- "$proof/mutant"
rm -r -- "$proof/objects"
sha256sum --check "$proof/dispatcher-inputs.sha256" > "$proof/dispatcher-integrity.log"
printf 'page-navigation: generated navigation, production factories/lifecycle and authorized control dispatch execute; eleven execution controls and one control-name compile refusal reject; %s\n' "$proof"

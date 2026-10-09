#!/usr/bin/env bash
set -euo pipefail
ulimit -c 0
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
dsn=${AGIRU_TEST_DSN:-postgresql://agiru:agiru@localhost:5433/agiru_gate}
proof=$(mktemp -d /tmp/agiru-page-navigation.XXXXXX)
cleanup() {
  local target
  for target in objects mutant; do
    if [[ -d "$proof/$target" ]]; then rm -r -- "$proof/$target"; fi
  done
  for target in runner runner.o; do
    if [[ -f "$proof/$target" ]]; then unlink "$proof/$target"; fi
  done
}
trap cleanup EXIT
sha256sum src/rt/PageDispatcher.cpp include/runtime/PageDispatcher.h include/runtime/PageCore.h \
  include/BuiltinsWritten.h test/gate/OptionGate.cpp \
  src/rt/PageCore.cpp src/rt/PageValue.cpp include/runtime/{PageValue,PageVariableValue}.h \
  src/rt/PageHtml.cpp src/rt/HtmlText.{h,cpp} include/runtime/PageHtml.h \
  src/rt/PageListHtml.h \
  src/rt/PageInstance.cpp include/runtime/PageInstance.h include/runtime/Catalogue.h \
  include/runtime/PageWindow.h include/runtime/RecordWindow.h src/rt/RecordWindow.cpp \
  include/runtime/Table.h include/runtime/RecordState.h include/runtime/TablePermissions.h \
  include/runtime/Page.h src/gen/{BodyWriter,PageWriter,RuntimeSurface}.cpp \
  include/runtime/PageSession.h include/runtime/test/TestPage.h \
  include/runtime/Session.h include/runtime/SessionCommand.h src/rt/Session.cpp \
  src/rt/SessionCommand.cpp src/rt/Cursor.cpp src/rt/Transaction.cpp \
  include/runtime/TablePermissions.h src/rt/TablePermissions.cpp src/rt/{Table,Navigate,Query,RecordRef}.cpp \
  include/runtime/PageCommandHost.h src/rt/PageCommandHost.cpp test/ui/page-host/Runner.cpp \
  include/runtime/ClientCredentials.h src/rt/ClientCredentials.cpp src/rt/CredentialFormat.h \
  src/rt/BrowserHttp.{h,cpp} include/runtime/{BrowserSession,BrowserSessionOptions}.h src/rt/BrowserSession.cpp \
  src/rt/PageInteraction.{h,cpp} include/runtime/UiHost.h src/rt/UiHost.cpp \
  src/rt/PageModal.{h,cpp} \
  src/rt/{CommandAuthority,PageCallAuthority,SessionUser}.{h,cpp} src/rt/SessionState.h \
  include/runtime/PermissionSetRegistry.h src/rt/PermissionSetRegistry.cpp \
  include/runtime/NativeService.h src/rt/{NativeService,NativeServiceConfig}.cpp deploy/dev/agiru.json \
  src/cli/{Main,Services}.cpp src/cli/Services.h \
  test/gate/PrivateAuthFile.h test/gate/NativePermissionFixture.h \
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
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/runtime/page-navigation/Runner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" -c test/runtime/page-navigation/Runner.cpp -o "$proof/runner.o" \
  > "$B/fixture-commands/page-navigation.json"
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
"$CXX" "${flags[@]}" "-DAGIRU_DATABASE=\"$dsn\"" src/cli/Main.cpp src/cli/Services.cpp \
  "${objects[@]}" "${links[@]}" -o "$host_target/agiru"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/ui/page-host/Runner.cpp" \
    --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
    "$CXX" "${flags[@]}" -c test/ui/page-host/Runner.cpp -o "$host_target/host.o" \
    > "$B/fixture-commands/page-host.json"
unlink "$host_target/host.o"
mkdir -p "$proof/mutant"
cp -a include "$proof/mutant/"
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
for control in object-id-number object-id-prefix object-id-caption object-id-fallback object-id-metadata; do
  awk -v control="$control" '
    control == "object-id-number" && /if \(!UseNames\) \{ return "Page " \+ std::to_string\(declaration.id.Value\(\)\); \}/ {
      sub(/declaration.id.Value\(\)/, "50131"); changed++
    }
    control == "object-id-prefix" && /if \(!UseNames\) \{ return "Page " \+ std::to_string\(declaration.id.Value\(\)\); \}/ {
      sub(/"Page " \+ /, ""); changed++
    }
    control == "object-id-caption" && /auto caption = Caption\(\);/ {
      sub(/Caption\(\)/, "std::string(declaration.name)"); changed++
    }
    control == "object-id-fallback" && /if \(caption.empty\(\)\) \{ caption = declaration.name; \}/ {
      sub(/caption = declaration.name/, "caption.clear()"); changed++
    }
    control == "object-id-metadata" && /throw Error\("Page.ObjectId requires declared page metadata", "PageIdentityUnavailable"\);/ {
      $0 = "      return \"Page 0\";"; changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' include/runtime/Page.h > "$proof/mutant/include/runtime/Page.h"
  "$CXX" "-I$proof/mutant/include" "${flags[@]}" test/gate/PageDispatcherGate.cpp \
    "${links[@]}" -o "$proof/$control"
  status=0
  "$proof/$control" > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    object-id-number) claim='another page retains its own numeric identity' ;;
    object-id-prefix) claim='ObjectId defaults to the declared numeric Page identity' ;;
    object-id-caption) claim='ObjectId true uses the declared caption, not its different AL name' ;;
    object-id-fallback) claim='ObjectId true falls back to the AL name when no caption exists' ;;
    object-id-metadata) claim='missing Page metadata refuses instead of fabricating Page zero' ;;
  esac
  rg -q "FAIL .*${claim}" "$proof/$control.log"
  unlink "$proof/$control"
done
cp include/runtime/Page.h "$proof/mutant/include/runtime/Page.h"
for control in window-no-row-trigger window-row-is-current window-no-current-trigger window-lose-current window-no-original-image; do
  awk -v control="$control" '
    control == "window-no-row-trigger" && /detail::AfterReadPageRecord\(page\);/ {
      sub(/detail::AfterReadPageRecord\(page\);/, "static_cast<void>(page);"); changed++
    }
    control == "window-row-is-current" && /detail::AfterReadPageRecord\(page\);/ {
      sub(/detail::AfterReadPageRecord\(page\);/, "detail::AfterGetRecord(page);"); changed++
    }
    control == "window-no-current-trigger" && /if \(opening \|\| selected != previous\)/ {
      sub(/opening \|\| selected != previous/, "(static_cast<void>(opening), false)"); changed++
    }
    control == "window-lose-current" && /if \(i == 0 \|\| identity == previous\)/ {
      sub(/i == 0 \|\| identity == previous/, "true"); changed++
    }
    control == "window-no-original-image" && /static_cast<void>\(static_cast<typename Source::Platform_Half &>\(rec\)\.Read\(true\)\);/ {
      sub(/static_cast<void>\(static_cast<typename Source::Platform_Half &>\(rec\)\.Read\(true\)\);/,
          "static_cast<void>(rec);"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' include/runtime/PageSession.h > "$proof/mutant/include/runtime/PageSession.h"
  mutant_objects=()
  for source in "${sources[@]}"; do
    object="$proof/objects/${source##*/}.o"
    if [[ "$source" == */NavigationWindow.def.cpp || "$source" == */NavigationList.def.cpp ]]; then
      object="$proof/objects/window-mutant-${source##*/}.o"
      "$CXX" "-I$proof/mutant/include" "${flags[@]}" -c "$source" -o "$object"
    fi
    mutant_objects+=("$object")
  done
  "$CXX" "-I$proof/mutant/include" "${flags[@]}" test/runtime/page-navigation/Runner.cpp \
    "${mutant_objects[@]}" "${links[@]}" -o "$proof/$control"
  status=0
  "$proof/$control" "$dsn" > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    window-no-row-trigger) claim='opening runs each loaded trigger before the selected trigger' ;;
    window-row-is-current) claim='row triggers do not make every loaded row current' ;;
    window-no-current-trigger) claim='opening runs each loaded trigger before the selected trigger' ;;
    window-lose-current) claim='display enumeration retains the first selected row, not its last' ;;
    window-no-original-image) claim='loaded rows capture their original stored image before AL changes' ;;
  esac
  rg -q "$claim" "$proof/$control.log"
  unlink "$proof/$control"
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
for control in custom-window-bound custom-window-boundary custom-window-reverse custom-window-more custom-window-stall custom-window-selection custom-window-restore; do
  cp include/runtime/Page.h "$proof/mutant/include/runtime/Page.h"
  cp include/runtime/RecordState.h "$proof/mutant/include/runtime/RecordState.h"
  awk -v control="$control" '
    control == "custom-window-bound" && /if \(rows.size\(\) == limit\)/ {
      sub(/rows.size\(\) == limit/, "false"); changed++
    }
    control == "custom-window-boundary" && /const auto &anchor = step > 0/ {
      $0 = "    const auto &anchor = customRows_.front();"; changed++
    }
    control == "custom-window-reverse" && /if \(backwards\) \{ std::ranges::reverse\(rows\); \}/ {
      sub(/std::ranges::reverse\(rows\)/, "static_cast<void>(rows)"); changed++
    }
    control == "custom-window-more" && /Present_\(rows.size\(\), visited, found,/ {
      sub(/visited, found,/, "visited, false,"); changed++
    }
    control == "custom-window-stall" && /std::ranges::find\(rows, identity, &CustomRow::identity\) != rows.end\(\)/ {
      sub(/std::ranges::find\(rows, identity, &CustomRow::identity\) != rows.end\(\)/, "false"); changed++
    }
    control == "custom-window-selection" && /if \(!Base::Landed_\(\[&\]\(auto &\) \{ return Position_Custom_\(\*found\); \}\)\)/ {
      $0 = "      if (!Base::Select_Record(identity)) {"; changed++
    }
    control == "custom-window-restore" && /rec = previousValues;/ {
      sub(/rec = previousValues/, "static_cast<void>(previousValues)"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' include/runtime/PageSession.h > "$proof/mutant/include/runtime/PageSession.h"
  mutant_objects=()
  for source in "${sources[@]}"; do
    object="$proof/objects/${source##*/}.o"
    if [[ "$source" == */NavigationProviderWindow.def.cpp || "$source" == */NavigationFindOnly.def.cpp ]]; then
      object="$proof/objects/provider-mutant-${source##*/}.o"
      "$CXX" "-I$proof/mutant/include" "${flags[@]}" -c "$source" -o "$object"
    fi
    mutant_objects+=("$object")
  done
  "$CXX" "-I$proof/mutant/include" "${flags[@]}" test/runtime/page-navigation/Runner.cpp \
    "${mutant_objects[@]}" "${links[@]}" -o "$proof/$control"
  status=0
  "$proof/$control" "$dsn" > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    custom-window-bound|custom-window-more) claim='custom SQL windows retain the bound and one undisplayed source probe' ;;
    custom-window-boundary) claim='temporary continuation uses the declared provider and excludes the boundary' ;;
    custom-window-reverse) claim='custom last reverses visits into provider forward order, not client key sorting' ;;
    custom-window-stall) claim='stalled custom navigation refuses even when the duplicate is only the probe' ;;
    custom-window-selection) claim='temporary retained row absent from SQL can be selected through its provider' ;;
    custom-window-restore) claim='custom exhaustion preserves the selected post-trigger buffer' ;;
  esac
  rg -q "FAIL .*${claim}" "$proof/$control.log"
  unlink "$proof/$control"
done
cp include/runtime/PageSession.h "$proof/mutant/include/runtime/PageSession.h"
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
for control in no-card wrong-row no-policy collect-production-errors source-insert source-identity source-key source-existence; do
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
    control == "source-insert" && /void ReconcileNewRecord_\(\) \{/ {
      sub(/\{/, "{ return;"); changed++
    }
    control == "source-identity" && /platform.SetRange\(probe.SystemId, probe.SystemId\);/ {
      sub(/platform.SetRange\(probe.SystemId, probe.SystemId\);/, "static_cast<void>(probe);"); changed++
    }
    control == "source-key" && /platform.SetRecFilter\(\);/ {
      sub(/platform.SetRecFilter\(\);/, "static_cast<void>(platform);"); changed++
    }
    control == "source-existence" && /if \(platform.IsEmpty\(\)\) \{ return; \}/ {
      sub(/if \(platform.IsEmpty\(\)\) \{ return; \}/, "static_cast<void>(platform);"); changed++
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
    source-insert) rg -q 'source-inserted new pages persist an ordinary field before leaving' "$proof/$control-execution.log" ;;
    source-identity|source-key|source-existence) rg -q 'pending pages require both stored primary key and SystemId' "$proof/$control-execution.log" ;;
  esac
  rm -- "$proof/$control"
done
modal_source="$proof/generated/fixture/fixture/page/NavigationModal.def.cpp"
[[ -f "$modal_source" ]]
modal_objects=()
for object in "${objects[@]}"; do
  if [[ "$object" != "$proof/objects/NavigationModal.def.cpp.o" ]]; then
    modal_objects+=("$object")
  fi
done
for control in modal-copy modal-veto modal-action modal-callback-policy modal-values variable-conversion; do
  cp include/runtime/Page.h "$proof/mutant/include/runtime/Page.h"
  cp include/runtime/PageSession.h "$proof/mutant/include/runtime/PageSession.h"
  case "$control" in
    modal-copy|modal-action|modal-values) header=PageSession.h ;;
    modal-veto|modal-callback-policy) header=Page.h ;;
    variable-conversion) header=fixture/page/NavigationModal.h ;;
  esac
  source="include/runtime/$header"
  destination="$proof/mutant/include/runtime/$header"
  if [[ "$control" = variable-conversion ]]; then
    source="$proof/generated/fixture/$header"
    destination="$proof/mutant/include/$header"
    mkdir -p "$(dirname "$destination")"
  fi
  awk -v control="$control" '
    control == "modal-copy" && /owned_ = prepared_ == nullptr;/ {
      sub(/owned_ = prepared_ == nullptr;/, "owned_ = true;"); changed++
    }
    control == "modal-veto" && /void RetryClose\(\) \{ closing_ = false; \}/ {
      sub(/closing_ = false/, "closing_ = true"); changed++
    }
    control == "modal-action" && /static_cast<Page<P> &>\(page\).CloseWith\(action\);/ {
      sub(/CloseWith\(action\)/, "CloseWith(Action::OK)"); changed++
    }
    control == "modal-callback-policy" && /if \(modal\) \{ RequireUiCallback\(\); \}/ {
      sub(/RequireUiCallback\(\);/, "static_cast<void>(modal);"); changed++
    }
    control == "modal-values" && /row != nullptr && row->value != nullptr/ {
      sub(/row->value != nullptr/, "false"); changed++
    }
    control == "variable-conversion" && /if \(!::agiru::Evaluate\(page.OwnerMarker_Var, text\)\)/ {
      sub(/!::agiru::Evaluate\(page.OwnerMarker_Var, text\)/, "(::agiru::Evaluate(page.OwnerMarker_Var, text), false)"); changed++
    }
    { print }
    END { if (changed != (control == "modal-callback-policy" ? 2 : 1)) exit 2 }
  ' "$source" > "$destination"
  modal_compile_source="$modal_source"
  if [[ "$control" = variable-conversion ]]; then
    modal_compile_source="$proof/mutant/include/fixture/page/NavigationModal.def.cpp"
    cp "$modal_source" "$modal_compile_source"
  fi
  "$CXX" "-I$proof/mutant/include" "${flags[@]}" -c "$modal_compile_source" \
    -o "$proof/mutant-modal.o"
  "$CXX" "-I$proof/mutant/include" "${flags[@]}" test/runtime/page-navigation/Runner.cpp \
    "$proof/mutant-modal.o" "${modal_objects[@]}" "${links[@]}" -o "$proof/$control"
  status=0
  "$proof/$control" "$dsn" > "$proof/$control-execution.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    modal-copy) rg -q 'modal opening preserves caller variables' "$proof/$control-execution.log" ;;
    modal-veto) rg -q 'an error on a later query-close leaves the modal open' "$proof/$control-execution.log" ;;
    modal-action) rg -q 'a card cancellation is not consent' "$proof/$control-execution.log" ;;
    modal-callback-policy) rg -q 'disabled modal callbacks refuse before native opening' "$proof/$control-execution.log" ;;
    modal-values) rg -q 'Control has no exact typed value binding' "$proof/$control-execution.log" ;;
    variable-conversion) rg -q 'invalid modal variables refuse before their AL validation trigger' "$proof/$control-execution.log" ;;
  esac
  rm -- "$proof/$control" "$proof/mutant-modal.o"
done

for control in integer-range integer-overflow integer-prefix; do
  awk -v control="$control" '
    control == "integer-range" && /if \(read < std::numeric_limits<T>::lowest\(\)/ {
      sub(/read < std::numeric_limits<T>::lowest\(\) \|\| read > std::numeric_limits<T>::max\(\)/, "false"); changed++
    }
    control == "integer-range" && /else if \(read < 0 \|\| static_cast<unsigned long long>\(read\)/ {
      sub(/read < 0 \|\| static_cast<unsigned long long>\(read\) > std::numeric_limits<T>::max\(\)/, "false"); changed++
    }
    control == "integer-overflow" && /errno == ERANGE \|\| end != held.c_str\(\)/ {
      sub(/errno == ERANGE \|\| /, ""); changed++
    }
    control == "integer-prefix" && /errno == ERANGE \|\| end != held.c_str\(\)/ {
      sub(/end != held.c_str\(\) \+ held.size\(\)/, "*end != char{}"); changed++
    }
    { print }
    END { if (changed != (control == "integer-range" ? 2 : 1)) exit 2 }
  ' include/BuiltinsWritten.h > "$proof/mutant/include/BuiltinsWritten.h"
  "$CXX" "-I$proof/mutant/include" "${flags[@]}" test/gate/OptionGate.cpp \
    "${links[@]}" -o "$proof/$control" > "$proof/$control-compile.log" 2>&1
  status=0
  "$proof/$control" > "$proof/$control-execution.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    integer-range) claim='Integer evaluation refuses narrowing without changing the destination' ;;
    integer-overflow) claim='BigInteger evaluation refuses overflow without clamping' ;;
    integer-prefix) claim='Integer evaluation consumes the complete input including embedded NUL' ;;
  esac
  rg -q "FAIL .*${claim}" "$proof/$control-execution.log"
done
for control in custom-no-find custom-no-next custom-no-image custom-no-position custom-no-copy-anchor custom-no-close; do
  for header in Page.h PageSession.h RecordState.h; do
    cp "include/runtime/$header" "$proof/mutant/include/runtime/$header"
  done
  case "$control" in
    custom-no-copy-anchor) header=RecordState.h ;;
    custom-no-close) header=PageSession.h ;;
    *) header=Page.h ;;
  esac
  awk -v control="$control" '
    control == "custom-no-find" && /if constexpr \(requires \{ page.OnFindRecord/ {
      $0 = "  if constexpr (false) {"; changed++
    }
    control == "custom-no-next" && /if constexpr \(requires \{ page.OnNextRecord/ {
      $0 = "  if constexpr (false) {"; changed++
    }
    control == "custom-no-image" && /return static_cast<typename Source::Platform_Half &>\(page.Rec\).Read\(found\);/ {
      $0 = "    static_cast<void>(sizeof(Source)); return found;"; changed++
    }
    control == "custom-no-position" && /Ensure\(\).positioned = found;/ {
      sub(/positioned = found/, "positioned = false"); changed++
    }
    control == "custom-no-copy-anchor" && /mine.temporary = std::move\(keep\);/ {
      print "      mine.positioned = false;"; changed++
    }
    /template <typename Step> Boolean Landed_/ { navigation = 1 }
    navigation && /^  }/ { navigation = 0 }
    control == "custom-no-close" && navigation && /Release_\(\);/ {
      sub(/Release_\(\);/, "static_cast<void>(page_);"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' "include/runtime/$header" > "$proof/mutant/include/runtime/$header"
  custom_objects=()
  for source in "${sources[@]}"; do
    object="$proof/objects/${source##*/}.o"
    if [[ "$source" == */NavigationCustom.cpp || "$source" == */NavigationCustom.def.cpp ]]; then
      object="$proof/objects/custom-mutant-${source##*/}.o"
      "$CXX" "-I$proof/mutant/include" "${flags[@]}" -c "$source" -o "$object"
    fi
    custom_objects+=("$object")
  done
  "$CXX" "-I$proof/mutant/include" "${flags[@]}" test/runtime/page-navigation/Runner.cpp \
    "${custom_objects[@]}" "${links[@]}" -o "$proof/$control"
  status=0
  "$proof/$control" "$dsn" > "$proof/$control-execution.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    custom-no-find) claim='custom opening invokes find instead of the default SQL provider' ;;
    custom-no-next|custom-no-position) claim='custom next uses its declared result without SQL fallback' ;;
    custom-no-image) claim='custom returned row captures its own xRec' ;;
    custom-no-copy-anchor) claim='Next after cross-provider Copy seeks from the copied buffer' ;;
    custom-no-close) claim='a custom next error closes the page and retains its diagnostic' ;;
  esac
  rg -q "FAIL .*${claim}" "$proof/$control-execution.log"
  unlink "$proof/$control"
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
if [[ -z ${AGIRU_PAGE_HOST_BUILD:-} ]]; then
  unlink "$host_target/host"
  unlink "$host_target/agiru"
fi
rm -r -- "$proof/mutant"
rm -r -- "$proof/objects"
sha256sum --check "$proof/dispatcher-inputs.sha256" > "$proof/dispatcher-integrity.log"
printf 'page-navigation: generated navigation, bounded custom SQL/temporary providers, production factories/lifecycle and authorized control dispatch execute; forty-seven execution controls and one control-name compile refusal reject; %s\n' "$proof"

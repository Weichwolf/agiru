#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
proof=$(mktemp -d /tmp/agiru-page-host.XXXXXX)
gate_database=${AGIRU_PAGE_GATE_DATABASE:-agiru_gate}
[[ "$gate_database" =~ ^[a-z_][a-z0-9_]{0,62}$ ]]
git rev-parse HEAD > "$proof/head.txt"
sha256sum include/runtime/{Page,PageValue,PageVariableValue}.h src/rt/PageValue.cpp \
  src/gen/{PageWriter,RuntimeSurface}.cpp > "$proof/scalar-inputs.sha256"
sha256sum Makefile include/runtime/{PageCommandHost,PageHtml,PageInstance,PageSession,SessionCommand}.h \
  src/rt/{PageCommandHost,PageHtml,PageInstance,SessionCommand,HtmlText}.cpp src/rt/HtmlText.h \
  src/rt/PageListHtml.h include/runtime/{PageWindow,RecordWindow}.h src/rt/RecordWindow.cpp \
  src/rt/PageInteraction.{h,cpp} include/runtime/UiHost.h src/rt/UiHost.cpp \
  src/rt/PageModal.{h,cpp} \
  src/rt/{CommandAuthority,PageCallAuthority,SessionUser}.{h,cpp} src/rt/SessionState.h src/rt/Transaction.cpp \
  include/runtime/ClientCredentials.h src/rt/ClientCredentials.cpp src/rt/CredentialFormat.h \
  src/rt/BrowserHttp.{h,cpp} include/runtime/{BrowserSession,BrowserSessionOptions}.h src/rt/BrowserSession.cpp \
  include/runtime/TablePermissions.h src/rt/{TablePermissions,Session,Table,Navigate,Query,RecordRef}.cpp \
  include/runtime/NativePermissions.h src/rt/{NativePermissions,NativePermissionSnapshot}.cpp \
  include/runtime/PermissionSetRegistry.h src/rt/PermissionSetRegistry.cpp \
  include/runtime/NativeService.h src/rt/{NativeService,NativeServiceConfig}.cpp deploy/dev/agiru.json \
  include/runtime/{HttpServerOptions,PageHostOptions,SessionOptions}.h \
  src/net/JsonEngine.{h,cpp} test/gate/NativeServiceConfigGate.cpp \
  src/cli/{Main,Services}.cpp src/cli/Services.h test/gate/NativePermissionFixture.h \
  test/ui/page-host.{sh,mjs} test/ui/{server-config,browser-client,dialog-fixture}.mjs test/ui/page-host/Runner.cpp test/runtime/page-navigation.sh \
  test/runtime/page-navigation/*.al test/gate/PrivateAuthFile.h \
  src/client/*.{mts,json} > "$proof/inputs.sha256"
make dev-exec COMMAND='findmnt -T /tmp'
make dev-exec COMMAND='df -h /tmp'
native=$(make --no-print-directory dev-exec COMMAND='mktemp -d /tmp/agiru-native-page-host.XXXXXX')
[[ "$native" =~ ^/tmp/agiru-native-page-host\.[A-Za-z0-9]+$ ]]
clean_auth() {
  local suffix
  for suffix in "" .second .peer; do
    if [[ -f "$proof/auth.json$suffix" ]]; then unlink "$proof/auth.json$suffix"; fi
  done
  make --no-print-directory dev-exec COMMAND="rm -f -- $native/auth.json $native/auth.json.second $native/auth.json.peer"
}
cleanup() {
  make --no-print-directory dev-exec COMMAND="rm -f -- $native/host $native/agiru $native/auth.json $native/auth.json.second $native/owner.cpp $native/owner.so $native/revision.cpp $native/revision.so $native/replay.cpp $native/replay.so $native/policy.cpp $native/policy.so $native/duplicates.cpp $native/duplicates.so" || :
  clean_auth || :
  for control in owner client-owner client-call-owner revision replay policy duplicates list-limit failure-receipt blocking-al creation-mode creation-policy; do
    if [[ -f "$proof/$control.cpp" ]]; then unlink "$proof/$control.cpp"; fi
    make --no-print-directory dev-exec COMMAND="rm -f -- $native/$control.cpp $native/$control.so" || :
  done
  for control in dialog-default dialog-commit dialog-replay modal-poll-receipt modal-commit modal-replay modal-cancel modal-message-replay modal-input-validation modal-idle; do
    make --no-print-directory dev-exec COMMAND="rm -f -- $native/$control.cpp $native/$control.so" || :
    if [[ -f "$proof/$control.cpp" ]]; then unlink "$proof/$control.cpp"; fi
  done
  make --no-print-directory dev-exec COMMAND="rmdir -- $native" || :
}
trap cleanup EXIT
make dev-exec COMMAND='make gate GATE=NativeServiceConfigGate JOBS=2 B=/workspace/build/podman' \
  > "$proof/config-gate.log" 2>&1 || { cat "$proof/config-gate.log"; exit 1; }
cat "$proof/config-gate.log"
make dev-exec COMMAND="env AGIRU_TEST_DSN=postgresql://agiru:agiru@127.0.0.1:5432/$gate_database AGIRU_PAGE_HOST_BUILD=$native make page-navigation JOBS=2 B=/workspace/build/podman" \
  > "$proof/native.log" 2>&1 || { cat "$proof/native.log"; exit 1; }
cat "$proof/native.log"
AGIRU_PAGE_HOST_NATIVE="$native" AGIRU_PAGE_HOST_PROOF="$proof" node --test test/ui/page-host.mjs \
  > "$proof/execution.log" 2>&1 || { cat "$proof/execution.log"; exit 1; }
cat "$proof/execution.log"
clean_auth
AGIRU_PAGE_HOST_NATIVE="$native" AGIRU_PAGE_HOST_PROOF="$proof" AGIRU_PAGE_HOST_APPLICATION=1 \
  node --test test/ui/page-host.mjs > "$proof/application.log" 2>&1 || { cat "$proof/application.log"; exit 1; }
cat "$proof/application.log"
clean_auth
AGIRU_PAGE_HOST_NATIVE="$native" AGIRU_PAGE_HOST_PROOF="$proof" AGIRU_PAGE_HOST_APPLICATION=1 \
  AGIRU_TRY_WRITE_DISABLED=1 AGIRU_LIST_ROWS=7 node --test test/ui/page-host.mjs \
  > "$proof/try-disabled.log" 2>&1 || { cat "$proof/try-disabled.log"; exit 1; }
cat "$proof/try-disabled.log"
clean_auth
AGIRU_PAGE_HOST_NATIVE="$native" AGIRU_PAGE_HOST_PROOF="$proof" AGIRU_PAGE_HOST_APPLICATION=1 \
  AGIRU_LIST_ROWS=80 node --test test/ui/page-host.mjs \
  > "$proof/list-80.log" 2>&1 || { cat "$proof/list-80.log"; exit 1; }
cat "$proof/list-80.log"
clean_auth
mutation='
  control == "owner" && /AND user_security_id = \$2::uuid AND host_id/ {
    sub(/user_security_id = \$2::uuid/, "($2::uuid IS NOT NULL)"); changed++
  }
  control == "owner" && /AND credential_digest = \$5 / {
    sub(/credential_digest = \$5/, "($5::text IS NOT NULL)"); changed++
  }
  /void CallOwnership\(/ { call_owner = 1 }
  /^  }$/ { call_owner = 0 }
  control == "client-owner" && /AND credential_digest = \$5 / {
    sub(/credential_digest = \$5/, "($5::text IS NOT NULL)"); changed++
  }
  control == "client-call-owner" && call_owner && /AND credential_digest = \$5 / {
    sub(/credential_digest = \$5/, "($5::text IS NOT NULL)"); changed++
  }
  control == "revision" && /Number\(\*revision\) != context.revision/ {
    sub(/Number\(\*revision\) != context.revision/, "false"); changed++
  }
  control == "replay" && /if \(rows.Rows\(\) == 0\)/ {
    sub(/rows.Rows\(\) == 0/, "(static_cast<void>(rows), true)"); changed++
  }
  control == "policy" && /session\(principal.user, options.session\)/ {
    sub(/session\(principal.user, options.session\)/, "session(principal.user, {})"); changed++
  }
  control == "list-limit" && /options.listRows, loader/ {
    sub(/options.listRows, loader/, "PageHostOptions::kDefaultListRows, loader"); changed++
  }
  control == "failure-receipt" && /AND outcome = \x27started\x27/ {
    sub(/AND outcome = \x27started\x27/, "AND outcome = \x27started\x27 AND false"); changed++
  }
  /ServerHttpResponse Await\(/ { awaiting = 1 }
  /^  }$/ { awaiting = 0 }
  control == "blocking-al" && awaiting && /call->ready.wait_for\(lock, options.responseWait,/ {
    sub(/options.responseWait/, "std::chrono::seconds(1)"); changed++
  }
  control == "creation-mode" && /card->Open\(mode\);/ {
    sub(/card->Open\(mode\)/, "card->Open(PageOpenMode::Edit)"); changed++
  }
  control == "creation-policy" && /if \(equal\("false"\)\) \{ return false; \}/ {
    sub(/return false/, "return true"); changed++
  }
  { print }
  END { if (changed != (control == "owner" ? 4 : control == "list-limit" || control == "client-owner" ? 2 : 1)) exit 2 }
'
prerequisites='native generated list retains|native list rows retain exact|external agent navigates list|actual shell CMD validates/saves|identical completed command replays|actual MCP saves the same|returning to the retained list rereads'
for control in owner client-owner client-call-owner revision replay policy list-limit failure-receipt blocking-al creation-mode creation-policy; do
  podman exec --user 1000:1001 "${AGIRU_DEV_CONTAINER:-agiru-dev}" \
    awk -v control="$control" "$mutation" /workspace/src/rt/PageCommandHost.cpp > "$proof/$control.cpp"
  podman cp "$proof/$control.cpp" "${AGIRU_DEV_CONTAINER:-agiru-dev}:$native/$control.cpp"
  make --no-print-directory dev-exec COMMAND="clang++-19 -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/rt -include runtime/Error.h -fPIC -shared $native/$control.cpp --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 -L/workspace/build/podman -Wl,-rpath,/workspace/build/podman -lagiru_rt -lagiru_net -lagiru_db -o $native/$control.so" \
    > "$proof/$control.compile.log" 2>&1
  status=0
  application=0
  if [[ "$control" = policy || "$control" = list-limit ]]; then application=1; fi
  case "$control" in
    owner) claim='foreign identities' ;;
    client-owner) claim='same-user clients cannot read' ;;
    client-call-owner) claim='native questions preserve the AL transaction' ;;
    revision) claim='PostgreSQL owns revision' ;;
    replay) claim='identical completed command replays' ;;
    policy) claim='startup configuration selects TryFunction write policy' ;;
    list-limit) claim='shared HTTP list windows obey the trusted bound' ;;
    failure-receipt) claim='a noncommitted AL action rolls back' ;;
    blocking-al) claim='a pending native write keeps one HTTP worker available' ;;
    creation-mode) claim='linked card New runs AL initialization' ;;
    creation-policy) claim='linked card InsertAllowed false' ;;
  esac
  AGIRU_PAGE_HOST_NATIVE="$native" AGIRU_PAGE_HOST_PROOF="$proof" \
    AGIRU_PAGE_HOST_APPLICATION="$application" AGIRU_TRY_WRITE_DISABLED=1 AGIRU_LIST_ROWS=7 \
    AGIRU_PAGE_HOST_PRELOAD="$native/$control.so" node --test --test-name-pattern="^($prerequisites|$claim)" test/ui/page-host.mjs \
    > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    owner) rg -q '^not ok .*foreign identities' "$proof/$control.log" ;;
    client-owner) rg -q '^not ok .*same-user clients cannot read' "$proof/$control.log" ;;
    client-call-owner) rg -q '^not ok .*native questions preserve the AL transaction' "$proof/$control.log" ;;
    revision) rg -q '^not ok .*PostgreSQL owns revision' "$proof/$control.log" ;;
    replay) rg -q '^not ok .*identical completed command replays' "$proof/$control.log" ;;
    policy) rg -q '^not ok .*startup configuration selects TryFunction write policy' "$proof/$control.log" ;;
    list-limit) rg -q '^not ok .*shared HTTP list windows obey the trusted bound' "$proof/$control.log" ;;
    failure-receipt) rg -q '^not ok .*a noncommitted AL action rolls back' "$proof/$control.log" ;;
    blocking-al) rg -q '^not ok .*a pending native write keeps one HTTP worker available' "$proof/$control.log" ;;
    creation-mode) rg -q '^not ok .*linked card New runs AL initialization' "$proof/$control.log" ;;
    creation-policy) rg -q '^not ok .*linked card InsertAllowed false' "$proof/$control.log" ;;
  esac
  clean_auth
  make --no-print-directory dev-exec COMMAND="rm -f -- $native/$control.cpp $native/$control.so"
  unlink "$proof/$control.cpp"
done
for control in modal-commit modal-replay modal-cancel modal-message-replay modal-input-validation modal-idle; do
  case "$control" in
    modal-commit) pattern='CMD MCP and Chromium explicitly select' ;;
    modal-replay|modal-cancel) pattern='modal SQL ownership revisions CSRF replay and permissions' ;;
    modal-message-replay) pattern='CMD MCP and Chromium edit exact original modal variables' ;;
    modal-input-validation) pattern='CMD MCP and Chromium refuse invalid typed modal input' ;;
    modal-idle) pattern='fresh authenticated modal input renews idle wait' ;;
  esac
  podman exec --user agiru "${AGIRU_DEV_CONTAINER:-agiru-dev}" awk -v control="$control" '
    control == "modal-commit" && /void PublishSnapshot\(\) \{/ {
      print; print "    agiru::Commit();"; changed++; next
    }
    control == "modal-replay" && /replay.Value\(0, 0\) != input.digest/ {
      sub(/replay.Value\(0, 0\) != input.digest/, "false"); changed++
    }
    control == "modal-cancel" && /return page_.CloseModal\(control ==/ {
      sub(/control == "\$agiru.modal_ok" \? Action::OK : Action::Cancel/, "Action::OK"); changed++
    }
    control == "modal-message-replay" && /^[[:space:]]+receiptHtml,$/ {
      sub(/receiptHtml/, "html"); changed++
    }
    control == "modal-input-validation" && /input->operation == "set" && executionError.Code\(\) == "TestValidation";/ {
      sub(/input->operation == "set" && executionError.Code\(\) == "TestValidation"/, "false"); changed++
    }
    control == "modal-idle" && /std::min\(call_->deadline, std::chrono::steady_clock::now\(\) \+ options_.dialogTimeout\)/ {
      sub(/std::chrono::steady_clock::now\(\)/, "started"); changed++
    }
    control == "modal-idle" && /Action Run\(\) \{/ {
      print; print "    const auto started = std::chrono::steady_clock::now();"; next
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' /workspace/src/rt/PageModal.cpp > "$proof/$control.cpp"
  podman cp "$proof/$control.cpp" "${AGIRU_DEV_CONTAINER:-agiru-dev}:$native/$control.cpp"
  make --no-print-directory dev-exec COMMAND="clang++-19 -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/rt -include runtime/Error.h -fPIC -shared $native/$control.cpp --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 -L/workspace/build/podman -Wl,-rpath,/workspace/build/podman -lagiru_rt -lagiru_net -lagiru_db -o $native/$control.so" \
    > "$proof/$control.compile.log" 2>&1
  status=0
  AGIRU_PAGE_HOST_NATIVE="$native" AGIRU_PAGE_HOST_PROOF="$proof" AGIRU_PAGE_HOST_PRELOAD="$native/$control.so" \
    node --test --test-name-pattern="$pattern" test/ui/page-host.mjs > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    modal-commit) rg -q '^not ok .*CMD MCP and Chromium explicitly select' "$proof/$control.log" ;;
    modal-replay|modal-cancel) rg -q '^not ok .*modal SQL ownership revisions CSRF replay and permissions' "$proof/$control.log" ;;
    modal-message-replay) rg -q '^not ok .*CMD MCP and Chromium edit exact original modal variables' "$proof/$control.log" ;;
    modal-input-validation) rg -q '^not ok .*CMD MCP and Chromium refuse invalid typed modal input' "$proof/$control.log" ;;
    modal-idle) rg -q '^not ok .*fresh authenticated modal input renews idle wait' "$proof/$control.log" ;;
  esac
  clean_auth
  make --no-print-directory dev-exec COMMAND="rm -f -- $native/$control.cpp $native/$control.so"
  unlink "$proof/$control.cpp"
done
for control in dialog-default dialog-commit dialog-replay modal-poll-receipt; do
  podman exec --user agiru "${AGIRU_DEV_CONTAINER:-agiru-dev}" awk -v control="$control" '
    control == "dialog-default" && /call->question = held;/ {
      $0 = $0 " held->answer = held->defaultChoice;"; changed++
    }
    control == "dialog-commit" && /std::unique_lock lock\(call->mutex\);/ {
      print "    agiru::Commit();"; changed++
    }
    control == "dialog-replay" && /replay.Value\(0, 0\) != std::to_string\(choice\)/ {
      sub(/replay.Value\(0, 0\) != std::to_string\(choice\)/, "false"); changed++
    }
    control == "modal-poll-receipt" && /const auto poll = ModalPollAttributes\(call\);/ {
      sub(/ModalPollAttributes\(call\)/, "(static_cast<void>(ModalPollAttributes(call)), std::string{})"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' /workspace/src/rt/PageInteraction.cpp > "$proof/$control.cpp"
  podman cp "$proof/$control.cpp" "${AGIRU_DEV_CONTAINER:-agiru-dev}:$native/$control.cpp"
  make --no-print-directory dev-exec COMMAND="clang++-19 -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/rt -include runtime/Error.h -fPIC -shared $native/$control.cpp --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 -L/workspace/build/podman -Wl,-rpath,/workspace/build/podman -lagiru_rt -lagiru_net -lagiru_db -o $native/$control.so" \
    > "$proof/$control.compile.log" 2>&1
  status=0
  case "$control" in
    dialog-default|dialog-commit) pattern='^native questions preserve the AL transaction' ;;
    dialog-replay) pattern='^nested native questions reject replaced answers' ;;
    modal-poll-receipt) pattern='^query-close veto' ;;
  esac
  AGIRU_PAGE_HOST_NATIVE="$native" AGIRU_PAGE_HOST_PROOF="$proof" AGIRU_PAGE_HOST_PRELOAD="$native/$control.so" \
    node --test --test-name-pattern="$pattern" test/ui/page-host.mjs > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    dialog-default|dialog-commit) rg -q '^not ok .*native questions preserve the AL transaction' "$proof/$control.log" ;;
    dialog-replay) rg -q '^not ok .*nested native questions reject replaced answers' "$proof/$control.log" ;;
    modal-poll-receipt) rg -q '^not ok .*query-close veto' "$proof/$control.log" ;;
  esac
  clean_auth
  make --no-print-directory dev-exec COMMAND="rm -f -- $native/$control.cpp $native/$control.so"
  unlink "$proof/$control.cpp"
done
podman exec --user agiru "${AGIRU_DEV_CONTAINER:-agiru-dev}" awk '
  /if \(rejectDuplicates && object.contains\(name\)\)/ {
    sub(/rejectDuplicates && object.contains\(name\)/, "false"); changed++
  }
  { print }
  END { if (changed != 1) exit 2 }
' /workspace/src/net/JsonEngine.cpp > "$proof/duplicates.cpp"
podman cp "$proof/duplicates.cpp" "${AGIRU_DEV_CONTAINER:-agiru-dev}:$native/duplicates.cpp"
make --no-print-directory dev-exec COMMAND="clang++-19 -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/net -fPIC -shared $native/duplicates.cpp --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 -L/workspace/build/podman -Wl,-rpath,/workspace/build/podman -lagiru_net -lyyjson -o $native/duplicates.so" \
  > "$proof/duplicates.compile.log" 2>&1
status=0
make --no-print-directory dev-exec COMMAND="env LD_PRELOAD=$native/duplicates.so /workspace/build/podman/gate_NativeServiceConfigGate" \
  > "$proof/duplicates.log" 2>&1 || status=$?
[[ "$status" = 2 ]]
rg -q 'FAIL.*duplicate configuration keys refuse' "$proof/duplicates.log"
make --no-print-directory dev-exec COMMAND="rm -f -- $native/duplicates.cpp $native/duplicates.so"
unlink "$proof/duplicates.cpp"
sha256sum --check "$proof/inputs.sha256" "$proof/scalar-inputs.sha256" > "$proof/integrity.log"
printf 'page-host: generated-page SQL effects, asynchronous AL calls, explicit questions/messages/modals, linked-card creation and failed-command diagnostics over Caddy/C++, external CMD/MCP/htmx and config-only agiru serve; list limits 7/40/80 and both TryFunction write policies; twenty-two compiled ownership/client-binding/revision/replay/policy/duplicate/list-bound/failed-receipt/blocking-AL/default-answer/implicit-commit/changed-answer/creation-mode/creation-policy/modal-commit/modal-replay/modal-cancel/modal-message-replay/modal-poll-receipt/modal-input-validation/modal-idle defects rejected; not full ERP acceptance; %s\n' "$proof"

#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
proof=$(mktemp -d /tmp/agiru-page-host.XXXXXX)
gate_database=${AGIRU_PAGE_GATE_DATABASE:-agiru_gate}
[[ "$gate_database" =~ ^[a-z_][a-z0-9_]{0,62}$ ]]
git rev-parse HEAD > "$proof/head.txt"
sha256sum Makefile include/runtime/{PageCommandHost,PageHtml,PageInstance,PageSession,SessionCommand}.h \
  src/rt/{PageCommandHost,PageHtml,PageInstance,SessionCommand,HtmlText}.cpp src/rt/HtmlText.h \
  src/rt/PageListHtml.h include/runtime/{PageWindow,RecordWindow}.h src/rt/RecordWindow.cpp \
  include/runtime/TablePermissions.h src/rt/{TablePermissions,Session,Table,Navigate,Query,RecordRef}.cpp \
  include/runtime/NativePermissions.h src/rt/{NativePermissions,NativePermissionSnapshot}.cpp \
  include/runtime/PermissionSetRegistry.h src/rt/PermissionSetRegistry.cpp \
  include/runtime/NativeService.h src/rt/{NativeService,NativeServiceConfig}.cpp deploy/dev/agiru.json \
  include/runtime/{HttpServerOptions,PageHostOptions,SessionOptions}.h \
  src/net/JsonEngine.{h,cpp} test/gate/NativeServiceConfigGate.cpp \
  src/cli/{Main,Services}.cpp src/cli/Services.h test/gate/NativePermissionFixture.h \
  test/ui/page-host.{sh,mjs} test/ui/{server-config,browser-client}.mjs test/ui/page-host/Runner.cpp test/runtime/page-navigation.sh \
  test/runtime/page-navigation/*.al test/gate/PrivateAuthFile.h \
  src/client/*.{mts,json} > "$proof/inputs.sha256"
make dev-exec COMMAND='findmnt -T /tmp'
make dev-exec COMMAND='df -h /tmp'
native=$(make --no-print-directory dev-exec COMMAND='mktemp -d /tmp/agiru-native-page-host.XXXXXX')
[[ "$native" =~ ^/tmp/agiru-native-page-host\.[A-Za-z0-9]+$ ]]
cleanup() {
  make --no-print-directory dev-exec COMMAND="rm -f -- $native/host $native/agiru $native/auth.json $native/auth.json.second $native/owner.cpp $native/owner.so $native/revision.cpp $native/revision.so $native/replay.cpp $native/replay.so $native/policy.cpp $native/policy.so $native/duplicates.cpp $native/duplicates.so" || :
  make --no-print-directory dev-exec COMMAND="rmdir -- $native" || :
  for file in "$proof/auth.json" "$proof/auth.json.second"; do
    if [[ -f "$file" ]]; then unlink "$file"; fi
  done
  for control in owner revision replay policy duplicates list-limit failure-receipt; do
    if [[ -f "$proof/$control.cpp" ]]; then unlink "$proof/$control.cpp"; fi
    make --no-print-directory dev-exec COMMAND="rm -f -- $native/$control.cpp $native/$control.so" || :
  done
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
for auth in "$proof/auth.json" "$proof/auth.json.second"; do unlink "$auth"; done
make --no-print-directory dev-exec COMMAND="rm -f -- $native/auth.json $native/auth.json.second"
AGIRU_PAGE_HOST_NATIVE="$native" AGIRU_PAGE_HOST_PROOF="$proof" AGIRU_PAGE_HOST_APPLICATION=1 \
  node --test test/ui/page-host.mjs > "$proof/application.log" 2>&1 || { cat "$proof/application.log"; exit 1; }
cat "$proof/application.log"
for auth in "$proof/auth.json" "$proof/auth.json.second"; do unlink "$auth"; done
make --no-print-directory dev-exec COMMAND="rm -f -- $native/auth.json $native/auth.json.second"
AGIRU_PAGE_HOST_NATIVE="$native" AGIRU_PAGE_HOST_PROOF="$proof" AGIRU_PAGE_HOST_APPLICATION=1 \
  AGIRU_TRY_WRITE_DISABLED=1 AGIRU_LIST_ROWS=7 node --test test/ui/page-host.mjs \
  > "$proof/try-disabled.log" 2>&1 || { cat "$proof/try-disabled.log"; exit 1; }
cat "$proof/try-disabled.log"
for auth in "$proof/auth.json" "$proof/auth.json.second"; do unlink "$auth"; done
make --no-print-directory dev-exec COMMAND="rm -f -- $native/auth.json $native/auth.json.second"
AGIRU_PAGE_HOST_NATIVE="$native" AGIRU_PAGE_HOST_PROOF="$proof" AGIRU_PAGE_HOST_APPLICATION=1 \
  AGIRU_LIST_ROWS=80 node --test test/ui/page-host.mjs \
  > "$proof/list-80.log" 2>&1 || { cat "$proof/list-80.log"; exit 1; }
cat "$proof/list-80.log"
for auth in "$proof/auth.json" "$proof/auth.json.second"; do unlink "$auth"; done
make --no-print-directory dev-exec COMMAND="rm -f -- $native/auth.json $native/auth.json.second"
mutation='
  control == "owner" && /AND user_security_id = \$2::uuid AND host_id/ {
    sub(/user_security_id = \$2::uuid/, "($2::uuid IS NOT NULL)"); changed++
  }
  control == "revision" && /Number\(\*revision\) != context.revision/ {
    sub(/Number\(\*revision\) != context.revision/, "false"); changed++
  }
  control == "replay" && /if \(rows.Rows\(\) == 0\)/ {
    sub(/rows.Rows\(\) == 0/, "(static_cast<void>(rows), true)"); changed++
  }
  control == "policy" && /session\(principal, options.session\)/ {
    sub(/session\(principal, options.session\)/, "session(principal, {})"); changed++
  }
  control == "list-limit" && /options.listRows, loader/ {
    sub(/options.listRows, loader/, "PageHostOptions::kDefaultListRows, loader"); changed++
  }
  control == "failure-receipt" && /AND outcome = \x27started\x27/ {
    sub(/AND outcome = \x27started\x27/, "AND outcome = \x27started\x27 AND false"); changed++
  }
  { print }
  END { if (changed != (control == "list-limit" ? 2 : 1)) exit 2 }
'
for control in owner revision replay policy list-limit failure-receipt; do
  podman exec --user 1000:1001 "${AGIRU_DEV_CONTAINER:-agiru-dev}" \
    awk -v control="$control" "$mutation" /workspace/src/rt/PageCommandHost.cpp > "$proof/$control.cpp"
  podman cp "$proof/$control.cpp" "${AGIRU_DEV_CONTAINER:-agiru-dev}:$native/$control.cpp"
  make --no-print-directory dev-exec COMMAND="clang++-19 -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/rt -fPIC -shared $native/$control.cpp --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 -L/workspace/build/podman -Wl,-rpath,/workspace/build/podman -lagiru_rt -lagiru_net -lagiru_db -o $native/$control.so" \
    > "$proof/$control.compile.log" 2>&1
  status=0
  application=0
  if [[ "$control" = policy || "$control" = list-limit ]]; then application=1; fi
  AGIRU_PAGE_HOST_NATIVE="$native" AGIRU_PAGE_HOST_PROOF="$proof" \
    AGIRU_PAGE_HOST_APPLICATION="$application" AGIRU_TRY_WRITE_DISABLED=1 AGIRU_LIST_ROWS=7 \
    AGIRU_PAGE_HOST_PRELOAD="$native/$control.so" node --test test/ui/page-host.mjs \
    > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    owner) rg -q '^not ok .*foreign identities' "$proof/$control.log" ;;
    revision) rg -q '^not ok .*PostgreSQL owns revision' "$proof/$control.log" ;;
    replay) rg -q '^not ok .*identical completed command replays' "$proof/$control.log" ;;
    policy) rg -q '^not ok .*startup configuration selects TryFunction write policy' "$proof/$control.log" ;;
    list-limit) rg -q '^not ok .*shared HTTP list windows obey the trusted bound' "$proof/$control.log" ;;
    failure-receipt) rg -q '^not ok .*a noncommitted AL action rolls back' "$proof/$control.log" ;;
  esac
  for auth in "$proof/auth.json" "$proof/auth.json.second"; do unlink "$auth"; done
  make --no-print-directory dev-exec COMMAND="rm -f -- $native/auth.json $native/auth.json.second $native/$control.cpp $native/$control.so"
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
sha256sum --check "$proof/inputs.sha256" > "$proof/integrity.log"
printf 'page-host: generated-page SQL effects and failed-command diagnostics over Caddy/C++, external CMD/MCP/htmx and config-only agiru serve; list limits 7/40/80 and both TryFunction write policies; seven compiled ownership/revision/replay/policy/duplicate/list-bound/failed-receipt defects rejected; not full ERP acceptance; %s\n' "$proof"

#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
proof=$(mktemp -d /tmp/agiru-page-host.XXXXXX)
git rev-parse HEAD > "$proof/head.txt"
sha256sum Makefile include/runtime/{PageCommandHost,PageHtml,PageInstance,PageSession,SessionCommand}.h \
  src/rt/{PageCommandHost,PageHtml,PageInstance,SessionCommand,HtmlText}.cpp src/rt/HtmlText.h \
  test/ui/page-host.{sh,mjs} test/ui/page-host/Runner.cpp test/runtime/page-navigation.sh \
  test/runtime/page-navigation/*.al test/gate/PrivateAuthFile.h \
  src/client/*.{mts,json} > "$proof/inputs.sha256"
make dev-exec COMMAND='findmnt -T /tmp'
make dev-exec COMMAND='df -h /tmp'
native=$(make --no-print-directory dev-exec COMMAND='mktemp -d /tmp/agiru-native-page-host.XXXXXX')
[[ "$native" =~ ^/tmp/agiru-native-page-host\.[A-Za-z0-9]+$ ]]
cleanup() {
  make --no-print-directory dev-exec COMMAND="rm -f -- $native/host $native/auth.json $native/auth.json.second $native/owner.cpp $native/owner.so $native/revision.cpp $native/revision.so $native/replay.cpp $native/replay.so" || :
  make --no-print-directory dev-exec COMMAND="rmdir -- $native" || :
  for file in "$proof/auth.json" "$proof/auth.json.second"; do
    if [[ -f "$file" ]]; then unlink "$file"; fi
  done
  for control in owner revision replay; do
    if [[ -f "$proof/$control.cpp" ]]; then unlink "$proof/$control.cpp"; fi
  done
}
trap cleanup EXIT
make dev-exec COMMAND="env AGIRU_TEST_DSN=postgresql://agiru:agiru@127.0.0.1:5432/agiru_gate AGIRU_PAGE_HOST_BUILD=$native make page-navigation JOBS=2 B=/workspace/build/podman" \
  > "$proof/native.log" 2>&1 || { cat "$proof/native.log"; exit 1; }
cat "$proof/native.log"
AGIRU_PAGE_HOST_NATIVE="$native" AGIRU_PAGE_HOST_PROOF="$proof" node --test test/ui/page-host.mjs \
  > "$proof/execution.log" 2>&1 || { cat "$proof/execution.log"; exit 1; }
cat "$proof/execution.log"
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
  { print }
  END { if (changed != 1) exit 2 }
'
for control in owner revision replay; do
  podman exec --user 1000:1001 "${AGIRU_DEV_CONTAINER:-agiru-dev}" \
    awk -v control="$control" "$mutation" /workspace/src/rt/PageCommandHost.cpp > "$proof/$control.cpp"
  podman cp "$proof/$control.cpp" "${AGIRU_DEV_CONTAINER:-agiru-dev}:$native/$control.cpp"
  make --no-print-directory dev-exec COMMAND="clang++-19 -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/rt -fPIC -shared $native/$control.cpp --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 -L/workspace/build/podman -Wl,-rpath,/workspace/build/podman -lagiru_rt -lagiru_net -lagiru_db -o $native/$control.so" \
    > "$proof/$control.compile.log" 2>&1
  status=0
  AGIRU_PAGE_HOST_NATIVE="$native" AGIRU_PAGE_HOST_PROOF="$proof" \
    AGIRU_PAGE_HOST_PRELOAD="$native/$control.so" node --test test/ui/page-host.mjs \
    > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    owner) rg -q '^not ok .*foreign identities' "$proof/$control.log" ;;
    revision) rg -q '^not ok .*PostgreSQL owns revision' "$proof/$control.log" ;;
    replay) rg -q '^not ok .*identical completed command replays' "$proof/$control.log" ;;
  esac
  for auth in "$proof/auth.json" "$proof/auth.json.second"; do unlink "$auth"; done
  make --no-print-directory dev-exec COMMAND="rm -f -- $native/auth.json $native/auth.json.second $native/$control.cpp $native/$control.so"
  unlink "$proof/$control.cpp"
done
sha256sum --check "$proof/inputs.sha256" > "$proof/integrity.log"
printf 'page-host: actual generated-page SQL effects over nginx/C++ and external CMD/MCP; three compiled ownership/revision/replay defects rejected; fixture authorization, not full ERP permission/browser acceptance; %s\n' "$proof"

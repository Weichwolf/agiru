#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
proof=$(mktemp -d /tmp/agiru-http-server.XXXXXX)
git rev-parse HEAD > "$proof/head.txt"
sha256sum CMakeLists.txt Makefile include/runtime/{HttpServer.h,PageHtml.h,PageCore.h} \
  src/net/HttpServer.cpp src/rt/PageHtml.cpp test/gate/{HttpServerGate.cpp,PageHtmlGate.cpp} \
  test/ui/http-server.{sh,mjs} src/client/*.{mts,json} deploy/dev/{Containerfile,nginx.conf,entrypoint.sh} \
  scripts/dev_container.sh > "$proof/inputs.sha256"
make dev-exec COMMAND='findmnt -T /tmp'
make dev-exec COMMAND='df -h /tmp'
make dev-exec COMMAND='make gate GATE=HttpServerGate JOBS=2 B=/workspace/build/podman' > "$proof/cpp-gate.log"
make dev-exec COMMAND='make gate GATE=PageHtmlGate JOBS=2 B=/workspace/build/podman' >> "$proof/cpp-gate.log"
cat "$proof/cpp-gate.log"
native=$(make --no-print-directory dev-exec COMMAND='mktemp -d /tmp/agiru-native-http.XXXXXX')
[[ "$native" =~ ^/tmp/agiru-native-http\.[A-Za-z0-9]+$ ]]
static_created=0
cleanup() {
  if [[ "$static_created" = 1 ]]; then
    podman exec --user 0 "${AGIRU_DEV_CONTAINER:-agiru-dev}" rm -f -- /usr/share/agiru/web/http-fixture.html || :
  fi
  make --no-print-directory dev-exec COMMAND="rm -f -- $native/page.html" || :
  make --no-print-directory dev-exec COMMAND="rmdir -- $native" || :
}
trap cleanup EXIT
make --no-print-directory dev-exec COMMAND='/workspace/build/podman/gate_PageHtmlGate --html' > "$proof/page.html"
podman cp "$proof/page.html" "${AGIRU_DEV_CONTAINER:-agiru-dev}:$native/page.html"
podman exec --user 0 "${AGIRU_DEV_CONTAINER:-agiru-dev}" test ! -e /usr/share/agiru/web/http-fixture.html
static_created=1
podman cp "$proof/page.html" "${AGIRU_DEV_CONTAINER:-agiru-dev}:/usr/share/agiru/web/http-fixture.html"
AGIRU_NATIVE_HTTP_HTML="$native/page.html" AGIRU_CLIENT_HTML="$proof/page.html" \
  node --test test/ui/http-server.mjs > "$proof/execution.log" 2>&1 || {
    cat "$proof/execution.log"
    exit 1
  }
cat "$proof/execution.log"
sha256sum --check "$proof/inputs.sha256" > "$proof/input-integrity.log"
podman inspect "${AGIRU_DEV_CONTAINER:-agiru-dev}" --format '{{.Image}} {{json .NetworkSettings.Ports}}' > "$proof/container.txt"
printf 'http-server: nginx, private C++ HTTP transport and PostgreSQL fixture in one container; not production ERP/session parity\n'
printf 'http-server: %s\n' "$proof"

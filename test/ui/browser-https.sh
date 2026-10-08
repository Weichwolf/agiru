#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
proof=$(mktemp -d /tmp/agiru-browser-https.XXXXXX)
name=agiru-browser-https-$$
image=${AGIRU_DEV_IMAGE:-localhost/agiru-dev:latest}
cleanup() {
  if podman container exists "$name"; then
    [[ "$(podman inspect --format '{{index .Config.Labels "io.agiru.browser.test"}}' "$name")" = "$name" ]] || return
    podman rm --force "$name" > "$proof/cleanup.log"
  fi
}
trap cleanup EXIT
findmnt -T /tmp > "$proof/mount.txt"
df -h /tmp > "$proof/space.txt"
git rev-parse HEAD > "$proof/head"
sha256sum Makefile deploy/dev/{Caddyfile,Containerfile,entrypoint.sh,agiru.json} \
  src/rt/BrowserHttp.{h,cpp} src/rt/BrowserSession.cpp src/net/{HttpServer,SecureToken}.cpp \
  test/gate/BrowserHttpGate.cpp test/ui/browser-https.{sh,mjs} > "$proof/inputs.sha256"
make dev-exec COMMAND='make gate GATE=BrowserHttpGate B=/workspace/build/podman JOBS=2' > "$proof/gate.log" 2>&1
cat "$proof/gate.log"
sha256sum build/podman/gate_BrowserHttpGate build/podman/libagiru_{rt,net,db}.so > "$proof/binaries.sha256"
podman image inspect "$image" --format '{{.Id}}' > "$proof/image"
podman run --detach --name "$name" --label "io.agiru.browser.test=$name" \
  --env AGIRU_HTTP_ADDRESS=localhost --publish 127.0.0.1::8443 \
  --tmpfs /var/lib/agiru:rw --volume "$PWD:/workspace:ro" \
  --volume "$PWD/deploy/dev/Caddyfile:/etc/agiru/Caddyfile:ro" \
  "$image" > "$proof/container-id"
root_certificate=/var/lib/agiru/caddy/data/caddy/pki/authorities/local/root.crt
ready=0
for ((attempt=0; attempt<60; attempt++)); do
  if podman exec "$name" curl --silent --cacert "$root_certificate" --output /dev/null \
    --write-out '%{http_code}' https://localhost:8443/ > "$proof/readiness.log" 2>&1 \
    && [[ "$(<"$proof/readiness.log")" = 502 ]]; then ready=1; break; fi
  [[ "$(podman inspect --format '{{.State.Running}}' "$name")" = true ]] || {
    podman logs "$name" >&2; exit 1;
  }
  sleep 0.5
done
[[ "$ready" = 1 ]] || { printf 'browser-https: Caddy TLS readiness timed out\n' >&2; exit 1; }
podman cp "$name:$root_certificate" "$proof/ca.crt"
podman port "$name" 8443/tcp > "$proof/port"
podman exec "$name" dpkg-query -W -f='${Version}\n' caddy > "$proof/caddy-package"
podman exec "$name" dpkg-query -S /usr/bin/caddy > "$proof/caddy-owner"
AGIRU_BROWSER_TLS_CONTAINER="$name" AGIRU_BROWSER_TLS_PROOF="$proof" \
  node --test test/ui/browser-https.mjs > "$proof/execution.log" 2>&1 || {
    cat "$proof/execution.log"; exit 1;
  }
cat "$proof/execution.log"
podman logs "$name" > "$proof/container.log" 2>&1
sha256sum --check --status "$proof/inputs.sha256"
sha256sum --check --status "$proof/binaries.sha256"
printf 'browser-https: trusted Caddy TLS/private native auth/PostgreSQL in one disposable container; protocol SQL probe, not browser UI or ERP acceptance; %s\n' "$proof"

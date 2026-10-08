#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
proof=$(mktemp -d /tmp/agiru-web-client.XXXXXX)
trap 'find "$proof" -maxdepth 1 -type f -name "*.mjs" -delete' EXIT
sha256sum src/client/*.{mts,json} src/client/web/* scripts/build_web.sh \
  test/ui/web-client.{sh,mjs} test/ui/{web-assets,browser-client,dialog-fixture}.mjs deploy/dev/Caddyfile \
  scripts/dev_container.sh build/web/* > "$proof/inputs.sha256"
container=${AGIRU_DEV_CONTAINER:-agiru-dev}
git rev-parse HEAD > "$proof/head.txt"
node --version > "$proof/node-version.txt"
podman exec --user agiru "$container" sha256sum /workspace/build/podman/gate_PageHtmlGate \
  /workspace/build/podman/libagiru_rt.so > "$proof/producer.sha256"
podman exec --user agiru "$container" /workspace/build/podman/gate_PageHtmlGate > "$proof/cpp-gate.log"
podman exec --user agiru "$container" /workspace/build/podman/gate_PageHtmlGate --html > "$proof/page.html"
cat "$proof/cpp-gate.log"
AGIRU_CLIENT_HTML="$proof/page.html" AGIRU_WEB_PROOF="$proof" \
  node --test test/ui/web-client.mjs > "$proof/browser.log" 2>&1 || {
  cat "$proof/browser.log"
  exit 1
}
cat "$proof/browser.log"
for control in response-effects altered-envelope concurrent-post; do
  awk -v control="$control" '
    control == "response-effects" && /if \(responseEffects.some\(hasHeader2\)\)/ {
      sub(/responseEffects.some\(hasHeader2\)/, "false"); changed++
    }
    control == "altered-envelope" && /supplied.some\(\(\[name, value\]\) => envelope.fields\[name\] !== value\)/ {
      sub(/supplied.some\(\(\[name, value\]\) => envelope.fields\[name\] !== value\)/, "false"); changed++
    }
    control == "concurrent-post" && /if \(active\) \{/ {
      sub(/if \(active\)/, "if (false)"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' build/web/web.js > "$proof/$control.mjs"
  node --check "$proof/$control.mjs"
  status=0
  AGIRU_CLIENT_HTML="$proof/page.html" AGIRU_WEB_PROOF="$proof" \
    AGIRU_WEB_SCRIPT="$proof/$control.mjs" node --test test/ui/web-client.mjs \
    > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  case "$control" in
    response-effects) claim='undeclared htmx response effects refuse';;
    altered-envelope) claim='altered hidden envelope is refused';;
    concurrent-post) claim='shared admission prevents two simultaneous';;
  esac
  rg -q "^not ok .*${claim}" "$proof/$control.log"
  rm -- "$proof/$control.mjs"
done
AGIRU_WEB_PROOF="$proof" node --test test/ui/web-assets.mjs > "$proof/caddy-test.log" 2>&1 || {
  cat "$proof/caddy-test.log"
  exit 1
}
cat "$proof/caddy-test.log"
sha256sum --check --status "$proof/inputs.sha256"
printf 'web-client: three compiled response/envelope/admission defects rejected by named browser cases\n'
printf 'web-client: actual Chromium/htmx and agent parity over native HTML fixture, not ERP acceptance\n'
printf 'web-client: %s\n' "$proof"

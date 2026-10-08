#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
proof=$(mktemp -d /tmp/agiru-agent-client.XXXXXX)
sha256sum src/client/*.{mts,json} test/ui/agent-client.{sh,mjs} test/ui/dialog-fixture.mjs \
  include/runtime/PageHtml.h src/rt/PageHtml.cpp src/rt/HtmlText.{h,cpp} test/gate/PageHtmlGate.cpp > "$proof/inputs.sha256"
if [[ -n ${AGIRU_PAGE_HTML_GATE:-} ]]; then
  "$AGIRU_PAGE_HTML_GATE" > "$proof/cpp-gate.log"
  "$AGIRU_PAGE_HTML_GATE" --html > "$proof/page.html"
else
  make dev-exec COMMAND='make gate GATE=PageHtmlGate JOBS=2 B=/workspace/build/podman' > "$proof/cpp-gate.log"
  make --no-print-directory dev-exec COMMAND='/workspace/build/podman/gate_PageHtmlGate --html' > "$proof/page.html"
fi
cat "$proof/cpp-gate.log"
AGIRU_CLIENT_HTML="$proof/page.html" node --test test/ui/agent-client.mjs > "$proof/client.log" 2>&1 || {
  cat "$proof/client.log"
  exit 1
}
cat "$proof/client.log"
for control in rounded-scalars disabled-command stale-revision double-post blocking-auth unsafe-read-hints opening-replay error-identity; do
  mutant="$proof/$control"
  mkdir "$mutant"
  cp build/client/*.mjs "$mutant/"
  ln -s "$(realpath src/client/node_modules)" "$mutant/node_modules"
  source=profile
  case "$control" in stale-revision|double-post|blocking-auth|opening-replay|error-identity) source=http;; esac
  if [[ "$control" = unsafe-read-hints ]]; then source=mcp; fi
  awk -v control="$control" '
    control == "rounded-scalars" && /value: attr\(node, "data-value"\)/ {
      sub(/value: attr\(node, "data-value"\)/, "value: String(Number(attr(node, \"data-value\")))"); changed++
    }
    control == "disabled-command" && /if \(!operation.enabled\)/ {
      sub(/!operation.enabled/, "false"); changed++
    }
    control == "stale-revision" && /if \(current.page.handle !== requested.page \|\| current.page.revision !== requested.revision\)/ {
      sub(/current.page.handle !== requested.page \|\| current.page.revision !== requested.revision/, "false"); changed++
    }
    control == "double-post" && /return this.#request\(envelope.path, envelope.fields, requested.command, this.#timeout, originCommand\);/ {
      sub(/return this.#request/, "await this.#request(envelope.path, envelope.fields, requested.command, this.#timeout, originCommand); return this.#request"); changed++
    }
    control == "blocking-auth" && /constants\.O_NONBLOCK/ {
      sub(/ \| constants\.O_NONBLOCK/, ""); changed++
    }
    control == "unsafe-read-hints" && /annotations: \{ readOnlyHint: false, destructiveHint: true, idempotentHint: false \}/ {
      sub(/readOnlyHint: false, destructiveHint: true, idempotentHint: false/,
        "readOnlyHint: name === \"read\", destructiveHint: name === \"execute\", idempotentHint: name === \"read\""); changed++
    }
    control == "opening-replay" && /if \(!retained\)/ {
      sub(/!retained/, "false"); changed++
    }
    control == "error-identity" && /if \(command && failure.command !== expected\)/ {
      sub(/command && failure.command !== expected/, "false"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' "build/client/$source.mjs" > "$mutant/$source.mjs"
  node --check "$mutant/$source.mjs"
  status=0
  AGIRU_CLIENT_HTML="$proof/page.html" AGIRU_CLIENT_MODULES="$mutant" \
    node --test test/ui/agent-client.mjs > "$proof/$control.log" 2>&1 || status=$?
  [[ "$status" = 1 ]]
  rg -q '^# fail [1-9]' "$proof/$control.log"
  if [[ "$control" = blocking-auth ]]; then
    rg -q '^not ok .*private auth files reject FIFOs without waiting for a writer' "$proof/$control.log"
  fi
  if [[ "$control" = unsafe-read-hints ]]; then
    rg -q '^not ok .*MCP real stdio initialize/discover/read/set/action' "$proof/$control.log"
  fi
  if [[ "$control" = opening-replay ]]; then
    rg -q '^not ok .*execute refuses opening or mismatched paths before any HTTP request' "$proof/$control.log"
  fi
  if [[ "$control" = error-identity ]]; then
    rg -q '^not ok .*typed server errors retain exact diagnostics' "$proof/$control.log"
  fi
  rm -- "$mutant/"*.mjs "$mutant/node_modules"
  rmdir "$mutant"
done
sha256sum --check --status "$proof/inputs.sha256"
printf 'agent-client: eight executable value/disabled/revision/duplicate-write/blocking-auth/read-hint/opening-replay/error-identity mutants rejected\n'
printf 'agent-client: semantic HTML/CMD/MCP transport fixture; no ERP SQL/browser parity claim\n'
printf 'agent-client: %s\n' "$proof"

#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
proof=$(mktemp -d /tmp/agiru-agent-client.XXXXXX)
sha256sum src/client/*.{mts,json} test/ui/agent-client.{sh,mjs} \
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
for control in rounded-scalars disabled-command stale-revision double-post; do
  mutant="$proof/$control"
  mkdir "$mutant"
  cp build/client/*.mjs "$mutant/"
  ln -s "$(realpath src/client/node_modules)" "$mutant/node_modules"
  source=profile
  case "$control" in stale-revision|double-post) source=http;; esac
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
    control == "double-post" && /return this.#request\(envelope.path, envelope.fields, requested.command\);/ {
      sub(/return this.#request/, "await this.#request(envelope.path, envelope.fields, requested.command); return this.#request"); changed++
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
  rm -- "$mutant/"*.mjs "$mutant/node_modules"
  rmdir "$mutant"
done
printf 'agent-client: four executable value/disabled/revision/duplicate-write mutants rejected\n'
printf 'agent-client: semantic HTML/CMD/MCP transport fixture; no ERP SQL/browser parity claim\n'
printf 'agent-client: %s\n' "$proof"

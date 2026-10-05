#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
if [ "$#" -ne 3 ]; then
  printf 'transpile: expected source root, apps.json and output root\n' >&2
  exit 2
fi
B=$(realpath "${B:-build}")
proof=$(mktemp -d /tmp/agiru-transpile.XXXXXX)
printf '%s\n' "$proof" > "$B/transpile.latest"
arguments=("$1" "$2" "$3")
if [ -n "${AGIRU_HOST_RUNTIME:-}" ]; then arguments+=(--host-runtime "$AGIRU_HOST_RUNTIME"); fi
sha256sum "$B/agirutc" "$2" "$(dirname "$2")/scope.json" > "$proof/inputs.sha256"
git rev-parse HEAD > "$proof/head.txt"
if [ -n "${AGIRU_SYSTEM_SYMBOLS:-}" ]; then
  package=$(realpath "$AGIRU_SYSTEM_SYMBOLS")
  python3 scripts/fetch_symbols.py --verify "$package" > "$proof/package-before.json"
  cp "$package/provenance.json" "$proof/package-provenance.json"
  printf '%s\n' '{"apps":[{"name":"native","source":"src"}]}' > "$proof/native-apps.json"
  python3 scripts/scope_inventory.py "$package" --apps "$proof/native-apps.json" \
    --scope "$(dirname "$2")/scope.json" --source-domain system-symbols \
    --output "$proof/native-inventory.json" > "$proof/native-inventory.log"
  arguments+=(--system-symbols "$package")
fi
jq -n --args '$ARGS.positional' -- "$B/agirutc" "${arguments[@]}" > "$proof/command.json"
status=0
"$B/agirutc" "${arguments[@]}" > "$proof/translation.log" 2>&1 || status=$?
cat "$proof/translation.log"
sha256sum --check --status "$proof/inputs.sha256"
if [ -n "${AGIRU_SYSTEM_SYMBOLS:-}" ]; then
  python3 scripts/fetch_symbols.py --verify "$package" > "$proof/package-after.json"
  cmp "$proof/package-before.json" "$proof/package-after.json"
fi
printf '%s\n' "$status" > "$proof/status"
printf 'transpile: exit %s; %s\n' "$status" "$proof"
exit "$status"

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
arguments=("$1" "$2" "$3" --host-runtime "${AGIRU_HOST_RUNTIME-18.0}")
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
origin_arguments=(--root "$(dirname "$2")" --bc-source "$1" --generated "$3")
if [ -n "${AGIRU_SYSTEM_SYMBOLS:-}" ]; then origin_arguments+=(--symbols "$package"); fi
python3 scripts/build_sources.py record "${origin_arguments[@]}" --output "$proof/source-origins-before.json"
status=0
"$B/agirutc" "${arguments[@]}" > "$proof/translation.log" 2>&1 || status=$?
cat "$proof/translation.log"
origin_status=0
python3 scripts/build_sources.py record "${origin_arguments[@]}" --previous "$proof/source-origins-before.json" \
  > "$proof/source-origins.log" 2>&1 || origin_status=$?
cat "$proof/source-origins.log"
printf '%s\n' "$status" > "$proof/generator-status"
printf '%s\n' "$origin_status" > "$proof/origin-status"
if [ "$status" -eq 0 ] && [ "$origin_status" -ne 0 ]; then status=$origin_status; fi
sha256sum --check --status "$proof/inputs.sha256"
if [ -n "${AGIRU_SYSTEM_SYMBOLS:-}" ]; then
  python3 scripts/fetch_symbols.py --verify "$package" > "$proof/package-after.json"
  cmp "$proof/package-before.json" "$proof/package-after.json"
fi
printf '%s\n' "$status" > "$proof/status"
printf 'transpile: exit %s; %s\n' "$status" "$proof"
exit "$status"

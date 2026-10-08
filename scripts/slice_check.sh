#!/usr/bin/env bash
set -euo pipefail
root=$(realpath "${1:-$(dirname "$0")/..}")
B=${B:-$root/build}
mkdir -p "$B"
proof=$(mktemp -d /tmp/agiru-slice-check.XXXXXX)
status=0
python3 "$root/scripts/build_sources.py" project --root "$root" --slice "$root/test/slice" \
  --receipt "$proof/population.json" > "$proof/sources.tsv" || status=$?
if [ -f "$proof/population.json" ]; then
  jq -r '"slice-check: \(.raw) raw, \(.selected) selected, \(.product_excluded) product-excluded, \(.omitted) omitted, \([.sources[] | select(.decision == "selected" and .present == false)] | length) missing"' \
    "$proof/population.json"
fi
printf 'slice-check: exit %s; %s\n' "$status" "$proof"
exit "$status"

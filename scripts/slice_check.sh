#!/usr/bin/env bash
set -euo pipefail
root=$(realpath "${1:-$(dirname "$0")/..}")
B=${B:-$root/build}
mkdir -p "$B"
proof=$(mktemp -d /tmp/agiru-slice-check.XXXXXX)
python3 "$root/scripts/unity_groups.py" "$root/test/slice" > "$proof/sources.tsv"
count=0
missing=0
while IFS='|' read -r source group; do
  count=$((count + 1))
  if [ ! -f "$root/apps/$source" ]; then
    printf 'slice-check: missing %s (%s)\n' "$source" "$group" >&2
    missing=$((missing + 1))
  fi
done < "$proof/sources.tsv"
printf 'slice-check: %s sources, %s missing; %s\n' "$count" "$missing" "$proof/sources.tsv"
if [ "$count" -eq 0 ]; then
  printf 'slice-check: empty source population\n' >&2
  exit 2
fi
[ "$missing" -eq 0 ]

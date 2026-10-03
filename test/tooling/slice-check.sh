#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=${B:-build}
mkdir -p "$B"
proof=$(mktemp -d /tmp/agiru-slice-check-controls.XXXXXX)
root=$(realpath "$proof/fixture")
mkdir -p "$root/test" "$root/scripts" "$root/apps/module"
cp scripts/unity_groups.py "$root/scripts/unity_groups.py"
touch "$root/apps/module/First.cpp" "$root/apps/module/Second.cpp"
printf '# The complete source population\nmodule/First.cpp\n\nmodule/Second.cpp\n' > "$root/test/slice"
B="$root/build" bash scripts/slice_check.sh "$root" > "$proof/present.log" 2>&1
rg -q '2 sources, 0 missing' "$proof/present.log"

mv "$root/apps/module/Second.cpp" "$root/apps/module/Second.saved"
if B="$root/build" bash scripts/slice_check.sh "$root" > "$proof/missing.log" 2>&1; then
  printf 'slice-check: missing source escaped the control\n' >&2
  exit 1
fi
rg -q 'missing module/Second.cpp' "$proof/missing.log"
rg -q '2 sources, 1 missing' "$proof/missing.log"
mv "$root/apps/module/Second.saved" "$root/apps/module/Second.cpp"

printf 'module/First.cpp\nmodule/First.cpp\n' > "$root/test/slice"
if B="$root/build" bash scripts/slice_check.sh "$root" > "$proof/duplicate.log" 2>&1; then
  printf 'slice-check: duplicate source escaped the control\n' >&2
  exit 1
fi
rg -q 'duplicate source' "$proof/duplicate.log"

printf '# no executable source\n\n' > "$root/test/slice"
if B="$root/build" bash scripts/slice_check.sh "$root" > "$proof/empty.log" 2>&1; then
  printf 'slice-check: empty source population escaped the control\n' >&2
  exit 1
fi
rg -q 'empty source population' "$proof/empty.log"
printf 'slice-check: all sources counted; missing/duplicate/empty controls refused\n'

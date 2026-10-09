#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=${B:-build}
mkdir -p "$B"
proof=$(mktemp -d /tmp/agiru-slice-check-controls.XXXXXX)
root=$(realpath "$proof/fixture")
mkdir -p "$root/test" "$root/scripts" "$root/apps/module"
cp scripts/{build_sources,unity_groups,scope_inventory,ut_manifest,source_revision}.py "$root/scripts/"
touch "$root/apps/module/First.cpp" "$root/apps/module/Second.cpp"
printf '%s\n' '{"apps":[{"name":"module","source":"source"}]}' > "$root/apps.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":[]}' > "$root/scope.json"
cp "$root/apps.json" "$root/apps/generation-apps.json"
cp "$root/scope.json" "$root/apps/generation-scope.json"
printf 'module/First.cpp\nmodule/Second.cpp\n' > "$root/apps/generation-sources.txt"
jq -n '{schema:1,sources:{"module/First.cpp":{source:"source/First.Codeunit.al",source_missing:false,namespace:"Microsoft",test:false},"module/Second.cpp":{source:"source/Second.Codeunit.al",source_missing:false,namespace:"Microsoft",test:false}}}' \
  > "$root/apps/source-origins.json"
printf '# The complete source population\nmodule/First.cpp\n\nmodule/Second.cpp\n' > "$root/test/slice"
B="$root/build" bash scripts/slice_check.sh "$root" > "$proof/present.log" 2>&1
rg -q '2 raw, 2 selected, 0 product-excluded, 0 omitted, 0 missing' "$proof/present.log"

mv "$root/apps/module/Second.cpp" "$root/apps/module/Second.saved"
if B="$root/build" bash scripts/slice_check.sh "$root" > "$proof/missing.log" 2>&1; then
  printf 'slice-check: missing source escaped the control\n' >&2
  exit 1
fi
rg -q 'missing module/Second.cpp' "$proof/missing.log"
rg -q '2 raw, 2 selected, 0 product-excluded, 0 omitted, 1 missing' "$proof/missing.log"
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
rg -q 'empty selected source population' "$proof/empty.log"

printf 'module/First.cpp\nmodule/Second.cpp\n' > "$root/test/slice"
jq '.sources[].namespace="Omitted"' "$root/apps/source-origins.json" > "$proof/omitted-origins.json"
mv "$proof/omitted-origins.json" "$root/apps/source-origins.json"
printf '%s\n' '{"include":["Microsoft"],"exclude":["Omitted"],"source_include":["source/First.Codeunit.al","source/Second.Codeunit.al"],"product_exclude":["microsoft-cloud:source/Second.Codeunit.al"]}' > "$root/scope.json"
cp "$root/scope.json" "$root/apps/generation-scope.json"
printf 'module/First.cpp\n' > "$root/apps/generation-sources.txt"
B="$root/build" bash scripts/slice_check.sh "$root" > "$proof/source-include.log" 2>&1
rg -q '2 raw, 1 selected, 1 product-excluded, 0 omitted, 0 missing' "$proof/source-include.log"

printf '%s\n' '{"include":["Microsoft"],"exclude":[],"source_include":["source/../First.Codeunit.al"]}' > "$root/scope.json"
if B="$root/build" bash scripts/slice_check.sh "$root" > "$proof/traversal.log" 2>&1; then
  printf 'slice-check: invalid source activation escaped the control\n' >&2
  exit 1
fi
rg -q 'source includes need bounded relative file paths' "$proof/traversal.log"
printf 'slice-check: all sources counted; exact activation and product precedence; missing/duplicate/empty/traversal controls refused\n'

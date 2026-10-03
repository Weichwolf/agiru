#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
if [ -z "${AGIRU_SYSTEM_SYMBOLS:-}" ]; then
  printf 'native-interface-package: explicit verified AGIRU_SYSTEM_SYMBOLS is required\n' >&2
  exit 2
fi
package=$(realpath "$AGIRU_SYSTEM_SYMBOLS")
proof=$(mktemp -d /tmp/agiru-native-interface-package.XXXXXX)
printf '%s\n' "$proof" > "$B/native-interface-package.latest"
git rev-parse HEAD > "$proof/head.txt"
git status --porcelain > "$proof/source-status.txt"
rg --files src include -g '*.h' -g '*.cpp' | LC_ALL=C sort \
  | xargs -d '\n' sha256sum > "$proof/source-inputs.sha256"
sha256sum test/transpiler/native-interface-package.sh >> "$proof/source-inputs.sha256"
sha256sum "$B/agirutc" "$B/libagiru_gen.so" "$B/libagiru_al.so" > "$proof/compiler-inputs.sha256"
python3 scripts/fetch_symbols.py --verify "$package" > "$proof/provenance-before.json"
printf '%s\n' '{"apps":[{"name":"inventory","source":"src"}]}' > "$proof/inventory-apps.json"
printf '%s\n' '{"include":["System","Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
python3 scripts/scope_inventory.py "$package" --apps "$proof/inventory-apps.json" \
  --scope "$proof/scope.json" --output "$proof/raw-inventory.json" > "$proof/inventory.log"
jq -e '.summary.unmeasured_files == 0 and (.errors | length) == 0' "$proof/raw-inventory.json" > /dev/null
jq -r '.objects[] | select(.kind == "interface") | .source' \
  "$proof/raw-inventory.json" > "$proof/interfaces.txt"
[ -s "$proof/interfaces.txt" ]
[ "$(LC_ALL=C sort -u "$proof/interfaces.txt" | wc -l)" -eq "$(wc -l < "$proof/interfaces.txt")" ]
mkdir -p "$proof/source"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$proof/apps.json"
translation_status=0
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" --system-symbols "$package" \
  > "$proof/translation.log" 2>&1 || translation_status=$?
case "$translation_status" in 0|1) ;; *) exit "$translation_status" ;; esac
expected=$(wc -l < "$proof/interfaces.txt")
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude
  "-I$proof/generated/platform" "-I$proof/generated/absent" "-I$proof/generated/shared")
passed=0
failed=0
bodies=0
control_header=
while IFS= read -r source; do
  index=$((passed + failed + 1))
  status=0
  mapfile -t headers < <(rg -l -F "// Generated from $source. Do not edit." \
    "$proof/generated/platform" -g '*.h' || [ "$?" -eq 1 ])
  header=
  body_status=0
  expected_body=false
  if rg -q '^\s*begin\s*$' "$package/$source"; then expected_body=true; fi
  if [ "${#headers[@]}" -ne 1 ]; then
    status=2
  else
    header=${headers[0]}
    "$CXX" "${flags[@]}" -x c++-header -fsyntax-only "$header" \
      > "$proof/$index.header.log" 2>&1 || status=$?
    body=${header%.h}.cpp
    if [ "$expected_body" = true ]; then
      bodies=$((bodies + 1))
      if [ ! -f "$body" ]; then body_status=2; else
        "$CXX" "${flags[@]}" -fsyntax-only "$body" \
          > "$proof/$index.body.log" 2>&1 || body_status=$?
      fi
    elif [ -f "$body" ]; then body_status=2;
    fi
    if [ "$status" -eq 0 ] && [ "$body_status" -ne 0 ]; then status=$body_status; fi
    if [ "$status" -eq 0 ] && [ -z "$control_header" ] \
        && rg -q '^#include "absent/Types.h"$' "$header"; then control_header=$header; fi
  fi
  if [ "$status" -eq 0 ]; then passed=$((passed + 1)); else failed=$((failed + 1)); fi
  jq -nc --arg source "$source" --arg header "$header" --argjson status "$status" \
    --argjson expected_body "$expected_body" --argjson body_status "$body_status" \
    '{source:$source,header:$header,status:$status,expected_body:$expected_body,
      body_status:$body_status}' >> "$proof/results.jsonl"
done < "$proof/interfaces.txt"
control=unexecuted
if [ -n "$control_header" ]; then
  sed '/^#include "absent\/Types.h"$/d' "$control_header" > "$proof/missing-include.h"
  if "$CXX" "${flags[@]}" -x c++-header -fsyntax-only "$proof/missing-include.h" \
      > "$proof/missing-include.log" 2>&1; then
    printf 'native-interface-package: missing absent declaration escaped the control\n' >&2
    exit 1
  fi
  rg -q "undeclared identifier 'absent'" "$proof/missing-include.log"
  control=rejected
fi
python3 scripts/fetch_symbols.py --verify "$package" > "$proof/provenance-after.json"
cmp "$proof/provenance-before.json" "$proof/provenance-after.json"
jq -n --argjson raw "$expected" --argjson passed "$passed" --argjson failed "$failed" \
  --argjson bodies "$bodies" --argjson translation_status "$translation_status" \
  --arg control "$control" --slurpfile results "$proof/results.jsonl" \
  '{raw_interfaces:$raw,passed:$passed,failed:$failed,default_body_files:$bodies,
    translation_status:$translation_status,results:$results,business_executed:0,
    missing_include_control:$control}' > "$proof/result.json"
sha256sum --check --status "$proof/source-inputs.sha256"
sha256sum --check --status "$proof/compiler-inputs.sha256"
[ "$((passed + failed))" -eq "$expected" ]
printf 'native-interface-package: %s/%s declarations and %s body files; execution unproved; %s\n' \
  "$passed" "$expected" "$bodies" "$proof"
[ "$failed" -eq 0 ]
[ "$control" = rejected ]

#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
manifest=test/native-binding/consumers.json
validate_manifest() {
  jq -e 'length == 4 and (map(.id) | sort) == [358,594,9621,9630]
    and (map(.source) | unique | length) == 4
    and (map(.unit) | unique | length) == 4
    and all(.[]; (.tables | length) > 0 and
      (.tables | unique | length) == (.tables | length) and
      (.app == "system" or .app == "base") and
      (.source | startswith("/") | not) and
      (.source | split("/") | index("..")) == null and
      (.unit | startswith("/") | not) and
      (.unit | split("/") | index("..")) == null)' "$1" > /dev/null
}
if [ "${1:-}" = --validate ]; then
  validate_manifest "${2:-$manifest}"
  exit
fi
validate_manifest "$manifest"
if [ -z "${AGIRU_SYSTEM_SYMBOLS:-}" ] || [ -z "${AGIRU_NATIVE_AUDIT:-}" ]; then
  printf 'native-consumers: explicit AGIRU_SYSTEM_SYMBOLS and matching AGIRU_NATIVE_AUDIT are required\n' >&2
  exit 2
fi
package=$(realpath "$AGIRU_SYSTEM_SYMBOLS")
audit=$(realpath "$AGIRU_NATIVE_AUDIT")
source=$(realpath "${AGIRU_BC_SOURCE:-$HOME/Git/BCApps/src}")
python3 scripts/fetch_symbols.py --verify "$package"
sha256sum --check --status "$audit/source-inputs.sha256"
sha256sum --check --status "$audit/libraries.sha256"
sha256sum --check --status "$audit/originals.sha256"
jq -e --slurpfile provenance "$package/provenance.json" \
  '.package_sha256 == $provenance[0].package_sha256 and
   (.tables | length) == .raw_native.objects_by_kind.table' "$audit/result.json" > /dev/null
proof=$(mktemp -d "$B/native-consumers.XXXXXX")
printf '%s\n' "$proof" > "$B/native-consumers.latest"
cp "$manifest" "$proof/consumers.json"
cp apps.json scope.json "$proof/"
cp "$audit/result.json" "$proof/native-audit.json"
sha256sum test/native-consumers.sh "$manifest" apps.json scope.json "$B/agirutc" \
  scripts/ut_manifest.py scripts/scope_inventory.py "$audit/emitter" \
  "$audit/result.json" "$audit/tables.json" > "$proof/inputs.sha256"
rg --files src include -g '*.h' -g '*.cpp' | LC_ALL=C sort \
  | xargs -d '\n' sha256sum >> "$proof/inputs.sha256"
git rev-parse HEAD > "$proof/head.txt"
git status --porcelain > "$proof/source-status.txt"
git -C "$source" rev-parse main > "$proof/bcapps-main.txt"
source_hashes() {
  (cd "$1"; find . -type f -print0 | LC_ALL=C sort -z | xargs -0 sha256sum)
}
if [ -n "$(find "$source" -type l -print -quit)" ]; then
  printf 'native-consumers: source symlinks need an explicit freeze policy\n' >&2
  exit 2
fi
source_hashes "$source" > "$proof/bc-source.sha256"
cp -a "$source" "$proof/bc_source"
source_hashes "$proof/bc_source" > "$proof/bc-frozen.sha256"
cmp "$proof/bc-source.sha256" "$proof/bc-frozen.sha256"
source_hashes "$source" > "$proof/bc-source-after.sha256"
cmp "$proof/bc-source.sha256" "$proof/bc-source-after.sha256"
python3 scripts/ut_manifest.py "$proof/bc_source/Layers/W1/Tests" > "$proof/ut-manifest.json"
python3 scripts/ut_manifest.py "$source/Layers/W1/Tests" > "$proof/ut-original.json"
jq 'map(del(.source))' "$proof/ut-manifest.json" > "$proof/ut-frozen-identities.json"
jq 'map(del(.source))' "$proof/ut-original.json" > "$proof/ut-original-identities.json"
cmp "$proof/ut-original-identities.json" "$proof/ut-frozen-identities.json"
census=0
python3 scripts/scope_inventory.py "$proof/bc_source" --apps "$proof/apps.json" \
  --scope "$proof/scope.json" --output "$proof/scope-inventory.json" \
  > "$proof/census.log" 2>&1 || census=$?
jq -e '.summary.unmeasured_files == 0' "$proof/scope-inventory.json" > /dev/null
translation=0
"$B/agirutc" "$proof/bc_source" "$proof/apps.json" "$proof/generated" \
  > "$proof/translation.log" 2>&1 || translation=$?
jq '.[1:]' "$manifest" > "$proof/missing-consumer.json"
jq '. + [.[0]]' "$manifest" > "$proof/duplicate-consumer.json"
if validate_manifest "$proof/missing-consumer.json" || validate_manifest "$proof/duplicate-consumer.json"; then
  printf 'native-consumers: changed consumer denominator escaped validation\n' >&2
  exit 1
fi
flags=(-std=c++23 -stdlib=libc++ -O2 -Wall -Wextra -Wpedantic -Werror -fsyntax-only -ferror-limit=0 -Iinclude)
: > "$proof/results.jsonl"
jq -c '.[]' "$manifest" | while IFS= read -r page; do
  id=$(jq -r '.id' <<< "$page")
  app=$(jq -r '.app' <<< "$page")
  unit=$(jq -r '.unit' <<< "$page")
  original="$proof/bc_source/$(jq -r '.source' <<< "$page")"
  name=$(jq -r '.name' <<< "$page")
  if ! rg -Fxq "page $id \"$name\"" "$original" && ! rg -Fxq "page $id $name" "$original"; then
    printf 'native-consumers: original page identity differs: %s\n' "$original" >&2
    exit 2
  fi
  includes=("-I$proof/generated/shared" "-I$proof/generated/absent" "-I$proof/generated/$app")
  while IFS= read -r dependency; do
    includes+=("-I$proof/generated/$dependency")
  done < <(jq -r --arg app "$app" '.apps[] | select(.name == $app) | .depends[]' "$proof/apps.json")
  contract="$proof/$id-contract.h"
  : > "$contract"
  while IFS= read -r table; do
    native=$(jq -er --argjson id "$table" 'first(.[] | select(.id == $id) | .source)' "$audit/tables.json")
    "$audit/emitter" "$package/$native" >> "$contract" 2>> "$proof/$id.emit.log"
  done < <(jq -r '.tables[]' <<< "$page")
  sha256sum "$contract" >> "$proof/inputs.sha256"
  for suffix in cpp def.cpp; do
    generated="$proof/generated/$app/$unit.$suffix"
    for variant in production native-contract; do
      status=0
      extra=()
      if [ "$variant" = native-contract ]; then extra=(-include "$contract"); fi
      log="$proof/$id.$suffix.$variant.log"
      jq -n --args '$ARGS.positional' -- "$CXX" "${flags[@]}" "${includes[@]}" "${extra[@]}" "$generated" \
        > "$proof/$id.$suffix.$variant.command.json"
      if [ ! -s "$generated" ]; then
        outcome=missing; status=2
        printf 'missing generated consumer: %s\n' "$generated" > "$log"
      else
        "$CXX" "${flags[@]}" "${includes[@]}" "${extra[@]}" "$generated" > "$log" 2>&1 || status=$?
        outcome=compile-pass
        if [ "$status" -ge 128 ]; then outcome=crashed
        elif [ "$status" -ne 0 ]; then outcome=compile-fail; fi
        sha256sum "$generated" >> "$proof/generated-consumers.sha256"
      fi
      jq -nc --argjson id "$id" --arg unit "$app/$unit.$suffix" --arg variant "$variant" \
        --arg status "$outcome" --argjson exit "$status" \
        '{id:$id,unit:$unit,variant:$variant,status:$status,exit:$exit,business_executed:false}' >> "$proof/results.jsonl"
    done
  done
done
native=$(jq -er 'first(.[] | select(.id == 2000000041) | .source)' "$audit/tables.json")
awk '!changed && /field\([0-9]+;/ {sub(/field\([0-9]+;/,"field(99999;"); changed++}
  {print} END {if (changed != 1) exit 1}' "$package/$native" > "$proof/wrong-field.al"
"$audit/emitter" "$proof/wrong-field.al" > "$proof/wrong-field.h"
control="$proof/generated/base/system/diagnostics/page/ChangeLogSetupFieldList.def.cpp"
if "$CXX" "${flags[@]}" "-I$proof/generated/base" "-I$proof/generated/system" \
  "-I$proof/generated/foundation" "-I$proof/generated/absent" "-I$proof/generated/shared" \
  -include "$proof/wrong-field.h" "$control" > "$proof/wrong-field.log" 2>&1; then
  printf 'native-consumers: wrong original native field number escaped verification\n' >&2
  exit 1
fi
rg -q 'native field declaration mismatch' "$proof/wrong-field.log"
jq -s '.' "$proof/results.jsonl" > "$proof/results.json"
jq -e 'length == 16 and ([.[] | select(.variant == "production")] | length) == 8
  and ([.[] | select(.variant == "native-contract")] | length) == 8
  and (map([.unit,.variant]) | unique | length) == 16' "$proof/results.json" > /dev/null
sha256sum --check --status "$proof/inputs.sha256"
sha256sum --check --status "$proof/generated-consumers.sha256"
sha256sum --check --status "$audit/source-inputs.sha256"
sha256sum --check --status "$audit/libraries.sha256"
sha256sum --check --status "$audit/originals.sha256"
source_hashes "$proof/bc_source" > "$proof/bc-frozen-after.sha256"
cmp "$proof/bc-source.sha256" "$proof/bc-frozen-after.sha256"
jq -n --argjson translation "$translation" --argjson census "$census" \
  --slurpfile inventory "$proof/scope-inventory.json" --slurpfile results "$proof/results.json" \
  --slurpfile raw "$proof/native-audit.json" --slurpfile ut "$proof/ut-manifest.json" \
  '{translation_exit:$translation,census_exit:$census,raw_al_inventory:$inventory[0].summary,
    census_errors:$inventory[0].errors,results:$results[0],raw_native_audit:$raw[0],ut_manifest:$ut[0],
    denominator:8,variants:2,negative_controls:{missing_consumer:"rejected",duplicate_consumer:"rejected",wrong_field_number:"rejected"},
    native_contracts_are_additional_compile_assertions:true,source_ast_loader_activated:false,
    business_execution_proved:false,complete_app_proved:false,g1_proved:false}' > "$proof/result.json"
jq '{translation_exit,denominator,statuses:(.results | group_by([.variant,.status]) |
  map({variant:.[0].variant,status:.[0].status,count:length})),negative_controls}' "$proof/result.json"
printf 'native-consumers: %s; no loader, provider, business execution or G1 proof\n' "$proof"
jq -e '.translation_exit == 0 and all(.results[]; .status == "compile-pass")' "$proof/result.json" > /dev/null

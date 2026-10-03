#!/usr/bin/env bash
set -euo pipefail
bundle=$(realpath -e "${1:?verify_layout_assets.sh bundle}")
jq -e '.schema == 1 and .population == "named-rendering-layouts"
  and .complete and .exit_status == 0 and .unresolved == 0
  and .declared == .bound and .bound == .packaged
  and .installed == 0 and .approved == 0 and .selected == 0 and .rendered == 0' \
  "$bundle/result.json" > /dev/null
for field in requests layouts notices; do
  expected=$(jq -er --arg key "${field}_sha256" '.[$key]' "$bundle/result.json")
  [[ $(sha256sum "$bundle/$field.json" | cut -d ' ' -f 1) == "$expected" ]]
done
jq -e 'type == "array" and all(.[]; (.sha256 | test("^[a-f0-9]{64}$"))
  and .file == ("notices/" + .sha256) and (.name | type == "string" and length > 0))' \
  "$bundle/notices.json" > /dev/null
mapfile -t notices < <(jq -r '.[].file' "$bundle/notices.json")
for notice in "${notices[@]}"; do [[ -f "$bundle/$notice" && ! -L "$bundle/$notice" ]]; done
jq -e --slurpfile requests "$bundle/requests.json" --slurpfile result "$bundle/result.json" \
  'length == $result[0].packaged and length == $requests[0].summary.declared
    and (map(del(.appVersion,.sha256,.bytes,.asset,.source_sha256,.app_manifest_sha256))
      == ($requests[0].layouts | map(.file |= (gsub("\\\\";"/")
        | split("/") | map(select(. != ".")) | join("/")))))' \
  "$bundle/layouts.json" > /dev/null
for directory in assets sources manifests notices; do
  [[ -d "$bundle/$directory" && ! -L "$bundle/$directory" ]]
  for file in "$bundle/$directory"/*; do
    [[ -e "$file" || -L "$file" ]] || continue
    digest=$(basename "$file")
    [[ "$digest" =~ ^[a-f0-9]{64}$ && -f "$file" && ! -L "$file" ]]
    [[ $(sha256sum "$file" | cut -d ' ' -f 1) == "$digest" ]]
  done
done
mapfile -t layouts < <(jq -c '.[]' "$bundle/layouts.json")
for layout in "${layouts[@]}"; do
  for pair in 'assets sha256' 'sources source_sha256' 'manifests app_manifest_sha256'; do
    read -r directory field <<< "$pair"
    digest=$(jq -er --arg key "$field" '.[$key]' <<< "$layout")
    [[ "$digest" =~ ^[a-f0-9]{64}$ && -f "$bundle/$directory/$digest" \
      && ! -L "$bundle/$directory/$digest" ]]
    [[ $(sha256sum "$bundle/$directory/$digest" | cut -d ' ' -f 1) == "$digest" ]]
  done
  asset=$(jq -r '.asset' <<< "$layout")
  digest=$(jq -r '.sha256' <<< "$layout")
  [[ "$asset" == "assets/$digest" ]]
  [[ $(stat -c %s "$bundle/$asset") == "$(jq -r '.bytes' <<< "$layout")" ]]
done
printf 'layout-assets: verified %s declared assets; no installation/rendering claim\n' "${#layouts[@]}"

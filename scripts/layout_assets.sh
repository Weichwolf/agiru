#!/usr/bin/env bash
set -euo pipefail

requests=${1:?layout_assets.sh requests source output [system-symbols] [notices-json]}
source_root=$(realpath -e "${2:?source root}")
output=${3:?new output directory}
system_root=${4:-}
notice_manifest=${5:-}
repository=$(cd "$(dirname "$0")/.." && pwd)
request_hash=$(sha256sum "$requests" | cut -d ' ' -f 1)
jq -e '.schema == 1 and .population == "named-rendering-layouts"
  and (.apps | type == "array") and (.layouts | type == "array")
  and (.summary.declared == (.layouts | length))
  and (.summary.bound == ([.layouts[] | select(.report > 0)] | length))
  and (.summary.unresolved == ([.layouts[] | select(.report == 0)] | length))
  and (.summary.declared == .summary.bound + .summary.unresolved)' "$requests" > /dev/null
declared=$(jq -r '.summary.declared' "$requests")
bound=$(jq -r '.summary.bound' "$requests")
unresolved=$(jq -r '.summary.unresolved' "$requests")
parent=$(realpath -e "$(dirname "$output")")
name=$(basename "$output")
[[ "$name" != . && "$name" != .. && -n "$name" ]]
output="$parent/$name"
[[ ! -e "$output" && ! -L "$output" ]]
stage="$parent/.$name.stage"
mkdir "$stage"
mkdir "$stage/assets" "$stage/sources" "$stage/notices" "$stage/manifests"
packaged=0
error=''
receipt() {
  local status=$?
  if [[ "$status" != 0 && -z "$error" ]]; then error='packaging failed before publication'; fi
  local layouts_hash=''
  local notices_hash=''
  if [[ -f "$stage/layouts.json" ]]; then layouts_hash=$(sha256sum "$stage/layouts.json" | cut -d ' ' -f 1); fi
  if [[ -f "$stage/notices.json" ]]; then notices_hash=$(sha256sum "$stage/notices.json" | cut -d ' ' -f 1); fi
  jq -n --arg sha256 "$request_hash" --arg error "$error" --argjson status "$status" \
    --arg layouts_sha256 "$layouts_hash" \
    --arg notices_sha256 "$notices_hash" \
    --argjson declared "$declared" --argjson bound "$bound" --argjson unresolved "$unresolved" \
    --argjson packaged "$packaged" \
    '{schema:1,population:"named-rendering-layouts",requests_sha256:$sha256,
      layouts_sha256:$layouts_sha256,notices_sha256:$notices_sha256,
      declared:$declared,bound:$bound,unresolved:$unresolved,
      packaged:$packaged,complete:($status == 0),exit_status:$status,error:$error,
      installed:0,approved:0,selected:0,rendered:0}' > "$stage/result.json"
}
trap receipt EXIT
refuse() { error=$*; printf 'layout-assets: %s\n' "$error" >&2; exit 2; }
relative() {
  local value=${1//\\//}
  [[ -n "$value" && "$value" != /* && ! "$value" =~ ^[a-zA-Z]: \
    && ! "$value" =~ (^|/)\.\.(/|$) \
    && "$value" != *//* && "$value" != */ ]] || refuse "unsafe relative path: $1"
  [[ ! "$value" =~ [[:cntrl:]] ]] || refuse "control character in path"
  local normalized='' part
  local -a parts
  IFS=/ read -r -a parts <<< "$value"
  for part in "${parts[@]}"; do
    [[ "$part" != . ]] || continue
    if [[ -n "$normalized" ]]; then normalized+=/; fi
    normalized+=$part
  done
  [[ -n "$normalized" ]] || refuse "relative path contains no file"
  printf '%s' "$normalized"
}
owned_file() {
  local path=$2 part cursor=$1
  local -a parts
  IFS=/ read -r -a parts <<< "$path"
  for part in "${parts[@]}"; do
    cursor="$cursor/$part"
    [[ ! -L "$cursor" ]] || refuse "symlink in owned path: $path"
  done
  [[ -f "$cursor" ]] || refuse "missing owned file: $path"
  printf '%s' "$cursor"
}
owned_directory() {
  local path=$2 part cursor=$1
  local -a parts
  IFS=/ read -r -a parts <<< "$path"
  for part in "${parts[@]}"; do
    cursor="$cursor/$part"
    [[ ! -L "$cursor" && -d "$cursor" ]] || refuse "missing or symlink app directory: $path"
  done
  printf '%s' "$cursor"
}
copy_checked() {
  local input=$1 directory=$2 digest copy after
  digest=$(sha256sum "$input" | cut -d ' ' -f 1)
  copy="$stage/$directory/$digest"
  cp -- "$input" "$copy"
  after=$(sha256sum "$input" | cut -d ' ' -f 1)
  [[ "$after" == "$digest" && "$(sha256sum "$copy" | cut -d ' ' -f 1)" == "$digest" ]] \
    || refuse "source changed while packaging: $input"
  sha256sum "$input" >> "$stage/inputs.sha256"
  printf '%s' "$digest"
}
declare -A roots versions manifests manifest_hashes identities platforms keys
resolve_app() {
  local id=$1 app app_source platform root manifest actual key expected digest
  [[ "$id" =~ ^[[:xdigit:]]{8}-[[:xdigit:]]{4}-[[:xdigit:]]{4}-[[:xdigit:]]{4}-[[:xdigit:]]{12}$ ]] \
    || refuse "invalid declaring app identity: $id"
  [[ ${roots[$id]+present} != present ]] || return 0
  [[ $(jq --arg id "$id" '[.apps[] | select(.id == $id)] | length' "$requests") == 1 ]] \
    || refuse "missing or ambiguous declaring app: $id"
  app=$(jq -c --arg id "$id" '.apps[] | select(.id == $id)' "$requests")
  app_source=$(jq -r '.source' <<< "$app")
  if [[ "$app_source" != . ]]; then app_source=$(relative "$app_source"); fi
  platform=$(jq -r '.platform' <<< "$app")
  if [[ "$platform" == true ]]; then
    [[ -n "$system_root" && "$app_source" == platform ]] || refuse "native package root required"
    root=$(realpath -e "$system_root")
    python3 "$repository/scripts/fetch_symbols.py" --verify "$root" > "$stage/native-provenance.log"
    manifest=$(owned_file "$root" provenance.json)
    actual=$(jq -c '.identity | {id:.Id,name:.Name,publisher:.Publisher,version:.Version}' "$manifest")
    root=$(owned_directory "$root" layout)
  elif [[ "$platform" == false ]]; then
    root=$source_root
    if [[ "$app_source" != . ]]; then root=$(owned_directory "$source_root" "$app_source"); fi
    manifest=$(owned_file "$root" app.json)
    actual=$(jq -c '{id,name,publisher,version}' "$manifest")
  else
    refuse "invalid app platform flag: $id"
  fi
  for key in id name publisher version; do
    expected=$(jq -er --arg key "$key" '.[$key] | select(type == "string" and length > 0)' <<< "$app")
    [[ $(jq -er --arg key "$key" '.[$key]' <<< "$actual") == "$expected" ]] \
      || refuse "declaring app $key mismatch: $id"
  done
  digest=$(copy_checked "$manifest" manifests)
  roots[$id]=$root
  versions[$id]=$(jq -r '.version' <<< "$actual")
  manifests[$id]=$manifest
  manifest_hashes[$id]=$digest
  identities[$id]=$app_source
  platforms[$id]=$platform
}
[[ "$unresolved" == 0 ]] || refuse "$unresolved unresolved layout targets of $declared declarations"
mapfile -t layouts < <(jq -c '.layouts[]' "$requests")
for layout in "${layouts[@]}"; do
  jq -e '.report > 0 and (.report | floor == .) and (.name | type == "string" and length > 0)
    and (.owner.id > 0) and (.owner.id | floor == .)
    and (.owner.extension | type == "boolean")
    and (.owner.extension or (.owner.id == .report))' <<< "$layout" > /dev/null \
    || refuse "invalid layout identity"
  id=$(jq -r '.owner.appId' <<< "$layout")
  resolve_app "$id"
  key=$(jq -c '[.report,(.name | ascii_downcase)]' <<< "$layout")
  [[ ${keys[$key]+present} != present ]] || refuse "duplicate report layout: $key"
  keys[$key]=1
  file=$(relative "$(jq -r '.file' <<< "$layout")")
  original=$(owned_file "${roots[$id]}" "$file")
  declaring=$(relative "$(jq -r '.owner.source' <<< "$layout")")
  prefix="${identities[$id]}/"
  if [[ "${identities[$id]}" == . ]]; then prefix=''; fi
  [[ "$declaring" == "$prefix"* ]] || refuse "source belongs to another app: $declaring"
  declaring=${declaring#"$prefix"}
  if [[ "${platforms[$id]}" == true ]]; then
    declaring=$(owned_file "${roots[$id]%/layout}" "$declaring")
  else
    declaring=$(owned_file "${roots[$id]}" "$declaring")
  fi
  jq -e '.type | ascii_downcase | . == "word" or . == "rdlc" or . == "excel" or . == "custom"' \
    <<< "$layout" > /dev/null || refuse "unsupported declared layout type"
  content_hash=$(copy_checked "$original" assets)
  source_hash=$(copy_checked "$declaring" sources)
  size=$(stat -c %s "$stage/assets/$content_hash")
  jq -c --arg file "$file" --arg version "${versions[$id]}" --arg sha256 "$content_hash" \
    --arg source_sha256 "$source_hash" --arg manifest_sha256 "${manifest_hashes[$id]}" \
    --argjson bytes "$size" \
    '. + {file:$file,appVersion:$version,sha256:$sha256,bytes:$bytes,
      asset:("assets/"+$sha256),source_sha256:$source_sha256,
      app_manifest_sha256:$manifest_sha256}' <<< "$layout" >> "$stage/layouts.jsonl"
  packaged=$((packaged + 1))
done
for id in "${!manifests[@]}"; do
  [[ $(sha256sum "${manifests[$id]}" | cut -d ' ' -f 1) == "${manifest_hashes[$id]}" ]] \
    || refuse "app manifest changed while packaging: $id"
done
copy_notice() {
  local file=$1 digest
  [[ -f "$file" && ! -L "$file" ]] || refuse "missing or symlink notice: $file"
  digest=$(copy_checked "$file" notices)
  jq -cn --arg source "$file" --arg name "$(basename "$file")" --arg sha256 "$digest" \
    '{source:$source,name:$name,sha256:$sha256,file:("notices/"+$sha256)}' >> "$stage/notices.jsonl"
}
notice_roots=("$source_root/..")
for id in "${!roots[@]}"; do
  notice_root=${roots[$id]}
  if [[ "${platforms[$id]}" == true ]]; then notice_root=${notice_root%/layout}; fi
  notice_roots+=("$notice_root")
done
for notice_root in "${notice_roots[@]}"; do
  for notice in LICENSE LICENSE.txt LICENSE.md License.txt license.txt NOTICE NOTICE.txt NOTICE.md; do
    if [[ -e "$notice_root/$notice" || -L "$notice_root/$notice" ]]; then
      copy_notice "$notice_root/$notice"
    fi
  done
done
if [[ -n "$notice_manifest" ]]; then
  jq -e 'type == "array" and all(.[]; type == "string" and length > 0
    and (test("[[:cntrl:]]") | not))' "$notice_manifest" > /dev/null
  notice_hash=$(sha256sum "$notice_manifest" | cut -d ' ' -f 1)
  mapfile -t explicit_notices < <(jq -r '.[]' "$notice_manifest")
  for notice in "${explicit_notices[@]}"; do copy_notice "$notice"; done
  [[ $(sha256sum "$notice_manifest" | cut -d ' ' -f 1) == "$notice_hash" ]] \
    || refuse "notice manifest changed while packaging"
fi
if [[ -f "$stage/notices.jsonl" ]]; then jq -s . "$stage/notices.jsonl" > "$stage/notices.json";
else printf '%s\n' '[]' > "$stage/notices.json"; fi
[[ $(sha256sum "$requests" | cut -d ' ' -f 1) == "$request_hash" ]] \
  || refuse "request manifest changed while packaging"
if [[ -f "$stage/inputs.sha256" ]]; then
  sha256sum -c --status "$stage/inputs.sha256" || refuse "owned input changed during packaging"
fi
[[ "$packaged" == "$declared" ]] || refuse "layout population lost while packaging"
if [[ "$packaged" == 0 ]]; then printf '%s\n' '[]' > "$stage/layouts.json";
else jq -s . "$stage/layouts.jsonl" > "$stage/layouts.json"; fi
cp -- "$requests" "$stage/requests.json"
[[ $(sha256sum "$stage/requests.json" | cut -d ' ' -f 1) == "$request_hash" ]] \
  || refuse "copied request manifest changed"
receipt
bash "$repository/scripts/verify_layout_assets.sh" "$stage" > "$stage/verification.log"
mv -nT -- "$stage" "$output"
[[ ! -d "$stage" ]] || refuse "output appeared before publication"
trap - EXIT
printf 'layout-assets: %s/%s packaged, 0 installed/approved/selected/rendered: %s\n' \
  "$packaged" "$declared" "$output"

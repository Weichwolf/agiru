#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
source_root=${AGIRU_BC_SOURCE:-$HOME/Git/BCApps/src}
base="$source_root/Layers/W1/BaseApp"
source_notice=${AGIRU_LAYOUT_SOURCE_NOTICE:-"$(dirname "$source_root")/LICENSE"}
[[ -f "$source_notice" ]]
proof=$(mktemp -d /tmp/agiru-layout-assets-check.XXXXXX)
printf '%s\n' "$proof" > "$B/layout-assets-check.latest"
git rev-parse HEAD > "$proof/head.txt"
sha256sum scripts/{layout_assets,verify_layout_assets}.sh src/gen/ReportAssets.{h,cpp} \
  src/tc/Main.cpp test/reporting/layout-assets.sh test/gate/ReportAssetGate.cpp \
  "$B/agirutc" "$B/libagiru_gen.so" > "$proof/compiler-inputs.sha256"
"$B/gate_ReportLayoutGate" "$base" "$proof/input" > "$proof/declarations.log"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' \
  > "$proof/input/scope.json"
"$B/agirutc" "$proof/input" "$proof/input/apps.json" "$proof/unresolved" \
  > "$proof/unresolved-generation.log" 2>&1
jq -e '.summary == {declared:4,bound:3,unresolved:1}
  and (.layouts | map(select(.report == 0)) | length == 1)' \
  "$proof/unresolved/layout-assets.json" > /dev/null
if bash scripts/layout_assets.sh "$proof/unresolved/layout-assets.json" "$proof/input" \
  "$proof/unresolved-bundle" > "$proof/unresolved.log" 2>&1; then
  printf 'layout-assets: an unresolved layout escaped packaging\n' >&2; exit 1
fi
[[ ! -e "$proof/unresolved-bundle" ]]
jq -e '.complete == false and .declared == 4 and .unresolved == 1 and .packaged == 0' \
  "$proof/.unresolved-bundle.stage/result.json" > /dev/null
mv "$proof/input/Addon/UnresolvedLayout.ReportExt.al" \
  "$proof/input/Addon/UnresolvedLayout.ReportExt.al.absent"
mkdir "$proof/input/Fixture/Layouts" "$proof/input/Addon/Layouts"
mapfile -t word < <(rg --files "$base" -g '*.docx' | LC_ALL=C sort)
mapfile -t excel < <(rg --files "$base" -g '*.xlsx' | LC_ALL=C sort)
[[ "${#word[@]}" -gt 0 && "${#excel[@]}" -gt 0 ]]
cp "${word[0]}" "$proof/input/Fixture/Layouts/Original.docx"
cp "$base/Foundation/Reporting/ReportParts/ReportTheme/Default.dotx" \
  "$proof/input/Addon/Layouts/Theme.dotx"
cp "${excel[0]}" "$proof/input/Addon/Layouts/Spreadsheet.xlsx"
[[ $(rg -F -c "LayoutFile = 'Layouts\\Original.docx';" "$proof/input/Fixture/LayoutContract.Report.al") == 1 ]]
sed "s@LayoutFile = 'Layouts\\\\Original.docx';@LayoutFile = './Layouts/./Original.docx';@" \
  "$proof/input/Fixture/LayoutContract.Report.al" > "$proof/input/Fixture/Changed.Report.al.tmp"
mv "$proof/input/Fixture/Changed.Report.al.tmp" "$proof/input/Fixture/LayoutContract.Report.al"
"$B/agirutc" "$proof/input" "$proof/input/apps.json" "$proof/generated" \
  > "$proof/generation.log" 2>&1
manifest="$proof/generated/layout-assets.json"
mkdir "$proof/write-failure" "$proof/write-failure/layout-assets.json"
if "$B/agirutc" "$proof/input" "$proof/input/apps.json" "$proof/write-failure" \
  > "$proof/write-failure.log" 2>&1; then
  printf 'layout-assets: an unwritable manifest escaped the generator\n' >&2; exit 1
fi
rg -q 'cannot write generated output: .*/layout-assets.json' "$proof/write-failure.log"
jq -e '.summary == {declared:3,bound:3,unresolved:0}
  and ([.layouts[] | select(.owner.extension) | .owner.appId] | unique
    == ["98765432-1234-5678-9012-123456789012"])' "$manifest" > /dev/null
jq -n --arg notice "$PWD/LICENSE" --arg source_notice "$source_notice" \
  '[$notice,$source_notice]' > "$proof/notices.json"
bash scripts/layout_assets.sh "$manifest" "$proof/input" "$proof/bundle" '' "$proof/notices.json" \
  > "$proof/package.log" 2>&1
bash scripts/verify_layout_assets.sh "$proof/bundle" > "$proof/verify.log" 2>&1
notice_copy=$(jq -er '.[0].file' "$proof/bundle/notices.json")
cmp LICENSE "$proof/bundle/$notice_copy"
source_notice_copy=$(jq -er '.[1].file' "$proof/bundle/notices.json")
cmp "$source_notice" "$proof/bundle/$source_notice_copy"
cp -a "$proof/input/Fixture" "$proof/root-input"
cp "$proof/input/scope.json" "$proof/root-input/scope.json"
jq '.apps |= map(select(.source == "Fixture") | .source = ".")
  | .layouts |= map(select(.owner.extension == false) | .owner.source |= ltrimstr("Fixture/"))
  | .summary = {declared:1,bound:1,unresolved:0}' "$manifest" > "$proof/root-requests.json"
bash scripts/layout_assets.sh "$proof/root-requests.json" \
  "$proof/root-input" "$proof/root-bundle" > "$proof/root-package.log" 2>&1
mkdir "$proof/nested-input" "$proof/nested-input/aggregate"
cp -a "$proof/input/Fixture" "$proof/nested-input/aggregate/"
cp -a "$proof/input/Addon" "$proof/nested-input/aggregate/"
cp "$proof/input/scope.json" "$proof/nested-input/scope.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"aggregate"}]}' > "$proof/nested-input/apps.json"
"$B/agirutc" "$proof/nested-input" "$proof/nested-input/apps.json" "$proof/nested-generated" \
  > "$proof/nested-generation.log" 2>&1
jq -e '.summary == {declared:3,bound:3,unresolved:0}
  and ([.layouts[].owner.appId] | unique | length == 2)
  and ([.apps[] | select(.id != "") | .source] | sort
    == ["aggregate/Addon","aggregate/Fixture"])' \
  "$proof/nested-generated/layout-assets.json" > /dev/null
bash scripts/layout_assets.sh "$proof/nested-generated/layout-assets.json" \
  "$proof/nested-input" "$proof/nested-bundle" > "$proof/nested-package.log" 2>&1
jq -e '.complete and .packaged == 3 and .installed == 0 and .rendered == 0' \
  "$proof/bundle/result.json" > /dev/null
mapfile -t layouts < <(jq -c '.[]' "$proof/bundle/layouts.json")
for layout in "${layouts[@]}"; do
  owner=$(jq -r '.owner.appId' <<< "$layout")
  app=Addon
  if [[ "$owner" == 12345678-1234-5678-9012-123456789012 ]]; then app=Fixture; fi
  file=$(jq -r '.file' <<< "$layout")
  asset=$(jq -r '.asset' <<< "$layout")
  cmp "$proof/input/$app/$file" "$proof/bundle/$asset"
done
controls=(wrong-owner wrong-version traversal absolute missing symlink directory-symlink duplicate summary)
for control in "${controls[@]}"; do
  input="$proof/$control-input"
  cp -a "$proof/input" "$input"
  request="$proof/$control.json"
  filter='.'
  case "$control" in
    wrong-owner) filter='.layouts[1].owner.appId = .layouts[0].owner.appId';;
    wrong-version) filter='.apps[0].version = "999.0.0.0"';;
    traversal) filter='.layouts[0].file = "../outside.docx"';;
    absolute) filter='.layouts[0].file = "/tmp/outside.docx"';;
    missing) mv "$input/Fixture/Layouts/Original.docx" "$input/Fixture/Layouts/Original.docx.absent";;
    symlink)
      mv "$input/Fixture/Layouts/Original.docx" "$input/Fixture/Layouts/Original.docx.absent"
      ln -s Original.docx.absent "$input/Fixture/Layouts/Original.docx";;
    directory-symlink)
      mv "$input/Fixture/Layouts" "$input/Fixture/OriginalLayouts"
      ln -s OriginalLayouts "$input/Fixture/Layouts";;
    duplicate)
      filter='.layouts += [.layouts[0]] | .summary.declared += 1 | .summary.bound += 1';;
    summary) filter='.summary.declared -= 1';;
  esac
  jq "$filter" "$manifest" > "$request"
  if bash scripts/layout_assets.sh "$request" "$input" "$proof/$control-bundle" \
    > "$proof/$control.log" 2>&1; then
    printf 'layout-assets: %s escaped packaging\n' "$control" >&2; exit 1
  fi
  [[ ! -e "$proof/$control-bundle" ]]
done
for control in modified-bytes modified-metadata modified-request lost-asset lost-notice; do
  bundle="$proof/$control"
  cp -a "$proof/bundle" "$bundle"
  asset=$(jq -r '.[0].asset' "$bundle/layouts.json")
  case "$control" in
    modified-bytes) printf 'changed' >> "$bundle/$asset";;
    modified-metadata)
      jq '.[0].owner.appId = "foreign"' "$bundle/layouts.json" > "$bundle/changed.json"
      mv "$bundle/changed.json" "$bundle/layouts.json";;
    modified-request)
      jq '.layouts = []' "$bundle/requests.json" > "$bundle/changed.json"
      mv "$bundle/changed.json" "$bundle/requests.json";;
    lost-asset) mv "$bundle/$asset" "$bundle/$asset.absent";;
    lost-notice) mv "$bundle/$notice_copy" "$bundle/$notice_copy.absent";;
  esac
  if bash scripts/verify_layout_assets.sh "$bundle" > "$proof/$control.log" 2>&1; then
    printf 'layout-assets: %s escaped verification\n' "$control" >&2; exit 1
  fi
done
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -Iinclude -Itest/gate -Isrc/gen -Isrc/al)
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_gen -lagiru_al)
for control in lost-declaration broken-json; do
  mutant="$proof/$control.cpp"
  if [[ "$control" == lost-declaration ]]; then
    [[ $(rg -F -c 'std::to_string(requests.layouts.size())' src/gen/ReportAssets.cpp) == 1 ]]
    sed 's/std::to_string(requests.layouts.size())/std::to_string(requests.layouts.size() - 1)/' \
      src/gen/ReportAssets.cpp > "$mutant"
  else
    [[ $(rg -F -c 'kJsonControlLimit = 0x20' src/gen/ReportAssets.cpp) == 1 ]]
    sed 's/kJsonControlLimit = 0x20/kJsonControlLimit = 0x09/' src/gen/ReportAssets.cpp > "$mutant"
  fi
  "$CXX" "${flags[@]}" test/gate/ReportAssetGate.cpp "$mutant" "${links[@]}" \
    -o "$proof/$control-gate"
  if "$proof/$control-gate" > "$proof/$control.log" 2>&1; then
    printf 'layout-assets: %s escaped the writer gate\n' "$control" >&2; exit 1
  fi
  rg -q '[1-9][0-9]* red' "$proof/$control.log"
done
if bash scripts/layout_assets.sh "$manifest" "$proof/input" "$proof/bundle" \
  > "$proof/existing-output.log" 2>&1; then
  printf 'layout-assets: existing bundle was overwritten\n' >&2; exit 1
fi
bash scripts/verify_layout_assets.sh "$proof/bundle" > "$proof/verify-after-controls.log" 2>&1
sha256sum -c --status "$proof/compiler-inputs.sha256"
jq -n --slurpfile package "$proof/bundle/result.json" \
  '{fixture_layouts:3,unresolved_fixture_layouts:1,package:$package[0],
    packaging_controls:10,verification_controls:5,writer_controls:2,
    generator_write_failure_control:"rejected",nested_apps_proved:true,
    portable_dot_paths_proved:true,source_root_asset_packaging_proved:true,notices_preserved:true,
    overwrite_control:"rejected",
    full_ERP_scope_proof:false}' > "$proof/result.json"
printf 'layout-assets: all 3 authored-fixture assets retained; unresolved/ownership/path/hash controls reject; %s\n' "$proof"

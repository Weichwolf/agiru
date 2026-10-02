#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
if [ -z "${AGIRU_SYSTEM_SYMBOLS:-}" ]; then
  printf 'native-report-layouts: explicit AGIRU_SYSTEM_SYMBOLS is required; no inferred package\n' >&2
  exit 2
fi
package=$(realpath "$AGIRU_SYSTEM_SYMBOLS")
python3 scripts/fetch_symbols.py --verify "$package"
base="${AGIRU_BC_SOURCE:-$HOME/Git/BCApps/src}/Layers/W1/BaseApp"
proof=$(mktemp -d "$B/native-report-layouts.XXXXXX")
printf '%s\n' "$proof" > "$B/native-report-layouts.latest"
jq '{apps:[{name:"native",source:"src"}]}' "$package/provenance.json" > "$proof/inventory-apps.json"
printf '%s\n' '{"include":["System","Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
python3 scripts/scope_inventory.py "$package" --apps "$proof/inventory-apps.json" \
  --scope "$proof/scope.json" --output "$proof/raw-inventory.json" > "$proof/raw-inventory.log"
jq -e '.summary.unmeasured_files == 0 and (.errors | length) == 0' "$proof/raw-inventory.json" > /dev/null
jq -r '.objects[] | select(.kind == "report") | .source' "$proof/raw-inventory.json" > "$proof/reports"
mapfile -t reports < "$proof/reports"
if [ "${#reports[@]}" != 1 ] || [ "${reports[0]}" != src/Reports/TenantReportDefaults.Report.al ]; then
  printf 'native-report-layouts: expected the source-backed native report; inspect full raw inventory\n' >&2
  exit 2
fi
input="$proof/input"
mkdir -p "$input/native/src/Reports" "$input/base/Foundation/Reporting"
cp "$package/${reports[0]}" "$input/native/${reports[0]}"
cp "$base/Foundation/Reporting/CompositeLayout.ReportExt.al" "$input/base/Foundation/Reporting/"
jq '.identity | {id:.Id,name:.Name,publisher:.Publisher,version:.Version,runtime:.Runtime}' \
  "$package/provenance.json" > "$input/native/app.json"
cp "$base/app.json" "$input/base/app.json"
cp "$proof/scope.json" "$input/scope.json"
printf '%s\n' '{"apps":[{"name":"native","source":"native"},{"name":"base","source":"base","depends":["native"]}]}' > "$input/apps.json"
native_id=$(jq -er '.id' "$input/native/app.json")
base_id=$(jq -er '.id' "$input/base/app.json")
[ "$native_id" != "$base_id" ]
sha256sum "$package/System.app" "$package/NavxManifest.xml" "$package/SymbolReference.json" \
  "$package/layout/LayoutManifest.xml" "$base/app.json" \
  "$base/Foundation/Reporting/CompositeLayout.ReportExt.al" > "$proof/originals.sha256"
rg --files --no-ignore "$package/layout" "$base/Foundation/Reporting/ReportParts" \
  | LC_ALL=C sort | xargs -d '\n' sha256sum >> "$proof/originals.sha256"
"$B/agirutc" "$input" "$input/apps.json" "$proof/generated" > "$proof/generation.log" 2>&1
rg -q 'layouts +2 report / 14 extension declarations; 16 bound layouts, 16 immutable declarations emitted, 0 retained on unresolved targets' "$proof/generation.log"
"$B/agirutc" "$input" "$input/apps.json" > "$proof/source-only.log" 2>&1
rg -q '16 bound layouts, 0 immutable declarations emitted, 0 retained on unresolved targets' "$proof/source-only.log"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -Iinclude -Itest/gate -Isrc/al "-I$proof/generated/native" "-I$proof/generated/shared")
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db -lagiru_al)
compile() {
  local generated=$1 output=$2
  local sources
  rg --files --no-ignore "$generated" -g '*.cpp' | LC_ALL=C sort > "$output.sources"
  mapfile -t sources < "$output.sources"
  [ "${#sources[@]}" -gt 0 ]
  "$CXX" "${flags[@]}" -c test/report-layouts/NativeRunner.cpp -o "$output.runner.o"
  "$CXX" "${flags[@]}" "$output.runner.o" "${sources[@]}" "${links[@]}" -o "$output"
}
compile "$proof/generated" "$proof/runner"
"$proof/runner" "$package" "$base" "$native_id" "$base_id" > "$proof/runner.log"
cat "$proof/runner.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/report-layouts/NativeRunner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" -c test/report-layouts/NativeRunner.cpp -o "$proof/runner.runner.o" \
  > "$B/fixture-commands/native-report-layouts.json"

definitions="$proof/generated/native/system/administration/reports/report/TenantReportDefaults.def.cpp"
for control in wrong-native-owner wrong-extension-owner dropped-property; do
  mutant="$proof/$control"
  cp -a "$proof/generated" "$mutant"
  changed="$mutant/native/system/administration/reports/report/TenantReportDefaults.def.cpp"
  case "$control" in
    wrong-native-owner)
      [ "$(rg -F -o "$native_id" "$definitions" | wc -l)" = 2 ]
      sed "s/$native_id/$base_id/g" "$definitions" > "$changed";;
    wrong-extension-owner)
      [ "$(rg -F -o "$base_id" "$definitions" | wc -l)" = 14 ]
      sed "s/$base_id/$native_id/g" "$definitions" > "$changed";;
    dropped-property)
      [ "$(rg -F -c '.name = "SubType", .text = "Theme"' "$definitions")" = 1 ]
      awk '
        /^constexpr ::agiru::ReportLayoutTokenDef kTenantReportDefaultsReportLayouts1Tokens1\[\]\{/ {
          if (++tokens != 1) exit 1
          dropping=1; next
        }
        dropping { if ($0 == "};") dropping=0; next }
        /[.]name = "SubType", [.]text = "Theme"/ { properties++; next }
        { print }
        END { if (tokens != 1 || properties != 1 || dropping) exit 1 }
      ' "$definitions" > "$changed";;
  esac
  compile "$mutant" "$proof/$control-runner"
  if "$proof/$control-runner" "$package" "$base" "$native_id" "$base_id" > "$proof/$control.log" 2>&1; then
    printf 'native-report-layouts: %s escaped the original source comparison\n' "$control" >&2
    exit 1
  fi
  rg -q 'original declaring app identity survives|complete original property population survives' "$proof/$control.log"
done
cp -a "$input" "$proof/unbound-input"
mv "$proof/unbound-input/native/src/Reports/TenantReportDefaults.Report.al" \
  "$proof/unbound-input/native/src/Reports/TenantReportDefaults.Report.al.absent"
"$B/agirutc" "$proof/unbound-input" "$proof/unbound-input/apps.json" "$proof/unbound" \
  > "$proof/unbound.log" 2>&1
rg -q '0 bound layouts, 0 immutable declarations emitted, 14 retained on unresolved targets' "$proof/unbound.log"
cp -a "$package" "$proof/missing-asset"
mv "$proof/missing-asset/layout/Reports/Layouts/StandardTheme.dotx" \
  "$proof/missing-asset/layout/Reports/Layouts/StandardTheme.dotx.absent"
if "$proof/runner" "$proof/missing-asset" "$base" "$native_id" "$base_id" > "$proof/missing-asset.log" 2>&1; then
  printf 'native-report-layouts: a missing native asset escaped source comparison\n' >&2
  exit 1
fi
rg -q 'original owned asset exists' "$proof/missing-asset.log"
sha256sum --check --status "$proof/originals.sha256"
python3 scripts/fetch_symbols.py --verify "$package" > "$proof/package-verified.json"
jq -n --slurpfile raw "$proof/raw-inventory.json" --slurpfile package "$package/provenance.json" \
  --slurpfile symbols "$package/SymbolReference.json" --slurpfile base "$base/app.json" \
  --arg demo_version "$(tr -d '\n' < BC_VERSION)" \
  --arg base_revision "$(git -C "$base" rev-parse HEAD)" \
  '{package:$package[0].identity,package_sha256:$package[0].package_sha256,platform_version:$package[0].bc_version,
    demo_version:$demo_version,base_revision:$base_revision,base_version:$base[0].version,
    symbol_reports:($symbols[0].Reports | length),raw_native:$raw[0].summary,
    selected_reports:1,selected_extensions:1,
    native_layouts:2,extension_layouts:14,compiled_layouts:16,
    native_objects_outside_compiled_fixture:($raw[0].summary.objects-1),
    unexecuted_native_objects:$raw[0].summary.objects,installed_assets:0,rendered_documents:0,
    production_native_loader_activated:false,complete_app_proof:false}' > "$proof/result.json"
printf 'native-report-layouts: original native source and fourteen extension layouts compile; ownership/property controls fail; no installation/rendering or G1 claim; %s\n' "$proof"

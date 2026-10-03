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
source_root=${AGIRU_BC_SOURCE:-$HOME/Git/BCApps/src}
base="$source_root/Layers/W1/BaseApp"
notice=${AGIRU_LAYOUT_SOURCE_NOTICE:-"$(dirname "$source_root")/LICENSE"}
[[ -f "$notice" ]]
proof=$(mktemp -d "$B/native-report-layouts.XXXXXX")
printf '%s\n' "$proof" > "$B/native-report-layouts.latest"
git rev-parse HEAD > "$proof/head.txt"
rg --files src/gen src/tc include cmake test/report-layouts test/native-report-layouts.sh \
  scripts/unlinked.py scripts/layout_assets.sh scripts/verify_layout_assets.sh CMakeLists.txt Makefile \
  | LC_ALL=C sort | xargs -d '\n' sha256sum > "$proof/compiler-inputs.sha256"
sha256sum "$B/agirutc" "$B/libagiru_gen.so" "$B/libagiru_al.so" \
  "$B/libagiru_rt.so" "$B/libagiru_net.so" "$B/libagiru_db.so" >> "$proof/compiler-inputs.sha256"
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
sha256sum "$notice" >> "$proof/originals.sha256"
jq -n --arg notice "$notice" '[$notice]' > "$proof/notices.json"
rg --files --no-ignore "$package/layout" "$base/Foundation/Reporting/ReportParts" \
  | LC_ALL=C sort | xargs -d '\n' sha256sum >> "$proof/originals.sha256"
"$B/agirutc" "$input" "$input/apps.json" "$proof/generated" > "$proof/generation.log" 2>&1
rg -q 'layouts +2 report / 14 extension declarations; 16 bound layouts, 16 immutable declarations emitted, 0 retained on unresolved targets' "$proof/generation.log"
"$B/agirutc" "$input" "$input/apps.json" > "$proof/source-only.log" 2>&1
rg -q '16 bound layouts, 0 immutable declarations emitted, 0 retained on unresolved targets' "$proof/source-only.log"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -Iinclude -Itest/gate -Isrc/al)
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db -lagiru_al)
compile() {
  local generated=$1 output=$2
  local sources
  rg --files --no-ignore "$generated" -g '*.cpp' | LC_ALL=C sort > "$output.sources"
  mapfile -t sources < "$output.sources"
  [ "${#sources[@]}" -gt 0 ]
  local includes=("-I$generated/native" "-I$generated/platform" "-I$generated/base" "-I$generated/shared")
  "$CXX" "${flags[@]}" "${includes[@]}" -c test/report-layouts/NativeRunner.cpp -o "$output.runner.o"
  "$CXX" "${flags[@]}" "${includes[@]}" "$output.runner.o" "${sources[@]}" "${links[@]}" -o "$output"
}
compile "$proof/generated" "$proof/runner"
"$proof/runner" "$package" "$base" "$native_id" "$base_id" > "$proof/runner.log"
cat "$proof/runner.log"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/report-layouts/NativeRunner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" "-I$proof/generated/native" "-I$proof/generated/shared" \
  -c test/report-layouts/NativeRunner.cpp -o "$proof/runner.runner.o" \
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

source_input="$proof/source-bound-input"
mkdir -p "$source_input/base/Foundation/Reporting"
cp "$input/base/app.json" "$source_input/base/app.json"
cp "$input/base/Foundation/Reporting/CompositeLayout.ReportExt.al" \
  "$source_input/base/Foundation/Reporting/"
cp -a "$base/Foundation/Reporting/ReportParts" "$source_input/base/Foundation/Reporting/"
cp test/report-layouts/LinkConsumer.Report.al "$source_input/base/"
cp "$input/scope.json" "$source_input/scope.json"
printf '%s\n' '{"apps":[{"name":"base","source":"base"}]}' > "$source_input/apps.json"
loader_status=0
"$B/agirutc" "$source_input" "$source_input/apps.json" "$proof/source-bound" \
  --system-symbols "$package" > "$proof/source-bound.log" 2>&1 || loader_status=$?
printf '%s\n' "$loader_status" > "$proof/source-bound.status"
[ "$loader_status" = 1 ]
rg -q 'native 1 report sources bound; 3 objects written into the platform app' "$proof/source-bound.log"
rg -q '16 bound layouts, 16 immutable declarations emitted, 0 retained on unresolved targets' \
  "$proof/source-bound.log"
AGIRU_BC_SOURCE="$source_input" AGIRU_SYSTEM_SYMBOLS="$package" \
  make --no-print-directory layout-assets REQUESTS="$proof/source-bound/layout-assets.json" \
    OUTPUT="$proof/layout-bundle" NOTICES="$proof/notices.json" > "$proof/layout-package.log" 2>&1
bash scripts/verify_layout_assets.sh "$proof/layout-bundle" > "$proof/layout-verify.log" 2>&1
jq -e '.complete and .declared == 16 and .packaged == 16 and .unresolved == 0
  and .installed == 0 and .rendered == 0' "$proof/layout-bundle/result.json" > /dev/null
notice_copy=$(jq -er '.[0].file' "$proof/layout-bundle/notices.json")
cmp "$notice" "$proof/layout-bundle/$notice_copy"
mapfile -t owned_layouts < <(jq -c '.[]' "$proof/layout-bundle/layouts.json")
for layout in "${owned_layouts[@]}"; do
  original_root="$base"
  if [[ $(jq -r '.owner.appId' <<< "$layout") == "$native_id" ]]; then original_root="$package/layout"; fi
  original_file=$(jq -r '.file' <<< "$layout")
  packaged_file=$(jq -r '.asset' <<< "$layout")
  cmp "$original_root/$original_file" "$proof/layout-bundle/$packaged_file"
done
compile "$proof/source-bound" "$proof/source-bound-runner"
"$proof/source-bound-runner" "$package" "$base" "$native_id" "$base_id" \
  platform/src/Reports/TenantReportDefaults.Report.al > "$proof/source-bound-runner.log"
cat "$proof/source-bound-runner.log"
rg -F -q "$native_id" "$proof/source-bound/platform/PlatformModule.h"
loader_source_status=0
"$B/agirutc" "$source_input" "$source_input/apps.json" --system-symbols "$package" \
  > "$proof/source-bound-source-only.log" 2>&1 || loader_source_status=$?
[ "$loader_source_status" = 1 ]
rg -q '16 bound layouts, 0 immutable declarations emitted, 0 retained on unresolved targets' \
  "$proof/source-bound-source-only.log"
for control in wrong-native-owner wrong-extension-owner dropped-property; do
  mutant="$proof/source-bound-$control"
  cp -a "$proof/source-bound" "$mutant"
  changed="$mutant/platform/system/administration/reports/report/TenantReportDefaults.def.cpp"
  case "$control" in
    wrong-native-owner)
      [ "$(rg -F -o "$native_id" "$changed" | wc -l)" = 2 ]
      sed -i "s/$native_id/$base_id/g" "$changed";;
    wrong-extension-owner)
      [ "$(rg -F -o "$base_id" "$changed" | wc -l)" = 14 ]
      sed -i "s/$base_id/$native_id/g" "$changed";;
    dropped-property)
      [ "$(rg -F -c '.name = "SubType", .text = "Theme"' "$changed")" = 1 ]
      awk '
        /^constexpr ::agiru::ReportLayoutTokenDef kTenantReportDefaultsReportLayouts1Tokens1\[\]\{/ {
          if (++tokens != 1) exit 1
          dropping=1; next
        }
        dropping { if ($0 == "};") dropping=0; next }
        /[.]name = "SubType", [.]text = "Theme"/ { properties++; next }
        { print }
        END { if (tokens != 1 || properties != 1 || dropping) exit 1 }
      ' "$changed" > "$mutant/definition.changed"
      mv "$mutant/definition.changed" "$changed";;
  esac
  compile "$mutant" "$proof/source-bound-$control-runner"
  if "$proof/source-bound-$control-runner" "$package" "$base" "$native_id" "$base_id" \
    platform/src/Reports/TenantReportDefaults.Report.al > "$proof/source-bound-$control.log" 2>&1; then
    printf 'native-report-layouts: source-bound %s escaped comparison\n' "$control" >&2
    exit 1
  fi
  rg -q 'original declaring app identity survives|complete original property population survives' \
    "$proof/source-bound-$control.log"
done
cmake_input="$proof/cmake-source"
mkdir -p "$cmake_input"
cp CMakeLists.txt "$cmake_input/"
cp -a src include cmake third_party test scripts "$cmake_input/"
cp -a "$proof/source-bound" "$cmake_input/apps"
cp "$source_input/apps.json" "$cmake_input/apps.json"
cp test/report-layouts/link-slice "$cmake_input/test/slice"
cmake -S "$cmake_input" -B "$proof/cmake-build" -G Ninja \
  -DCMAKE_CXX_COMPILER="$CXX" -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DAGIRU_BUILD_SLICE=OFF -DAGIRU_BUILD_APPS=ON -DAGIRU_NATIVE_LINK_PROOF=ON \
  > "$proof/cmake-configure.log" 2>&1
cmake --build "$proof/cmake-build" --target help > "$proof/cmake-targets.log"
rg -q 'agiru_app_platform' "$proof/cmake-targets.log"
jq -e '[.[] | select(.file | contains("/apps/platform/"))] | length == 2' \
  "$proof/cmake-build/compile_commands.json" > /dev/null
cmake --build "$proof/cmake-build" -j "${JOBS:-2}" --target native_report_registry agiru \
  > "$proof/cmake-build.log" 2>&1
"$proof/cmake-build/native_report_registry" 2000000001 'Tenant Report Defaults' \
  > "$proof/shared-registry.log"
readelf -d "$proof/cmake-build/native_report_registry" > "$proof/shared-registry-needed.log"
rg -q 'Shared library: \[libagiru_app_platform.so\]' "$proof/shared-registry-needed.log"
readelf -d "$proof/cmake-build/agiru" > "$proof/cli-apps-needed.log"
rg -q 'Shared library: \[libagiru_app_platform.so\]' "$proof/cli-apps-needed.log"
"$proof/cmake-build/agiru" --help > "$proof/cli-apps-help.log"
"$CXX" "${flags[@]}" -c test/report-layouts/RegistryRunner.cpp -o "$proof/registry-runner.o"
"$CXX" "$proof/registry-runner.o" -stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind \
  -fuse-ld=lld-19 -Wl,--as-needed "-L$proof/cmake-build" "-Wl,-rpath,$proof/cmake-build" \
  -lagiru_app_platform -lagiru_rt -o "$proof/dropped-registry"
if "$proof/dropped-registry" 2000000001 'Tenant Report Defaults' > "$proof/dropped-registry.log" 2>&1; then
  printf 'native-report-layouts: dropping unreferenced registration escaped the lookup control\n' >&2
  exit 1
fi
rg -q 'registry-only lookup finds the linked native report' "$proof/dropped-registry.log"
readelf -d "$proof/dropped-registry" > "$proof/dropped-registry-needed.log"
if rg -q 'Shared library: \[libagiru_app_platform.so\]' "$proof/dropped-registry-needed.log"; then
  printf 'native-report-layouts: negative control did not actually drop the library\n' >&2
  exit 1
fi
cmake -S "$cmake_input" -B "$proof/cmake-build" -G Ninja \
  -DCMAKE_CXX_COMPILER="$CXX" -DAGIRU_BUILD_SLICE=ON -DAGIRU_BUILD_APPS=OFF \
  -DAGIRU_NATIVE_LINK_PROOF=ON > "$proof/cmake-slice-configure.log" 2>&1
cmake --build "$proof/cmake-build" -j "${JOBS:-2}" --target native_report_registry agiru \
  > "$proof/cmake-slice-build.log" 2>&1
rg -q 'unlinked: 0 AL procedure\(s\)' "$proof/cmake-slice-build.log"
if rg -q 'agiru_unlinked_|void Unlinked' "$proof/cmake-build/unlinked.cpp"; then
  printf 'native-report-layouts: a linked native entrypoint was replaced by a refusal\n' >&2
  exit 1
fi
"$proof/cmake-build/native_report_registry" 2000000001 'Tenant Report Defaults' \
  > "$proof/shared-registry-slice.log"
readelf -d "$proof/cmake-build/agiru" > "$proof/cli-slice-needed.log"
rg -q 'Shared library: \[libagiru_app_platform.so\]' "$proof/cli-slice-needed.log"
rg -q 'Shared library: \[libagiru_slice.so\]' "$proof/cli-slice-needed.log"
"$proof/cmake-build/agiru" --help > "$proof/cli-slice-help.log"
mkdir -p "$B/fixture-commands"
jq --arg repository "$PWD" \
  '[.[] | select(.file | endswith("/test/report-layouts/RegistryRunner.cpp")) |
    .file = ($repository + "/test/report-layouts/RegistryRunner.cpp")]' \
  "$proof/cmake-build/compile_commands.json" > "$B/fixture-commands/native-registry.json"
mv "$cmake_input/apps/platform/PlatformModule.h" "$cmake_input/apps/platform/PlatformModule.h.absent"
if cmake -S "$cmake_input" -B "$proof/cmake-missing-module" -G Ninja \
  -DCMAKE_CXX_COMPILER="$CXX" -DAGIRU_BUILD_SLICE=OFF -DAGIRU_BUILD_APPS=ON \
  > "$proof/cmake-missing-module.log" 2>&1; then
  printf 'native-report-layouts: CMake accepted native reports without their module\n' >&2
  exit 1
fi
rg -q 'lack their source-owned platform module' "$proof/cmake-missing-module.log"
sha256sum --check --status "$proof/compiler-inputs.sha256"
sha256sum --check --status "$proof/originals.sha256"
python3 scripts/fetch_symbols.py --verify "$package" > "$proof/package-verified.json"
jq -n --slurpfile raw "$proof/raw-inventory.json" --slurpfile package "$package/provenance.json" \
  --slurpfile symbols "$package/SymbolReference.json" --slurpfile base "$base/app.json" \
  --slurpfile assets "$proof/layout-bundle/result.json" \
  --arg demo_version "$(tr -d '\n' < BC_VERSION)" \
  --arg base_revision "$(git -C "$base" rev-parse HEAD)" \
  '{package:$package[0].identity,package_sha256:$package[0].package_sha256,platform_version:$package[0].bc_version,
    demo_version:$demo_version,base_revision:$base_revision,base_version:$base[0].version,
    symbol_reports:($symbols[0].Reports | length),raw_native:$raw[0].summary,
    selected_reports:1,selected_extensions:1,
    native_layouts:2,extension_layouts:14,compiled_layouts:16,
    packaged_assets:16,asset_package:$assets[0],
    native_objects_outside_compiled_fixture:($raw[0].summary.objects-1),
    unexecuted_native_objects:$raw[0].summary.objects,installed_assets:0,rendered_documents:0,
    production_native_report_loader_activated:true,complete_native_loader_activated:false,
    source_bound_compiled_layouts:16,source_bound_translation_exit:1,
    cmake_platform_configured:true,cmake_platform_build_executed:true,
    shared_registry_proved:true,registration_drop_control:"rejected",
    fixture_slice_sources:2,fixture_cli_apps_linked:true,fixture_cli_slice_linked:true,
    complete_root_slice_proof:false,
    complete_app_proof:false}' > "$proof/result.json"
printf 'native-report-layouts: original source and production-loader variants compile all sixteen layouts; ownership/property controls fail; native gaps keep translation red; no installation/rendering or G1 claim; %s\n' "$proof"

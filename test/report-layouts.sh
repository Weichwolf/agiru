#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
source_root=${AGIRU_BC_SOURCE:-$HOME/Git/BCApps/src}
base="$source_root/Layers/W1/BaseApp"
source="$base/Foundation/Reporting/CompositeLayout.ReportExt.al"
proof=$(mktemp -d "$B/report-layouts.XXXXXX")
[ "$(rg -i -c '^\s*layout\s*\(' "$source")" = 14 ]
[ "$(rg -i -c '^\s*LayoutFile\s*=' "$source")" = 14 ]
"$B/gate_ReportLayoutGate" "$base" "$proof/input" "$proof/unknown-input"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' \
  > "$proof/input/scope.json"
"$B/agirutc" "$proof/input" "$proof/input/apps.json" "$proof/generated" \
  > "$proof/generation.log" 2>&1
rg -q 'layouts +1 report / 3 extension declarations; 3 bound layouts, 3 immutable declarations emitted, 1 retained on unresolved targets; unresolved not emitted, none installed or rendered' "$proof/generation.log"
rg -q 'report absent native report' "$proof/generation.log"
"$B/agirutc" "$proof/input" "$proof/input/apps.json" > "$proof/source-only.log" 2>&1
rg -q '3 bound layouts, 0 immutable declarations emitted, 1 retained on unresolved targets' "$proof/source-only.log"
for count in '4 x layout.type' '4 x layout.layoutfile' '2 x layout.caption' \
  '1 x layout.summary' '1 x layout.subtype' '1 x layout.mimetype' \
  '1 x layout.obsoletestate' '1 x layout.obsoletereason' '1 x layout.obsoletetag' \
  '1 x layout.excellayoutmultipledatasheets'; do
  rg -q "${count/ x/ +x}" "$proof/generation.log"
  rg -q "${count/ x/ +x}" "$proof/source-only.log"
done
cp "$proof/input/scope.json" "$proof/unknown-input/scope.json"
if "$B/agirutc" "$proof/unknown-input" "$proof/unknown-input/apps.json" "$proof/unknown" \
  > "$proof/unknown.log" 2>&1; then
  printf 'report-layouts: an unknown property escaped the capability census\n' >&2
  exit 1
fi
rg -q '1 x layout.futureproperty' "$proof/unknown.log"
rg -q '^ABORT +1 property declaration' "$proof/unknown.log"
if "$B/agirutc" "$proof/unknown-input" "$proof/unknown-input/apps.json" \
  > "$proof/source-only-unknown.log" 2>&1; then
  printf 'report-layouts: source-only mode hid an unknown bound property\n' >&2
  exit 1
fi
rg -q '1 x layout.futureproperty' "$proof/source-only-unknown.log"
rg -q '^ABORT +1 property declaration' "$proof/source-only-unknown.log"

for variant in unresolved dropped refused unresolved-refused; do
  input="$proof/$variant-input"
  cp -a "$proof/input" "$input"
  target=ExtraLayouts.ReportExt.al
  property=SummaryML
  if [ "$variant" = unresolved ] || [ "$variant" = unresolved-refused ]; then
    target=UnresolvedLayout.ReportExt.al
  fi
  if [ "$variant" = unresolved ]; then property=FutureProperty; fi
  if [ "$variant" = dropped ]; then property=ToolTip; fi
  awk -v property="$property" '
    /^            Type = / && !inserted {
      print "            " property " = '\''Must not silently pass'\'';"; inserted++
    }
    { print }
    END { if (inserted != 1) exit 1 }
  ' "$proof/input/Addon/$target" > "$input/Addon/$target"
  if "$B/agirutc" "$input" "$input/apps.json" "$proof/$variant" \
    > "$proof/$variant.log" 2>&1; then
    printf 'report-layouts: %s property escaped the census\n' "$variant" >&2
    exit 1
  fi
  if [ "$property" = SummaryML ]; then
    rg -q 'ABORT +1 report-layout property declaration' "$proof/$variant.log"
    rg -q 'SummaryML in reportextension .* layout ' "$proof/$variant.log"
  else
    rg -qi "1 x layout.$property" "$proof/$variant.log"
    rg -q '^ABORT +1 property declaration' "$proof/$variant.log"
  fi
done

flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Itest/gate)
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db)
compile() {
  local input=$1 output=$2
  local sources
  rg --files --no-ignore "$input" -g '*.cpp' | LC_ALL=C sort > "$output.sources"
  mapfile -t sources < "$output.sources"
  [ "${#sources[@]}" -gt 0 ] || { printf 'report-layouts: no generated sources\n' >&2; return 2; }
  "$CXX" "${flags[@]}" "-I$input/fixture" "-I$input/shared" "-I$input/absent" \
    -c test/report-layouts/Runner.cpp -o "$output.runner.o"
  "$CXX" "${flags[@]}" "-I$input/fixture" "-I$input/shared" "-I$input/absent" \
    "$output.runner.o" "${sources[@]}" "${links[@]}" -o "$output"
}
compile "$proof/generated" "$proof/runner"
"$proof/runner"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/report-layouts/Runner.cpp" \
  --args '[{directory: $directory, file: $file, arguments: $ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" "-I$proof/generated/fixture" "-I$proof/generated/shared" \
  "-I$proof/generated/absent" -c test/report-layouts/Runner.cpp -o "$proof/runner.runner.o" \
  > "$proof/compile_commands.json"
cp "$proof/compile_commands.json" "$B/fixture-commands/report-layouts.json"

cp -a "$proof/generated" "$proof/wrong-owner"
definitions="$proof/generated/fixture/test/reporting/report/LayoutContract.def.cpp"
[ "$(rg -F -o 'ReportExtensionId{50081}' "$definitions" | wc -l)" = 2 ]
sed 's/ReportExtensionId{50081}/ReportExtensionId{50080}/g' "$definitions" \
  > "$proof/wrong-owner/fixture/test/reporting/report/LayoutContract.def.cpp"
compile "$proof/wrong-owner" "$proof/wrong-owner-runner"
if "$proof/wrong-owner-runner" > "$proof/wrong-owner.log" 2>&1; then
  printf 'report-layouts: changing declaring ownership escaped the execution control\n' >&2
  exit 1
fi
rg -q 'extension identity reaches metadata' "$proof/wrong-owner.log"

cp -a "$proof/generated" "$proof/wrong-app"
[ "$(rg -F -o '98765432-1234-5678-9012-123456789012' "$definitions" | wc -l)" = 2 ]
sed 's/98765432-1234-5678-9012-123456789012/12345678-1234-5678-9012-123456789012/g' "$definitions" \
  > "$proof/wrong-app/fixture/test/reporting/report/LayoutContract.def.cpp"
compile "$proof/wrong-app" "$proof/wrong-app-runner"
if "$proof/wrong-app-runner" > "$proof/wrong-app.log" 2>&1; then
  printf 'report-layouts: target app stole extension ownership without a failure\n' >&2
  exit 1
fi
rg -q 'extension app owns the asset' "$proof/wrong-app.log"

cp -a "$proof/generated" "$proof/missing-owner"
sed 's/ReportExtensionId{50081}/ReportExtensionId{0}/g' "$definitions" \
  > "$proof/missing-owner/fixture/test/reporting/report/LayoutContract.def.cpp"
if compile "$proof/missing-owner" "$proof/missing-owner-runner" \
  > "$proof/missing-owner.log" 2>&1; then
  printf 'report-layouts: an ownerless layout escaped compile-time validation\n' >&2
  exit 1
fi
rg -q 'static assertion failed' "$proof/missing-owner.log"

flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -Iinclude -Itest/gate -Isrc/al)
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_al)
[ "$(rg -F -c 'into.push_back(std::move(layout));' src/al/Parser.cpp)" = 1 ]
sed 's/into.push_back(std::move(layout));/static_cast<void>(layout);/' src/al/Parser.cpp \
  > "$proof/LayoutsDiscarded.cpp"
"$CXX" "${flags[@]}" test/gate/ReportLayoutGate.cpp "$proof/LayoutsDiscarded.cpp" \
  "${links[@]}" -o "$proof/layouts-discarded"
if "$proof/layouts-discarded" "$base" > "$proof/discarded.log" 2>&1; then
  printf 'report-layouts: discarded declarations escaped the control\n' >&2
  exit 1
fi
rg -q 'report rendering was discarded|one report layout survives' "$proof/discarded.log"
printf 'report-layouts: fourteen declarations/assets retained; generated metadata executes; capability/ownership/removal controls fail; %s\n' "$proof"

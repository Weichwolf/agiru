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
"$B/gate_ReportLayoutGate" "$base" "$proof/input"
printf '%s\n' '{"include":["Microsoft"],"exclude":[],"product_exclude":[]}' \
  > "$proof/input/scope.json"
"$B/agirutc" "$proof/input" "$proof/input/apps.json" "$proof/generated" \
  > "$proof/generation.log" 2>&1
rg -q 'layouts +1 report / 3 extension declarations; 3 retained on bound AST reports, 1 on unresolved targets; not emitted, installed or rendered' "$proof/generation.log"
rg -q 'report absent native report' "$proof/generation.log"
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
printf 'report-layouts: fourteen declarations/assets retained; merge and unresolved counts checked; removal control fails; %s\n' "$proof"

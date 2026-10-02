#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
input=$(realpath "${1:-apps}")
proof=$(mktemp -d "$B/report-layout-metadata.XXXXXX")
rg -l --no-ignore '^constexpr ::agiru::ReportLayoutDef ' "$input" -g '*.def.cpp' \
  | LC_ALL=C sort > "$proof/sources"
mapfile -t sources < "$proof/sources"
[ "${#sources[@]}" -gt 0 ] || { printf 'report-layout-metadata: no declarations\n' >&2; exit 2; }
sha256sum "${sources[@]}" > "$proof/source.sha256"
printf '#include "meta/ReportLayoutDef.h"\n' > "$proof/metadata.cpp"
for source in "${sources[@]}"; do
  awk '
    /^namespace / && !started { started=1 }
    started && /^constexpr / && !/^constexpr ::agiru::ReportLayout/ {
      print "}"; done=1; exit
    }
    started { print }
    /^constexpr ::agiru::ReportLayoutDef / { definitions++ }
    END { if (!done || definitions != 1) exit 1 }
  ' "$source" >> "$proof/metadata.cpp"
done
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -fsyntax-only)
"$CXX" "${flags[@]}" "$proof/metadata.cpp" > "$proof/compile.log" 2>&1
layouts=$(rg -c '^static_assert\(.*ReportLayouts' "$proof/metadata.cpp")
[ "$layouts" -gt 0 ]
sed -E 's/ReportId\{[1-9][0-9]*\}/ReportId{0}/g' "$proof/metadata.cpp" \
  > "$proof/ownerless.cpp"
if "$CXX" "${flags[@]}" "$proof/ownerless.cpp" > "$proof/ownerless.log" 2>&1; then
  printf 'report-layout-metadata: removing ownership escaped the compile control\n' >&2
  exit 1
fi
rg -q 'static assertion failed' "$proof/ownerless.log"
sha256sum --check --status "$proof/source.sha256"
printf 'report-layout-metadata: %s immutable declarations from %s reports compile; ownerless control refuses; not complete app/installation/rendering proof; %s\n' \
  "$layouts" "${#sources[@]}" "$proof"

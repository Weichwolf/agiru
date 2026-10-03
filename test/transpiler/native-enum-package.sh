#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
if [ -z "${AGIRU_SYSTEM_SYMBOLS:-}" ]; then
  printf 'native-enum-package: explicit verified AGIRU_SYSTEM_SYMBOLS is required\n' >&2
  exit 2
fi
package=$(realpath "$AGIRU_SYSTEM_SYMBOLS")
proof=$(mktemp -d /tmp/agiru-native-enum-package.XXXXXX)
printf '%s\n' "$proof" > "$B/native-enum-package.latest"
git rev-parse HEAD > "$proof/head.txt"
git status --porcelain > "$proof/source-status.txt"
rg --files src include -g '*.h' -g '*.cpp' | LC_ALL=C sort \
  | xargs -d '\n' sha256sum > "$proof/source-inputs.sha256"
sha256sum test/transpiler/native-enums/EmitContracts.cpp test/transpiler/native-enum-package.sh \
  >> "$proof/source-inputs.sha256"
sha256sum "$B/agirutc" "$B/libagiru_gen.so" "$B/libagiru_al.so" > "$proof/compiler-inputs.sha256"
python3 scripts/fetch_symbols.py --verify "$package" > "$proof/provenance-before.json"
printf '%s\n' '{"apps":[{"name":"inventory","source":"src"}]}' > "$proof/inventory-apps.json"
printf '%s\n' '{"include":["System","Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
python3 scripts/scope_inventory.py "$package" --apps "$proof/inventory-apps.json" \
  --scope "$proof/scope.json" --output "$proof/raw-inventory.json" > "$proof/inventory.log"
jq -e '.summary.unmeasured_files == 0 and (.errors | length) == 0' "$proof/raw-inventory.json" > /dev/null
jq -r '.objects[] | select(.kind == "enum") | [.id,.source] | @tsv' \
  "$proof/raw-inventory.json" > "$proof/enums.tsv"
[ -s "$proof/enums.tsv" ]
mkdir -p "$proof/source"
cp test/transpiler/native-enums/source/app.json "$proof/source/app.json"
printf '%s\n' '{"apps":[{"name":"fixture","source":"source"}]}' > "$proof/apps.json"
translation_status=0
"$B/agirutc" "$proof" "$proof/apps.json" "$proof/generated" --system-symbols "$package" \
  > "$proof/translation.log" 2>&1 || translation_status=$?
case "$translation_status" in 0|1) ;; *) exit "$translation_status" ;; esac
expected=$(wc -l < "$proof/enums.tsv")
rg -q "native $expected enum sources bound;" "$proof/translation.log"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/al -Isrc/gen)
"$CXX" "${flags[@]}" -c test/transpiler/native-enums/EmitContracts.cpp -o "$proof/emitter.o"
"$CXX" "$proof/emitter.o" -stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
  "-L$B" "-Wl,-rpath,$B" -lagiru_gen -lagiru_al -o "$proof/emitter"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/transpiler/native-enums/EmitContracts.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" -c test/transpiler/native-enums/EmitContracts.cpp -o "$proof/emitter.o" \
  > "$B/fixture-commands/native-enum-package.json"
passed=0
failed=0
while IFS=$'\t' read -r id source; do
  status=0
  "$proof/emitter" "$package/$source" > "$proof/$id.cpp" 2> "$proof/$id.emit.log" || status=$?
  if [ "$status" -eq 0 ]; then
    "$CXX" "${flags[@]}" "-I$proof/generated/platform" -fsyntax-only "$proof/$id.cpp" \
      > "$proof/$id.compile.log" 2>&1 || status=$?
  fi
  if [ "$status" -eq 0 ]; then passed=$((passed + 1)); else failed=$((failed + 1)); fi
  jq -nc --argjson id "$id" --arg source "$source" --argjson status "$status" \
    '{id:$id,source:$source,status:$status}' >> "$proof/results.jsonl"
done < "$proof/enums.tsv"
IFS=$'\t' read -r control_id control_source < "$proof/enums.tsv"
header=$(sed -n 's/^#include "\(.*\)"/\1/p' "$proof/$control_id.cpp")
[ "$(rg -Fc "kObjectID = $control_id" "$proof/generated/platform/$header")" -eq 1 ]
mkdir -p "$proof/wrong-id/$(dirname "$header")"
sed "s/kObjectID = $control_id/kObjectID = 1/" "$proof/generated/platform/$header" \
  > "$proof/wrong-id/$header"
if "$CXX" "${flags[@]}" "-I$proof/wrong-id" "-I$proof/generated/platform" \
  -fsyntax-only "$proof/$control_id.cpp" > "$proof/wrong-id.log" 2>&1; then
  printf 'native-enum-package: wrong original enum identity escaped the contract\n' >&2
  exit 1
fi
rg -q 'static assertion failed' "$proof/wrong-id.log"
python3 scripts/fetch_symbols.py --verify "$package" > "$proof/provenance-after.json"
cmp "$proof/provenance-before.json" "$proof/provenance-after.json"
jq -n --argjson raw "$expected" --argjson passed "$passed" --argjson failed "$failed" \
  --argjson translation_status "$translation_status" --slurpfile results "$proof/results.jsonl" \
  '{raw_enums:$raw,passed:$passed,failed:$failed,translation_status:$translation_status,
    results:$results,business_executed:0,wrong_id_control:"rejected"}' > "$proof/result.json"
sha256sum --check --status "$proof/source-inputs.sha256"
sha256sum --check --status "$proof/compiler-inputs.sha256"
[ "$((passed + failed))" -eq "$expected" ]
printf 'native-enum-package: %s/%s declarations compile; business execution unproved; %s\n' \
  "$passed" "$expected" "$proof"
[ "$failed" -eq 0 ]

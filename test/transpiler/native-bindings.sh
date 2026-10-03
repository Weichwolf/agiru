#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
if [ -z "${AGIRU_SYSTEM_SYMBOLS:-}" ]; then
  printf 'native-bindings: explicit AGIRU_SYSTEM_SYMBOLS is required\n' >&2
  exit 2
fi
package=$(realpath "$AGIRU_SYSTEM_SYMBOLS")
python3 scripts/fetch_symbols.py --verify "$package"
proof=$(mktemp -d /tmp/agiru-native-bindings.XXXXXX)
printf '%s\n' "$proof" > "$B/native-bindings.latest"
rg --files src include -g '*.h' -g '*.cpp' | LC_ALL=C sort \
  | xargs -d '\n' sha256sum > "$proof/source-inputs.sha256"
sha256sum test/transpiler/native-binding/Emit.cpp test/transpiler/native-bindings.sh >> "$proof/source-inputs.sha256"
sha256sum test/transpiler/native-binding/PageRunner.cpp >> "$proof/source-inputs.sha256"
git status --porcelain > "$proof/source-status.txt"
printf '%s\n' '{"apps":[{"name":"native","source":"src"}]}' > "$proof/inventory-apps.json"
printf '%s\n' '{"include":["System","Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
python3 scripts/scope_inventory.py "$package" --apps "$proof/inventory-apps.json" \
  --scope "$proof/scope.json" --output "$proof/raw-inventory.json" > "$proof/raw-inventory.log"
jq -e '.summary.unmeasured_files == 0 and (.errors | length) == 0' "$proof/raw-inventory.json" > /dev/null
sha256sum "$package/System.app" "$package/NavxManifest.xml" "$package/SymbolReference.json" \
  > "$proof/originals.sha256"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/al -Isrc/gen)
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_gen -lagiru_al)
sha256sum "$B/libagiru_al.so" "$B/libagiru_gen.so" "$B/libagiru_rt.so" \
  "$B/libagiru_net.so" "$B/libagiru_db.so" > "$proof/libraries.sha256"
"$CXX" "${flags[@]}" -c test/transpiler/native-binding/Emit.cpp -o "$proof/emitter.o"
"$CXX" "$proof/emitter.o" "${links[@]}" -o "$proof/emitter"
mkdir -p "$B/fixture-commands"
jq -n --arg directory "$PWD" --arg file "$PWD/test/transpiler/native-binding/Emit.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${flags[@]}" -c test/transpiler/native-binding/Emit.cpp -o "$proof/emitter.o" \
  > "$B/fixture-commands/native-bindings.json"
jq -r '.objects[] | select(.kind == "table") | [.id,.source] | @tsv' \
  "$proof/raw-inventory.json" > "$proof/tables.tsv"
while IFS=$'\t' read -r id source; do
  sha256sum "$package/$source" >> "$proof/originals.sha256"
  status=0
  "$proof/emitter" "$package/$source" > "$proof/$id.cpp" 2> "$proof/$id.emit.log" || status=$?
  case "$status" in
    0)
      compile_status=0
      "$CXX" "${flags[@]}" -fsyntax-only -ferror-limit=0 "$proof/$id.cpp" \
        > "$proof/$id.compile.log" 2>&1 || compile_status=$?
      if [ "$compile_status" -eq 0 ]; then
        outcome=contract-pass
      elif [ "$compile_status" -ge 128 ]; then
        outcome=crashed
      else
        outcome=contract-fail
      fi;;
    3) outcome=unbound;;
    4) outcome=parse-refused;;
    2) outcome=binding-refused;;
    *) printf 'unexpected emitter status: %s\n' "$status" >&2; outcome=crashed;;
  esac
  jq -nc --argjson id "$id" --arg source "$source" --arg status "$outcome" \
    '{id:$id,source:$source,status:$status}' >> "$proof/tables.jsonl"
done < "$proof/tables.tsv"
jq -s '.' "$proof/tables.jsonl" > "$proof/tables.json"
passed=$(jq -r 'first(.[] | select(.status == "contract-pass") | .source) // empty' "$proof/tables.json")
if [ -z "$passed" ]; then
  printf 'native-bindings: no passing original for a meaningful negative control\n' >&2
  exit 2
fi
awk '
  !changed && /field\([0-9]+;/ {
    sub(/field\([0-9]+;/, "field(99999;"); changed++
  }
  { print }
  END { if (changed != 1) exit 1 }
' "$package/$passed" > "$proof/wrong-field-number.al"
"$proof/emitter" "$proof/wrong-field-number.al" > "$proof/wrong-field-number.cpp" \
  2> "$proof/wrong-field-number.emit.log"
if "$CXX" "${flags[@]}" -fsyntax-only "$proof/wrong-field-number.cpp" \
  > "$proof/wrong-field-number.compile.log" 2>&1; then
  printf 'native-bindings: wrong source field number escaped its generated contract\n' >&2
  exit 1
fi
rg -q 'native field declaration mismatch' "$proof/wrong-field-number.compile.log"
passed_id=$(jq -r 'first(.[] | select(.status == "contract-pass") | .id)' "$proof/tables.json")
for control in wrong-system-offset wrong-system-type wrong-system-number; do
  overlay="$proof/$control/include/meta"
  mkdir -p "$overlay"
  awk -v control="$control" '
    control == "wrong-system-offset" && /offsetof\(T, SystemId\)/ {
      sub(/offsetof\(T, SystemId\)/, "offsetof(T, SystemCreatedBy)"); changed++
    }
    control == "wrong-system-type" && /Declare<&T::SystemId>/ {
      sub(/Declare<&T::SystemId>/, "Declare<\\&T::SystemCreatedAt>"); changed++
    }
    control == "wrong-system-number" && /\.no = FieldNo\{2000000000\}/ {
      sub(/FieldNo\{2000000000\}/, "FieldNo{2000000099}"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' include/meta/Declare.h > "$overlay/Declare.h"
  if "$CXX" "-I$proof/$control/include" "${flags[@]}" -fsyntax-only "$proof/$passed_id.cpp" \
    > "$proof/$control.compile.log" 2>&1; then
    printf 'native-bindings: %s escaped the original source contract\n' "$control" >&2
    exit 1
  fi
  rg -q 'native (system field offset|field declaration) mismatch' "$proof/$control.compile.log"
done
jq -n --slurpfile raw "$proof/raw-inventory.json" --slurpfile tables "$proof/tables.json" \
  --slurpfile package "$package/provenance.json" --arg head "$(git rev-parse HEAD)" \
  '{head:$head,package:$package[0].identity,package_sha256:$package[0].package_sha256,
    raw_native:$raw[0].summary,tables:$tables[0],
    statuses:($tables[0] | group_by(.status) | map({key:.[0].status,value:length}) | from_entries),
    non_table_objects:($raw[0].summary.objects-($tables[0] | length)),
    unexecuted_native_objects:$raw[0].summary.objects,
    source_field_number_negative_control:"rejected",
    system_field_negative_controls:{offset:"rejected",type:"rejected",number:"rejected"},
    contract_surface:["identity","field-count","field-type-length-caption","field-subtype-class","base-system-field-number-type-offset","option-codes-names-captions","keys","company-scope","replication-declaration","table-caption","inherent-permissions-declaration"],
    production_native_loader_activated:false,complete_declaration_proof:false,complete_app_proof:false}' \
  > "$proof/result.json"
base="${AGIRU_BC_SOURCE:-$HOME/Git/BCApps/src}/Layers/W1/BaseApp"
page="$base/Modules/System/PageDesigner/PageFieldsSelectionList.Page.al"
sha256sum "$page" >> "$proof/originals.sha256"
"$proof/emitter" --page "$package/src/Virtual Tables/PageTableField.Table.al" "$page" "$proof/page" \
  > "$proof/page-generation.log" 2>&1
page_flags=("${flags[@]}" -Itest/gate "-I$proof/page")
page_sources=("$proof/page/system/tooling/page/PageFieldsSelectionList.cpp"
  "$proof/page/system/tooling/page/PageFieldsSelectionList.def.cpp")
page_status=0
"$CXX" "${page_flags[@]}" -c test/transpiler/native-binding/PageRunner.cpp -o "$proof/page-runner.o" \
  > "$proof/page-runner.compile.log" 2>&1 || page_status=$?
if [ "$page_status" -eq 0 ]; then
  jq -n --arg directory "$PWD" --arg file "$PWD/test/transpiler/native-binding/PageRunner.cpp" \
    --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
    "$CXX" "${page_flags[@]}" -c test/transpiler/native-binding/PageRunner.cpp -o "$proof/page-runner.o" \
    > "$B/fixture-commands/native-page-binding.json"
  "$CXX" "${page_flags[@]}" "$proof/page-runner.o" "${page_sources[@]}" "${links[@]}" \
    -lagiru_rt -lagiru_net -lagiru_db -o "$proof/page-runner" \
    > "$proof/page-consumers.compile.log" 2>&1 || page_status=$?
fi
if [ "$page_status" -eq 0 ]; then
  "$proof/page-runner" > "$proof/page-runner.log" 2>&1 || page_status=$?
fi
page_control=unexecuted
if [ "$page_status" -eq 0 ]; then
  awk '
    /field\(Caption; Caption\)/ { sub(/field\(Caption; Caption\)/, "field(Caption; Name)"); changed++ }
    { print }
    END { if (changed != 1) exit 1 }
  ' "$page" > "$proof/wrong-page-source.al"
  "$proof/emitter" --page "$package/src/Virtual Tables/PageTableField.Table.al" \
    "$proof/wrong-page-source.al" "$proof/wrong-page" > "$proof/wrong-page-generation.log" 2>&1
  "$CXX" "${flags[@]}" -Itest/gate "-I$proof/wrong-page" \
    -c test/transpiler/native-binding/PageRunner.cpp -o "$proof/wrong-page-runner.o"
  "$CXX" "${flags[@]}" -Itest/gate "-I$proof/wrong-page" "$proof/wrong-page-runner.o" \
    "$proof/wrong-page/system/tooling/page/PageFieldsSelectionList.cpp" \
    "$proof/wrong-page/system/tooling/page/PageFieldsSelectionList.def.cpp" "${links[@]}" \
    -lagiru_rt -lagiru_net -lagiru_db -o "$proof/wrong-page-runner" \
    > "$proof/wrong-page.compile.log" 2>&1
  if "$proof/wrong-page-runner" > "$proof/wrong-page-runner.log" 2>&1; then
    printf 'native-bindings: wrong page source expression escaped the original field binding\n' >&2
    exit 1
  fi
  rg -q 'original bare Caption reads Rec|original Caption field number survives' "$proof/wrong-page-runner.log"
  page_control=rejected
fi
jq --argjson status "$page_status" --arg control "$page_control" \
  '.original_page_fixture={name:"PageFieldsSelectionList",consumer_units:2,status:$status,
    source_expression_negative_control:$control,full_eight_consumer_replay:false,live_provider_proved:false}' \
  "$proof/result.json" > "$proof/result-page.json"
mv "$proof/result-page.json" "$proof/result.json"
if [ "$page_status" -eq 0 ]; then cat "$proof/page-runner.log"; fi
jq -e --slurpfile raw "$proof/raw-inventory.json" \
  'length == $raw[0].summary.objects_by_kind.table and (map(.id) | unique | length) == length' \
  "$proof/tables.json" > /dev/null
sha256sum --check --status "$proof/originals.sha256"
sha256sum --check --status "$proof/source-inputs.sha256"
sha256sum --check --status "$proof/libraries.sha256"
python3 scripts/fetch_symbols.py --verify "$package" > "$proof/package-verified.json"
jq '{raw_native, statuses, non_table_objects, unexecuted_native_objects,original_page_fixture}' "$proof/result.json"
printf 'native-bindings: full source-counted table matrix; no provider, full-app or G1 proof; %s\n' "$proof"
jq -e 'all(.tables[]; .status == "contract-pass")' "$proof/result.json" > /dev/null

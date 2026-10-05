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
sha256sum test/transpiler/native-binding/SourceRunner.cpp scripts/transpile.sh >> "$proof/source-inputs.sha256"
sha256sum scope.json scripts/scope_inventory.py >> "$proof/source-inputs.sha256"
git status --porcelain > "$proof/source-status.txt"
printf '%s\n' '{"apps":[{"name":"native","source":"src"}]}' > "$proof/inventory-apps.json"
printf '%s\n' '{"include":["System","Microsoft"],"exclude":[],"product_exclude":[]}' > "$proof/scope.json"
python3 scripts/scope_inventory.py "$package" --apps "$proof/inventory-apps.json" \
  --scope "$PWD/scope.json" --source-domain system-symbols \
  --output "$proof/raw-inventory.json" > "$proof/raw-inventory.log"
jq -e '.summary.unmeasured_files == 0 and (.errors | length) == 0' "$proof/raw-inventory.json" > /dev/null
sha256sum "$package/System.app" "$package/NavxManifest.xml" "$package/SymbolReference.json" \
  > "$proof/originals.sha256"
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude -Isrc/al -Isrc/gen)
links=(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19
  "-L$B" "-Wl,-rpath,$B" -lagiru_gen -lagiru_al)
sha256sum "$B/libagiru_al.so" "$B/libagiru_gen.so" "$B/libagiru_rt.so" \
  "$B/libagiru_net.so" "$B/libagiru_db.so" "$B/agirutc" > "$proof/libraries.sha256"
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
jq --slurpfile raw "$proof/raw-inventory.json" \
  'map(. as $table | . + {product_exclusion_reason:
    (first($raw[0].objects[] | select(.kind == "table" and .id == $table.id) |
      .product_exclusion_reason) // null)})' "$proof/tables.json" > "$proof/tables-scoped.json"
mv "$proof/tables-scoped.json" "$proof/tables.json"
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
  header=Declare.h
  if [ "$control" = wrong-system-number ]; then header=SystemFields.h; fi
  awk -v control="$control" '
    control == "wrong-system-offset" && /offsetof\(T, SystemId\)/ {
      sub(/offsetof\(T, SystemId\)/, "offsetof(T, SystemCreatedBy)"); changed++
    }
    control == "wrong-system-type" && /<&T::SystemId>/ {
      sub(/SystemId>/, "SystemCreatedAt>"); changed++
    }
    control == "wrong-system-number" && /\.no = FieldNo\{2000000000\}/ {
      sub(/FieldNo\{2000000000\}/, "FieldNo{2000000099}"); changed++
    }
    { print }
    END { if (changed < 1) exit 2 }
  ' "include/meta/$header" > "$overlay/$header"
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
    selected_tables:($tables[0] | map(select(.product_exclusion_reason == null))),
    excluded_tables:($tables[0] | map(select(.product_exclusion_reason != null))),
    selected_statuses:($tables[0] | map(select(.product_exclusion_reason == null)) |
      group_by(.status) | map({key:.[0].status,value:length}) | from_entries),
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
mkdir -p "$proof/qualification-input/empty"
printf '%s\n' '{"apps":[{"name":"empty","source":"empty"}]}' > "$proof/qualification-input/apps.json"
jq '.product_exclude |= map(select(split(":")[1] | startswith("system-symbols/")))' \
  scope.json > "$proof/qualification-input/scope.json"
qualification_status=0
"$B/agirutc" "$proof/qualification-input" "$proof/qualification-input/apps.json" \
  "$proof/qualified" --system-symbols "$package" > "$proof/qualification-generation.log" 2>&1 \
  || qualification_status=$?
[ "$qualification_status" -eq 0 ] || [ "$qualification_status" -eq 1 ]
rg -q 'native .* table declarations emitted with original module/namespace ownership' \
  "$proof/qualification-generation.log"
jq -r --slurpfile tables "$proof/tables.json" \
  '.objects[] | select(.kind == "table") | . as $source |
    select(any($tables[0][]; .id == $source.id and .status == "contract-pass" and
      .product_exclusion_reason == null)) |
    [.id,.namespace,.name] | @tsv' "$proof/raw-inventory.json" > "$proof/qualified-tables.tsv"
rg --files --no-ignore "$proof/qualified/platform/native/table" -g '*.cpp' \
  | LC_ALL=C sort > "$proof/qualified-sources"
mapfile -t qualified_sources < "$proof/qualified-sources"
[ "${#qualified_sources[@]}" -eq "$(jq '[.[] | select(.product_exclusion_reason == null) |
  select(.status == "contract-pass" or .status == "contract-fail")] | length' "$proof/tables.json")" ]
[ "${#qualified_sources[@]}" -gt 0 ]
qualified_flags=("${flags[@]}" -fvisibility-inlines-hidden -Itest/gate "-I$proof/qualified/platform")
qualified_links=("${links[@]}" -lagiru_rt -lagiru_net -lagiru_db)
qualified_objects=()
for source in "${qualified_sources[@]}"; do
  object="$proof/$(basename "${source%.cpp}").qualified.o"
  compile_status=0
  "$CXX" "${qualified_flags[@]}" -fPIC -c "$source" -o "$object" \
    > "$object.log" 2>&1 || compile_status=$?
  jq -nc --argjson id "$(basename "${source%.cpp}")" --argjson status "$compile_status" \
    '{id:$id,compiler_exit:$status}' >> "$proof/qualification-matrix.jsonl"
  if [ "$compile_status" -eq 0 ]; then qualified_objects+=("$object"); fi
done
[ "${#qualified_objects[@]}" -eq "$(wc -l < "$proof/qualified-tables.tsv")" ]
"$CXX" -shared "${qualified_objects[@]}" "${qualified_links[@]}" -o "$proof/libqualified.so"
"$CXX" "${qualified_flags[@]}" -c test/transpiler/native-binding/SourceRunner.cpp \
  -o "$proof/source-runner.o"
jq -n --arg directory "$PWD" --arg file "$PWD/test/transpiler/native-binding/SourceRunner.cpp" \
  --args '[{directory:$directory,file:$file,arguments:$ARGS.positional}]' -- \
  "$CXX" "${qualified_flags[@]}" -c test/transpiler/native-binding/SourceRunner.cpp \
  -o "$proof/source-runner.o" > "$B/fixture-commands/native-source-qualification.json"
mapfile -t original_identity < <(jq -r '.identity | .Id,.Name,.Publisher,.Version' "$package/provenance.json")
"$CXX" "$proof/source-runner.o" -Wl,--as-needed -Wl,--push-state,--no-as-needed \
  "$proof/libqualified.so" -Wl,--pop-state "${qualified_links[@]}" -o "$proof/source-runner"
"$proof/source-runner" "$proof/qualified-tables.tsv" "${original_identity[@]}" \
  > "$proof/source-runner.log" 2>&1
cat "$proof/source-runner.log"
"$CXX" "$proof/source-runner.o" -Wl,--as-needed "$proof/libqualified.so" \
  "${qualified_links[@]}" -o "$proof/dropped-source-runner"
if "$proof/dropped-source-runner" "$proof/qualified-tables.tsv" "${original_identity[@]}" \
    > "$proof/dropped-source-runner.log" 2>&1; then
  printf 'native-bindings: dropped native qualification library escaped source ownership\n' >&2
  exit 1
fi
rg -q 'FAIL .*original native declaration is installed' "$proof/dropped-source-runner.log"
wrong_namespace="$proof/wrong-native-namespace.cpp"
awk '
  /table[.]nameSpace = / { $0="  table.nameSpace = \"Wrong.Native\";"; changed++ }
  { print }
  END { if (changed != 1) exit 2 }
' "$proof/qualified/platform/native/table/2000000136.cpp" > "$wrong_namespace"
"$CXX" "${qualified_flags[@]}" -fPIC -c "$wrong_namespace" -o "$proof/wrong-native-namespace.o"
wrong_objects=()
for object in "${qualified_objects[@]}"; do
  if [ "$(basename "$object")" = 2000000136.qualified.o ]; then
    wrong_objects+=("$proof/wrong-native-namespace.o")
  else
    wrong_objects+=("$object")
  fi
done
"$CXX" -shared "${wrong_objects[@]}" "${qualified_links[@]}" -o "$proof/libwrong-namespace.so"
"$CXX" "$proof/source-runner.o" -Wl,--as-needed -Wl,--push-state,--no-as-needed \
  "$proof/libwrong-namespace.so" -Wl,--pop-state "${qualified_links[@]}" -o "$proof/wrong-namespace-runner"
if "$proof/wrong-namespace-runner" "$proof/qualified-tables.tsv" "${original_identity[@]}" \
    > "$proof/wrong-namespace-runner.log" 2>&1; then
  printf 'native-bindings: wrong native namespace escaped the original source comparison\n' >&2
  exit 1
fi
rg -q 'FAIL .*original native namespace survives' "$proof/wrong-namespace-runner.log"
jq --argjson count "${#qualified_sources[@]}" --argjson compiled "${#qualified_objects[@]}" \
  --argjson status "$qualification_status" --slurpfile matrix "$proof/qualification-matrix.jsonl" \
  '.native_source_qualification={emitted_candidates:$count,compiled_candidates:$compiled,
    retained_failed_candidates:($count-$compiled),matrix:$matrix,production_generator_exit:$status,
    separate_library:true,no_pch:true,linux_as_needed_retained:true,library_drop_control:"rejected",
    wrong_namespace_control:"rejected",
    typed_and_recordref_shared:true,live_provider_proved:false,complete_declaration_proof:false}' \
  "$proof/result.json" > "$proof/result-qualification.json"
mv "$proof/result-qualification.json" "$proof/result.json"
jq -e --slurpfile raw "$proof/raw-inventory.json" \
  'length == $raw[0].summary.objects_by_kind.table and (map(.id) | unique | length) == length' \
  "$proof/tables.json" > /dev/null
sha256sum --check --status "$proof/originals.sha256"
sha256sum --check --status "$proof/source-inputs.sha256"
sha256sum --check --status "$proof/libraries.sha256"
python3 scripts/fetch_symbols.py --verify "$package" > "$proof/package-verified.json"
jq '{raw_native, statuses, selected_statuses, excluded_tables, non_table_objects,
  unexecuted_native_objects,original_page_fixture,native_source_qualification}' "$proof/result.json"
printf 'native-bindings: full source-counted table matrix; no provider, full-app or G1 proof; %s\n' "$proof"
jq -e 'all(.selected_tables[]; .status == "contract-pass")' "$proof/result.json" > /dev/null

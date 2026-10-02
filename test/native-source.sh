#!/usr/bin/env bash
set -euo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
build=${B:-"$root/build"}
fixtures="$root/test/native-source"
compiler=$(sed -n 's/^CMAKE_CXX_COMPILER:[^=]*=//p' "$build/CMakeCache.txt")
[[ -n "$compiler" ]] || { printf 'native-source: no configured compiler\n' >&2; exit 2; }
run=$(mktemp -d "$build/native-source.XXXXXX")
trap 'status=$?; printf "native-source: failed (exit %s); logs: %s\n" "$status" "$run" >&2; exit "$status"' ERR
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -I"$root/include")
cases=0

generate() {
  local folder=$1 input="$fixtures/al"
  [[ ! -d "$folder/al" ]] || input="$folder/al"
  shift
  "$build/agirutc" "$input" "$fixtures/apps.json" "$folder/generated" \
    --system-symbols "$folder/symbols" "$@" > "$folder/generation.log" 2>&1
}

expect() {
  rg -Fq -- "$2" "$1" || { printf 'native-source: missing diagnostic %s\n' "$2" >&2; exit 1; }
}

replace() {
  local file=$1 before=$2 after=$3 text prefix suffix
  text=$(< "$file")
  [[ "$text" == *"$before"* ]] || { printf 'native-source: missing mutant anchor\n' >&2; exit 1; }
  prefix=${text%%"$before"*}
  suffix=${text#*"$before"}
  [[ "$suffix" != *"$before"* ]] || { printf 'native-source: duplicate mutant anchor\n' >&2; exit 1; }
  printf '%s%s%s\n' "$prefix" "$after" "$suffix" > "$file"
}

prepare() {
  mkdir -p -- "$1"
  cp -R -- "$fixtures/symbols" "$1/symbols"
}

original="$run/original"
prepare "$original"
generate "$original"
expect "$original/generation.log" 'SYSTEM TABLES: 2 parsed from 2 AL files'
expect "$original/generation.log" 'SYSTEM TABLES: 2 bound, 0 unbound'

compile_consumer() {
  local folder=$1
  local -a sources
  mapfile -d '' sources < <(find "$folder/generated" -name '*.cpp' -print0 | sort -z)
  [[ "${#sources[@]}" -gt 0 ]] || { printf 'native-source: no generated consumers\n' >&2; exit 1; }
  "$compiler" "${flags[@]}" -I"$folder/generated" -I"$folder/generated/fixture" \
    --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
    -x c++ "$fixtures/Consumer.cpp.in" -x none "${sources[@]}" \
    -L"$build" -Wl,-rpath,"$build" -lagiru_rt -lagiru_al -lagiru_net -lagiru_db \
    -o "$folder/consumer" > "$folder/compile.log" 2>&1
}

compile_consumer "$original"
"$original/consumer"
cases=$((cases + 1))

property_mutant() {
  local file=$1 before=$2 after=$3 diagnostic=$4 folder
  cases=$((cases + 1))
  folder="$run/property-$cases"
  prepare "$folder"
  cp -R -- "$fixtures/al" "$folder/al"
  replace "$folder/al/fixture/$file" "$before" "$after"
  generate "$folder"
  compile_consumer "$folder"
  if "$folder/consumer" > "$folder/execution.log" 2>&1; then
    printf 'native-source: property mutant escaped the consumer\n' >&2
    exit 1
  fi
  expect "$folder/execution.log" "$diagnostic"
}

property_mutant FieldProperties.Table.al 'DataClassification = CustomerContent;' \
  'DataClassification = ToBeClassified;' "base field inherits its declaring table's classification"
property_mutant FieldProperties.Table.al 'DataClassification = AccountData;' \
  'DataClassification = SystemMetadata;' 'field classification overrides table classification'
property_mutant FieldProperties.Table.al 'SqlDataType = Integer;' \
  'SqlDataType = Variant;' 'Code retains SqlDataType Integer'
property_mutant FieldProperties.Table.al 'Access = Protected;' \
  'Access = Local;' 'field retains Protected access'
property_mutant FieldProperties.Table.al 'AllowInCustomizations = Never;' \
  'AllowInCustomizations = AsReadOnly;' 'field Never overrides table AsReadWrite'
property_mutant FieldProperties.Table.al 'OptimizeForTextSearch = true;' \
  'OptimizeForTextSearch = false;' 'normal field retains text search property'
property_mutant FieldProperties.TableExt.al 'AllowInCustomizations = Never;' \
  'AllowInCustomizations = AsReadWrite;' 'extension field inherits its own Never'
property_mutant FieldProperties.TableExt.al "Caption = 'Changed caption';" \
  "Caption = 'Original caption';" 'modify replaces the original caption'
property_mutant FieldProperties.TableExt.al \
  'TableRelation = if (ID = const(2)) "Field Properties Fixture".ID;' \
  'TableRelation = if (ID = const(3)) "Field Properties Fixture".ID;' \
  'the added conditional relation remains reachable'

mutant() {
  local table=$1 before=$2 after=$3 diagnostic=$4 stem folder unit
  cases=$((cases + 1))
  folder="$run/mutant-$cases"
  prepare "$folder"
  replace "$folder/symbols/src/$table.al" "$before" "$after"
  generate "$folder"
  stem=system/tooling/page/PageFieldsSelectionList
  [[ "$table" != Field ]] || stem=system/reflection/page/NativeFieldBindingFixture
  unit="$folder/generated/fixture/$stem.def.cpp"
  if "$compiler" "${flags[@]}" -I"$folder/generated" -I"$folder/generated/fixture" \
      -fsyntax-only "$unit" > "$folder/compile.log" 2>&1; then
    printf 'native-source: contract accepted mutant %s\n' "$cases" >&2
    exit 1
  fi
  expect "$folder/compile.log" "$diagnostic"
}

field_error='native field declaration mismatch'
key_error='native key declaration mismatch'
mutant PageTableField 'field(5; Caption; Text[80])' 'field(5; Caption; Text[81])' "$field_error"
mutant PageTableField 'field(5; Caption; Text[80])' 'field(5; Caption; Code[80])' "$field_error"
mutant PageTableField 'field(5; Caption; Text[80])' 'field(5; DifferentCaption; Text[80])' "$field_error"
mutant PageTableField 'field(5; Caption; Text[80])' 'field(4; Caption; Text[80])' "$field_error"
mutant PageTableField 'OptionMembers = New,Ready,Placed;' 'OptionMembers = New,Placed,Ready;' "$field_error"
mutant PageTableField '4912, 4988,' '4913, 4988,' "$field_error"
mutant PageTableField 'key(pk; "Page ID", Index)' 'key(pk; Index, "Page ID")' "$key_error"
mutant PageTableField $'key(pk; "Page ID", Index)\n        {' \
  $'key(pk; "Page ID", Index)\n        {\n            Clustered = false;' "$key_error"
mutant PageTableField 'DataPerCompany = false;' 'DataPerCompany = true;' 'native company scope mismatch'
mutant Field 'field(10; ExternalName; Text[100])' 'field(10; ExternalName; Text[101])' "$field_error"
mutant Field 'field(10; ExternalName; Text[100])' 'field(29; ExternalName; Text[100])' "$field_error"
mutant Field '31488, 31489,' '31488, 31490,' "$field_error"
mutant Field 'OptionMembers = Normal,FlowField,FlowFilter;' 'OptionMembers = Normal,FlowFilter,FlowField;' "$field_error"
mutant Field 'OptionMembers = No,Pending,Removed;' 'OptionMembers = No,Pending,Removed,Moved;' "$field_error"
mutant Field 'OptionMembers = Varchar,Integer,Variant,BigInteger;' 'OptionMembers = Varchar,Variant,Integer,BigInteger;' "$field_error"
mutant Field 'OptionMembers = CustomerContent,ToBeClassified,' 'OptionMembers = ToBeClassified,CustomerContent,' "$field_error"
mutant Field 'field(60; "App Package ID"; Guid)' 'field(60; "App Package ID"; Integer)' "$field_error"

for condition in missing symlink duplicate unbound_duplicate parse_error extra_argument; do
  folder="$run/$condition"
  prepare "$folder"
  extra=()
  case "$condition" in
    missing) mv -- "$folder/symbols/NavxManifest.xml" "$folder/removed-manifest.xml";;
    symlink) ln -s -- PageTableField.al "$folder/symbols/src/link.al";;
    duplicate) cp -- "$folder/symbols/src/PageTableField.al" "$folder/symbols/src/Another.al";;
    unbound_duplicate)
      printf 'table 2000000999 Unknown { fields { field(1; ID; Integer) {} } }\n' > "$folder/symbols/src/A.al"
      cp -- "$folder/symbols/src/A.al" "$folder/symbols/src/B.al";;
    parse_error) printf 'table 2000000999 Broken { fields {\n' > "$folder/symbols/src/Broken.al";;
    extra_argument) extra=(--ignored);;
  esac
  if generate "$folder" "${extra[@]}"; then
    printf 'native-source: invalid input accepted: %s\n' "$condition" >&2
    exit 1
  fi
  [[ ! -e "$folder/generated" ]] || { printf 'native-source: invalid input changed output\n' >&2; exit 1; }
  cases=$((cases + 1))
done

folder="$run/unbound"
prepare "$folder"
printf 'table 2000000999 Unknown {}\n' > "$folder/symbols/src/Unknown.al"
printf 'namespace System.Fixture;\n' > "$folder/symbols/src/Namespace.al"
generate "$folder"
expect "$folder/generation.log" 'SYSTEM TABLES: 3 parsed from 4 AL files'
expect "$folder/generation.log" 'SYSTEM TABLE UNBOUND: Unknown ID 2000000999 source src/Unknown.al'
expect "$folder/generation.log" 'SYSTEM SOURCE UNBOUND: src/Namespace.al'
expect "$folder/generation.log" 'SYSTEM TABLES: 2 bound, 1 unbound'
cases=$((cases + 1))
printf 'native-source: %s controls passed; original consumers execute without PCH\n' "$cases"

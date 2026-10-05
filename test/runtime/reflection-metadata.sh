#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-reflection-metadata.XXXXXX)
gate="$B/gate_ReflectionMetadataGate"
"$gate" > "$proof/current.log" 2>&1
indexed_gate="$B/gate_RecordRefGate"
"$indexed_gate" > "$proof/indexed-fields.log" 2>&1
field_gate="$B/gate_PlatformFieldGate"
"$field_gate" > "$proof/field-names.log" 2>&1
"$B/gate_PlatformSystemFieldsGate" > "$proof/system-profiles.log" 2>&1
if [ "${AGIRU_METADATA_ID_REFERENCE+x}" = x ]; then
  "$gate" "$AGIRU_METADATA_ID_REFERENCE" > "$proof/reference.log" 2>&1
  sha256sum "$AGIRU_METADATA_ID_REFERENCE" > "$proof/reference.sha256"
fi
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -fPIC -shared -Iinclude -Isrc/rt --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)

has_typed_dependencies() {
  rg -q '/(type|runtime)/|/c\+\+/v1/(vector|map|unordered_map|memory|format)([[:space:]]|$)' "$1"
}

for header in SystemFields Declare; do
  "$CXX" -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror -Iinclude \
    -include "meta/$header.h" -x c++ -M /dev/null > "$proof/$header.d"
done
if has_typed_dependencies "$proof/SystemFields.d"; then
  printf 'reflection-metadata: system-field identities depend on typed record machinery\n' >&2
  exit 1
fi
if ! has_typed_dependencies "$proof/Declare.d"; then
  printf 'reflection-metadata: typed declaration dependency control was not detected\n' >&2
  exit 1
fi

awk '
  /return field.reflectionName;/ {
    sub(/return field.reflectionName;/, "return field.name;"); changed++
  }
  { print }
  END { if (changed != 1) exit 2 }
' src/rt/written/PlatformField.cpp > "$proof/source-name.cpp"
"$CXX" "${flags[@]}" "$proof/source-name.cpp" -L"$B" -Wl,-rpath,"$B" \
  -lagiru_rt -lagiru_net -lagiru_db -o "$proof/source-name.so"
if LD_PRELOAD="$proof/source-name.so" "$field_gate" > "$proof/source-name.log" 2>&1; then
  printf 'reflection-metadata: source-name substitution escaped the caller-path gate\n' >&2
  exit 1
fi
rg -q 'FAIL .*Record.FieldName uses the original getter spelling' "$proof/source-name.log"
sha256sum "$proof/source-name.cpp" "$proof/source-name.so" > "$proof/name-control.sha256"
rm -- "$proof/source-name.cpp" "$proof/source-name.so"

for control in linked-audit old-profile-lookups; do
  mkdir -p "$proof/$control/meta"
  awk -v control="$control" '
    control == "linked-audit" && /return !linked &&/ {
      sub(/return !linked &&/, "static_cast<void>(linked); return"); changed++
    }
    control == "old-profile-lookups" && /profile == SystemFieldProfile::Runtime18/ {
      sub(/Runtime18/, "Runtime17"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' include/meta/SystemFields.h > "$proof/$control/meta/SystemFields.h"
  "$CXX" -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror \
    --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
    -I"$proof/$control" -Iinclude -Itest/gate test/gate/PlatformSystemFieldsGate.cpp \
    -L"$B" -Wl,-rpath,"$B" -lagiru_rt -lagiru_net -lagiru_db -o "$proof/$control/gate"
  if "$proof/$control/gate" > "$proof/$control.log" 2>&1; then
    printf 'reflection-metadata: %s escaped the complete profile gate\n' "$control" >&2
    exit 1
  fi
  rg -q 'FAIL .*complete selected identity population agrees' "$proof/$control.log"
  sha256sum "$proof/$control/meta/SystemFields.h" "$proof/$control/gate" \
    >> "$proof/profile-controls.sha256"
  rm -- "$proof/$control/meta/SystemFields.h" "$proof/$control/gate"
  rmdir "$proof/$control/meta" "$proof/$control"
done

awk '
  /if \(IsImplicitSystemField\(def.no\)\) \{ continue; \}/ {
    sub(/IsImplicitSystemField\(def.no\)/, "IsReservedSystemField(def.no)"); changed++
  }
  { print }
  END { if (changed != 1) exit 2 }
' src/rt/RecordRef.cpp > "$proof/indexed-timestamp.cpp"
"$CXX" "${flags[@]}" "$proof/indexed-timestamp.cpp" -L"$B" -Wl,-rpath,"$B" \
  -lagiru_rt -lagiru_net -lagiru_db -o "$proof/indexed-timestamp.so"
if LD_PRELOAD="$proof/indexed-timestamp.so" "$indexed_gate" \
    > "$proof/indexed-timestamp.log" 2>&1; then
  printf 'reflection-metadata: timestamp escaped the declared field index control\n' >&2
  exit 1
fi
rg -q 'FAIL .*FieldCount excludes timestamp' "$proof/indexed-timestamp.log"
sha256sum "$proof/indexed-timestamp.cpp" "$proof/indexed-timestamp.so" > "$proof/indexed-control.sha256"
rm -- "$proof/indexed-timestamp.cpp" "$proof/indexed-timestamp.so"

if [ "${AGIRU_METADATA_ID_REFERENCE+x}" = x ]; then
  "$CXX" -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror \
    --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Iinclude -Isrc/rt -Itest/gate test/gate/ReflectionMetadataGate.cpp \
    src/rt/MetadataSystemId.cpp -L"$B" -Wl,-rpath,"$B" \
    -lagiru_rt -lagiru_net -lagiru_db -o "$proof/sanitized"
  ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 \
    "$proof/sanitized" "$AGIRU_METADATA_ID_REFERENCE" > "$proof/sanitized.log" 2>&1
  sha256sum "$proof/sanitized" > "$proof/sanitized.sha256"
  rm -- "$proof/sanitized"
fi

for control in guid-byte-order guid-key-order; do
  awk -v control="$control" '
    control == "guid-byte-order" && /3, 2, 1, 0, 5, 4, 7, 6, 8, 9, 10, 11, 12, 13, 14, 15/ {
      sub(/3, 2, 1, 0, 5, 4, 7, 6/, "0, 1, 2, 3, 4, 5, 6, 7"); changed++
    }
    control == "guid-key-order" && /const std::array parts\{provider.Value\(\), id1, id2, id3\};/ {
      sub(/id1, id2, id3/, "id1, id3, id2"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/rt/MetadataSystemId.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
    -lagiru_rt -lagiru_net -lagiru_db -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$gate" > "$proof/$control.log" 2>&1; then
    printf 'reflection-metadata: %s escaped the stable identity gate\n' "$control" >&2
    exit 1
  fi
  rg -q 'FAIL ' "$proof/$control.log"
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done

for control in filter-union filter-flowfilter filter-empty; do
  awk -v control="$control" '
    control == "filter-union" && /crossColumnHit = crossColumnHit \|\| passes;/ {
      $0 = "      crossColumnHit = crossColumnHit && passes;"; changed++
    }
    control == "filter-flowfilter" && /field->fieldClass == FieldClass::FlowFilter/ {
      sub(/field->fieldClass == FieldClass::FlowFilter/, "false"); changed++
    }
    control == "filter-empty" && /\.expression = ParseFilter\(filter.text\)/ {
      sub(/ParseFilter\(filter.text\)/, "ParseFilter(\"\")"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' src/rt/RecordFilter.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
    -lagiru_rt -lagiru_net -lagiru_db -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$gate" > "$proof/$control.log" 2>&1; then
    printf 'reflection-metadata: %s escaped the shared compiled filter gate\n' "$control" >&2
    exit 1
  fi
  rg -q 'FAIL ' "$proof/$control.log"
done

for control in ordinal-cast cds-query cds-refusal default-fallback property-fallback property-ordinal scope-aliases; do
  awk -v control="$control" '
    control == "ordinal-cast" && /case PageType::HeadlinePart: return Native::HeadlinePart;/ {
      sub(/return Native::HeadlinePart;/, "return static_cast<Native>(type);"); changed++
    }
    (control == "cds-query" || control == "cds-refusal") && /case TableType::CDS: return Native::CRM;/ {
      if (control == "cds-query") print "    case TableType::CDS: return Native::Query;"
      else print "    case TableType::CDS: return std::unexpected(\"CDS refused\");"
      changed++; next
    }
    skipping { if (/;$/) skipping=0; next }
    control == "default-fallback" && /return std::unexpected\("unknown TableType/ {
      $0 = "  return Native::Normal;"; changed++
    }
    control == "property-fallback" && /return std::unexpected\("Table Metadata\."/ {
      print "  static_cast<void>(property); return static_cast<Native>(0);"; changed++; skipping=1; next
    }
    control == "property-ordinal" && /static_cast<Native>\(value.ordinal\)/ {
      sub(/static_cast<Native>\(value.ordinal\)/, "static_cast<Native>(value.ordinal + 1)"); changed++
    }
    control == "scope-aliases" && /SameProperty\(name, alias\)/ {
      $0 = "    static_cast<void>(alias); static_cast<void>(value);"; changed++
    }
    { print }
    END { if (changed != 1 || skipping) exit 2 }
  ' src/rt/ReflectionMetadata.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$gate" > "$proof/$control.log" 2>&1; then
    printf 'reflection-metadata: %s escaped the identity/refusal gate\n' "$control" >&2
    exit 1
  fi
  rg -q 'FAIL ' "$proof/$control.log"
done

for control in caption-names absent-caption-field null-owner wrong-access wrong-default-classification native-defaults declaration-company-scope null-system-id wrong-system-provider; do
  awk -v control="$control" '
    control == "caption-names" && /result \+= std::to_string\(no.Value\(\)\);/ {
      $0 = "    result += Field(source, no)->name;"; changed++
    }
    control == "absent-caption-field" && /if \(Field\(source, no\) == nullptr\)/ {
      $0 = "    if (false) {"; changed++
    }
    control == "null-owner" && /return \*owner;/ {
      $0 = "  return Guid{};"; changed++
    }
    control == "wrong-access" && /result.Access = Verified\(MetadataAccess\(EffectiveProperty/ {
      $0 = "  result.Access = platform::TableMetadataAccess::Public;"; changed++
    }
    control == "wrong-default-classification" && /EffectiveProperty\(source.dataClassification, "CustomerContent"\)/ {
      sub(/"CustomerContent"/, "\"ToBeClassified\""); changed++
    }
    control == "native-defaults" && /platform::TableMetadata_Table result;/ {
      print "  if (IsPlatformTable(source.id) && source.dataClassification.empty()) { throw Error(\"native omitted property\"); }"; changed++
    }
    control == "declaration-company-scope" && /result.DataPerCompany =/ {
      print "  result.DataPerCompany = source.dataPerCompany;"; changed++; skipping=1; next
    }
    skipping { if (/;$/) skipping=0; next }
    control == "null-system-id" && /result.SystemId = MetadataSystemId/ {
      $0 = "  result.SystemId = Guid{};"; changed++
    }
    control == "wrong-system-provider" && /result.SystemId = MetadataSystemId/ {
      sub(/platform::TableMetadata_Table::kId/, "source.id"); changed++
    }
    { print }
    END { if (changed != 1 || skipping) exit 2 }
  ' src/rt/TableMetadata.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
    -lagiru_rt -lagiru_net -lagiru_db -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$gate" > "$proof/$control.log" 2>&1; then
    printf 'reflection-metadata: %s escaped the source projection gate\n' "$control" >&2
    exit 1
  fi
  rg -q 'FAIL ' "$proof/$control.log"
done

awk '
  /^void RequireTableProvider\(const TableDef &table\) \{$/ {
    print "void RequireTableProvider(const TableDef &) {}"; skipping=1; changed++; next
  }
  skipping { if (/^}$/) skipping=0; next }
  { print }
  END { if (changed != 1 || skipping) exit 2 }
' src/rt/Storage.cpp > "$proof/UncheckedStorage.cpp"
"$CXX" "${flags[@]}" "$proof/UncheckedStorage.cpp" -L"$B" -Wl,-rpath,"$B" \
  -lagiru_rt -lagiru_net -lagiru_db -o "$proof/unchecked-storage.so"
if LD_PRELOAD="$proof/unchecked-storage.so" "$gate" > "$proof/unchecked-storage.log" 2>&1; then
  printf 'reflection-metadata: unqualified physical metadata access escaped the guard\n' >&2
  exit 1
fi
rg -q 'unqualified live metadata refuses' "$proof/unchecked-storage.log"
printf 'reflection-metadata: source projection, original field names, complete profile selection, stable identities, declared field indices, compiled filters, qualified defaults/company scope, CDS-to-CRM and temporary rows pass; twenty-six compiled controls and the typed-header dependency control refuse; %s\n' "$proof"

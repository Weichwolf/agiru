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
"$B/gate_GenSourceBindingGate" > "$proof/source-binding.log" 2>&1
if [ "${AGIRU_METADATA_ID_REFERENCE+x}" = x ]; then
  "$gate" "$AGIRU_METADATA_ID_REFERENCE" > "$proof/reference.log" 2>&1
  sha256sum "$AGIRU_METADATA_ID_REFERENCE" > "$proof/reference.sha256"
fi
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -fPIC -shared -Iinclude -Isrc/rt --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)
mapfile -t image_objects < <(find "$B/CMakeFiles/gate_image.dir/test/transpiler/golden" \
  -maxdepth 1 -type f -name '*.o' -print | LC_ALL=C sort)
[ "${#image_objects[@]}" -gt 0 ]

for control in public-owner assignment-state; do
  header=type/Variant.h
  if [ "$control" = assignment-state ]; then header=runtime/RecordState.h; fi
  mkdir -p "$proof/$control/$(dirname "$header")"
  awk -v control="$control" '
    /^class RecordInVariant \{/ {inside=1}
    control == "public-owner" && inside && /^private:/ {$0 = "public:"; changed++}
    inside && /^};/ {inside=0}
    control == "assignment-state" && /^  StateHandle &operator=\(const StateHandle &o\)/ {
      assigning=1
    }
    assigning && /if \(this == &o\) \{ return \*this; \}/ {
      $0 = "    CopyStateFrom(o);"; changed++; assigning=0
    }
    {print} END {if (changed != 1 || assigning) exit 2}' \
    "include/$header" > "$proof/$control/$header"
  status=0
  "$CXX" -O2 -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror \
    --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
    "-I$proof/$control" -Iinclude -Itest/gate -Itest/transpiler/golden -Isrc/rt \
    test/gate/RecordRefGate.cpp "${image_objects[@]}" \
    "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db \
    -o "$proof/$control/gate" > "$proof/$control.compile.log" 2>&1 || status=$?
  if [ "$control" = public-owner ]; then
    [ "$status" -eq 1 ]
    rg -q 'static assertion failed.*PublicRecordBoxAuthority' "$proof/$control.compile.log"
  else
    [ "$status" -eq 0 ]
    status=0
    "$proof/$control/gate" > "$proof/$control.log" 2>&1 || status=$?
    [ "$status" -eq 1 ]
    rg -q 'FAIL .*record field assignment does not replace the destination state' "$proof/$control.log"
  fi
  sha256sum "$proof/$control/$header" >> "$proof/ownership-controls.sha256"
  find "$proof/$control" -depth -delete
done

control=moved-get-key
mkdir -p "$proof/$control/runtime"
awk '
  /^[[:space:]]*key_\(std::move\(other.key_\)\),[[:space:]]*$/ { changed++; next }
  { print }
  END { if (changed != 1) exit 2 }
' include/runtime/Table.h > "$proof/$control/runtime/Table.h"
"$CXX" -O2 -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror \
  --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
  "-I$proof/$control" -Iinclude -Itest/gate -Itest/transpiler/golden -Isrc/rt \
  test/gate/PlatformFieldGate.cpp "${image_objects[@]}" \
  "-L$B" "-Wl,-rpath,$B" -lagiru_rt -lagiru_net -lagiru_db \
  -o "$proof/$control/gate" > "$proof/$control.compile.log" 2>&1
if "$proof/$control/gate" > "$proof/$control.log" 2>&1; then
  printf 'reflection-metadata: dropped read key escaped the move contract\n' >&2
  exit 1
fi
rg -q 'FAIL .*moved read failure retains exact key text' "$proof/$control.log"
sha256sum "$proof/$control/runtime/Table.h" "$proof/$control/gate" > "$proof/read-controls.sha256"
find "$proof/$control" -depth -delete

control=field-value-context
awk '
  /return \{false, kName, PrimaryKeyText\(\)\};/ {
    print "  detail::Found missing{false, kName, PrimaryKeyText()};";
    print "  static_cast<void>(static_cast<bool>(missing));";
    print "  return missing;"; changed++; next
  }
  { print }
  END { if (changed != 1) exit 2 }
' src/rt/written/PlatformField.cpp > "$proof/$control.cpp"
"$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
  -lagiru_rt -lagiru_net -lagiru_db -o "$proof/$control.so"
if LD_PRELOAD="$proof/$control.so" "$field_gate" > "$proof/$control.log" 2>&1; then
  printf 'reflection-metadata: silently consumed missing Field.Get escaped its contract\n' >&2
  exit 1
fi
rg -q 'FAIL .*a discarded native missing-field Get carries its searched key' "$proof/$control.log"
sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/read-controls.sha256"
rm -- "$proof/$control.cpp" "$proof/$control.so"

for control in field-access-default field-search-default field-customization-default field-customization-editable; do
  awk -v control="$control" '
    control == "field-access-default" && /row.Access = Option<platform::FieldAccess>/ {
      print "  row.Access = platform::FieldAccess::Public;"; changed++; next
    }
    control == "field-search-default" && /row.OptimizeForTextSearch = def.optimizeForTextSearch;/ {
      print "  row.OptimizeForTextSearch = false;"; changed++; next
    }
    control == "field-customization-default" && /row.IsAllowedInCustomizations = \*customizable != 0;/ {
      print "  row.IsAllowedInCustomizations = true;"; changed++; next
    }
    control == "field-customization-editable" && /row.IsAllowedInCustomizations = \*customizable != 0;/ {
      print "  row.IsAllowedInCustomizations = def.editable;"; changed++; next
    }
    { print } END { if (changed != 1) exit 2 }
  ' src/rt/written/PlatformField.cpp > "$proof/$control.cpp"
  "$CXX" "${flags[@]}" "$proof/$control.cpp" -L"$B" -Wl,-rpath,"$B" \
    -lagiru_rt -lagiru_net -lagiru_db -o "$proof/$control.so"
  if LD_PRELOAD="$proof/$control.so" "$field_gate" > "$proof/$control.log" 2>&1; then
    printf 'reflection-metadata: %s escaped declared field policies\n' "$control" >&2
    exit 1
  fi
  case "$control" in
    field-access-default) claim='metadata access retains the declared native member';;
    field-search-default) claim='metadata text search retains the declared flag';;
    field-customization-default|field-customization-editable) claim='metadata customization retains the declared availability';;
  esac
  rg -q "FAIL .*${claim}" "$proof/$control.log"
  sha256sum "$proof/$control.cpp" "$proof/$control.so" >> "$proof/policy-controls.sha256"
  rm -- "$proof/$control.cpp" "$proof/$control.so"
done

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

awk '
  /void ValidateImplicitAssignment\(/ { inside=1 }
  inside && /field.role == SystemFieldRole::Timestamp/ {
    sub(/SystemFieldRole::Timestamp/, "SystemFieldRole::Identity"); changed++
  }
  inside && /field.role == SystemFieldRole::AuditLookup/ {
    sub(/SystemFieldRole::AuditLookup/, "SystemFieldRole::Audit"); changed++
  }
  inside && /^  }$/ { inside=0 }
  { print }
  END { if (changed != 2 || inside) exit 2 }
' src/gen/BodyWriter.cpp > "$proof/writable-implicit-fields.cpp"
"$CXX" "${flags[@]}" -Isrc/gen -Isrc/al "$proof/writable-implicit-fields.cpp" \
  -L"$B" -Wl,-rpath,"$B" -lagiru_gen -lagiru_al -o "$proof/writable-implicit-fields.so"
write_status=0
LD_PRELOAD="$proof/writable-implicit-fields.so" "$B/gate_GenSourceBindingGate" \
  > "$proof/writable-implicit-fields.log" 2>&1 || write_status=$?
if [ "$write_status" -ne 1 ]; then
  printf 'reflection-metadata: implicit source writes were accepted or crashed (status %s)\n' "$write_status" >&2
  exit 1
fi
rg -q 'FAIL .*typed record writes to read-only implicit fields refuse' "$proof/writable-implicit-fields.log"
sha256sum "$proof/writable-implicit-fields.cpp" "$proof/writable-implicit-fields.so" \
  > "$proof/source-write-control.sha256"
rm -- "$proof/writable-implicit-fields.cpp" "$proof/writable-implicit-fields.so"

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
  compile_status=0
  "$CXX" -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror \
    --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
    -I"$proof/$control" -Iinclude -Itest/gate test/gate/PlatformSystemFieldsGate.cpp \
    -L"$B" -Wl,-rpath,"$B" -lagiru_rt -lagiru_net -lagiru_db -o "$proof/$control/gate" \
    > "$proof/$control.compile.log" 2>&1 || compile_status=$?
  if [ "$compile_status" -eq 0 ]; then
    if "$proof/$control/gate" > "$proof/$control.log" 2>&1; then
      printf 'reflection-metadata: %s escaped the complete profile gate\n' "$control" >&2
      exit 1
    fi
    rg -q 'FAIL .*complete selected identity population agrees' "$proof/$control.log"
    sha256sum "$proof/$control/gate" >> "$proof/profile-controls.sha256"
    rm -- "$proof/$control/gate"
  elif [ "$compile_status" -eq 1 ]; then
    case "$control" in
      linked-audit)
        rg -q "no member named 'SystemCreatedAt' in 'IdentityOnly'" "$proof/$control.compile.log";;
      old-profile-lookups)
        rg -q 'one-past-the-end|outside its lifetime' "$proof/$control.compile.log";;
    esac
  else
    printf 'reflection-metadata: unexpected mutant compiler status %s\n' "$compile_status" >&2
    exit 1
  fi
  sha256sum "$proof/$control/meta/SystemFields.h" >> "$proof/profile-controls.sha256"
  rm -- "$proof/$control/meta/SystemFields.h"
  rmdir "$proof/$control/meta" "$proof/$control"
done

for control in timestamp-offset stored-lookups creator-lookup-owner native-profile-downgrade; do
  case "$control" in
    timestamp-offset|stored-lookups) header=meta/Declare.h;;
    creator-lookup-owner) header=meta/SystemFields.h;;
    native-profile-downgrade) header=platform/TableMetadata.h;;
  esac
  mkdir -p "$proof/$control/$(dirname "$header")"
  awk -v control="$control" '
    control == "timestamp-offset" && /offsetof\(T, SystemRowVersion\)/ {
      sub(/offsetof\(T, SystemRowVersion\)/, "offsetof(T, SystemId)"); changed++
    }
    control == "stored-lookups" && /\.fieldClass = field.role == SystemFieldRole::AuditLookup/ {
      sub(/field.role == SystemFieldRole::AuditLookup/, "false"); changed++
    }
    control == "creator-lookup-owner" && !changed && /lookup\(User.*field\(SystemCreatedBy\)/ {
      sub(/field\(SystemCreatedBy\)/, "field(SystemModifiedBy)"); changed++
    }
    control == "native-profile-downgrade" && /SystemFieldProfile::Runtime18/ {
      sub(/SystemFieldProfile::Runtime18/, "SystemFieldProfile::Runtime17"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
  ' "include/$header" > "$proof/$control/$header"
  "$CXX" -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror \
    --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19 \
    -I"$proof/$control" -Iinclude -Itest/gate test/gate/PlatformSystemFieldsGate.cpp \
    -L"$B" -Wl,-rpath,"$B" -lagiru_rt -lagiru_net -lagiru_db -o "$proof/$control/gate" \
    > "$proof/$control.compile.log" 2>&1
  if "$proof/$control/gate" > "$proof/$control.log" 2>&1; then
    printf 'reflection-metadata: %s escaped materialized field qualification\n' "$control" >&2
    exit 1
  fi
  case "$control" in
    timestamp-offset) claim='materialized offsets reach the actual selected members';;
    stored-lookups) claim='lookups are nonstored FlowFields';;
    creator-lookup-owner) claim='lookup calculations retain the original SID owner and User field';;
    native-profile-downgrade) claim='original Table Metadata has 23 declared plus ten implicit fields';;
  esac
  rg -q "FAIL .*${claim}" "$proof/$control.log"
  sha256sum "$proof/$control/$header" "$proof/$control/gate" >> "$proof/materialization-controls.sha256"
  rm -- "$proof/$control/$header" "$proof/$control/gate"
  rmdir "$proof/$control/$(dirname "$header")" "$proof/$control"
done

awk '
  /^void RecordRef::Open\(Integer tableNo\) \{$/ {
    print "void RecordRef::Open(Integer) { Close(); }"; skipping=1; changed++; next
  }
  skipping { if (/^}$/) skipping=0; next }
  { print }
  END { if (changed != 1 || skipping) exit 2 }
' src/rt/RecordRef.cpp > "$proof/unopened-buffer.cpp"
"$CXX" "${flags[@]}" "$proof/unopened-buffer.cpp" -L"$B" -Wl,-rpath,"$B" \
  -lagiru_rt -lagiru_net -lagiru_db -o "$proof/unopened-buffer.so"
buffer_status=0
LD_PRELOAD="$proof/unopened-buffer.so" "$B/gate_PlatformSystemFieldsGate" \
  > "$proof/unopened-buffer.log" 2>&1 || buffer_status=$?
if [ "$buffer_status" -ne 1 ]; then
  printf 'reflection-metadata: an unopened buffer was accepted or crashed (status %s)\n' "$buffer_status" >&2
  exit 1
fi
rg -q 'RecordRef.GetTable: the opened table has no record buffer' "$proof/unopened-buffer.log"
sha256sum "$proof/unopened-buffer.cpp" "$proof/unopened-buffer.so" > "$proof/buffer-control.sha256"
rm -- "$proof/unopened-buffer.cpp" "$proof/unopened-buffer.so"

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
    control == "property-fallback" && /return std::unexpected\(std::string\(owner\) \+/ {
      print "  static_cast<void>(property); static_cast<void>(owner); return 0;";
      changed++; skipping=1; next
    }
    control == "property-ordinal" && /return value.ordinal;/ {
      sub(/return value.ordinal;/, "return value.ordinal + 1;"); changed++
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
  if [ "$control" = property-fallback ]; then
    if LD_PRELOAD="$proof/$control.so" "$field_gate" > "$proof/field-access-refusal.log" 2>&1; then
      printf 'reflection-metadata: unknown field access escaped the shared decoder\n' >&2
      exit 1
    fi
    rg -q 'FAIL .*unknown field access refuses rather than granting Public' "$proof/field-access-refusal.log"
    rg -q 'FAIL .*unknown field customization refuses rather than granting availability' "$proof/field-access-refusal.log"
  fi
done

for control in caption-names absent-caption-field null-owner wrong-access wrong-default-classification native-defaults declaration-company-scope null-system-id wrong-system-provider zero-metadata-version false-metadata-missing stored-metadata-flowfield; do
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
    control == "zero-metadata-version" && /result.SystemRowVersion = kFrozenCatalogueRowVersion;/ {
      $0 = "  result.SystemRowVersion = kFrozenCatalogueRowVersion - 1;"; changed++
    }
    control == "false-metadata-missing" && /if \(!row.has_value\(\)\) \{ return false; \}/ {
      $0 = "  if (!row.has_value()) { return true; }"; changed++
    }
    control == "stored-metadata-flowfield" && /if \(!Stored\(field\)\) \{ continue; \}/ {
      changed++; next
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
printf 'reflection-metadata: installed Table Metadata.Get through typed/RecordRef paths, source projection, original field names, selected materialized profiles, current User lookups, read-only AL assignment, stable identities, declared field indices, compiled filters, qualified defaults/company scope, CDS-to-CRM and temporary rows pass; forty-three compiled controls and the typed-header dependency control refuse; %s\n' "$proof"

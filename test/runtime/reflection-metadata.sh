#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
B=$(realpath "${B:-build}")
CXX=${CXX:-clang++-19}
proof=$(mktemp -d /tmp/agiru-reflection-metadata.XXXXXX)
gate="$B/gate_ReflectionMetadataGate"
"$gate" > "$proof/current.log" 2>&1
flags=(-std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror
  -fPIC -shared -Iinclude -Isrc/rt --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)

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

for control in ordinal-cast cds-query default-fallback property-fallback property-ordinal scope-aliases; do
  awk -v control="$control" '
    control == "ordinal-cast" && /case PageType::HeadlinePart: return Native::HeadlinePart;/ {
      sub(/return Native::HeadlinePart;/, "return static_cast<Native>(type);"); changed++
    }
    control == "cds-query" && /case TableType::CDS:/ {
      print "    case TableType::CDS: return Native::Query;"; changed++; skipping=1; next
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

for control in caption-names absent-caption-field null-owner wrong-access wrong-default-classification native-defaults; do
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
    control == "wrong-default-classification" && /EffectiveProperty\(source, source.dataClassification, "CustomerContent"\)/ {
      sub(/"CustomerContent"/, "\"ToBeClassified\""); changed++
    }
    control == "native-defaults" && /return value.empty\(\) && !IsPlatformTable\(source.id\)/ {
      sub(/!IsPlatformTable\(source.id\)/, "source.id.Value() != 0"); changed++
    }
    { print }
    END { if (changed != 1) exit 2 }
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
printf 'reflection-metadata: source projection, compiled filters, AL defaults and temporary rows pass; sixteen controls refuse; %s\n' "$proof"

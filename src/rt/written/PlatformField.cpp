#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
#include "platform/ReflectionOptions.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/StringValue.h"

#include "FieldMetadata.h"
#include "ReflectionMetadata.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace agiru {

std::string_view detail::ReflectionFieldName(const FieldDef &def) {
  if (!IsImplicitSystemField(def.no)) { return def.name; }
  for (const SystemFieldDecl &field : kImplicitSystemFields) {
    if (field.no == def.no) { return field.reflectionName; }
  }
  return def.name;
}

std::uint16_t detail::EffectiveFieldLength(const FieldDef &def) {
  constexpr std::uint16_t kIntegerLength = 4;
  constexpr std::uint16_t kBigIntegerLength = 8;
  constexpr std::uint16_t kDecimalLength = 12;
  constexpr std::uint16_t kGuidLength = 16;
  constexpr std::uint16_t kDateFormulaLength = 32;
  constexpr std::uint16_t kRecordIdLength = 448;
  constexpr std::uint16_t kTableFilterLength = 504;
  switch (def.type) {
    case FieldType::Text:
    case FieldType::Code: return def.length;
    case FieldType::Boolean:
    case FieldType::Integer:
    case FieldType::Option:
    case FieldType::Enum:
    case FieldType::Date:
    case FieldType::Time: return kIntegerLength;
    case FieldType::BigInteger:
    case FieldType::Duration:
    case FieldType::DateTime:
    case FieldType::Blob: return kBigIntegerLength;
    case FieldType::Decimal: return kDecimalLength;
    case FieldType::Guid:
    case FieldType::Media:
    case FieldType::MediaSet: return kGuidLength;
    case FieldType::DateFormula: return kDateFormulaLength;
    case FieldType::RecordId: return kRecordIdLength;
    case FieldType::TableFilter: return kTableFilterLength;
    default: throw Error("Field metadata: unsupported length");
  }
}

std::string detail::FieldOptionMembers(const FieldDef &def) {
  std::string out;
  bool first = true;
  for (const EnumValueDef &value : def.values) {
    if (!first) { out += ','; }
    first = false;
    out += value.name;
  }
  return out;
}

namespace {

Option<platform::FieldDataType> NativeFieldTypeOf(const FieldDef &def) {
  using Native = platform::FieldDataType;
  switch (def.type) {
    case FieldType::TableFilter: return Native::TableFilter;
    case FieldType::RecordId: return Native::RecordId;
    case FieldType::Date: return Native::Date;
    case FieldType::Time: return Native::Time;
    case FieldType::DateFormula: return Native::DateFormula;
    case FieldType::Decimal: return Native::Decimal;
    case FieldType::Media: return Native::Media;
    case FieldType::MediaSet: return Native::MediaSet;
    case FieldType::Text: return Native::Text;
    case FieldType::Code: return Native::Code;
    case FieldType::Blob: return Native::Blob;
    case FieldType::Boolean: return Native::Boolean;
    case FieldType::Integer: return Native::Integer;
    case FieldType::Option:
    case FieldType::Enum: return Native::Option;
    case FieldType::BigInteger: return Native::BigInteger;
    case FieldType::Duration: return Native::Duration;
    case FieldType::Guid: return Native::Guid;
    case FieldType::DateTime: return Native::DateTime;
    default: throw Error("Field metadata: unsupported type name");
  }
}

std::string TypeNameOf(const FieldDef &def, Option<platform::FieldDataType> native) {
  const FieldType type = def.type == FieldType::Enum ? FieldType::Option : def.type;
  std::string out{native.Name()};
  if (type == FieldType::Code || type == FieldType::Text) { out += std::to_string(def.length); }
  return out;
}

bool InPrimaryKey(const TableDef &table, FieldNo no) {
  if (table.keys.empty()) { return false; }
  return std::ranges::any_of(table.keys[0].fields,
                             [no](const FieldNo held) { return held.Value() == no.Value(); });
}

std::string_view FittedFieldText(std::string_view text, std::size_t length) {
  std::size_t end = detail::ByteOfUnit(text, length + 1);
  if (detail::Utf16Length(text.substr(0, end)) > length) { end = detail::ByteOfUnit(text, length); }
  return text.substr(0, end);
}

Option<platform::ObsoleteState> ObsoleteStateOf(const FieldDef &def) {
  const std::string_view name = def.obsoleteState.empty() ? "No" : def.obsoleteState;
  using State = platform::ObsoleteState;
  constexpr std::array values{
      Option<State>{State::No}, Option<State>{State::Pending}, Option<State>{State::Removed}};
  const auto *const value = std::ranges::find_if(
      values, [name](const Option<State> state) { return state.Name() == name; });
  if (value == values.end()) {
    throw Error("Field metadata: unsupported ObsoleteState " + std::string(name));
  }
  return *value;
}

}

void detail::LoadFieldMetadata(platform::Field &row, const TableDef &table, const FieldDef &def) {
  const auto obsoleteState = ObsoleteStateOf(def);
  const auto nativeType = NativeFieldTypeOf(def);
  const std::string typeName = TypeNameOf(def, nativeType);
  const auto access = MetadataPropertyOrdinal(OptionTraits<platform::FieldAccess>::kValues,
                                              def.access.empty() ? "Public" : def.access,
                                              "Access",
                                              "Field");
  if (!access) { throw Error(access.error()); }
  row.TableNo = table.id.Value();
  row.No = def.no.Value();
  row.TableName = FittedFieldText(table.name, platform::Field::kNameLength);
  row.FieldName = FittedFieldText(ReflectionFieldName(def), platform::Field::kNameLength);
  row.Type = nativeType;
  row.Len = EffectiveFieldLength(def);
  row.Class = def.fieldClass;
  row.TypeName = typeName;
  row.OptionString = FieldOptionMembers(def);
  row.RelationTableNo = RelationTableNo(&def);
  row.RelationFieldNo = RelationFieldNo(&def);
  row.ObsoleteState = obsoleteState;
  row.ObsoleteReason = FittedFieldText(def.obsoleteReason, platform::Field::kReasonLength);
  row.ExternalName = def.externalName;
  row.FieldCaption = FittedFieldText(def.caption.empty() ? def.name : def.caption,
                                     platform::Field::kCaptionLength);
  row.Enabled = def.enabled;
  row.IsPartOfPrimaryKey = InPrimaryKey(table, def.no);
  row.OptimizeForTextSearch = def.optimizeForTextSearch;
  row.Access = Option<platform::FieldAccess>{*access};
}

detail::Found platform::Field::Get(::agiru::Integer TableNo, ::agiru::Integer No) {
  if (IsTemporary()) { return Table<Field>::Get(TableNo, No); }
  AssignPrimaryKey(TableNo, No);
  const TableEntry *entry = FindTable(TableId{TableNo});
  if (entry != nullptr) {
    const auto wanted = std::ranges::find_if(
        entry->table->fields, [No](const FieldDef &def) { return def.no.Value() == No; });
    if (wanted != entry->table->fields.end()) {
      detail::LoadFieldMetadata(*this, *entry->table, *wanted);
      return {true, kName};
    }
  }
  return {false, kName, PrimaryKeyText()};
}

}

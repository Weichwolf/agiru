#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
#include "platform/ReflectionOptions.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/StringValue.h"

#include "FieldMetadata.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>

namespace agiru {

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
  const FieldType type = def.type == FieldType::Enum ? FieldType::Option : def.type;
  const std::string_view name = type == FieldType::TableFilter ? std::string_view{"TableFilter"}
                                                               : Option<FieldType>{type}.Name();
  const auto &values = OptionTraits<platform::FieldDataType>::kValues;
  const auto *const found = std::ranges::find(values, name, &EnumValueDef::name);
  if (name.empty() || found == values.end()) {
    throw Error("Field metadata: unsupported type name");
  }
  return Option<platform::FieldDataType>{found->ordinal};
}

std::string TypeNameOf(const FieldDef &def, Option<platform::FieldDataType> type) {
  std::string out{type.Name()};
  if (type == platform::FieldDataType::Code || type == platform::FieldDataType::Text) {
    out += std::to_string(def.length);
  }
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

bool SamePropertyName(std::string_view left, std::string_view right) {
  return std::ranges::equal(left, right, [](unsigned char a, unsigned char b) {
    return std::tolower(a) == std::tolower(b);
  });
}

template <typename Kind>
Option<Kind>
FieldPropertyOption(std::string_view name, std::string_view fallback, std::string_view property) {
  if (name.empty()) { name = fallback; }
  const auto &values = OptionTraits<Kind>::kValues;
  const auto *const value = std::ranges::find_if(
      values, [name](const EnumValueDef &held) { return SamePropertyName(held.name, name); });
  if (value == values.end()) {
    throw Error("Field metadata: unsupported " + std::string(property) + " " + std::string(name));
  }
  return Option<Kind>{value->ordinal};
}

bool AllowsCustomization(const FieldDef &def) {
  const std::string_view name = def.allowInCustomizations;
  if (SamePropertyName(name, "Never")) { return false; }
  constexpr std::array<std::string_view, 5> allowed{
      "", "ToBeClassified", "Always", "AsReadOnly", "AsReadWrite"};
  if (std::ranges::none_of(allowed, [name](auto held) { return SamePropertyName(name, held); })) {
    throw Error("Field metadata: unsupported AllowInCustomizations " + std::string(name));
  }
  return true;
}

}

void detail::LoadFieldMetadata(platform::Field &row, const TableDef &table, const FieldDef &def) {
  const auto obsoleteState = ObsoleteStateOf(def);
  const auto type = NativeFieldTypeOf(def);
  const std::string typeName = TypeNameOf(def, type);
  const auto classification = FieldPropertyOption<platform::FieldDataClassification>(
      def.fieldClass == ::agiru::FieldClass::Normal ? def.dataClassification : "SystemMetadata",
      "ToBeClassified",
      "DataClassification");
  const auto sqlDataType =
      FieldPropertyOption<platform::FieldSQLDataType>(def.sqlDataType, "Varchar", "SqlDataType");
  const auto access = FieldPropertyOption<platform::FieldAccess>(def.access, "Public", "Access");
  const bool allowsCustomization = AllowsCustomization(def);
  row.TableNo = table.id.Value();
  row.No = def.no.Value();
  row.TableName = FittedFieldText(table.name, platform::Field::kNameLength);
  row.FieldName = FittedFieldText(def.name, platform::Field::kNameLength);
  row.Type = type;
  row.Len = static_cast<::agiru::Integer>(def.length);
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
  row.DataClassification = classification;
  row.SQLDataType = sqlDataType;
  row.Access = access;
  row.OptimizeForTextSearch =
      def.fieldClass == ::agiru::FieldClass::Normal && def.optimizeForTextSearch;
  row.IsAllowedInCustomizations = allowsCustomization;
}

Boolean platform::Field::Get(::agiru::Integer TableNo, ::agiru::Integer No) {
  if (IsTemporary()) { return Table<Field>::Get(TableNo, No); }
  const TableEntry *entry = FindTable(TableId{TableNo});
  if (entry == nullptr) { return false; }
  const auto wanted = std::ranges::find_if(
      entry->table->fields, [No](const FieldDef &def) { return def.no.Value() == No; });
  if (wanted == entry->table->fields.end()) { return false; }
  detail::LoadFieldMetadata(*this, *entry->table, *wanted);
  return true;
}

}

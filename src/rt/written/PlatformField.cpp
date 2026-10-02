#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
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

std::string TypeNameOf(const FieldDef &def) {
  const FieldType type = def.type == FieldType::Enum ? FieldType::Option : def.type;
  const std::string_view name = type == FieldType::TableFilter ? std::string_view{"TableFilter"}
                                                               : Option<FieldType>{type}.Name();
  if (name.empty()) { throw Error("Field metadata: unsupported type name"); }
  std::string out{name};
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
  const std::string typeName = TypeNameOf(def);
  row.TableNo = table.id.Value();
  row.No = def.no.Value();
  row.TableName = FittedFieldText(table.name, platform::Field::kNameLength);
  row.FieldName = FittedFieldText(def.name, platform::Field::kNameLength);
  row.Type = def.type == FieldType::Enum ? FieldType::Option : def.type;
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

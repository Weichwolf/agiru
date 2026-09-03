#include "runtime/RecordRef.h"

#include "meta/EnumDef.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/Table.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/List.h"
#include "type/RecordId.h"
#include "type/StringValue.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace agiru {

namespace {

const std::byte *At(const void *record, const FieldDef &def) {
  return static_cast<const std::byte *>(record) + def.offset;
}

template <typename T> const T &As(const void *record, const FieldDef &def) {
  return *reinterpret_cast<const T *>(At(record, def));
}

}

FieldType FieldRef::Type() const {
  return def_->type == FieldType::Enum ? FieldType::Option : def_->type;
}

std::string_view FieldRef::GetEnumValueName(Integer index) const {
  if (index < 1 || static_cast<std::size_t>(index) > def_->values.size()) { return {}; }
  return def_->values[static_cast<std::size_t>(index) - 1].name;
}

Integer FieldRef::GetEnumValueOrdinal(Integer index) const {
  if (index < 1 || static_cast<std::size_t>(index) > def_->values.size()) { return 0; }
  return def_->values[static_cast<std::size_t>(index) - 1].ordinal;
}

std::string_view FieldRef::GetEnumValueNameFromOrdinalValue(Integer ordinal) const {
  const EnumValueDef *value = ValueOf(def_->values, ordinal);
  return value != nullptr ? value->name : std::string_view{};
}

List<std::string> FieldRef::OptionMembers() const {
  List<std::string> members;
  for (const EnumValueDef &value : def_->values) { members.Add(std::string(value.name)); }
  return members;
}

Variant FieldRef::Value() const {
  switch (def_->type) {
    case FieldType::Boolean: return Variant{As<Boolean>(record_, *def_)};
    case FieldType::Integer: return Variant{As<Integer>(record_, *def_)};
    case FieldType::BigInteger: return Variant{As<BigInteger>(record_, *def_)};
    case FieldType::Decimal: return Variant{As<Decimal>(record_, *def_)};
    case FieldType::Code:
    case FieldType::Text: return Variant{std::string(As<StringValue>(record_, *def_).Value())};
    case FieldType::Date: return Variant{As<Date>(record_, *def_)};
    case FieldType::Time: return Variant{As<Time>(record_, *def_)};
    case FieldType::DateTime: return Variant{As<DateTime>(record_, *def_)};
    case FieldType::Duration: return Variant{As<Duration>(record_, *def_)};
    case FieldType::Guid: return Variant{As<Guid>(record_, *def_)};
    case FieldType::RecordId: return Variant{As<RecordId>(record_, *def_)};
    case FieldType::DateFormula: return Variant{As<DateFormula>(record_, *def_)};
    case FieldType::Option:
    case FieldType::Enum: return Variant{Integer{As<OrdinalValue>(record_, *def_).AsInteger()}};
    case FieldType::Blob:
      throw Error("a Blob is not read with its record, so it has no value here (board:0017)");
    case FieldType::Media:
    case FieldType::MediaSet:
      throw Error("a Media is an object rather than a value, and a Variant holds no objects yet");
    case FieldType::TableFilter:
      throw Error("a TableFilter is a filter rather than a value (board:0018)");
  }
  throw Error("that field type has no value yet");
}

void FieldRef::SetValue(std::string_view text) {
  detail::SetFieldText(record_, *def_, text);
}

void FieldRef::TestField() const {
  agiru::detail::TestField(record_, *table_, def_->no);
}

// NOLINTBEGIN(readability-convert-member-functions-to-static)
RecordRef FieldRef::Record() const {
  throw Error("FieldRef.Record() is declared and not implemented yet (board:0035)");
}

// NOLINTEND(readability-convert-member-functions-to-static)

void RecordRef::Open(Integer tableNo) {
  Close();
  const TableEntry *entry = FindTable(TableId{tableNo});
  if (entry == nullptr) {
    throw Error("this installation carries no table " + std::to_string(tableNo));
  }
  owned_ = detail::SharedRecord(entry->make(), entry->free);
  record_ = owned_.Get();
  table_ = entry->table;
}

const TableDef &RecordRef::Table() const {
  if (table_ == nullptr) { throw Error("the RecordRef is not open"); }
  return *table_;
}

Integer RecordRef::Number() const {
  return Table().id.Value();
}

std::string_view RecordRef::Name() const {
  return Table().name;
}

Integer RecordRef::FieldCount() const {
  return static_cast<Integer>(Table().fields.size());
}

Integer RecordRef::KeyCount() const {
  return static_cast<Integer>(Table().keys.size());
}

FieldRef RecordRef::Field(Integer fieldNo) const {
  const FieldDef *def = agiru::Field(Table(), FieldNo{fieldNo});
  if (def == nullptr) { throw Error("the table declares no field " + std::to_string(fieldNo)); }
  return FieldRef{record_, Table(), *def};
}

FieldRef RecordRef::FieldIndex(Integer index) const {
  const TableDef &table = Table();
  if (index < 1 || static_cast<std::size_t>(index) > table.fields.size()) {
    throw Error("the field index " + std::to_string(index) + " is outside 1.." +
                std::to_string(table.fields.size()));
  }
  return FieldRef{record_, table, table.fields[static_cast<std::size_t>(index) - 1]};
}

bool RecordRef::FieldExist(Integer fieldNo) const {
  return table_ != nullptr && agiru::Field(*table_, FieldNo{fieldNo}) != nullptr;
}

}

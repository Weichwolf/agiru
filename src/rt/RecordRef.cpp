#include "runtime/RecordRef.h"

#include "meta/EnumDef.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
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
#include "type/KeyRef.h"
#include "type/RecordId.h"
#include "type/StringValue.h"
#include "type/Time.h"
#include "type/Variant.h"

#include "BuiltinsWritten.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace agiru {

namespace {

const std::byte *At(const void *record, const FieldDef &def) {
  return static_cast<const std::byte *>(record) + def.offset;
}

template <typename T> const T &As(const void *record, const FieldDef &def) {
  return *reinterpret_cast<const T *>(At(record, def));
}

}

::agiru::Boolean RecordRef::IsEmpty() {
  return detail::RuntimeIsEmpty(record_, Table());
}

::agiru::Integer RecordRef::Count() {
  return detail::RuntimeCount(record_, Table());
}

::agiru::Boolean RecordRef::Find(std::string_view Which) {
  return detail::RuntimeFind(record_, Table(), Which.empty() ? "=" : Which);
}

::agiru::Boolean RecordRef::FindFirst() {
  return detail::RuntimeFind(record_, Table(), "-");
}

std::string FieldRef::ToText() const {
  if (def_ == nullptr) { throw Error("this FieldRef names no field"); }
  return ::agiru::FieldText(record_, *def_);
}

::agiru::Boolean RecordRef::FindLast() {
  return detail::RuntimeFind(record_, Table(), "+");
}

::agiru::RecordId RecordRef::RecordId() const {
  const TableDef &table = Table();
  std::vector<std::string> key;
  if (!table.keys.empty()) {
    for (const FieldNo no : table.keys.front().fields) {
      const FieldDef *def = agiru::Field(table, no);
      if (def != nullptr) { key.push_back(::agiru::detail::StorageText(record_, *def)); }
    }
  }
  return ::agiru::RecordId{table.id, std::string(table.caption), std::move(key)};
}

void RecordRef::GetTable(Variant &rec) {
  const RecordInVariant *held = rec.HeldRecord();
  if (held == nullptr) { throw Error("RecordRef.GetTable: this Variant holds no record"); }
  const TableEntry *entry = FindTable(held->table);
  if (entry == nullptr) {
    throw Error("RecordRef.GetTable: table " + std::to_string(held->table.Value()) +
                " is not installed in this binary");
  }
  record_ = const_cast<void *>(held->record); // NOLINT(cppcoreguidelines-pro-type-const-cast)
  table_ = entry->table;
}

std::string RecordRef::GetFilters() const {
  if (record_ == nullptr) { return {}; }
  const detail::RecordState *state = reinterpret_cast<const detail::StateHandle *>(record_)->Peek();
  if (state == nullptr) { return {}; }
  std::vector<const detail::FieldFilter *> standing;
  for (const detail::FieldFilter &one : state->filters) {
    if (one.group == state->group && !one.text.empty()) { standing.push_back(&one); }
  }
  std::ranges::sort(standing, [](const detail::FieldFilter *a, const detail::FieldFilter *b) {
    return a->field.Value() < b->field.Value();
  });
  std::string out;
  for (const detail::FieldFilter *one : standing) {
    const FieldDef *def = ::agiru::Field(Table(), one->field);
    if (def == nullptr) { continue; }
    if (!out.empty()) { out += ", "; }
    out += def->caption;
    out += ": ";
    out += one->text;
  }
  return out;
}

::agiru::Boolean RecordRef::Insert() {
  detail::RuntimeInsert(record_, Table());
  return true;
}

::agiru::Boolean RecordRef::Insert(::agiru::Boolean RunTrigger) {
  static_cast<void>(RunTrigger);
  return Insert();
}

::agiru::Boolean RecordRef::Modify(::agiru::Boolean RunTrigger) {
  static_cast<void>(RunTrigger);
  if (!detail::RuntimeModify(record_, Table())) {
    throw Error("The " + std::string(Table().name) + " does not exist");
  }
  return true;
}

::agiru::Boolean RecordRef::Delete(::agiru::Boolean RunTrigger) {
  static_cast<void>(RunTrigger);
  if (!detail::RuntimeDelete(record_, Table())) {
    throw Error("The " + std::string(Table().name) + " does not exist");
  }
  return true;
}

constexpr ::agiru::Integer kInvariantFormat = 9;

std::string FieldRef::GetFilter() const {
  if (record_ == nullptr || def_ == nullptr) { return {}; }
  const detail::RecordState *state = reinterpret_cast<const detail::StateHandle *>(record_)->Peek();
  if (state == nullptr) { return {}; }
  for (const detail::FieldFilter &one : state->filters) {
    if (one.field == def_->no && one.group == state->group) { return one.text; }
  }
  return {};
}

void FieldRef::SetRange(const ::agiru::Variant &FromValue, const ::agiru::Variant &ToValue) const {
  if (record_ == nullptr || def_ == nullptr) {
    throw Error("FieldRef.SetRange: the FieldRef names no field yet");
  }
  detail::RecordState &state = reinterpret_cast<detail::StateHandle *>(record_)->Ensure();
  if (FromValue.IsEmpty()) {
    detail::Narrow(state, def_->no, {});
    return;
  }
  const std::string from = detail::Literally(Format(FromValue, 0, kInvariantFormat));
  if (ToValue.IsEmpty()) {
    detail::Narrow(state, def_->no, from);
    return;
  }
  detail::Narrow(
      state, def_->no, from + ".." + detail::Literally(Format(ToValue, 0, kInvariantFormat)));
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

std::string_view FieldRef::GetEnumValueCaption(Integer index) const {
  if (index < 1 || static_cast<std::size_t>(index) > def_->values.size()) { return {}; }
  return def_->values[static_cast<std::size_t>(index) - 1].caption;
}

std::string_view FieldRef::GetEnumValueCaptionFromOrdinalValue(Integer ordinal) const {
  const EnumValueDef *value = ValueOf(def_->values, ordinal);
  return value != nullptr ? value->caption : std::string_view{};
}

std::string FieldRef::OptionMembers() const {
  std::string members;
  for (const EnumValueDef &value : def_->values) {
    if (!members.empty()) { members += ','; }
    members += value.name;
  }
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

KeyRef RecordRef::KeyIndex(Integer Index) const {
  const TableDef &table = Table();
  if (Index < 1 || static_cast<std::size_t>(Index) > table.keys.size()) {
    throw Error("the key index " + std::to_string(Index) + " is outside 1.." +
                std::to_string(table.keys.size()));
  }
  return KeyRef{record_, table, table.keys[static_cast<std::size_t>(Index) - 1]};
}

Boolean KeyRef::Active() const {
  if (def_ == nullptr) { throw Error("this KeyRef selects no key"); }
  return def_->enabled;
}

Integer KeyRef::FieldCount() const {
  if (def_ == nullptr) { throw Error("this KeyRef selects no key"); }
  return static_cast<Integer>(def_->fields.size());
}

FieldRef KeyRef::FieldIndex(Integer Index) const {
  if (def_ == nullptr || table_ == nullptr) { throw Error("this KeyRef selects no key"); }
  if (Index < 1 || static_cast<std::size_t>(Index) > def_->fields.size()) {
    throw Error("the field index " + std::to_string(Index) + " is outside 1.." +
                std::to_string(def_->fields.size()) + " of key " + std::string(def_->name));
  }
  const FieldNo no = def_->fields[static_cast<std::size_t>(Index) - 1];
  const FieldDef *field = agiru::Field(*table_, no);
  if (field == nullptr) {
    throw Error("key " + std::string(def_->name) + " names field " + std::to_string(no.Value()) +
                ", which the table does not declare");
  }
  return FieldRef{record_, *table_, *field};
}

RecordRef KeyRef::Record() const {
  if (table_ == nullptr) { throw Error("this KeyRef selects no key"); }
  return RecordRef{record_, *table_};
}

bool RecordRef::FieldExist(Integer fieldNo) const {
  return table_ != nullptr && agiru::Field(*table_, FieldNo{fieldNo}) != nullptr;
}

}

#include "runtime/RecordRef.h"

#include "meta/EnumDef.h"
#include "meta/Ids.h"
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
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>
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
  return detail::RuntimeIsEmpty(State().record, Table());
}

std::string RecordRef::GetView(::agiru::Boolean UseNames) const {
  if (State().record == nullptr) { throw Error("RecordRef.GetView: the RecordRef is not open"); }
  return detail::ViewOf(reinterpret_cast<const detail::StateHandle *>(State().record)->Peek(),
                        Table(),
                        static_cast<bool>(UseNames));
}

void RecordRef::SetView(std::string_view String) {
  if (State().record == nullptr) { throw Error("RecordRef.SetView: the RecordRef is not open"); }
  detail::ApplyView(
      reinterpret_cast<detail::StateHandle *>(State().record)->Ensure(), Table(), String);
}

::agiru::SecurityFilter RecordRef::SecurityFiltering() const {
  if (State().record == nullptr) { return ::agiru::SecurityFilter::Validated; }
  const detail::RecordState *state =
      reinterpret_cast<const detail::StateHandle *>(State().record)->Peek();
  return state == nullptr ? ::agiru::SecurityFilter::Validated : state->securityFiltering;
}

::agiru::SecurityFilter
RecordRef::SecurityFiltering(const ::agiru::SecurityFilter &NewSecurityFiltering) {
  if (State().record == nullptr) {
    throw Error("RecordRef.SecurityFiltering: the RecordRef is not open");
  }
  detail::RecordState &state = reinterpret_cast<detail::StateHandle *>(State().record)->Ensure();
  const ::agiru::SecurityFilter was = state.securityFiltering;
  state.securityFiltering = NewSecurityFiltering;
  return was;
}

::agiru::Integer RecordRef::FilterGroup(::agiru::Integer NewGroup) {
  return detail::RuntimeFilterGroup(State().record, NewGroup);
}

void FieldRef::Validate(const ::agiru::Variant &NewValue) const {
  const TableEntry *entry = FindTable(table_->id);
  if (entry == nullptr || entry->validate == nullptr) {
    throw Error("FieldRef.Validate: this build carries no table " + std::string(table_->name));
  }
  const std::string text =
      NewValue.IsEmpty() ? FieldText(record_, *def_) : ::agiru::AsText(NewValue);
  entry->validate(record_, def_->no, text);
}

::agiru::Boolean RecordRef::IsTemporary() {
  return detail::RuntimeIsTemporary(State().record);
}

::agiru::Integer RecordRef::Count() {
  return detail::RuntimeCount(State().record, Table());
}

::agiru::Boolean RecordRef::Find(std::string_view Which) {
  return detail::RuntimeFind(State().record, Table(), Which.empty() ? "=" : Which);
}

::agiru::Boolean RecordRef::Get(::agiru::RecordId RecordID) {
  if (RecordID.IsEmpty()) { throw Error("RecordRef.Get: the RecordId names no record"); }
  if (State().table == nullptr || State().table->id.Value() != RecordID.TableNo()) {
    Open(RecordID.TableNo());
  }
  const TableDef &table = Table();
  const std::span<const std::string> values = RecordID.KeyValues();
  if (table.keys.empty() || values.size() != table.keys[0].fields.size()) {
    throw Error("RecordRef.Get: the RecordId carries " + std::to_string(values.size()) +
                " key value(s) and the primary key has " +
                std::to_string(table.keys.empty() ? 0 : table.keys[0].fields.size()));
  }
  for (std::size_t at = 0; at < values.size(); ++at) {
    const FieldDef *def = agiru::Field(table, table.keys[0].fields[at]);
    if (def == nullptr) {
      throw Error("RecordRef.Get: the primary key names a field the table lacks");
    }
    detail::SetFieldText(State().record, *def, values[at]);
  }
  return detail::RuntimeGet(State().record, table);
}

::agiru::Integer RecordRef::Next(::agiru::Integer Steps) {
  return detail::RuntimeNext(State().record, Table(), Steps == 0 ? 1 : Steps);
}

::agiru::Boolean RecordRef::FindSet() {
  return detail::RuntimeFindSet(State().record, Table());
}

void RecordRef::Reset() {
  if (State().record == nullptr) { throw Error("RecordRef.Reset: the RecordRef is not open"); }
  detail::RuntimeReset(State().record);
}

void RecordRef::SetRecFilter() {
  detail::RuntimeSetRecFilter(State().record, Table());
}

::agiru::Boolean RecordRef::FindFirst() {
  return detail::RuntimeFind(State().record, Table(), "-");
}

std::string FieldRef::ToText() const {
  if (def_ == nullptr) { throw Error("this FieldRef names no field"); }
  return ::agiru::FieldText(record_, *def_);
}

::agiru::Boolean RecordRef::FindLast() {
  return detail::RuntimeFind(State().record, Table(), "+");
}

::agiru::RecordId RecordRef::RecordId() const {
  const TableDef &table = Table();
  std::vector<std::string> key;
  if (!table.keys.empty()) {
    for (const FieldNo no : table.keys.front().fields) {
      const FieldDef *def = agiru::Field(table, no);
      if (def != nullptr) { key.push_back(::agiru::detail::StorageText(State().record, *def)); }
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
  Open(held->table.Value());
  entry->copy(State().record, held->record);
}

std::string RecordRef::GetFilters() const {
  if (State().record == nullptr) { return {}; }
  const detail::RecordState *state =
      reinterpret_cast<const detail::StateHandle *>(State().record)->Peek();
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
  if (!detail::RuntimeInsert(State().record, Table())) {
    throw Error("The " + std::string(Table().caption) + " already exists.");
  }
  return true;
}

::agiru::Boolean RecordRef::Insert(::agiru::Boolean RunTrigger) {
  static_cast<void>(RunTrigger);
  return Insert();
}

::agiru::Boolean RecordRef::Modify(::agiru::Boolean RunTrigger) {
  static_cast<void>(RunTrigger);
  if (!detail::RuntimeModify(State().record, Table())) {
    throw Error("The " + std::string(Table().name) + " does not exist");
  }
  return true;
}

::agiru::Boolean RecordRef::Delete(::agiru::Boolean RunTrigger) {
  static_cast<void>(RunTrigger);
  if (!detail::RuntimeDelete(State().record, Table())) {
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

::agiru::Variant FieldRef::RangeBound_(bool upper) const {
  if (record_ == nullptr || def_ == nullptr) { return {}; }
  const detail::RecordState *state = reinterpret_cast<const detail::StateHandle *>(record_)->Peek();
  const std::string text = detail::RangeBoundText(state, def_->no, upper);
  const TableEntry *entry = FindTable(table_->id);
  if (entry == nullptr) { return {}; }
  const std::unique_ptr<void, void (*)(void *)> bound(entry->make(), entry->free);
  if (!text.empty()) { detail::EvaluateInto(bound.get(), *table_, def_->no, text); }
  return FieldRef(bound.get(), *table_, *def_).Value();
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

void FieldRef::SetFilterText(const std::string &text) const {
  if (record_ == nullptr || def_ == nullptr) {
    throw Error("FieldRef.SetFilter: the FieldRef names no field yet");
  }
  detail::Narrow(reinterpret_cast<detail::StateHandle *>(record_)->Ensure(), def_->no, text);
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

RecordRef FieldRef::Record() const {
  throw Error("FieldRef.Record() is declared and not implemented yet (board:0035)");
}

void RecordRef::Open(Integer tableNo) {
  Close();
  const TableEntry *entry = FindTable(TableId{tableNo});
  if (entry == nullptr) {
    throw Error("this installation carries no table " + std::to_string(tableNo));
  }
  State().owned = detail::SharedRecord(entry->make(), entry->free);
  State().record = State().owned.Get();
  State().table = entry->table;
}

const TableDef &RecordRef::Table() const {
  if (State().table == nullptr) { throw Error("the RecordRef is not open"); }
  return *State().table;
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
  return FieldRef{State().record, Table(), *def};
}

FieldRef RecordRef::FieldIndex(Integer index) const {
  const TableDef &table = Table();
  if (index < 1 || static_cast<std::size_t>(index) > table.fields.size()) {
    throw Error("the field index " + std::to_string(index) + " is outside 1.." +
                std::to_string(table.fields.size()));
  }
  return FieldRef{State().record, table, table.fields[static_cast<std::size_t>(index) - 1]};
}

KeyRef RecordRef::KeyIndex(Integer Index) const {
  const TableDef &table = Table();
  if (Index < 1 || static_cast<std::size_t>(Index) > table.keys.size()) {
    throw Error("the key index " + std::to_string(Index) + " is outside 1.." +
                std::to_string(table.keys.size()));
  }
  return KeyRef{State().record, table, table.keys[static_cast<std::size_t>(Index) - 1]};
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
  return State().table != nullptr && agiru::Field(*State().table, FieldNo{fieldNo}) != nullptr;
}

}

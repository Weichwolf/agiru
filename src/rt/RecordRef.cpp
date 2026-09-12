#include "runtime/RecordRef.h"

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Catalogue.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/Session.h"
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

::agiru::Boolean RecordRef::Ascending() const {
  if (State().record == nullptr) { return true; }
  const detail::RecordState *held =
      reinterpret_cast<const detail::StateHandle *>(State().record)->Peek();
  return held == nullptr || held->ascending;
}

::agiru::Boolean RecordRef::Ascending(::agiru::Boolean SetAscending) {
  if (State().record == nullptr) { throw Error("RecordRef.Ascending: the RecordRef is not open"); }
  reinterpret_cast<detail::StateHandle *>(State().record)->Ensure().ascending = SetAscending;
  return SetAscending;
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
  const TableEntry *entry = FindTable(Table_().id);
  if (entry == nullptr || entry->validate == nullptr) {
    throw Error("FieldRef.Validate: this build carries no table " + std::string(Table_().name));
  }
  const std::string text = NewValue.IsEmpty()
                               ? FieldText(record_, Def_())
                               : std::string(std::string_view(::agiru::AsText(NewValue)));
  entry->validate(record_, Def_().no, text);
}

::agiru::Boolean RecordRef::IsTemporary() {
  return detail::RuntimeIsTemporary(State().record);
}

::agiru::Integer RecordRef::Count() {
  return detail::RuntimeCount(State().record, Table());
}

namespace {

std::string KeyTextOf(const void *record, const TableDef &table) {
  if (table.keys.empty()) { return {}; }
  std::string out;
  for (const FieldNo no : table.keys[0].fields) {
    const FieldDef *def = Field(table, no);
    if (def == nullptr) { continue; }
    if (!out.empty()) { out += ", "; }
    out += std::string(def->caption) + "='" + ::agiru::FieldText(record, *def) + "'";
  }
  return out;
}

}

detail::Found RecordRef::Find(std::string_view Which) {
  const std::string_view which = Which.empty() ? "=" : Which;
  const bool found = detail::RuntimeFind(State().record, Table(), which);
  if (which == "=") {
    return detail::Found{
        found, Table().name, found ? std::string{} : KeyTextOf(State().record, Table())};
  }
  return detail::Found{found, Table().name};
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

detail::Found RecordRef::FindSet() {
  return detail::Found{detail::RuntimeFindSet(State().record, Table()), Table().name};
}

void RecordRef::Reset() {
  if (State().record == nullptr) { throw Error("RecordRef.Reset: the RecordRef is not open"); }
  detail::RuntimeReset(State().record);
}

void RecordRef::SetRecFilter() {
  detail::RuntimeSetRecFilter(State().record, Table());
}

detail::Found RecordRef::FindFirst() {
  return detail::Found{detail::RuntimeFind(State().record, Table(), "-"), Table().name};
}

std::string FieldRef::ToText() const {
  if (def_ == nullptr) { throw Error("this FieldRef names no field"); }
  return ::agiru::FieldText(record_, Def_());
}

detail::Found RecordRef::FindLast() {
  return detail::Found{detail::RuntimeFind(State().record, Table(), "+"), Table().name};
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
  detail::RecordRefFromVariant(*this, rec);
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
  const TableEntry *entry = FindTable(Table().id);
  if (entry == nullptr || entry->insert == nullptr) { return Insert(); }
  if (!entry->insert(State().record, static_cast<bool>(RunTrigger))) {
    throw Error("The " + std::string(Table().caption) + " already exists.");
  }
  return true;
}

::agiru::Boolean RecordRef::Modify(::agiru::Boolean RunTrigger) {
  const TableEntry *entry = FindTable(Table().id);
  if (entry != nullptr && entry->modify != nullptr) {
    return entry->modify(State().record, static_cast<bool>(RunTrigger));
  }
  if (!detail::RuntimeModify(State().record, Table())) {
    throw Error("The " + std::string(Table().name) + " does not exist");
  }
  return true;
}

::agiru::Boolean RecordRef::Delete(::agiru::Boolean RunTrigger) {
  const TableEntry *entry = FindTable(Table().id);
  if (entry != nullptr && entry->remove != nullptr) {
    return entry->remove(State().record, static_cast<bool>(RunTrigger));
  }
  if (!detail::RuntimeDelete(State().record, Table())) {
    throw Error("The " + std::string(Table().name) + " does not exist");
  }
  return true;
}

::agiru::Boolean RecordRef::RenameKeys_(std::span<const ::agiru::Variant> keys) {
  const TableDef &table = Table();
  if (table.keys.empty() || keys.size() != table.keys[0].fields.size()) {
    throw Error("RecordRef.Rename: " + std::string(table.name) + " has a primary key of " +
                std::to_string(table.keys.empty() ? 0 : table.keys[0].fields.size()) +
                " field(s), and " + std::to_string(keys.size()) + " value(s) were given");
  }
  const TableEntry *entry = FindTable(table.id);
  if (entry == nullptr || entry->rename == nullptr || entry->copy == nullptr) {
    throw Error("RecordRef.Rename: this installation carries no " + std::string(table.name));
  }
  void *before = entry->make();
  try {
    entry->copy(before, State().record);
    std::size_t position = 0;
    for (const FieldNo no : table.keys[0].fields) { Field(no.Value()).Value(keys[position++]); }
    const bool renamed = entry->rename(State().record, before);
    entry->free(before);
    return renamed;
  } catch (...) {
    entry->free(before);
    throw;
  }
}

constexpr ::agiru::Integer kInvariantFormat = 9;

std::string FieldRef::GetFilter() const {
  if (record_ == nullptr || def_ == nullptr) { return {}; }
  const detail::RecordState *state = reinterpret_cast<const detail::StateHandle *>(record_)->Peek();
  if (state == nullptr) { return {}; }
  for (const detail::FieldFilter &one : state->filters) {
    if (one.field == Def_().no && one.group == state->group) { return one.text; }
  }
  return {};
}

::agiru::Variant FieldRef::RangeBound_(bool upper) const {
  if (record_ == nullptr || def_ == nullptr) { return {}; }
  const detail::RecordState *state = reinterpret_cast<const detail::StateHandle *>(record_)->Peek();
  const std::string text = detail::RangeBoundText(state, Def_().no, upper);
  const TableEntry *entry = FindTable(Table_().id);
  if (entry == nullptr) { return {}; }
  const std::unique_ptr<void, void (*)(void *)> bound(entry->make(), entry->free);
  if (!text.empty()) { detail::EvaluateInto(bound.get(), Table_(), Def_().no, text); }
  return FieldRef(bound.get(), Table_(), Def_()).Value();
}

void FieldRef::SetRange(const ::agiru::Variant &FromValue, const ::agiru::Variant &ToValue) const {
  if (record_ == nullptr || def_ == nullptr) {
    throw Error("FieldRef.SetRange: the FieldRef names no field yet");
  }
  detail::RecordState &state = reinterpret_cast<detail::StateHandle *>(record_)->Ensure();
  if (FromValue.IsEmpty()) {
    detail::Narrow(state, Def_().no, {});
    return;
  }
  const auto asFilter = [](const ::agiru::Variant &held) {
    return held.IsRecordId() ? held.Get<RecordId>().ToStorageText()
                             : std::string(Format(held, 0, kInvariantFormat));
  };
  const std::string from = detail::Literally(asFilter(FromValue));
  if (ToValue.IsEmpty()) {
    detail::Narrow(state, Def_().no, from);
    return;
  }
  detail::Narrow(state, Def_().no, from + ".." + detail::Literally(asFilter(ToValue)));
}

void FieldRef::SetFilterText(const std::string &text) const {
  if (record_ == nullptr || def_ == nullptr) {
    throw Error("FieldRef.SetFilter: the FieldRef names no field yet");
  }
  detail::Narrow(reinterpret_cast<detail::StateHandle *>(record_)->Ensure(), Def_().no, text);
}

FieldType FieldRef::Type() const {
  return Def_().type == FieldType::Enum ? FieldType::Option : Def_().type;
}

std::string_view FieldRef::GetEnumValueName(Integer index) const {
  if (index < 1 || static_cast<std::size_t>(index) > Def_().values.size()) { return {}; }
  return Def_().values[static_cast<std::size_t>(index) - 1].name;
}

Integer FieldRef::GetEnumValueOrdinal(Integer index) const {
  if (index < 1 || static_cast<std::size_t>(index) > Def_().values.size()) { return 0; }
  return Def_().values[static_cast<std::size_t>(index) - 1].ordinal;
}

std::string_view FieldRef::GetEnumValueNameFromOrdinalValue(Integer ordinal) const {
  const EnumValueDef *value = ValueOf(Def_().values, ordinal);
  return value != nullptr ? value->name : std::string_view{};
}

std::string_view FieldRef::GetEnumValueCaption(Integer index) const {
  if (index < 1 || static_cast<std::size_t>(index) > Def_().values.size()) { return {}; }
  return Def_().values[static_cast<std::size_t>(index) - 1].caption;
}

std::string_view FieldRef::GetEnumValueCaptionFromOrdinalValue(Integer ordinal) const {
  const EnumValueDef *value = ValueOf(Def_().values, ordinal);
  return value != nullptr ? value->caption : std::string_view{};
}

std::string FieldRef::OptionMembers() const {
  std::string members;
  for (const EnumValueDef &value : Def_().values) {
    if (!members.empty()) { members += ','; }
    members += value.name;
  }
  return members;
}

Variant FieldRef::Value() const {
  switch (Def_().type) {
    case FieldType::Boolean: return Variant{As<Boolean>(record_, Def_())};
    case FieldType::Integer: return Variant{As<Integer>(record_, Def_())};
    case FieldType::BigInteger: return Variant{As<BigInteger>(record_, Def_())};
    case FieldType::Decimal: return Variant{As<Decimal>(record_, Def_())};
    case FieldType::Code:
    case FieldType::Text: return Variant{std::string(As<StringValue>(record_, Def_()).Value())};
    case FieldType::Date: return Variant{As<Date>(record_, Def_())};
    case FieldType::Time: return Variant{As<Time>(record_, Def_())};
    case FieldType::DateTime: return Variant{As<DateTime>(record_, Def_())};
    case FieldType::Duration: return Variant{As<Duration>(record_, Def_())};
    case FieldType::Guid: return Variant{As<Guid>(record_, Def_())};
    case FieldType::RecordId: return Variant{As<RecordId>(record_, Def_())};
    case FieldType::DateFormula: return Variant{As<DateFormula>(record_, Def_())};
    case FieldType::Option:
    case FieldType::Enum:
      return Variant{OrdinalInVariant{.ordinal = As<OrdinalValue>(record_, Def_()).AsInteger(),
                                      .values = Def_().values}};
    case FieldType::Blob: return Variant{As<Blob>(record_, Def_())};
    case FieldType::Media:
    case FieldType::MediaSet:
      throw Error("a Media is an object rather than a value, and a Variant holds no objects yet");
    case FieldType::TableFilter:
      throw Error("a TableFilter is a filter rather than a value (board:0018)");
  }
  throw Error("that field type has no value yet");
}

void FieldRef::SetValue(std::string_view text) {
  detail::SetFieldText(record_, Def_(), text);
}

void FieldRef::Value(const ::agiru::Blob &blob) {
  if (Def_().type != FieldType::Blob) {
    throw Error("FieldRef.Value: " + std::string(Def_().name) + " is not a BLOB field");
  }
  *reinterpret_cast<Blob *>(static_cast<std::byte *>(record_) + Def_().offset) = blob;
}

void FieldRef::TestField() const {
  agiru::detail::TestField(record_, Table_(), Def_().no);
}

RecordRef FieldRef::Record() const {
  if (record_ == nullptr || table_ == nullptr) { throw Error("this FieldRef names no field"); }
  return RecordRef{record_, Table_()};
}

::agiru::Integer RecordRef::SystemIdNo() {
  return SystemFieldNumbers::SystemId.Value();
}

::agiru::IsolationLevel RecordRef::ReadIsolation(const ::agiru::IsolationLevel &ReadIsolation) {
  if (State().record == nullptr) {
    throw Error("RecordRef.ReadIsolation: the RecordRef is not open");
  }
  detail::RecordState &state = reinterpret_cast<detail::StateHandle *>(State().record)->Ensure();
  if (ReadIsolation != IsolationLevel::Default) { state.isolation = ReadIsolation; }
  return state.isolation;
}

void RecordRef::Init() {
  detail::RuntimeInit(State().record, Table());
}

std::string RecordRef::CurrentCompany() {
  return std::string(Session::Current().CompanyName());
}

void RecordRef::Copy(const RecordRef &FromRecordRef, Boolean ShareTable) {
  if (ShareTable) {
    *this = FromRecordRef;
    return;
  }
  const TableDef &table = FromRecordRef.Table();
  Open(table.id.Value());
  const TableEntry *entry = FindTable(table.id);
  if (entry == nullptr) { throw Error("the RecordRef is not open"); }
  entry->copy(State().record, FromRecordRef.State().record);
}

RecordRef RecordRef::Duplicate() {
  RecordRef copy;
  copy.Open(Table().id.Value());
  const TableEntry *entry = FindTable(Table().id);
  if (entry == nullptr) { throw Error("the RecordRef is not open"); }
  entry->copy(copy.State().record, State().record);
  return copy;
}

Boolean FieldRef::CalcField() const {
  detail::CalcField(
      record_, Table_(), reinterpret_cast<const detail::StateHandle *>(record_)->Peek(), Def_().no);
  return true;
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

namespace {

std::vector<const FieldDef *> IndexedFields(const TableDef &table) {
  std::vector<const FieldDef *> indexed;
  indexed.reserve(table.fields.size());
  if (!table.keys.empty()) {
    for (const FieldNo no : table.keys[0].fields) {
      for (const FieldDef &def : table.fields) {
        if (def.no == no) { indexed.push_back(&def); }
      }
    }
  }
  for (const FieldDef &def : table.fields) {
    if (def.no.Value() >= kSystemFields.front().no.Value()) { continue; }
    if (std::ranges::find(indexed, &def) == indexed.end()) { indexed.push_back(&def); }
  }
  return indexed;
}

}

Integer RecordRef::FieldCount() const {
  return static_cast<Integer>(IndexedFields(Table()).size());
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
  const std::vector<const FieldDef *> indexed = IndexedFields(table);
  if (index < 1 || static_cast<std::size_t>(index) > indexed.size()) {
    throw Error("the field index " + std::to_string(index) + " is outside 1.." +
                std::to_string(indexed.size()));
  }
  return FieldRef{State().record, table, *indexed[static_cast<std::size_t>(index) - 1]};
}

Integer RecordRef::CurrentKeyIndex(Integer NewKeyIndex) {
  if (State().record == nullptr || State().table == nullptr) { return -1; }
  const TableDef &table = Table();
  detail::RecordState &state = reinterpret_cast<detail::StateHandle *>(State().record)->Ensure();
  if (NewKeyIndex != 0) {
    if (NewKeyIndex < 1 || static_cast<std::size_t>(NewKeyIndex) > table.keys.size()) {
      throw Error("the key index " + std::to_string(NewKeyIndex) + " is outside 1.." +
                  std::to_string(table.keys.size()));
    }
    state.key.clear();
    for (const FieldNo no : table.keys[static_cast<std::size_t>(NewKeyIndex) - 1].fields) {
      state.key.push_back(detail::SortField{.field = no, .ascending = true});
    }
    return NewKeyIndex;
  }
  if (state.key.empty()) { return table.keys.empty() ? -1 : 1; }
  for (std::size_t i = 0; i < table.keys.size(); ++i) {
    const KeyDef &key = table.keys[i];
    if (key.fields.size() != state.key.size()) { continue; }
    bool same = true;
    for (std::size_t f = 0; f < key.fields.size() && same; ++f) {
      same = key.fields[f].Value() == state.key[f].field.Value();
    }
    if (same) { return static_cast<Integer>(i + 1); }
  }
  return -1;
}

void RecordRef::SetTable(Variant &Rec) {
  if (State().record == nullptr || State().table == nullptr) {
    throw Error("RecordRef.SetTable: the RecordRef is not open");
  }
  if (!Rec.IsRecord()) { throw Error("RecordRef.SetTable(Variant): the Variant holds no record"); }
  const RecordInVariant &record = Rec.Get<RecordInVariant>();
  if (record.table != State().table->id) {
    throw Error("RecordRef.SetTable: the RecordRef refers to " + std::string(State().table->name) +
                " and the Variant holds a record of table " + std::to_string(record.table.Value()));
  }
  const TableEntry *entry = FindTable(record.table);
  if (entry == nullptr) {
    throw Error("RecordRef.SetTable: the record's table " + std::to_string(record.table.Value()) +
                " is not translated in this build");
  }
  entry->copy(record.record, State().record);
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

namespace detail {

::agiru::Integer RelationTableNo(const FieldDef *def) {
  if (def == nullptr || def->relationTable.empty()) { return 0; }
  const TableEntry *entry = FindTable(def->relationTable);
  return entry == nullptr ? 0 : entry->table->id.Value();
}

::agiru::Integer RelationFieldNo(const FieldDef *def) {
  if (def == nullptr || def->relationTable.empty() || def->relationField.empty()) { return 0; }
  const TableEntry *entry = FindTable(def->relationTable);
  if (entry == nullptr) { return 0; }
  for (const FieldDef &field : entry->table->fields) {
    if (field.name.size() == def->relationField.size() &&
        std::equal(
            field.name.begin(),
            field.name.end(),
            def->relationField.begin(),
            [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); })) {
      return field.no.Value();
    }
  }
  return 0;
}

void RecordRefFromVariant(RecordRef &into, const Variant &held) {
  if (held.IsRecordRef()) {
    into.Copy(static_cast<const RecordRef &>(held));
    return;
  }
  if (!held.IsRecord()) {
    throw Error("RecordRef.GetTable(Variant): the Variant holds neither a record nor a RecordRef");
  }
  const RecordInVariant &record = held.Get<RecordInVariant>();
  const TableEntry *entry = FindTable(record.table);
  if (entry == nullptr) {
    throw Error("RecordRef.GetTable: the record's table " + std::to_string(record.table.Value()) +
                " is not translated in this build");
  }
  into.Open(record.table.Value());
  entry->copy(into.State().record, record.record);
  if (RuntimeIsTemporary(record.record)) {
    RuntimeAdoptTemporary(into.State().record, record.record);
  }
}

}

}

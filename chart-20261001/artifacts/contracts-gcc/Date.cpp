#include "platform/Date.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::Date>::kTable.id == ::agiru::TableId{2000000007} && ::agiru::TableTraits<::agiru::platform::Date>::kTable.name == "Date", "native table identity mismatch: Date");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::Date>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 6;
}(), "native field count mismatch: Date");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Date>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "Period Type" || field->caption != "Period Type" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 5) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Date" || field->values[0].caption != "Date") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Week" || field->values[1].caption != "Week") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "Month" || field->values[2].caption != "Month") { return false; }
  if (field->values[3].ordinal != 3 || field->values[3].name != "Quarter" || field->values[3].caption != "Quarter") { return false; }
  if (field->values[4].ordinal != 4 || field->values[4].name != "Year" || field->values[4].caption != "Year") { return false; }
  return true;
}(), "native field declaration mismatch: Date.Period Type");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Date>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "Period Start" || field->caption != "Period Start" || field->type != ::agiru::FieldType::Date || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Date.Period Start");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Date>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "Period End" || field->caption != "Period End" || field->type != ::agiru::FieldType::Date || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Date.Period End");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Date>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "Period No." || field->caption != "Period No." || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Date.Period No.");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Date>::kTable, ::agiru::FieldNo{5});
  if (field == nullptr || field->name != "Period Name" || field->caption != "Period Name" || field->type != ::agiru::FieldType::Text || field->length != 31) { return false; }
  return true;
}(), "native field declaration mismatch: Date.Period Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Date>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "Period Invariant Name" || field->caption != "Period Invariant Name" || field->type != ::agiru::FieldType::Text || field->length != 31) { return false; }
  return true;
}(), "native field declaration mismatch: Date.Period Invariant Name");
static_assert(::agiru::TableTraits<::agiru::platform::Date>::kTable.keys.size() == 1, "native key count mismatch: Date");
static_assert(::agiru::TableTraits<::agiru::platform::Date>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::Date>::kTable.keys[0].name == "pk" && ::agiru::TableTraits<::agiru::platform::Date>::kTable.keys[0].fields.size() == 2 && ::agiru::TableTraits<::agiru::platform::Date>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::Date>::kTable.keys[0].fields[1] == ::agiru::FieldNo{2} && ::agiru::TableTraits<::agiru::platform::Date>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::Date>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::Date>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::Date>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::Date>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::Date>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::Date>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::Date>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::Date>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: Date.pk");
static_assert(::agiru::TableTraits<::agiru::platform::Date>::kTable.dataPerCompany == false, "native company scope mismatch: Date");


#include "platform/Chart.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::Chart>::kTable.id == ::agiru::TableId{2000000078} && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.name == "Chart", "native table identity mismatch: Chart");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::Chart>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 3;
}(), "native field count mismatch: Chart");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Chart>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "ID" || field->caption != "ID" || field->type != ::agiru::FieldType::Code || field->length != 20) { return false; }
  return true;
}(), "native field declaration mismatch: Chart.ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Chart>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "Name" || field->caption != "Name" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: Chart.Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Chart>::kTable, ::agiru::FieldNo{9});
  if (field == nullptr || field->name != "BLOB" || field->caption != "BLOB" || field->type != ::agiru::FieldType::Blob || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Chart.BLOB");
static_assert(::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys.size() == 1, "native key count mismatch: Chart");
static_assert(::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].fields[0] == ::agiru::FieldNo{3} && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: Chart.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::Chart>::kTable.dataPerCompany == false, "native company scope mismatch: Chart");


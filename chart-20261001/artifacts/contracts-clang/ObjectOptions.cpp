#include "platform/ObjectOptions.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.id == ::agiru::TableId{2000000196} && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.name == "Object Options", "native table identity mismatch: Object Options");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 9;
}(), "native field count mismatch: Object Options");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "Parameter Name" || field->caption != "Parameter Name" || field->type != ::agiru::FieldType::Text || field->length != 50) { return false; }
  return true;
}(), "native field declaration mismatch: Object Options.Parameter Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "Object ID" || field->caption != "Object ID" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Object Options.Object ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "Object Type" || field->caption != "Object Type" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 20) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "" || field->values[0].caption != "") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "" || field->values[1].caption != "") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "" || field->values[2].caption != "") { return false; }
  if (field->values[3].ordinal != 3 || field->values[3].name != "Report" || field->values[3].caption != "Report") { return false; }
  if (field->values[4].ordinal != 4 || field->values[4].name != "" || field->values[4].caption != "") { return false; }
  if (field->values[5].ordinal != 5 || field->values[5].name != "" || field->values[5].caption != "") { return false; }
  if (field->values[6].ordinal != 6 || field->values[6].name != "XMLport" || field->values[6].caption != "XMLport") { return false; }
  if (field->values[7].ordinal != 7 || field->values[7].name != "" || field->values[7].caption != "") { return false; }
  if (field->values[8].ordinal != 8 || field->values[8].name != "Page" || field->values[8].caption != "\"Page\"") { return false; }
  if (field->values[9].ordinal != 9 || field->values[9].name != "" || field->values[9].caption != "") { return false; }
  if (field->values[10].ordinal != 10 || field->values[10].name != "" || field->values[10].caption != "") { return false; }
  if (field->values[11].ordinal != 11 || field->values[11].name != "" || field->values[11].caption != "") { return false; }
  if (field->values[12].ordinal != 12 || field->values[12].name != "" || field->values[12].caption != "") { return false; }
  if (field->values[13].ordinal != 13 || field->values[13].name != "" || field->values[13].caption != "") { return false; }
  if (field->values[14].ordinal != 14 || field->values[14].name != "" || field->values[14].caption != "") { return false; }
  if (field->values[15].ordinal != 15 || field->values[15].name != "" || field->values[15].caption != "") { return false; }
  if (field->values[16].ordinal != 16 || field->values[16].name != "" || field->values[16].caption != "") { return false; }
  if (field->values[17].ordinal != 17 || field->values[17].name != "" || field->values[17].caption != "") { return false; }
  if (field->values[18].ordinal != 18 || field->values[18].name != "" || field->values[18].caption != "") { return false; }
  if (field->values[19].ordinal != 19 || field->values[19].name != "" || field->values[19].caption != "") { return false; }
  return true;
}(), "native field declaration mismatch: Object Options.Object Type");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "Company Name" || field->caption != "Company Name" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: Object Options.Company Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable, ::agiru::FieldNo{5});
  if (field == nullptr || field->name != "User Name" || field->caption != "User Name" || field->type != ::agiru::FieldType::Code || field->length != 50) { return false; }
  return true;
}(), "native field declaration mismatch: Object Options.User Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "Option Data" || field->caption != "Option Data" || field->type != ::agiru::FieldType::Blob || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Object Options.Option Data");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable, ::agiru::FieldNo{7});
  if (field == nullptr || field->name != "Public Visible" || field->caption != "Public Visible" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Object Options.Public Visible");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable, ::agiru::FieldNo{8});
  if (field == nullptr || field->name != "Temporary" || field->caption != "Temporary" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Object Options.Temporary");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable, ::agiru::FieldNo{9});
  if (field == nullptr || field->name != "Created By" || field->caption != "Created By" || field->type != ::agiru::FieldType::Code || field->length != 50) { return false; }
  return true;
}(), "native field declaration mismatch: Object Options.Created By");
static_assert(::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys.size() == 1, "native key count mismatch: Object Options");
static_assert(::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].fields.size() == 5 && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].fields[1] == ::agiru::FieldNo{2} && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].fields[2] == ::agiru::FieldNo{3} && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].fields[3] == ::agiru::FieldNo{5} && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].fields[4] == ::agiru::FieldNo{4} && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: Object Options.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::ObjectOptions>::kTable.dataPerCompany == false, "native company scope mismatch: Object Options");


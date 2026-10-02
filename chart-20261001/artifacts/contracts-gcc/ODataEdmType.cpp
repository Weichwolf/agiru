#include "platform/ODataEdmType.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.id == ::agiru::TableId{2000000179} && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.name == "OData Edm Type", "native table identity mismatch: OData Edm Type");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 3;
}(), "native field count mismatch: OData Edm Type");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "Key" || field->caption != "Key" || field->type != ::agiru::FieldType::Code || field->length != 50) { return false; }
  return true;
}(), "native field declaration mismatch: OData Edm Type.Key");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "Description" || field->caption != "Description" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: OData Edm Type.Description");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable, ::agiru::FieldNo{10});
  if (field == nullptr || field->name != "Edm Xml" || field->caption != "Edm Xml" || field->type != ::agiru::FieldType::Blob || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: OData Edm Type.Edm Xml");
static_assert(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys.size() == 1, "native key count mismatch: OData Edm Type");
static_assert(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: OData Edm Type.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.dataPerCompany == false, "native company scope mismatch: OData Edm Type");


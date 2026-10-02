#include "platform/Field.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::Field>::kTable.id == ::agiru::TableId{2000000041} && ::agiru::TableTraits<::agiru::platform::Field>::kTable.name == "Field", "native table identity mismatch: Field");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::Field>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 24;
}(), "native field count mismatch: Field");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "TableNo" || field->caption != "TableNo" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Field.TableNo");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "No." || field->caption != "No." || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Field.No.");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "TableName" || field->caption != "TableName" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: Field.TableName");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "FieldName" || field->caption != "FieldName" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: Field.FieldName");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{5});
  if (field == nullptr || field->name != "Type" || field->caption != "Type" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 21) { return false; }
  if (field->values[0].ordinal != 4912 || field->values[0].name != "TableFilter" || field->values[0].caption != "TableFilter") { return false; }
  if (field->values[1].ordinal != 4988 || field->values[1].name != "RecordID" || field->values[1].caption != "RecordID") { return false; }
  if (field->values[2].ordinal != 11519 || field->values[2].name != "OemText" || field->values[2].caption != "OemText") { return false; }
  if (field->values[3].ordinal != 11775 || field->values[3].name != "Date" || field->values[3].caption != "Date") { return false; }
  if (field->values[4].ordinal != 11776 || field->values[4].name != "Time" || field->values[4].caption != "Time") { return false; }
  if (field->values[5].ordinal != 11797 || field->values[5].name != "DateFormula" || field->values[5].caption != "DateFormula") { return false; }
  if (field->values[6].ordinal != 12799 || field->values[6].name != "Decimal" || field->values[6].caption != "Decimal") { return false; }
  if (field->values[7].ordinal != 26207 || field->values[7].name != "Media" || field->values[7].caption != "Media") { return false; }
  if (field->values[8].ordinal != 26208 || field->values[8].name != "MediaSet" || field->values[8].caption != "MediaSet") { return false; }
  if (field->values[9].ordinal != 31488 || field->values[9].name != "Text" || field->values[9].caption != "Text") { return false; }
  if (field->values[10].ordinal != 31489 || field->values[10].name != "Code" || field->values[10].caption != "Code") { return false; }
  if (field->values[11].ordinal != 33791 || field->values[11].name != "Binary" || field->values[11].caption != "Binary") { return false; }
  if (field->values[12].ordinal != 33793 || field->values[12].name != "BLOB" || field->values[12].caption != "BLOB") { return false; }
  if (field->values[13].ordinal != 34047 || field->values[13].name != "Boolean" || field->values[13].caption != "Boolean") { return false; }
  if (field->values[14].ordinal != 34559 || field->values[14].name != "Integer" || field->values[14].caption != "Integer") { return false; }
  if (field->values[15].ordinal != 35071 || field->values[15].name != "OemCode" || field->values[15].caption != "OemCode") { return false; }
  if (field->values[16].ordinal != 35583 || field->values[16].name != "Option" || field->values[16].caption != "Option") { return false; }
  if (field->values[17].ordinal != 36095 || field->values[17].name != "BigInteger" || field->values[17].caption != "BigInteger") { return false; }
  if (field->values[18].ordinal != 36863 || field->values[18].name != "Duration" || field->values[18].caption != "Duration") { return false; }
  if (field->values[19].ordinal != 37119 || field->values[19].name != "GUID" || field->values[19].caption != "GUID") { return false; }
  if (field->values[20].ordinal != 37375 || field->values[20].name != "DateTime" || field->values[20].caption != "DateTime") { return false; }
  return true;
}(), "native field declaration mismatch: Field.Type");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "Len" || field->caption != "Len" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Field.Len");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{7});
  if (field == nullptr || field->name != "Class" || field->caption != "Class" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 3) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Normal" || field->values[0].caption != "Normal") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "FlowField" || field->values[1].caption != "FlowField") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "FlowFilter" || field->values[2].caption != "FlowFilter") { return false; }
  return true;
}(), "native field declaration mismatch: Field.Class");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{8});
  if (field == nullptr || field->name != "Enabled" || field->caption != "Enabled" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Field.Enabled");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{9});
  if (field == nullptr || field->name != "Type Name" || field->caption != "Type Name" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: Field.Type Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{10});
  if (field == nullptr || field->name != "ExternalName" || field->caption != "ExternalName" || field->type != ::agiru::FieldType::Text || field->length != 100) { return false; }
  return true;
}(), "native field declaration mismatch: Field.ExternalName");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{20});
  if (field == nullptr || field->name != "Field Caption" || field->caption != "Field Caption" || field->type != ::agiru::FieldType::Text || field->length != 80) { return false; }
  return true;
}(), "native field declaration mismatch: Field.Field Caption");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{21});
  if (field == nullptr || field->name != "RelationTableNo" || field->caption != "RelationTableNo" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Field.RelationTableNo");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{22});
  if (field == nullptr || field->name != "RelationFieldNo" || field->caption != "RelationFieldNo" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Field.RelationFieldNo");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{23});
  if (field == nullptr || field->name != "SQLDataType" || field->caption != "SQLDataType" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 4) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Varchar" || field->values[0].caption != "Varchar") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Integer" || field->values[1].caption != "Integer") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "Variant" || field->values[2].caption != "Variant") { return false; }
  if (field->values[3].ordinal != 3 || field->values[3].name != "BigInteger" || field->values[3].caption != "BigInteger") { return false; }
  return true;
}(), "native field declaration mismatch: Field.SQLDataType");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{24});
  if (field == nullptr || field->name != "OptionString" || field->caption != "OptionString" || field->type != ::agiru::FieldType::Text || field->length != 2047) { return false; }
  return true;
}(), "native field declaration mismatch: Field.OptionString");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{25});
  if (field == nullptr || field->name != "ObsoleteState" || field->caption != "ObsoleteState" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 3) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "No" || field->values[0].caption != "No") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Pending" || field->values[1].caption != "Pending") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "Removed" || field->values[2].caption != "Removed") { return false; }
  return true;
}(), "native field declaration mismatch: Field.ObsoleteState");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{26});
  if (field == nullptr || field->name != "ObsoleteReason" || field->caption != "ObsoleteReason" || field->type != ::agiru::FieldType::Text || field->length != 248) { return false; }
  return true;
}(), "native field declaration mismatch: Field.ObsoleteReason");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{27});
  if (field == nullptr || field->name != "DataClassification" || field->caption != "DataClassification" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 7) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "CustomerContent" || field->values[0].caption != "CustomerContent") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "ToBeClassified" || field->values[1].caption != "ToBeClassified") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "EndUserIdentifiableInformation" || field->values[2].caption != "EndUserIdentifiableInformation") { return false; }
  if (field->values[3].ordinal != 3 || field->values[3].name != "AccountData" || field->values[3].caption != "AccountData") { return false; }
  if (field->values[4].ordinal != 4 || field->values[4].name != "EndUserPseudonymousIdentifiers" || field->values[4].caption != "EndUserPseudonymousIdentifiers") { return false; }
  if (field->values[5].ordinal != 5 || field->values[5].name != "OrganizationIdentifiableInformation" || field->values[5].caption != "OrganizationIdentifiableInformation") { return false; }
  if (field->values[6].ordinal != 6 || field->values[6].name != "SystemMetadata" || field->values[6].caption != "SystemMetadata") { return false; }
  return true;
}(), "native field declaration mismatch: Field.DataClassification");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{28});
  if (field == nullptr || field->name != "IsPartOfPrimaryKey" || field->caption != "IsPartOfPrimaryKey" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Field.IsPartOfPrimaryKey");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{60});
  if (field == nullptr || field->name != "App Package ID" || field->caption != "App Package ID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Field.App Package ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{61});
  if (field == nullptr || field->name != "App Runtime Package ID" || field->caption != "App Runtime Package ID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Field.App Runtime Package ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{62});
  if (field == nullptr || field->name != "OptimizeForTextSearch" || field->caption != "OptimizeForTextSearch" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Field.OptimizeForTextSearch");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{63});
  if (field == nullptr || field->name != "Access" || field->caption != "Access" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 4) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Public" || field->values[0].caption != "Public") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Internal" || field->values[1].caption != "Internal") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "Protected" || field->values[2].caption != "Protected") { return false; }
  if (field->values[3].ordinal != 3 || field->values[3].name != "Local" || field->values[3].caption != "Local") { return false; }
  return true;
}(), "native field declaration mismatch: Field.Access");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Field>::kTable, ::agiru::FieldNo{64});
  if (field == nullptr || field->name != "IsAllowedInCustomizations" || field->caption != "IsAllowedInCustomizations" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Field.IsAllowedInCustomizations");
static_assert(::agiru::TableTraits<::agiru::platform::Field>::kTable.keys.size() == 1, "native key count mismatch: Field");
static_assert(::agiru::TableTraits<::agiru::platform::Field>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::Field>::kTable.keys[0].name == "pk" && ::agiru::TableTraits<::agiru::platform::Field>::kTable.keys[0].fields.size() == 2 && ::agiru::TableTraits<::agiru::platform::Field>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::Field>::kTable.keys[0].fields[1] == ::agiru::FieldNo{2} && ::agiru::TableTraits<::agiru::platform::Field>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::Field>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::Field>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::Field>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::Field>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::Field>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::Field>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::Field>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::Field>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: Field.pk");
static_assert(::agiru::TableTraits<::agiru::platform::Field>::kTable.dataPerCompany == true, "native company scope mismatch: Field");


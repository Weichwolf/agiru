#include "platform/TableMetadata.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.id == ::agiru::TableId{2000000136} && ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.name == "Table Metadata", "native table identity mismatch: Table Metadata");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 23;
}(), "native field count mismatch: Table Metadata");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "ID" || field->caption != "ID" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "Name" || field->caption != "Name" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "Caption" || field->caption != "Caption" || field->type != ::agiru::FieldType::Text || field->length != 80) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.Caption");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "DataPerCompany" || field->caption != "DataPerCompany" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.DataPerCompany");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{5});
  if (field == nullptr || field->name != "LookupPageID" || field->caption != "LookupPageID" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.LookupPageID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "DrillDownPageId" || field->caption != "DrillDownPageId" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.DrillDownPageId");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{7});
  if (field == nullptr || field->name != "DataCaptionFields" || field->caption != "DataCaptionFields" || field->type != ::agiru::FieldType::Text || field->length != 80) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.DataCaptionFields");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{8});
  if (field == nullptr || field->name != "PasteIsValid" || field->caption != "PasteIsValid" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.PasteIsValid");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{9});
  if (field == nullptr || field->name != "LinkedObject" || field->caption != "LinkedObject" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.LinkedObject");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{10});
  if (field == nullptr || field->name != "DataIsExternal" || field->caption != "DataIsExternal" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.DataIsExternal");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{11});
  if (field == nullptr || field->name != "TableType" || field->caption != "TableType" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 7) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Normal" || field->values[0].caption != "Normal") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "CRM" || field->values[1].caption != "CRM") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "ExternalSQL" || field->values[2].caption != "ExternalSQL") { return false; }
  if (field->values[3].ordinal != 3 || field->values[3].name != "Exchange" || field->values[3].caption != "Exchange") { return false; }
  if (field->values[4].ordinal != 4 || field->values[4].name != "MicrosoftGraph" || field->values[4].caption != "MicrosoftGraph") { return false; }
  if (field->values[5].ordinal != 5 || field->values[5].name != "Query" || field->values[5].caption != "Query") { return false; }
  if (field->values[6].ordinal != 6 || field->values[6].name != "Temporary" || field->values[6].caption != "Temporary") { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.TableType");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{12});
  if (field == nullptr || field->name != "ExternalName" || field->caption != "ExternalName" || field->type != ::agiru::FieldType::Text || field->length != 248) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.ExternalName");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{13});
  if (field == nullptr || field->name != "ObsoleteState" || field->caption != "ObsoleteState" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 3) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "No" || field->values[0].caption != "No") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Pending" || field->values[1].caption != "Pending") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "Removed" || field->values[2].caption != "Removed") { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.ObsoleteState");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{14});
  if (field == nullptr || field->name != "ObsoleteReason" || field->caption != "ObsoleteReason" || field->type != ::agiru::FieldType::Text || field->length != 248) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.ObsoleteReason");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{15});
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
}(), "native field declaration mismatch: Table Metadata.DataClassification");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{16});
  if (field == nullptr || field->name != "ReplicateData" || field->caption != "ReplicateData" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.ReplicateData");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{17});
  if (field == nullptr || field->name != "CompressionType" || field->caption != "CompressionType" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 4) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Unspecified" || field->values[0].caption != "Unspecified") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "None" || field->values[1].caption != "None") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "Row" || field->values[2].caption != "Row") { return false; }
  if (field->values[3].ordinal != 3 || field->values[3].name != "Page" || field->values[3].caption != "Page") { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.CompressionType");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{18});
  if (field == nullptr || field->name != "App ID" || field->caption != "App ID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.App ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{19});
  if (field == nullptr || field->name != "InherentPermissions" || field->caption != "InherentPermissions" || field->type != ::agiru::FieldType::Text || field->length != 5) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.InherentPermissions");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{20});
  if (field == nullptr || field->name != "InherentEntitlements" || field->caption != "InherentEntitlements" || field->type != ::agiru::FieldType::Text || field->length != 5) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.InherentEntitlements");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{21});
  if (field == nullptr || field->name != "Scope" || field->caption != "Scope" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 2) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Cloud" || field->values[0].caption != "Cloud") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "OnPrem" || field->values[1].caption != "OnPrem") { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.Scope");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{22});
  if (field == nullptr || field->name != "Access" || field->caption != "Access" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 2) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Public" || field->values[0].caption != "Public") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Internal" || field->values[1].caption != "Internal") { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.Access");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable, ::agiru::FieldNo{23});
  if (field == nullptr || field->name != "AL Namespace" || field->caption != "AL Namespace" || field->type != ::agiru::FieldType::Text || field->length != 500) { return false; }
  return true;
}(), "native field declaration mismatch: Table Metadata.AL Namespace");
static_assert(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys.size() == 1, "native key count mismatch: Table Metadata");
static_assert(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys[0].name == "ID" && ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: Table Metadata.ID");
static_assert(::agiru::TableTraits<::agiru::platform::TableMetadata>::kTable.dataPerCompany == false, "native company scope mismatch: Table Metadata");


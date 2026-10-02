#include "platform/PageMetadata.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.id == ::agiru::TableId{2000000138} && ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.name == "Page Metadata", "native table identity mismatch: Page Metadata");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 32;
}(), "native field count mismatch: Page Metadata");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "ID" || field->caption != "ID" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "Name" || field->caption != "Name" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "Caption" || field->caption != "Caption" || field->type != ::agiru::FieldType::Text || field->length != 80) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.Caption");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "Editable" || field->caption != "Editable" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.Editable");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{5});
  if (field == nullptr || field->name != "PageType" || field->caption != "PageType" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 13) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Card" || field->values[0].caption != "Card") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "List" || field->values[1].caption != "List") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "RoleCenter" || field->values[2].caption != "RoleCenter") { return false; }
  if (field->values[3].ordinal != 3 || field->values[3].name != "CardPart" || field->values[3].caption != "CardPart") { return false; }
  if (field->values[4].ordinal != 4 || field->values[4].name != "ListPart" || field->values[4].caption != "ListPart") { return false; }
  if (field->values[5].ordinal != 5 || field->values[5].name != "Document" || field->values[5].caption != "Document") { return false; }
  if (field->values[6].ordinal != 6 || field->values[6].name != "Worksheet" || field->values[6].caption != "Worksheet") { return false; }
  if (field->values[7].ordinal != 7 || field->values[7].name != "ListPlus" || field->values[7].caption != "ListPlus") { return false; }
  if (field->values[8].ordinal != 8 || field->values[8].name != "ConfirmationDialog" || field->values[8].caption != "ConfirmationDialog") { return false; }
  if (field->values[9].ordinal != 9 || field->values[9].name != "NavigatePage" || field->values[9].caption != "NavigatePage") { return false; }
  if (field->values[10].ordinal != 10 || field->values[10].name != "StandardDialog" || field->values[10].caption != "StandardDialog") { return false; }
  if (field->values[11].ordinal != 11 || field->values[11].name != "API" || field->values[11].caption != "API") { return false; }
  if (field->values[12].ordinal != 12 || field->values[12].name != "HeadlinePart" || field->values[12].caption != "HeadlinePart") { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.PageType");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "CardPageID" || field->caption != "CardPageID" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.CardPageID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{7});
  if (field == nullptr || field->name != "DataCaptionExpr." || field->caption != "DataCaptionExpr." || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.DataCaptionExpr.");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{8});
  if (field == nullptr || field->name != "RefreshOnActivate" || field->caption != "RefreshOnActivate" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.RefreshOnActivate");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{9});
  if (field == nullptr || field->name != "APIPublisher" || field->caption != "APIPublisher" || field->type != ::agiru::FieldType::Text || field->length != 40) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.APIPublisher");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{10});
  if (field == nullptr || field->name != "APIGroup" || field->caption != "APIGroup" || field->type != ::agiru::FieldType::Text || field->length != 40) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.APIGroup");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{11});
  if (field == nullptr || field->name != "APIVersion" || field->caption != "APIVersion" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.APIVersion");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{12});
  if (field == nullptr || field->name != "EntitySetName" || field->caption != "EntitySetName" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.EntitySetName");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{13});
  if (field == nullptr || field->name != "EntityName" || field->caption != "EntityName" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.EntityName");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{14});
  if (field == nullptr || field->name != "SourceTable" || field->caption != "SourceTable" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.SourceTable");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{15});
  if (field == nullptr || field->name != "SourceTableView" || field->caption != "SourceTableView" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.SourceTableView");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{16});
  if (field == nullptr || field->name != "InsertAllowed" || field->caption != "InsertAllowed" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.InsertAllowed");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{17});
  if (field == nullptr || field->name != "ModifyAllowed" || field->caption != "ModifyAllowed" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.ModifyAllowed");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{18});
  if (field == nullptr || field->name != "DeleteAllowed" || field->caption != "DeleteAllowed" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.DeleteAllowed");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{19});
  if (field == nullptr || field->name != "DelayedInsert" || field->caption != "DelayedInsert" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.DelayedInsert");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{20});
  if (field == nullptr || field->name != "ShowFilter" || field->caption != "ShowFilter" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.ShowFilter");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{21});
  if (field == nullptr || field->name != "MultipleNewLines" || field->caption != "MultipleNewLines" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.MultipleNewLines");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{22});
  if (field == nullptr || field->name != "SaveValues" || field->caption != "SaveValues" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.SaveValues");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{23});
  if (field == nullptr || field->name != "AutoSplitKey" || field->caption != "AutoSplitKey" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.AutoSplitKey");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{24});
  if (field == nullptr || field->name != "DataCaptionFields" || field->caption != "DataCaptionFields" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.DataCaptionFields");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{25});
  if (field == nullptr || field->name != "SourceTableTemporary" || field->caption != "SourceTableTemporary" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.SourceTableTemporary");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{26});
  if (field == nullptr || field->name != "LinksAllowed" || field->caption != "LinksAllowed" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.LinksAllowed");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{27});
  if (field == nullptr || field->name != "ChangeTrackingAllowed" || field->caption != "ChangeTrackingAllowed" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.ChangeTrackingAllowed");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{28});
  if (field == nullptr || field->name != "PopulateAllFields" || field->caption != "PopulateAllFields" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.PopulateAllFields");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{29});
  if (field == nullptr || field->name != "App ID" || field->caption != "App ID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.App ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{30});
  if (field == nullptr || field->name != "InherentPermissions" || field->caption != "InherentPermissions" || field->type != ::agiru::FieldType::Text || field->length != 5) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.InherentPermissions");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{31});
  if (field == nullptr || field->name != "InherentEntitlements" || field->caption != "InherentEntitlements" || field->type != ::agiru::FieldType::Text || field->length != 5) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.InherentEntitlements");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable, ::agiru::FieldNo{32});
  if (field == nullptr || field->name != "AL Namespace" || field->caption != "AL Namespace" || field->type != ::agiru::FieldType::Text || field->length != 500) { return false; }
  return true;
}(), "native field declaration mismatch: Page Metadata.AL Namespace");
static_assert(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys.size() == 1, "native key count mismatch: Page Metadata");
static_assert(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys[0].name == "pk" && ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: Page Metadata.pk");
static_assert(::agiru::TableTraits<::agiru::platform::PageMetadata>::kTable.dataPerCompany == false, "native company scope mismatch: Page Metadata");


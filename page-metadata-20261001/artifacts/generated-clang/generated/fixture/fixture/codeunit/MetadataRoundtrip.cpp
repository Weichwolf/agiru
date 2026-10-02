// Generated from Metadata.Codeunit.al. Do not edit.

#include "MetadataRoundtrip.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "platform/PageMetadata.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/Record.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/List.h"
#include "type/Option.h"

#include "platform/PageMetadata.h"
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

#include "platform/PageMetadata.h"

namespace agiru::Fixture {

namespace {
namespace MetadataRoundtrip_unit {
const RegisterCodeunit<MetadataRoundtrip_Codeunit> kInCodeunitCatalogue;
} // namespace MetadataRoundtrip_unit
} // namespace

::agiru::Integer MetadataRoundtrip_Codeunit::Exercise() {
  [[maybe_unused]] Temporary<::agiru::platform::PageMetadata> Metadata{};
  [[maybe_unused]] ::agiru::RecordRef Reflected{};
  [[maybe_unused]] ::agiru::Integer Ordinal{};
  [[maybe_unused]] Guid Identifier{};

  Metadata.PageType = ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::Card};
  Ordinal = Metadata.PageType;
  if (Ordinal != 0) {
    ::agiru::RaiseOrCollect("Card ordinal");
  }
  Metadata.PageType = ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::List};
  Ordinal = Metadata.PageType;
  if (Ordinal != 1) {
    ::agiru::RaiseOrCollect("List ordinal");
  }
  Metadata.PageType = ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::RoleCenter};
  Ordinal = Metadata.PageType;
  if (Ordinal != 2) {
    ::agiru::RaiseOrCollect("RoleCenter ordinal");
  }
  Metadata.PageType = ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::CardPart};
  Ordinal = Metadata.PageType;
  if (Ordinal != 3) {
    ::agiru::RaiseOrCollect("CardPart ordinal");
  }
  Metadata.PageType = ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::ListPart};
  Ordinal = Metadata.PageType;
  if (Ordinal != 4) {
    ::agiru::RaiseOrCollect("ListPart ordinal");
  }
  Metadata.PageType = ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::Document};
  Ordinal = Metadata.PageType;
  if (Ordinal != 5) {
    ::agiru::RaiseOrCollect("Document ordinal");
  }
  Metadata.PageType = ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::Worksheet};
  Ordinal = Metadata.PageType;
  if (Ordinal != 6) {
    ::agiru::RaiseOrCollect("Worksheet ordinal");
  }
  Metadata.PageType = ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::ListPlus};
  Ordinal = Metadata.PageType;
  if (Ordinal != 7) {
    ::agiru::RaiseOrCollect("ListPlus ordinal");
  }
  Metadata.PageType = ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::ConfirmationDialog};
  Ordinal = Metadata.PageType;
  if (Ordinal != 8) {
    ::agiru::RaiseOrCollect("ConfirmationDialog ordinal");
  }
  Metadata.PageType = ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::NavigatePage};
  Ordinal = Metadata.PageType;
  if (Ordinal != 9) {
    ::agiru::RaiseOrCollect("NavigatePage ordinal");
  }
  Metadata.PageType = ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::StandardDialog};
  Ordinal = Metadata.PageType;
  if (Ordinal != 10) {
    ::agiru::RaiseOrCollect("StandardDialog ordinal");
  }
  Metadata.PageType = ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::Api};
  Ordinal = Metadata.PageType;
  if (Ordinal != 11) {
    ::agiru::RaiseOrCollect("API ordinal");
  }
  Metadata.PageType = ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::HeadlinePart};
  Ordinal = Metadata.PageType;
  if (Ordinal != 12) {
    ::agiru::RaiseOrCollect("HeadlinePart ordinal");
  }
  Metadata.SetRange(Metadata.PageType, ::agiru::Option<::agiru::platform::PageMetadataPageType>{::agiru::platform::PageMetadataPageType::HeadlinePart});
  Ordinal = Metadata.GetRangeMin(Metadata.PageType);
  if (Ordinal != 12) {
    ::agiru::RaiseOrCollect("HeadlinePart typed range");
  }
  if (Format(Metadata.PageType) != "HeadlinePart") {
    ::agiru::RaiseOrCollect("HeadlinePart format");
  }
  if (Metadata.FieldNo(Metadata.Editable) != 4) {
    ::agiru::RaiseOrCollect("Editable field identity");
  }
  if (Metadata.FieldNo(Metadata.PageType) != 5) {
    ::agiru::RaiseOrCollect("PageType field identity");
  }
  if (Metadata.FieldNo(Metadata.DataCaptionExpr) != 7) {
    ::agiru::RaiseOrCollect("Caption expression field identity");
  }
  if (Metadata.FieldNo(Metadata.SourceTable) != 14) {
    ::agiru::RaiseOrCollect("SourceTable field identity");
  }
  if (Metadata.FieldNo(Metadata.AppID) != 29) {
    ::agiru::RaiseOrCollect("App identity field");
  }
  if (Metadata.FieldNo(Metadata.ALNamespace) != 32) {
    ::agiru::RaiseOrCollect("Namespace field identity");
  }
  Metadata.ID = 50242;
  Metadata.SourceTable = 18;
  Metadata.CardPageID = 21;
  if (Metadata.ID != 50242 || Metadata.SourceTable != 18 || Metadata.CardPageID != 21) {
    ::agiru::RaiseOrCollect("Integer fields");
  }
  Metadata.Name = "Original AL Name";
  Metadata.Caption = "Visible Caption";
  if (Metadata.Name != "Original AL Name" || Metadata.Caption != "Visible Caption") {
    ::agiru::RaiseOrCollect("Name and caption");
  }
  Metadata.DataCaptionExpr = "Caption expression";
  Metadata.APIPublisher = "agiru";
  Metadata.APIGroup = "erp";
  Metadata.APIVersion = "v1.0";
  Metadata.EntityName = "customer";
  Metadata.EntitySetName = "customers";
  Metadata.SourceTableView = "sorting(No.)";
  Metadata.DataCaptionFields = "No.,Name";
  Metadata.InherentPermissions = "rX";
  Metadata.InherentEntitlements = "X";
  Metadata.ALNamespace = "Microsoft.Fixture";
  if (Metadata.DataCaptionExpr != "Caption expression" || Metadata.APIPublisher != "agiru" || Metadata.APIGroup != "erp" || Metadata.APIVersion != "v1.0" || Metadata.EntityName != "customer" || Metadata.EntitySetName != "customers" || Metadata.SourceTableView != "sorting(No.)" || Metadata.DataCaptionFields != "No.,Name" || Metadata.InherentPermissions != "rX" || Metadata.InherentEntitlements != "X" || Metadata.ALNamespace != "Microsoft.Fixture") {
    ::agiru::RaiseOrCollect("Original Text fields");
  }
  Metadata.Editable = true;
  Metadata.RefreshOnActivate = true;
  Metadata.InsertAllowed = true;
  Metadata.ModifyAllowed = true;
  Metadata.DeleteAllowed = true;
  Metadata.DelayedInsert = true;
  Metadata.ShowFilter = true;
  Metadata.MultipleNewLines = true;
  Metadata.SaveValues = true;
  Metadata.AutoSplitKey = true;
  Metadata.SourceTableTemporary = true;
  Metadata.LinksAllowed = true;
  Metadata.ChangeTrackingAllowed = true;
  Metadata.PopulateAllFields = true;
  if (!Metadata.Editable || !Metadata.RefreshOnActivate || !Metadata.InsertAllowed || !Metadata.ModifyAllowed || !Metadata.DeleteAllowed || !Metadata.DelayedInsert || !Metadata.ShowFilter || !Metadata.MultipleNewLines || !Metadata.SaveValues || !Metadata.AutoSplitKey || !Metadata.SourceTableTemporary || !Metadata.LinksAllowed || !Metadata.ChangeTrackingAllowed || !Metadata.PopulateAllFields) {
    ::agiru::RaiseOrCollect("Original Boolean fields");
  }
  Identifier = CreateGuid();
  Metadata.AppID = Identifier;
  if (Metadata.AppID != Identifier) {
    ::agiru::RaiseOrCollect("Original Guid field");
  }
  Reflected.GetTable(Metadata);
  if (Reflected.Field(5).OptionMembers() != "Card,List,RoleCenter,CardPart,ListPart,Document,Worksheet,ListPlus,ConfirmationDialog,NavigatePage,StandardDialog,API,HeadlinePart") {
    ::agiru::RaiseOrCollect("Source option reflection");
  }
  return 27;
}

void MetadataRoundtrip_Codeunit::ClearAll() {
}

constexpr CodeunitDef kMetadataRoundtripCodeunit{
    .id = ::agiru::CodeunitTraits<MetadataRoundtrip_Codeunit>::kId,
    .name = ::agiru::CodeunitTraits<MetadataRoundtrip_Codeunit>::kName,
    .subtype = ::agiru::CodeunitTraits<MetadataRoundtrip_Codeunit>::kSubtype,
};

} // namespace agiru::Fixture

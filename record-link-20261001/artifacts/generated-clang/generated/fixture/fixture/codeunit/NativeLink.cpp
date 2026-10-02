// Generated from Link.Codeunit.al. Do not edit.

#include "NativeLink.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "platform/AllProfile.h"
#include "platform/RecordLink.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/Blob.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/RecordId.h"

#include "fixture/table/CollisionSource.h"
#include "platform/AllProfile.h"
#include "platform/RecordLink.h"
#include "platform/AllProfile.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.id == ::agiru::TableId{2000000178} && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.name == "All Profile", "native table identity mismatch: All Profile");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 17;
}(), "native field count mismatch: All Profile");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "Scope" || field->caption != "Scope" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 2) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "System" || field->values[0].caption != "System") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Tenant" || field->values[1].caption != "Tenant") { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Scope");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "App ID" || field->caption != "App ID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.App ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "Profile ID" || field->caption != "Profile ID" || field->type != ::agiru::FieldType::Code || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Profile ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "Description" || field->caption != "Description" || field->type != ::agiru::FieldType::Text || field->length != 2048) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Description");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{5});
  if (field == nullptr || field->name != "Role Center ID" || field->caption != "Role Center ID" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Role Center ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "Default Role Center" || field->caption != "Default Role Center" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Default Role Center");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{7});
  if (field == nullptr || field->name != "Use Comments" || field->caption != "Use Comments" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Use Comments");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{8});
  if (field == nullptr || field->name != "Use Notes" || field->caption != "Use Notes" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Use Notes");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{9});
  if (field == nullptr || field->name != "Use Record Notes" || field->caption != "Use Record Notes" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Use Record Notes");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{10});
  if (field == nullptr || field->name != "Record Notebook" || field->caption != "Record Notebook" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Record Notebook");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{11});
  if (field == nullptr || field->name != "Use Page Notes" || field->caption != "Use Page Notes" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Use Page Notes");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{12});
  if (field == nullptr || field->name != "Page Notebook" || field->caption != "Page Notebook" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Page Notebook");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{13});
  if (field == nullptr || field->name != "Disable Personalization" || field->caption != "Disable Personalization" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Disable Personalization");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{14});
  if (field == nullptr || field->name != "App Name" || field->caption != "App Name" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.App Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{15});
  if (field == nullptr || field->name != "Enabled" || field->caption != "Enabled" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Enabled");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{16});
  if (field == nullptr || field->name != "Caption" || field->caption != "Caption" || field->type != ::agiru::FieldType::Text || field->length != 100) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Caption");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable, ::agiru::FieldNo{17});
  if (field == nullptr || field->name != "Promoted" || field->caption != "Promoted" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: All Profile.Promoted");
static_assert(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys.size() == 1, "native key count mismatch: All Profile");
static_assert(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].name == "PK" && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].fields.size() == 3 && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].fields[1] == ::agiru::FieldNo{2} && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].fields[2] == ::agiru::FieldNo{3} && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: All Profile.PK");
static_assert(::agiru::TableTraits<::agiru::platform::AllProfile>::kTable.dataPerCompany == false, "native company scope mismatch: All Profile");

#include "platform/RecordLink.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.id == ::agiru::TableId{2000000068} && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.name == "Record Link", "native table identity mismatch: Record Link");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 14;
}(), "native field count mismatch: Record Link");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "Link ID" || field->caption != "Link ID" || field->type != ::agiru::FieldType::Integer || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Link ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "Record ID" || field->caption != "Record ID" || field->type != ::agiru::FieldType::RecordId || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Record ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "URL1" || field->caption != "URL1" || field->type != ::agiru::FieldType::Text || field->length != 2048) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.URL1");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "URL2" || field->caption != "URL2" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.URL2");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{5});
  if (field == nullptr || field->name != "URL3" || field->caption != "URL3" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.URL3");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "URL4" || field->caption != "URL4" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.URL4");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{7});
  if (field == nullptr || field->name != "Description" || field->caption != "Description" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Description");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{8});
  if (field == nullptr || field->name != "Type" || field->caption != "Type" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 2) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Link" || field->values[0].caption != "Link") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Note" || field->values[1].caption != "Note") { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Type");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{9});
  if (field == nullptr || field->name != "Note" || field->caption != "Note" || field->type != ::agiru::FieldType::Blob || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Note");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{10});
  if (field == nullptr || field->name != "Created" || field->caption != "Created" || field->type != ::agiru::FieldType::DateTime || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Created");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{11});
  if (field == nullptr || field->name != "User ID" || field->caption != "User ID" || field->type != ::agiru::FieldType::Text || field->length != 132) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.User ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{12});
  if (field == nullptr || field->name != "Company" || field->caption != "Company" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Company");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{13});
  if (field == nullptr || field->name != "Notify" || field->caption != "Notify" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.Notify");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable, ::agiru::FieldNo{14});
  if (field == nullptr || field->name != "To User ID" || field->caption != "To User ID" || field->type != ::agiru::FieldType::Text || field->length != 132) { return false; }
  return true;
}(), "native field declaration mismatch: Record Link.To User ID");
static_assert(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys.size() == 3, "native key count mismatch: Record Link");
static_assert(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: Record Link.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys.size() > 1 && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].name == "Key2" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].fields[0] == ::agiru::FieldNo{2} && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].clustered == false && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].enabled == true && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].unique == false && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].includedFields == "" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].description == "" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[1].sumIndexFields.size() == 0, "native key declaration mismatch: Record Link.Key2");
static_assert(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys.size() > 2 && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].name == "Key3" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].fields.size() == 2 && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].fields[0] == ::agiru::FieldNo{12} && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].fields[1] == ::agiru::FieldNo{2} && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].clustered == false && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].enabled == true && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].unique == false && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].includedFields == "" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].description == "" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.keys[2].sumIndexFields.size() == 0, "native key declaration mismatch: Record Link.Key3");
static_assert(::agiru::TableTraits<::agiru::platform::RecordLink>::kTable.dataPerCompany == false, "native company scope mismatch: Record Link");

#include "fixture/table/CollisionSource.h"
#include "platform/AllProfile.h"
#include "platform/RecordLink.h"

namespace agiru::Fixture {

namespace {
namespace NativeLink_unit {
const RegisterCodeunit<NativeLink_Codeunit> kInCodeunitCatalogue;
} // namespace NativeLink_unit
} // namespace

::agiru::Integer NativeLink_Codeunit::Exercise() {
  [[maybe_unused]] Temporary<::agiru::platform::AllProfile> Related{};
  [[maybe_unused]] Temporary<::agiru::platform::RecordLink> Link{};
  [[maybe_unused]] Temporary<::agiru::Fixture::CollisionSource_Table> Ordinary{};
  [[maybe_unused]] ::agiru::RecordId Own{};
  [[maybe_unused]] ::agiru::RecordId Target{};
  [[maybe_unused]] ::agiru::RecordRef Reflected{};

  Related.ProfileID = "related";
  Target = Related.RecordId();
  Link.LinkID = 7;
  Link.RecordID = Target;
  Link.URL1 = PadStr("u", 2048, "u");
  Link.UserID = PadStr("Creator.Mixed Case", 132, "x");
  Link.ToUserID = PadStr("Recipient.Mixed Case", 132, "y");
  Link.Description = PadStr("d", 250, "d");
  Link.Company = "Company.Mixed";
  Link.Type = ::agiru::Option<::agiru::platform::RecordLinkType>{::agiru::platform::RecordLinkType::Note};
  Link.Notify = true;
  if (!Link.Ok_Insert()) {
    ::agiru::RaiseOrCollect("Native temporary insert");
  }
  Link.Reset();
  if (!Link.FindFirst()) {
    ::agiru::RaiseOrCollect("Native temporary lookup");
  }
  if (StrLen(Link.URL1) != 2048) {
    ::agiru::RaiseOrCollect("Source URL1 length");
  }
  if (Link.UserID != PadStr("Creator.Mixed Case", 132, "x")) {
    ::agiru::RaiseOrCollect("Creator Text preserves all characters");
  }
  if (Link.ToUserID != PadStr("Recipient.Mixed Case", 132, "y")) {
    ::agiru::RaiseOrCollect("Recipient Text preserves all characters");
  }
  if (StrLen(Link.Description) != 250) {
    ::agiru::RaiseOrCollect("Source description length");
  }
  if (Link.Company != "Company.Mixed") {
    ::agiru::RaiseOrCollect("Company Text case");
  }
  if (Link.Type != ::agiru::Option<::agiru::platform::RecordLinkType>{::agiru::platform::RecordLinkType::Note}) {
    ::agiru::RaiseOrCollect("Source note option");
  }
  if (!Link.Notify) {
    ::agiru::RaiseOrCollect("Source notification field");
  }
  if (Link.RecordID != Target) {
    ::agiru::RaiseOrCollect("Quoted field preserves related record");
  }
  Own = Link.::agiru::Table<::agiru::platform::RecordLink>::RecordId();
  if (Own.TableNo() != 2000000068) {
    ::agiru::RaiseOrCollect("Implicit native method is not related field");
  }
  Own = Link.::agiru::Table<::agiru::platform::RecordLink>::RecordId();
  if (Own.TableNo() != 2000000068) {
    ::agiru::RaiseOrCollect("Uppercase native implicit method");
  }
  Own = Link.::agiru::Table<::agiru::platform::RecordLink>::RecordId();
  if (Own.TableNo() != 2000000068) {
    ::agiru::RaiseOrCollect("Lowercase native implicit method");
  }
  Own = Link.::agiru::Table<::agiru::platform::RecordLink>::RecordId();
  if (Own.TableNo() != 2000000068) {
    ::agiru::RaiseOrCollect("Capital native implicit method");
  }
  Own = Link.::agiru::Table<::agiru::platform::RecordLink>::RecordId();
  if (Own.TableNo() != 2000000068) {
    ::agiru::RaiseOrCollect("Explicit native method");
  }
  Own = Link.::agiru::Table<::agiru::platform::RecordLink>::RecordId();
  if (Own.TableNo() != 2000000068) {
    ::agiru::RaiseOrCollect("Uppercase native explicit method");
  }
  Own = Link.::agiru::Table<::agiru::platform::RecordLink>::RecordId();
  if (Own.TableNo() != 2000000068) {
    ::agiru::RaiseOrCollect("Lowercase native explicit method");
  }
  Own = Link.::agiru::Table<::agiru::platform::RecordLink>::RecordId();
  if (Own.TableNo() != 2000000068) {
    ::agiru::RaiseOrCollect("Capital native explicit method");
  }
  if (Own == Target) {
    ::agiru::RaiseOrCollect("Own and related identities stay distinct");
  }
  if (Link.RecordID != Target) {
    ::agiru::RaiseOrCollect("Calling the method never mutates the related field");
  }
  Reflected.GetTable(Link);
  if (Reflected.KeyCount() != 3) {
    ::agiru::RaiseOrCollect("All declared keys");
  }
  if (!Reflected.FieldExist(14)) {
    ::agiru::RaiseOrCollect("Original recipient field is reflected");
  }
  Own = Reflected.RecordId();
  if (Own.TableNo() != 2000000068) {
    ::agiru::RaiseOrCollect("Reflected current identity");
  }
  Ordinary.EntryNo = 9;
  Ordinary.RecordID = Target;
  if (!Ordinary.Ok_Insert()) {
    ::agiru::RaiseOrCollect("Ordinary temporary insert");
  }
  Own = Ordinary.RecordId();
  if (Own.TableNo() != 50197) {
    ::agiru::RaiseOrCollect("Ordinary implicit method");
  }
  Own = Ordinary.::agiru::Table<::agiru::Fixture::CollisionSource_Table>::RecordId();
  if (Own.TableNo() != 50197) {
    ::agiru::RaiseOrCollect("Ordinary uppercase implicit method");
  }
  Own = Ordinary.RecordId();
  if (Own.TableNo() != 50197) {
    ::agiru::RaiseOrCollect("Ordinary explicit method");
  }
  if (Ordinary.RecordID != Target) {
    ::agiru::RaiseOrCollect("Ordinary quoted field is not its own identity");
  }
  return 28;
}

void NativeLink_Codeunit::ClearAll() {
}

constexpr CodeunitDef kNativeLinkCodeunit{
    .id = ::agiru::CodeunitTraits<NativeLink_Codeunit>::kId,
    .name = ::agiru::CodeunitTraits<NativeLink_Codeunit>::kName,
    .subtype = ::agiru::CodeunitTraits<NativeLink_Codeunit>::kSubtype,
};

} // namespace agiru::Fixture

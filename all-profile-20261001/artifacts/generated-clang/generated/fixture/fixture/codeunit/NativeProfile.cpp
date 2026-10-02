// Generated from Profile.Codeunit.al. Do not edit.

#include "NativeProfile.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "platform/AllProfile.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"

#include "platform/AllProfile.h"
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

#include "platform/AllProfile.h"

namespace agiru::Fixture {

namespace {
namespace NativeProfile_unit {
const RegisterCodeunit<NativeProfile_Codeunit> kInCodeunitCatalogue;
} // namespace NativeProfile_unit
} // namespace

::agiru::Integer NativeProfile_Codeunit::Exercise() {
  [[maybe_unused]] Temporary<::agiru::platform::AllProfile> Profile{};
  [[maybe_unused]] ::agiru::RecordRef Reflected{};

  if (Profile.FieldNo(Profile.UseComments) != 7) {
    ::agiru::RaiseOrCollect("Original comments number");
  }
  if (Profile.FieldNo(Profile.UseNotes) != 8) {
    ::agiru::RaiseOrCollect("Original notes number");
  }
  if (Profile.FieldNo(Profile.UseRecordNotes) != 9) {
    ::agiru::RaiseOrCollect("Original record notes number");
  }
  if (Profile.FieldNo(Profile.RecordNotebook) != 10) {
    ::agiru::RaiseOrCollect("Original record notebook number");
  }
  if (Profile.FieldNo(Profile.UsePageNotes) != 11) {
    ::agiru::RaiseOrCollect("Original page notes number");
  }
  if (Profile.FieldNo(Profile.PageNotebook) != 12) {
    ::agiru::RaiseOrCollect("Original page notebook number");
  }
  if (Profile.FieldNo(Profile.DisablePersonalization) != 13) {
    ::agiru::RaiseOrCollect("Original personalization number");
  }
  if (Profile.FieldNo(Profile.AppName) != 14) {
    ::agiru::RaiseOrCollect("Original app name number");
  }
  if (Profile.FieldNo(Profile.Enabled) != 15) {
    ::agiru::RaiseOrCollect("Original enabled number");
  }
  if (Profile.FieldNo(Profile.Caption) != 16) {
    ::agiru::RaiseOrCollect("Original caption number");
  }
  if (Profile.FieldNo(Profile.Promoted) != 17) {
    ::agiru::RaiseOrCollect("Original promoted number");
  }
  Profile.Scope = ::agiru::Option<::agiru::platform::PersonalizationScope>{::agiru::platform::PersonalizationScope::System};
  Profile.ProfileID = "profile-a";
  Profile.Description = PadStr("d", 2048, "d");
  Profile.AppName = PadStr("a", 250, "a");
  Profile.Caption = PadStr("c", 100, "c");
  Profile.RecordNotebook = "Record Notebook";
  Profile.PageNotebook = "Page Notebook";
  Profile.DisablePersonalization = true;
  Profile.Enabled = true;
  Profile.Promoted = true;
  if (!Profile.Ok_Insert()) {
    ::agiru::RaiseOrCollect("Temporary profile insert");
  }
  Profile.Reset();
  if (!Profile.FindFirst()) {
    ::agiru::RaiseOrCollect("Temporary profile lookup");
  }
  if (StrLen(Profile.Description) != 2048) {
    ::agiru::RaiseOrCollect("Full source description");
  }
  if (StrLen(Profile.AppName) != 250) {
    ::agiru::RaiseOrCollect("Independent app name length");
  }
  if (StrLen(Profile.Caption) != 100) {
    ::agiru::RaiseOrCollect("Independent caption length");
  }
  if (Profile.RecordNotebook != "Record Notebook") {
    ::agiru::RaiseOrCollect("Record notebook retained");
  }
  if (Profile.PageNotebook != "Page Notebook") {
    ::agiru::RaiseOrCollect("Page notebook retained");
  }
  if (!Profile.DisablePersonalization) {
    ::agiru::RaiseOrCollect("Personalization retained");
  }
  if (!Profile.Enabled) {
    ::agiru::RaiseOrCollect("Enabled retained");
  }
  if (!Profile.Promoted) {
    ::agiru::RaiseOrCollect("Promoted retained");
  }
  Reflected.GetTable(Profile);
  if (Reflected.KeyCount() != 1) {
    ::agiru::RaiseOrCollect("Only the original primary key");
  }
  if (Reflected.Number() != 2000000178) {
    ::agiru::RaiseOrCollect("Original table identity");
  }
  if (Reflected.FieldCount() != 17) {
    ::agiru::RaiseOrCollect("All declared fields");
  }
  return 24;
}

void NativeProfile_Codeunit::ClearAll() {
}

constexpr CodeunitDef kNativeProfileCodeunit{
    .id = ::agiru::CodeunitTraits<NativeProfile_Codeunit>::kId,
    .name = ::agiru::CodeunitTraits<NativeProfile_Codeunit>::kName,
    .subtype = ::agiru::CodeunitTraits<NativeProfile_Codeunit>::kSubtype,
};

} // namespace agiru::Fixture

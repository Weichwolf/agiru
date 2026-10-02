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


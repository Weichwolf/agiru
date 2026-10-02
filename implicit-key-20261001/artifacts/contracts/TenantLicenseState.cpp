#include "platform/TenantLicenseState.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.id == ::agiru::TableId{2000000189} && ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.name == "Tenant License State", "native table identity mismatch: Tenant License State");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 4;
}(), "native field count mismatch: Tenant License State");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "Start Date" || field->caption != "Start Date" || field->type != ::agiru::FieldType::DateTime || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Tenant License State.Start Date");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "End Date" || field->caption != "End Date" || field->type != ::agiru::FieldType::DateTime || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Tenant License State.End Date");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "State" || field->caption != "State" || field->type != ::agiru::FieldType::Option || field->length != 0) { return false; }
  if (field->values.size() != 10) { return false; }
  if (field->values[0].ordinal != 0 || field->values[0].name != "Evaluation" || field->values[0].caption != "Evaluation") { return false; }
  if (field->values[1].ordinal != 1 || field->values[1].name != "Trial" || field->values[1].caption != "Trial") { return false; }
  if (field->values[2].ordinal != 2 || field->values[2].name != "Paid" || field->values[2].caption != "Paid") { return false; }
  if (field->values[3].ordinal != 3 || field->values[3].name != "Warning" || field->values[3].caption != "Warning") { return false; }
  if (field->values[4].ordinal != 4 || field->values[4].name != "Suspended" || field->values[4].caption != "Suspended") { return false; }
  if (field->values[5].ordinal != 5 || field->values[5].name != "Deleted" || field->values[5].caption != "Deleted") { return false; }
  if (field->values[6].ordinal != 6 || field->values[6].name != "" || field->values[6].caption != "") { return false; }
  if (field->values[7].ordinal != 7 || field->values[7].name != "" || field->values[7].caption != "") { return false; }
  if (field->values[8].ordinal != 8 || field->values[8].name != "" || field->values[8].caption != "") { return false; }
  if (field->values[9].ordinal != 9 || field->values[9].name != "LockedOut" || field->values[9].caption != "LockedOut") { return false; }
  return true;
}(), "native field declaration mismatch: Tenant License State.State");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "User Security ID" || field->caption != "User Security ID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Tenant License State.User Security ID");
static_assert(::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys.size() == 1, "native key count mismatch: Tenant License State");
static_assert(::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: Tenant License State.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::TenantLicenseState>::kTable.dataPerCompany == false, "native company scope mismatch: Tenant License State");


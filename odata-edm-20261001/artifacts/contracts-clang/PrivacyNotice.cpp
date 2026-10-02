#include "platform/PrivacyNotice.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.id == ::agiru::TableId{2000000237} && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.name == "Privacy Notice", "native table identity mismatch: Privacy Notice");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 6;
}(), "native field count mismatch: Privacy Notice");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "ID" || field->caption != "Privacy Notice ID" || field->type != ::agiru::FieldType::Code || field->length != 50) { return false; }
  return true;
}(), "native field declaration mismatch: Privacy Notice.ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "Integration Service Name" || field->caption != "Integration Service Name" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: Privacy Notice.Integration Service Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "Link" || field->caption != "Privacy Link" || field->type != ::agiru::FieldType::Text || field->length != 2048) { return false; }
  return true;
}(), "native field declaration mismatch: Privacy Notice.Link");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "User SID Filter" || field->caption != "User SID Filter" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Privacy Notice.User SID Filter");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable, ::agiru::FieldNo{5});
  if (field == nullptr || field->name != "Enabled" || field->caption != "Enabled" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Privacy Notice.Enabled");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "Disabled" || field->caption != "Disabled" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Privacy Notice.Disabled");
static_assert(::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys.size() == 2, "native key count mismatch: Privacy Notice");
static_assert(::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: Privacy Notice.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys.size() > 1 && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[1].name == "Key2" && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[1].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[1].fields[0] == ::agiru::FieldNo{2} && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[1].clustered == false && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[1].enabled == true && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[1].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[1].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[1].unique == false && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[1].includedFields == "" && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[1].description == "" && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[1].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.keys[1].sumIndexFields.size() == 0, "native key declaration mismatch: Privacy Notice.Key2");
static_assert(::agiru::TableTraits<::agiru::platform::PrivacyNotice>::kTable.dataPerCompany == false, "native company scope mismatch: Privacy Notice");


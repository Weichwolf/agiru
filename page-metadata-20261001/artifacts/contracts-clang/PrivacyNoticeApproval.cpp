#include "platform/PrivacyNoticeApproval.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.id == ::agiru::TableId{2000000238} && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.name == "Privacy Notice Approval", "native table identity mismatch: Privacy Notice Approval");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 4;
}(), "native field count mismatch: Privacy Notice Approval");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "ID" || field->caption != "Privacy Notice ID" || field->type != ::agiru::FieldType::Code || field->length != 50) { return false; }
  return true;
}(), "native field declaration mismatch: Privacy Notice Approval.ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "User SID" || field->caption != "User SID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Privacy Notice Approval.User SID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "Approver User SID" || field->caption != "Approver User ID" || field->type != ::agiru::FieldType::Guid || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Privacy Notice Approval.Approver User SID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable, ::agiru::FieldNo{4});
  if (field == nullptr || field->name != "Approved" || field->caption != "Approved" || field->type != ::agiru::FieldType::Boolean || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Privacy Notice Approval.Approved");
static_assert(::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys.size() == 1, "native key count mismatch: Privacy Notice Approval");
static_assert(::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys[0].fields.size() == 2 && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys[0].fields[1] == ::agiru::FieldNo{2} && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: Privacy Notice Approval.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::PrivacyNoticeApproval>::kTable.dataPerCompany == false, "native company scope mismatch: Privacy Notice Approval");


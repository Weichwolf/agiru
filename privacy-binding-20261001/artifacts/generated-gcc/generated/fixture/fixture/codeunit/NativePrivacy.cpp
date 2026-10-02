// Generated from Privacy.Codeunit.al. Do not edit.

#include "NativePrivacy.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "platform/PrivacyNotice.h"
#include "platform/PrivacyNoticeApproval.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/Guid.h"
#include "type/Integer.h"

#include "platform/PrivacyNotice.h"
#include "platform/PrivacyNoticeApproval.h"
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

#include "platform/PrivacyNotice.h"
#include "platform/PrivacyNoticeApproval.h"

namespace agiru::Fixture {

namespace {
namespace NativePrivacy_unit {
const RegisterCodeunit<NativePrivacy_Codeunit> kInCodeunitCatalogue;
} // namespace NativePrivacy_unit
} // namespace

::agiru::Integer NativePrivacy_Codeunit::Exercise() {
  [[maybe_unused]] Temporary<::agiru::platform::PrivacyNotice> Notice{};
  [[maybe_unused]] Temporary<::agiru::platform::PrivacyNoticeApproval> Approval{};
  [[maybe_unused]] ::agiru::RecordRef Reflected{};
  [[maybe_unused]] Guid User{};
  [[maybe_unused]] Guid OtherUser{};

  if (Notice.FieldNo(Notice.UserSIDFilter) != 4) {
    ::agiru::RaiseOrCollect("Original filter number");
  }
  if (Notice.FieldNo(Notice.Enabled) != 5) {
    ::agiru::RaiseOrCollect("Original enabled number");
  }
  if (Notice.FieldNo(Notice.Disabled) != 6) {
    ::agiru::RaiseOrCollect("Original disabled number");
  }
  Notice.ID = "notice-a";
  Notice.IntegrationServiceName = " Mixed.Case Service ";
  Notice.Link = "https://example.com/Privacy?Key=Mixed";
  Reflected.GetTable(Notice);
  if (Reflected.Number() != 2000000237) {
    ::agiru::RaiseOrCollect("Original notice identity");
  }
  if (Reflected.Field(3).Value() != "https://example.com/Privacy?Key=Mixed") {
    ::agiru::RaiseOrCollect("Text Link reflection");
  }
  if (!Notice.Ok_Insert()) {
    ::agiru::RaiseOrCollect("Temporary notice insert");
  }
  Notice.ID = "notice-b";
  Notice.IntegrationServiceName = "Other Service";
  if (!Notice.Ok_Insert()) {
    ::agiru::RaiseOrCollect("Second temporary notice insert");
  }
  if (Notice.Count() != 2) {
    ::agiru::RaiseOrCollect("Temporary notice population");
  }
  if (!Notice.SetCurrentKey(Notice.IntegrationServiceName)) {
    ::agiru::RaiseOrCollect("Original secondary key");
  }
  Notice.SetRange(Notice.IntegrationServiceName, " Mixed.Case Service ");
  if (!Notice.FindFirst()) {
    ::agiru::RaiseOrCollect("Text service lookup");
  }
  if (Notice.ID != "NOTICE-A") {
    ::agiru::RaiseOrCollect("Original notice row");
  }
  User = CreateGuid();
  OtherUser = CreateGuid();
  Approval.ID = Notice.ID;
  Approval.UserSID = User;
  Approval.ApproverUserSID = OtherUser;
  Approval.Approved = true;
  Reflected.GetTable(Approval);
  if (Reflected.Number() != 2000000238) {
    ::agiru::RaiseOrCollect("Original approval identity");
  }
  if (!Approval.Ok_Insert()) {
    ::agiru::RaiseOrCollect("First temporary approval insert");
  }
  Approval.UserSID = OtherUser;
  Approval.Approved = false;
  if (!Approval.Ok_Insert()) {
    ::agiru::RaiseOrCollect("Approval key retains user");
  }
  if (Approval.Count() != 2) {
    ::agiru::RaiseOrCollect("Temporary approval population");
  }
  Approval.SetRange(Approval.UserSID, User);
  if (!Approval.FindFirst()) {
    ::agiru::RaiseOrCollect("User-specific approval lookup");
  }
  if (!Approval.Approved) {
    ::agiru::RaiseOrCollect("Original approval value");
  }
  if (Approval.ApproverUserSID != OtherUser) {
    ::agiru::RaiseOrCollect("Distinct approver identity");
  }
  return 18;
}

void NativePrivacy_Codeunit::ClearAll() {
}

constexpr CodeunitDef kNativePrivacyCodeunit{
    .id = ::agiru::CodeunitTraits<NativePrivacy_Codeunit>::kId,
    .name = ::agiru::CodeunitTraits<NativePrivacy_Codeunit>::kName,
    .subtype = ::agiru::CodeunitTraits<NativePrivacy_Codeunit>::kSubtype,
};

} // namespace agiru::Fixture

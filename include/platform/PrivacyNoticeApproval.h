#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/Code.h"
#include "type/DateTime.h"
#include "type/Guid.h"

#include <array>
#include <cstddef>
#include <string_view>

/// \file
/// \brief System `Privacy Notice Approval`, from `PrivacyNoticeApproval.Table.al`.
/// \note The source key retains both notice and user; an empty User SID is organization-wide.

namespace agiru::platform {

class PrivacyNoticeApproval_Table : public Table<PrivacyNoticeApproval_Table> {
public:
  static constexpr TableId kId{2000000238};
  static constexpr std::string_view kName{"Privacy Notice Approval"};
  detail::StateHandle State_Block;
  static constexpr std::size_t kIdLength = 50;
  Code<kIdLength> ID;
  Guid UserSID;
  Guid ApproverUserSID;
  Boolean Approved{};
  /// \brief AL `PrivacyNoticeApproval.SystemId`.
  Guid SystemId;
  /// \brief AL `PrivacyNoticeApproval.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `PrivacyNoticeApproval.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `PrivacyNoticeApproval.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `PrivacyNoticeApproval.SystemModifiedBy`.
  Guid SystemModifiedBy;

  struct Field_No {
    static constexpr ::agiru::FieldNo ID{1};
    static constexpr ::agiru::FieldNo UserSID{2};
    static constexpr ::agiru::FieldNo ApproverUserSID{3};
    static constexpr ::agiru::FieldNo Approved{4};
  };

  static constexpr std::array<::agiru::FieldNo, 2> kKey1{{Field_No::ID, Field_No::UserSID}};
};

using PrivacyNoticeApproval = PrivacyNoticeApproval_Table;

inline constexpr std::array<FieldDef, 4> kPrivacyNoticeApprovalFields{{
    Declare<&PrivacyNoticeApproval::ID>(PrivacyNoticeApproval::Field_No::ID,
                                        "ID",
                                        "Privacy Notice ID",
                                        offsetof(PrivacyNoticeApproval, ID),
                                        Declared{.relation = "\"Privacy Notice\""}),
    Declare<&PrivacyNoticeApproval::UserSID>(PrivacyNoticeApproval::Field_No::UserSID,
                                             "User SID",
                                             "User SID",
                                             offsetof(PrivacyNoticeApproval, UserSID),
                                             Declared{.relation = "User.\"User Security ID\""}),
    Declare<&PrivacyNoticeApproval::ApproverUserSID>(
        PrivacyNoticeApproval::Field_No::ApproverUserSID,
        "Approver User SID",
        "Approver User ID",
        offsetof(PrivacyNoticeApproval, ApproverUserSID),
        Declared{.relation = "User.\"User Security ID\""}),
    Declare<&PrivacyNoticeApproval::Approved>(PrivacyNoticeApproval::Field_No::Approved,
                                              "Approved",
                                              "Approved",
                                              offsetof(PrivacyNoticeApproval, Approved)),
}};

inline constexpr std::array<KeyDef, 1> kPrivacyNoticeApprovalKeys{{
    KeyDef{.name = "Key1", .fields = PrivacyNoticeApproval::kKey1, .clustered = true},
}};

inline constexpr TableDef kPrivacyNoticeApprovalTable{
    .id = PrivacyNoticeApproval::kId,
    .name = PrivacyNoticeApproval::kName,
    .caption = PrivacyNoticeApproval::kName,
    .fields = kPrivacyNoticeApprovalFields,
    .keys = kPrivacyNoticeApprovalKeys,
    .dataPerCompany = false,
    .replicateData = false,
};

static_assert(FieldsAreSorted(kPrivacyNoticeApprovalTable),
              "the field table is searched by number");

}

template <> struct agiru::TableTraits<agiru::platform::PrivacyNoticeApproval> {
  static constexpr const agiru::TableDef &kTable = agiru::platform::kPrivacyNoticeApprovalTable;
};

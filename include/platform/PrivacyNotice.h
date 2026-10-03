#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/Code.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <string_view>

/// \file
/// \brief Privacy declarations from System.app 28.0.53152.0,
///        `src/Tenant Database Tables/PrivacyNotice.Table.al`; not service integration support.

namespace agiru::platform {

/// \brief Source-backed table 2000000237; populated-schema activation is a separate contract.
class PrivacyNotice_Table : public Table<PrivacyNotice_Table> {
public:
  static constexpr TableId kId{2000000237};
  static constexpr std::string_view kName{"Privacy Notice"};
  /// \brief Original extension availability.
  static constexpr std::string_view kScope{"Cloud"};
  detail::StateHandle State_Block;
  static constexpr std::size_t kIdLength = 50;
  static constexpr std::size_t kNameLength = 250;
  static constexpr std::size_t kLinkLength = 2048;
  Code<kIdLength> ID;
  Text<kNameLength> IntegrationServiceName;
  Text<kLinkLength> Link;
  Boolean Enabled{};
  Boolean Disabled{};
  Guid UserSIDFilter{};
  /// \brief AL `PrivacyNotice.SystemId`.
  Guid SystemId;
  /// \brief AL `PrivacyNotice.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `PrivacyNotice.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `PrivacyNotice.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `PrivacyNotice.SystemModifiedBy`.
  Guid SystemModifiedBy;

  struct Field_No : SystemFieldNumbers {
    static constexpr ::agiru::FieldNo ID{1};
    static constexpr ::agiru::FieldNo IntegrationServiceName{2};
    static constexpr ::agiru::FieldNo Link{3};
    static constexpr ::agiru::FieldNo UserSIDFilter{4};
    static constexpr ::agiru::FieldNo Enabled{5};
    static constexpr ::agiru::FieldNo Disabled{6};
  };

  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::ID}};
  /// \brief Original service-name lookup key.
  static constexpr std::array<::agiru::FieldNo, 1> kKey2{{Field_No::IntegrationServiceName}};
};

using PrivacyNotice = PrivacyNotice_Table;

inline constexpr auto kPrivacyNoticeFields =
    WithSystemFields<PrivacyNotice>(std::array<FieldDef, 6>{{
        Declare<&PrivacyNotice::ID>(
            PrivacyNotice::Field_No::ID, "ID", "Privacy Notice ID", offsetof(PrivacyNotice, ID)),
        Declare<&PrivacyNotice::IntegrationServiceName>(
            PrivacyNotice::Field_No::IntegrationServiceName,
            "Integration Service Name",
            "Integration Service Name",
            offsetof(PrivacyNotice, IntegrationServiceName)),
        Declare<&PrivacyNotice::Link>(
            PrivacyNotice::Field_No::Link, "Link", "Privacy Link", offsetof(PrivacyNotice, Link)),
        Declare<&PrivacyNotice::UserSIDFilter>(PrivacyNotice::Field_No::UserSIDFilter,
                                               "User SID Filter",
                                               "User SID Filter",
                                               offsetof(PrivacyNotice, UserSIDFilter),
                                               {.fieldClass = ::agiru::FieldClass::FlowFilter,
                                                .relationTable = "User",
                                                .relationField = "User Security ID",
                                                .relation = "User.\"User Security ID\""}),
        Declare<&PrivacyNotice::Enabled>(
            PrivacyNotice::Field_No::Enabled,
            "Enabled",
            "Enabled",
            offsetof(PrivacyNotice, Enabled),
            Declared{.fieldClass = ::agiru::FieldClass::FlowField,
                     .calcFormula =
                         "exist(\"Privacy Notice Approval\" where(ID = field(ID), \"User "
                         "SID\" = field(\"User SID Filter\"), Approved = const(true)))"}),
        Declare<&PrivacyNotice::Disabled>(
            PrivacyNotice::Field_No::Disabled,
            "Disabled",
            "Disabled",
            offsetof(PrivacyNotice, Disabled),
            Declared{.fieldClass = ::agiru::FieldClass::FlowField,
                     .calcFormula =
                         "exist(\"Privacy Notice Approval\" where(ID = field(ID), \"User "
                         "SID\" = field(\"User SID Filter\"), Approved = const(false)))"}),
    }});

inline constexpr std::array<KeyDef, 2> kPrivacyNoticeKeys{{
    KeyDef{.name = "Key1", .fields = PrivacyNotice::kKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = PrivacyNotice::kKey2},
}};

inline constexpr TableDef kPrivacyNoticeTable{
    .id = PrivacyNotice::kId,
    .name = PrivacyNotice::kName,
    .caption = PrivacyNotice::kName,
    .fields = kPrivacyNoticeFields,
    .keys = kPrivacyNoticeKeys,
    .dataPerCompany = false,
    .replicateData = false,
};

static_assert(FieldsAreSorted(kPrivacyNoticeTable), "the field table is searched by number");

}

template <> struct agiru::TableTraits<agiru::platform::PrivacyNotice> {
  static constexpr const agiru::TableDef &kTable = agiru::platform::kPrivacyNoticeTable;
};

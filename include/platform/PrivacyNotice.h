#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/Code.h"
#include "type/Guid.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <string_view>

/// \file
/// \brief The platform's `Privacy Notice` table: what an integration must have the user agree to.
///
/// \note IT HAS NO AL SOURCE; the columns are the demo database's (`system."Privacy Notice"`:
///       `ID`, `Integration Service Name`, `Link`) and the two FlowFields are the ones the System
///       Application reads (`Privacy Notice Impl.`: `Enabled`, `Disabled`), each an `Exist` over
///       `Privacy Notice Approval` for the organisation-wide row, whose `User SID` is the empty
///       Guid. The field numbers are assigned here [SET].

namespace agiru::platform {

class PrivacyNotice_Table : public Table<PrivacyNotice_Table> {
public:
  static constexpr TableId kId{1560};
  static constexpr std::string_view kName{"Privacy Notice"};
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

  struct Field_No {
    static constexpr ::agiru::FieldNo ID{1};
    static constexpr ::agiru::FieldNo IntegrationServiceName{2};
    static constexpr ::agiru::FieldNo Link{3};
    static constexpr ::agiru::FieldNo Enabled{4};
    static constexpr ::agiru::FieldNo Disabled{5};
    static constexpr ::agiru::FieldNo UserSIDFilter{6};
  };

  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::ID}};
};

using PrivacyNotice = PrivacyNotice_Table;

inline constexpr std::array<FieldDef, 6> kPrivacyNoticeFields{{
    Declare<&PrivacyNotice::ID>(
        PrivacyNotice::Field_No::ID, "ID", "ID", offsetof(PrivacyNotice, ID)),
    Declare<&PrivacyNotice::IntegrationServiceName>(
        PrivacyNotice::Field_No::IntegrationServiceName,
        "Integration Service Name",
        "Integration Service Name",
        offsetof(PrivacyNotice, IntegrationServiceName)),
    Declare<&PrivacyNotice::Link>(
        PrivacyNotice::Field_No::Link, "Link", "Link", offsetof(PrivacyNotice, Link)),
    Declare<&PrivacyNotice::Enabled>(
        PrivacyNotice::Field_No::Enabled,
        "Enabled",
        "Enabled",
        offsetof(PrivacyNotice, Enabled),
        Declared{.fieldClass = ::agiru::FieldClass::FlowField,
                 .calcFormula = "Exist(\"Privacy Notice Approval\" WHERE (ID = field(ID), \"User "
                                "SID\" = field(\"User SID Filter\"), Approved = CONST(true)))"}),
    Declare<&PrivacyNotice::Disabled>(
        PrivacyNotice::Field_No::Disabled,
        "Disabled",
        "Disabled",
        offsetof(PrivacyNotice, Disabled),
        Declared{.fieldClass = ::agiru::FieldClass::FlowField,
                 .calcFormula = "Exist(\"Privacy Notice Approval\" WHERE (ID = field(ID), \"User "
                                "SID\" = field(\"User SID Filter\"), Approved = CONST(false)))"}),
    Declare<&PrivacyNotice::UserSIDFilter>(PrivacyNotice::Field_No::UserSIDFilter,
                                           "User SID Filter",
                                           "User SID Filter",
                                           offsetof(PrivacyNotice, UserSIDFilter),
                                           Declared{.fieldClass = ::agiru::FieldClass::FlowFilter}),
}};

inline constexpr std::array<KeyDef, 1> kPrivacyNoticeKeys{{
    KeyDef{.name = "Key1", .fields = PrivacyNotice::kKey1, .clustered = true},
}};

inline constexpr TableDef kPrivacyNoticeTable{
    .id = PrivacyNotice::kId,
    .name = PrivacyNotice::kName,
    .caption = PrivacyNotice::kName,
    .fields = kPrivacyNoticeFields,
    .keys = kPrivacyNoticeKeys,
    .dataPerCompany = false,
};

static_assert(FieldsAreSorted(kPrivacyNoticeTable), "the field table is searched by number");

}

template <> struct agiru::TableTraits<agiru::platform::PrivacyNotice> {
  static constexpr const agiru::TableDef &kTable = agiru::platform::kPrivacyNoticeTable;
};

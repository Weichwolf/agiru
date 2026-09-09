#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/UserPersonalization.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/Code.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <string_view>

/// \file
/// \brief The platform's `All Profile` table (2000000178): every installed profile, and the one the
///        tenant made its default role centre.
///
/// \note IT HAS NO AL SOURCE AND THE FIELD LIST IS TAKEN FROM WHAT THE BASEAPP READS. 246 UT cases
///       reach it through `Conf./Personalization Mgt.GetDefaultProfileID` (2026-09-09), and the
///       fields below are the ones that code names: `Scope`, `App ID`, `Profile ID`, `Description`,
///       `Role Center ID`, `Default Role Center`, `Caption`, `Enabled`, `Promoted` and
///       `Disable Personalization`. The FIELD NUMBERS ARE ASSIGNED HERE in that order [SET]; BC's own
///       are not in the documentation this tree reads, and nothing in AL names a field of this table
///       by number.
/// \warning ITS ROWS COME FROM THE PROFILE CATALOGUE. `ProvisionInstalled` writes one row per
///          translated `profile` object into a clone that lacks it (board:0004), which is what BC
///          does at app install; `Default Role Center` is then the tenant's to set.

namespace agiru::platform {

class AllProfile_Table : public Table<AllProfile_Table> {
public:
  static constexpr TableId kId{2000000178};
  static constexpr std::string_view kName{"All Profile"};
  detail::StateHandle State_Block;
  static constexpr std::size_t kProfileIdLength = 30;
  static constexpr std::size_t kDescriptionLength = 250;
  static constexpr std::size_t kCaptionLength = 100;
  Option<PersonalizationScope> Scope;
  Guid AppID;
  Code<kProfileIdLength> ProfileID;
  Text<kDescriptionLength> Description;
  ::agiru::Integer RoleCenterID{};
  Boolean DefaultRoleCenter{};
  Boolean DisablePersonalization{};
  Text<kCaptionLength> Caption;
  Boolean Enabled{};
  Boolean Promoted{};
  struct Field_No {
    static constexpr ::agiru::FieldNo Scope{1};
    static constexpr ::agiru::FieldNo AppID{2};
    static constexpr ::agiru::FieldNo ProfileID{3};
    static constexpr ::agiru::FieldNo Description{4};
    static constexpr ::agiru::FieldNo RoleCenterID{5};
    static constexpr ::agiru::FieldNo DefaultRoleCenter{6};
    static constexpr ::agiru::FieldNo DisablePersonalization{7};
    static constexpr ::agiru::FieldNo Caption{8};
    static constexpr ::agiru::FieldNo Enabled{9};
    static constexpr ::agiru::FieldNo Promoted{10};
  };
  static constexpr std::array<::agiru::FieldNo, 3> kKey1{
      {Field_No::Scope, Field_No::AppID, Field_No::ProfileID}};
  static constexpr std::array<::agiru::FieldNo, 1> kKey2{{Field_No::RoleCenterID}};
};

using AllProfile = AllProfile_Table;

inline constexpr std::array<FieldDef, 10> kAllProfileFields{{
    Declare<&AllProfile::Scope>(
        AllProfile::Field_No::Scope, "Scope", "Scope", offsetof(AllProfile, Scope)),
    Declare<&AllProfile::AppID>(
        AllProfile::Field_No::AppID, "App ID", "App ID", offsetof(AllProfile, AppID)),
    Declare<&AllProfile::ProfileID>(AllProfile::Field_No::ProfileID,
                                    "Profile ID",
                                    "Profile ID",
                                    offsetof(AllProfile, ProfileID)),
    Declare<&AllProfile::Description>(AllProfile::Field_No::Description,
                                      "Description",
                                      "Description",
                                      offsetof(AllProfile, Description)),
    Declare<&AllProfile::RoleCenterID>(AllProfile::Field_No::RoleCenterID,
                                       "Role Center ID",
                                       "Role Center ID",
                                       offsetof(AllProfile, RoleCenterID)),
    Declare<&AllProfile::DefaultRoleCenter>(AllProfile::Field_No::DefaultRoleCenter,
                                            "Default Role Center",
                                            "Default Role Center",
                                            offsetof(AllProfile, DefaultRoleCenter)),
    Declare<&AllProfile::DisablePersonalization>(AllProfile::Field_No::DisablePersonalization,
                                                 "Disable Personalization",
                                                 "Disable Personalization",
                                                 offsetof(AllProfile, DisablePersonalization)),
    Declare<&AllProfile::Caption>(
        AllProfile::Field_No::Caption, "Caption", "Caption", offsetof(AllProfile, Caption)),
    Declare<&AllProfile::Enabled>(
        AllProfile::Field_No::Enabled, "Enabled", "Enabled", offsetof(AllProfile, Enabled)),
    Declare<&AllProfile::Promoted>(
        AllProfile::Field_No::Promoted, "Promoted", "Promoted", offsetof(AllProfile, Promoted)),
}};

inline constexpr std::array<KeyDef, 2> kAllProfileKeys{{
    KeyDef{.name = "Key1", .fields = AllProfile::kKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = AllProfile::kKey2, .clustered = false},
}};

inline constexpr TableDef kAllProfileTable{
    .id = AllProfile::kId,
    .name = AllProfile::kName,
    .caption = AllProfile::kName,
    .fields = kAllProfileFields,
    .keys = kAllProfileKeys,
    .dataPerCompany = false,
};

static_assert(FieldsAreSorted(kAllProfileTable), "the field table is searched by number");

}

template <> struct agiru::TableTraits<agiru::platform::AllProfile> {
  static constexpr const agiru::TableDef &kTable = agiru::platform::kAllProfileTable;
};

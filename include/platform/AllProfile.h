#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "platform/UserPersonalization.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Code.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <string_view>

/// \file
/// \brief Source-declared System `All Profile` (2000000178).
/// \note Fields, lengths and the sole PK follow System 28 `Virtual Tables/AllProfile.Table.al`.
///       This declaration does not implement the read-only live profile provider (0044).

namespace agiru::platform {

class AllProfile_Table : public Table<AllProfile_Table> {
public:
  static constexpr TableId kId{2000000178};
  static constexpr std::string_view kName{"All Profile"};
  /// \brief Original System declaration's extension availability; not a deployment restriction.
  static constexpr std::string_view kScope{"Cloud"};
  detail::StateHandle State_Block;
  static constexpr std::size_t kProfileIdLength = 30;
  static constexpr std::size_t kDescriptionLength = 2048;
  /// \brief Declared length of app names and obsolete notebook fields.
  static constexpr std::size_t kAppNameLength = 250;
  static constexpr std::size_t kCaptionLength = 100;
  Option<PersonalizationScope> Scope;
  Guid AppID;
  Code<kProfileIdLength> ProfileID;
  Text<kDescriptionLength> Description;
  ::agiru::Integer RoleCenterID{};
  Boolean DefaultRoleCenter{};
  /// \brief Source field 7; pending removal.
  Boolean UseComments{};
  /// \brief Source field 8; pending removal.
  Boolean UseNotes{};
  /// \brief Source field 9; pending removal.
  Boolean UseRecordNotes{};
  /// \brief Source field 10; pending removal.
  Text<kAppNameLength> RecordNotebook;
  /// \brief Source field 11; pending removal.
  Boolean UsePageNotes{};
  /// \brief Source field 12; pending removal.
  Text<kAppNameLength> PageNotebook;
  Boolean DisablePersonalization{};
  Text<kCaptionLength> Caption;
  Boolean Enabled{};
  /// \brief AL `AllProfile."App Name"` -- the name of the app the profile came with.
  Text<kAppNameLength> AppName;
  /// \brief AL `AllProfile.SystemId`.
  Guid SystemId;
  /// \brief AL `AllProfile.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `AllProfile.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `AllProfile.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `AllProfile.SystemModifiedBy`.
  Guid SystemModifiedBy;

  /// \brief Implicit read-only SQL rowversion buffer; provider ownership is separate.
  BigInteger SystemRowVersion{};
  /// \brief Current creator User name; nonstored Runtime-18 FlowField.
  Text<kSystemUserNameLength> SystemCreatedByUserName{};
  /// \brief Current creator full name; nonstored Runtime-18 FlowField.
  Text<kSystemFullNameLength> SystemCreatedByFullName{};
  /// \brief Current modifier User name; nonstored Runtime-18 FlowField.
  Text<kSystemUserNameLength> SystemModifiedByUserName{};
  /// \brief Current modifier full name; nonstored Runtime-18 FlowField.
  Text<kSystemFullNameLength> SystemModifiedByFullName{};

  Boolean Promoted{};

  struct Field_No : SystemFieldNumbers {
    static constexpr ::agiru::FieldNo Scope{1};
    static constexpr ::agiru::FieldNo AppID{2};
    static constexpr ::agiru::FieldNo ProfileID{3};
    static constexpr ::agiru::FieldNo Description{4};
    static constexpr ::agiru::FieldNo RoleCenterID{5};
    static constexpr ::agiru::FieldNo DefaultRoleCenter{6};
    /// \brief Original AL field number.
    static constexpr ::agiru::FieldNo UseComments{7};
    /// \brief Original AL field number.
    static constexpr ::agiru::FieldNo UseNotes{8};
    /// \brief Original AL field number.
    static constexpr ::agiru::FieldNo UseRecordNotes{9};
    /// \brief Original AL field number.
    static constexpr ::agiru::FieldNo RecordNotebook{10};
    /// \brief Original AL field number.
    static constexpr ::agiru::FieldNo UsePageNotes{11};
    /// \brief Original AL field number.
    static constexpr ::agiru::FieldNo PageNotebook{12};
    static constexpr ::agiru::FieldNo DisablePersonalization{13};
    /// \brief Original AL field number of `App Name`.
    static constexpr ::agiru::FieldNo AppName{14};
    static constexpr ::agiru::FieldNo Enabled{15};
    static constexpr ::agiru::FieldNo Caption{16};
    static constexpr ::agiru::FieldNo Promoted{17};
  };

  static constexpr std::array<::agiru::FieldNo, 3> kKey1{
      {Field_No::Scope, Field_No::AppID, Field_No::ProfileID}};
};

using AllProfile = AllProfile_Table;

/// \brief Removal metadata shared by fields 7–12 in System 28 AllProfile.Table.al.
inline constexpr Declared kAllProfileObsoleteNotes{
    .obsoleteState = "Pending",
    .obsoleteReason = "Capacity related to System profiles for which support has been removed.",
};

inline constexpr auto kAllProfileFields = WithImplicitFields<AllProfile,
                                                             ::agiru::SystemFieldProfile::Runtime18,
                                                             ::agiru::TableType::Normal,
                                                             false>(std::array<FieldDef, 17>{{
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
    Declare<&AllProfile::UseComments>(AllProfile::Field_No::UseComments,
                                      "Use Comments",
                                      "Use Comments",
                                      offsetof(AllProfile, UseComments),
                                      kAllProfileObsoleteNotes),
    Declare<&AllProfile::UseNotes>(AllProfile::Field_No::UseNotes,
                                   "Use Notes",
                                   "Use Notes",
                                   offsetof(AllProfile, UseNotes),
                                   kAllProfileObsoleteNotes),
    Declare<&AllProfile::UseRecordNotes>(AllProfile::Field_No::UseRecordNotes,
                                         "Use Record Notes",
                                         "Use Record Notes",
                                         offsetof(AllProfile, UseRecordNotes),
                                         kAllProfileObsoleteNotes),
    Declare<&AllProfile::RecordNotebook>(AllProfile::Field_No::RecordNotebook,
                                         "Record Notebook",
                                         "Record Notebook",
                                         offsetof(AllProfile, RecordNotebook),
                                         kAllProfileObsoleteNotes),
    Declare<&AllProfile::UsePageNotes>(AllProfile::Field_No::UsePageNotes,
                                       "Use Page Notes",
                                       "Use Page Notes",
                                       offsetof(AllProfile, UsePageNotes),
                                       kAllProfileObsoleteNotes),
    Declare<&AllProfile::PageNotebook>(AllProfile::Field_No::PageNotebook,
                                       "Page Notebook",
                                       "Page Notebook",
                                       offsetof(AllProfile, PageNotebook),
                                       kAllProfileObsoleteNotes),
    Declare<&AllProfile::DisablePersonalization>(AllProfile::Field_No::DisablePersonalization,
                                                 "Disable Personalization",
                                                 "Disable Personalization",
                                                 offsetof(AllProfile, DisablePersonalization)),
    Declare<&AllProfile::AppName>(
        AllProfile::Field_No::AppName, "App Name", "App Name", offsetof(AllProfile, AppName)),
    Declare<&AllProfile::Enabled>(
        AllProfile::Field_No::Enabled, "Enabled", "Enabled", offsetof(AllProfile, Enabled)),
    Declare<&AllProfile::Caption>(
        AllProfile::Field_No::Caption, "Caption", "Caption", offsetof(AllProfile, Caption)),
    Declare<&AllProfile::Promoted>(
        AllProfile::Field_No::Promoted, "Promoted", "Promoted", offsetof(AllProfile, Promoted)),
}});

inline constexpr std::array<KeyDef, 1> kAllProfileKeys{{
    KeyDef{.name = "PK", .fields = AllProfile::kKey1, .clustered = true},
}};

inline constexpr TableDef kAllProfileTable{
    .id = AllProfile::kId,
    .name = AllProfile::kName,
    .caption = AllProfile::kName,
    .fields = kAllProfileFields,
    .keys = kAllProfileKeys,
    .dataPerCompany = false,
    .inherentPermissions = "rX",
};

static_assert(FieldsAreSorted(kAllProfileTable), "the field table is searched by number");

}

template <> struct agiru::TableTraits<agiru::platform::AllProfile> {
  static constexpr const agiru::TableDef &kTable = agiru::platform::kAllProfileTable;
};

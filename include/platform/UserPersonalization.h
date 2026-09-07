#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Code.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

/// \file
/// \brief The AL system table `User Personalization` (2000000073) -- which profile a user works in.

namespace agiru::platform {

/// \brief The vocabulary of AL `User Personalization.Scope` -- whose personalisation a row is.
///
/// From the declaration: `OptionMembers = System,Tenant`.
enum class PersonalizationScope : std::int32_t {
  System = 0, ///< Shipped with the object.
  Tenant = 1, ///< The tenant's own.
};

}

/// \brief The vocabulary of AL `User Personalization.Scope`.
template <> struct agiru::OptionTraits<agiru::platform::PersonalizationScope> {
  /// \brief The two scopes.
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      {.ordinal = 0, .name = "System", .caption = "System"},
      {.ordinal = 1, .name = "Tenant", .caption = "Tenant"},
  }};
};

namespace agiru::platform {

/// \brief AL `User Personalization` -- the platform's own table, which no `.al` file declares.
///
/// \note THE DECLARATION IS THE SYSTEM SYMBOLS', not a measurement.
///       `work/symbols/src/Tenant Database Tables/UserPersonalization.Table.al` (`make symbols`)
///       carries it. The predecessor's measured layout, which this file carried until 2026-09-07,
///       had every number wrong but one: this table starts at 3 and runs to 34 with eleven gaps,
///       and a column order sees none of them (board:0607).
///
/// \note NINE OF THE TWENTY DECLARED FIELDS ARE HERE. The debugger flags, `Full Name`,
///       `Language Name`, `Region`, `License Type`, `Customization Status`, `Role` and
///       `Emit Version` are not, and their absence is a hole with a number rather than a decision
///       -- board:0607 replaces this file with the transpiled declaration.
class UserPersonalization_Table : public Table<UserPersonalization_Table> {
public:
  /// \brief The AL table number.
  static constexpr TableId kId{2000000073};

  /// \brief The AL name.
  static constexpr std::string_view kName{"User Personalization"};

  detail::StateHandle State_Block;

  /// \brief The declared length of `Profile ID`.
  static constexpr std::size_t kProfileIdLength = 30;
  /// \brief The declared length of `Company`.
  static constexpr std::size_t kCompanyLength = 30;
  /// \brief The declared length of `Time Zone`.
  static constexpr std::size_t kTimeZoneLength = 180;
  /// \brief The declared length of `User ID`.
  static constexpr std::size_t kUserIdLength = 50;

  /// \brief AL `User Personalization."User SID"`.
  Guid UserSID;
  /// \brief AL `User Personalization."Profile ID"`.
  Code<kProfileIdLength> ProfileID;
  /// \brief AL `User Personalization."Language ID"`.
  ::agiru::Integer LanguageID{};
  /// \brief AL `User Personalization.Company`.
  Text<kCompanyLength> Company;
  /// \brief AL `User Personalization.Scope`.
  Option<PersonalizationScope> Scope;
  /// \brief AL `User Personalization."App ID"`.
  Guid AppID;
  /// \brief AL `User Personalization."Locale ID"`.
  ::agiru::Integer LocaleID{};
  /// \brief AL `User Personalization."Time Zone"`.
  Text<kTimeZoneLength> TimeZone;
  /// \brief AL `User Personalization."User ID"`.
  Code<kUserIdLength> UserID;

  /// \brief The field numbers, from the system symbols' declaration.
  struct Field_No {
    /// \brief The AL field number of `User SID`.
    static constexpr ::agiru::FieldNo UserSID{3};
    /// \brief The AL field number of `Profile ID`.
    static constexpr ::agiru::FieldNo ProfileID{9};
    /// \brief The AL field number of `Language ID`.
    static constexpr ::agiru::FieldNo LanguageID{12};
    /// \brief The AL field number of `Company`.
    static constexpr ::agiru::FieldNo Company{15};
    /// \brief The AL field number of `Scope`.
    static constexpr ::agiru::FieldNo Scope{11};
    /// \brief The AL field number of `App ID`.
    static constexpr ::agiru::FieldNo AppID{10};
    /// \brief The AL field number of `Locale ID`.
    static constexpr ::agiru::FieldNo LocaleID{27};
    /// \brief The AL field number of `Time Zone`.
    static constexpr ::agiru::FieldNo TimeZone{30};
    /// \brief The AL field number of `User ID`.
    static constexpr ::agiru::FieldNo UserID{6};
  };

  /// \brief The primary key.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::UserSID}};
  /// \brief The key on the profile.
  static constexpr std::array<::agiru::FieldNo, 1> kKey2{{Field_No::ProfileID}};
  /// \brief The key on the company.
  static constexpr std::array<::agiru::FieldNo, 1> kKey3{{Field_No::Company}};
};

/// \brief AL `User Personalization`, under the name AL gives it.
using UserPersonalization = UserPersonalization_Table;

/// \brief The field table of the system `User Personalization` table.
inline constexpr std::array<FieldDef, 9> kUserPersonalizationFields{{
    Declare<&UserPersonalization::UserSID>(UserPersonalization::Field_No::UserSID,
                                           "User SID",
                                           "User SID",
                                           offsetof(UserPersonalization, UserSID)),
    Declare<&UserPersonalization::UserID>(UserPersonalization::Field_No::UserID,
                                          "User ID",
                                          "User ID",
                                          offsetof(UserPersonalization, UserID)),
    Declare<&UserPersonalization::ProfileID>(UserPersonalization::Field_No::ProfileID,
                                             "Profile ID",
                                             "Profile ID",
                                             offsetof(UserPersonalization, ProfileID)),
    Declare<&UserPersonalization::AppID>(UserPersonalization::Field_No::AppID,
                                         "App ID",
                                         "App ID",
                                         offsetof(UserPersonalization, AppID)),
    Declare<&UserPersonalization::Scope>(UserPersonalization::Field_No::Scope,
                                         "Scope",
                                         "Scope",
                                         offsetof(UserPersonalization, Scope)),
    Declare<&UserPersonalization::LanguageID>(UserPersonalization::Field_No::LanguageID,
                                              "Language ID",
                                              "Language ID",
                                              offsetof(UserPersonalization, LanguageID)),
    Declare<&UserPersonalization::Company>(UserPersonalization::Field_No::Company,
                                           "Company",
                                           "Company",
                                           offsetof(UserPersonalization, Company)),
    Declare<&UserPersonalization::LocaleID>(UserPersonalization::Field_No::LocaleID,
                                            "Locale ID",
                                            "Locale ID",
                                            offsetof(UserPersonalization, LocaleID)),
    Declare<&UserPersonalization::TimeZone>(UserPersonalization::Field_No::TimeZone,
                                            "Time Zone",
                                            "Time Zone",
                                            offsetof(UserPersonalization, TimeZone)),
}};

/// \brief The keys of the system `User Personalization` table.
inline constexpr std::array<KeyDef, 3> kUserPersonalizationKeys{{
    KeyDef{.name = "Key1", .fields = UserPersonalization::kKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = UserPersonalization::kKey2, .clustered = false},
    KeyDef{.name = "Key3", .fields = UserPersonalization::kKey3, .clustered = false},
}};

/// \brief The declaration of the system `User Personalization` table.
inline constexpr TableDef kUserPersonalizationTable{
    .id = UserPersonalization::kId,
    .name = UserPersonalization::kName,
    .caption = UserPersonalization::kName,
    .fields = kUserPersonalizationFields,
    .keys = kUserPersonalizationKeys,
};

static_assert(FieldsAreSorted(kUserPersonalizationTable), "the field table is searched by number");

}

/// \brief What the runtime reaches the system `User Personalization` table through.
template <> struct agiru::TableTraits<agiru::platform::UserPersonalization> {
  /// \brief The table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kUserPersonalizationTable;
};

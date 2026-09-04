#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Table.h"
#include "type/Code.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <string_view>

/// \file
/// \brief The AL system table `User Personalization` (2000000073) -- which profile a user works in.

namespace agiru::platform {

/// \brief AL `User Personalization` -- the platform's own table, which no `.al` file declares.
///
/// \note THE FIELD NUMBERS ARE THE PREDECESSOR'S, MEASURED RATHER THAN DOCUMENTED, exactly as
///       `platform/User.h` records for the user table: `~/Git/openerp/openerp/runtime/base/
///       system_tables.py` carries them and is 97 % green on the suite that reads them. Numbers 2,
///       4 and 5 are absent there and absent here -- inventing one would put a number in the
///       metadata that nothing can check.
///
/// \note `Scope` CARRIES NO VOCABULARY, for the reason `User.State` gives: the AL source names the
///       members and nowhere states their ORDER, so an ordinal here would be a guess wearing a
///       number. `Option<>` holds the ordinal without claiming a vocabulary (board:0032).
class UserPersonalization_Table : public Table<UserPersonalization_Table> {
public:
  /// \brief The AL table number.
  static constexpr TableId kId{2000000073};

  /// \brief The AL name.
  static constexpr std::string_view kName{"User Personalization"};

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
  Option<void> Scope;
  /// \brief AL `User Personalization."App ID"`.
  Guid AppID;
  /// \brief AL `User Personalization."Locale ID"`.
  ::agiru::Integer LocaleID{};
  /// \brief AL `User Personalization."Time Zone"`.
  Text<kTimeZoneLength> TimeZone;
  /// \brief AL `User Personalization."User ID"`.
  Code<kUserIdLength> UserID;

  /// \brief The field numbers, from the predecessor's measured layout.
  struct Field_No {
    /// \brief The AL field number of `User SID`.
    static constexpr ::agiru::FieldNo UserSID{1};
    /// \brief The AL field number of `Profile ID`.
    static constexpr ::agiru::FieldNo ProfileID{3};
    /// \brief The AL field number of `Language ID`.
    static constexpr ::agiru::FieldNo LanguageID{6};
    /// \brief The AL field number of `Company`.
    static constexpr ::agiru::FieldNo Company{7};
    /// \brief The AL field number of `Scope`.
    static constexpr ::agiru::FieldNo Scope{8};
    /// \brief The AL field number of `App ID`.
    static constexpr ::agiru::FieldNo AppID{9};
    /// \brief The AL field number of `Locale ID`.
    static constexpr ::agiru::FieldNo LocaleID{10};
    /// \brief The AL field number of `Time Zone`.
    static constexpr ::agiru::FieldNo TimeZone{11};
    /// \brief The AL field number of `User ID`.
    static constexpr ::agiru::FieldNo UserID{12};
  };

  /// \brief The primary key.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::UserSID}};
};

/// \brief AL `User Personalization`, under the name AL gives it.
using UserPersonalization = UserPersonalization_Table;

/// \brief The field table of the system `User Personalization` table.
inline constexpr std::array<FieldDef, 9> kUserPersonalizationFields{{
    Declare<&UserPersonalization::UserSID>(UserPersonalization::Field_No::UserSID,
                                           "User SID",
                                           "User SID",
                                           offsetof(UserPersonalization, UserSID)),
    Declare<&UserPersonalization::ProfileID>(UserPersonalization::Field_No::ProfileID,
                                             "Profile ID",
                                             "Profile ID",
                                             offsetof(UserPersonalization, ProfileID)),
    Declare<&UserPersonalization::LanguageID>(UserPersonalization::Field_No::LanguageID,
                                              "Language ID",
                                              "Language ID",
                                              offsetof(UserPersonalization, LanguageID)),
    Declare<&UserPersonalization::Company>(UserPersonalization::Field_No::Company,
                                           "Company",
                                           "Company",
                                           offsetof(UserPersonalization, Company)),
    Declare<&UserPersonalization::Scope>(UserPersonalization::Field_No::Scope,
                                         "Scope",
                                         "Scope",
                                         offsetof(UserPersonalization, Scope)),
    Declare<&UserPersonalization::AppID>(UserPersonalization::Field_No::AppID,
                                         "App ID",
                                         "App ID",
                                         offsetof(UserPersonalization, AppID)),
    Declare<&UserPersonalization::LocaleID>(UserPersonalization::Field_No::LocaleID,
                                            "Locale ID",
                                            "Locale ID",
                                            offsetof(UserPersonalization, LocaleID)),
    Declare<&UserPersonalization::TimeZone>(UserPersonalization::Field_No::TimeZone,
                                            "Time Zone",
                                            "Time Zone",
                                            offsetof(UserPersonalization, TimeZone)),
    Declare<&UserPersonalization::UserID>(UserPersonalization::Field_No::UserID,
                                          "User ID",
                                          "User ID",
                                          offsetof(UserPersonalization, UserID)),
}};

/// \brief The keys of the system `User Personalization` table.
inline constexpr std::array<KeyDef, 1> kUserPersonalizationKeys{{
    KeyDef{.name = "PK", .fields = UserPersonalization::kKey1, .clustered = true},
}};

/// \brief The declaration of the system `User Personalization` table.
inline constexpr TableDef kUserPersonalizationTable{
    .id = UserPersonalization::kId,
    .name = UserPersonalization::kName,
    .caption = UserPersonalization::kName,
    .fields = kUserPersonalizationFields,
    .keys = kUserPersonalizationKeys,
};

}

/// \brief What the runtime reaches the system `User Personalization` table through.
template <> struct agiru::TableTraits<agiru::platform::UserPersonalization> {
  /// \brief The table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kUserPersonalizationTable;
};

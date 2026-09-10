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
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

/// \file
/// \brief The AL system table `User` (2000000120) -- who may sign in, and as what.

namespace agiru::platform {

/// \brief The vocabulary of AL `User.State`.
///
/// From the declaration: `OptionMembers = Enabled,Disabled`.
enum class UserState : std::int32_t {
  Enabled = 0,  ///< The user may sign in.
  Disabled = 1, ///< The user may not.
};

/// \brief The vocabulary of AL `User."License Type"` -- what a user counts as against the licence.
///
/// From the declaration, in the declared ORDER, which is what an ordinal comparison depends on.
enum class UserLicenseType : std::int32_t {
  FullUser = 0,              ///< A named user with the full application.
  LimitedUser = 1,           ///< Restricted to three writable tables outside the free range.
  DeviceOnlyUser = 2,        ///< Licensed per device rather than per person.
  WindowsGroup = 3,          ///< A group, not a person: the members inherit it.
  ExternalUser = 4,          ///< Signs in from outside the tenant.
  ExternalAdministrator = 5, ///< A delegated administrator.
  ExternalAccountant = 6,    ///< A delegated accountant.
  Application = 7,           ///< A service principal rather than a person.
  AADGroup = 8,              ///< An Entra group.
  Agent = 9,                 ///< An agent acting on a user's behalf.
};

}

/// \brief The vocabulary of AL `User.State`.
template <> struct agiru::OptionTraits<agiru::platform::UserState> {
  /// \brief The two states.
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      {.ordinal = 0, .name = "Enabled", .caption = "Enabled"},
      {.ordinal = 1, .name = "Disabled", .caption = "Disabled"},
  }};
};

/// \brief The vocabulary of AL `User."License Type"`.
template <> struct agiru::OptionTraits<agiru::platform::UserLicenseType> {
  /// \brief The ten types, spelled as AL spells them.
  static constexpr std::array<agiru::EnumValueDef, 10> kValues{{
      {.ordinal = 0, .name = "Full User", .caption = "Full User"},
      {.ordinal = 1, .name = "Limited User", .caption = "Limited User"},
      {.ordinal = 2, .name = "Device Only User", .caption = "Device Only User"},
      {.ordinal = 3, .name = "Windows Group", .caption = "Windows Group"},
      {.ordinal = 4, .name = "External User", .caption = "External User"},
      {.ordinal = 5, .name = "External Administrator", .caption = "External Administrator"},
      {.ordinal = 6, .name = "External Accountant", .caption = "External Accountant"},
      {.ordinal = 7, .name = "Application", .caption = "Application"},
      {.ordinal = 8, .name = "AAD Group", .caption = "AAD Group"},
      {.ordinal = 9, .name = "Agent", .caption = "Agent"},
  }};
};

namespace agiru::platform {

/// \brief AL `User` -- the platform's own user table, which no `.al` file declares.
///
/// \note THE DECLARATION IS THE SYSTEM SYMBOLS', not a measurement.
///       `work/symbols/src/Tenant Database Tables/User.Table.al` (`make symbols`) carries every
///       number, length and option member. The predecessor's measured layout, which this file
///       carried until 2026-09-07, was off by one from field 2 onwards and put `Application ID` at
///       13 where the declaration says 16 -- a column order cannot see a gap, and this table has
///       four (6, 9, 12, 13) (board:0607).
class User_Table : public Table<User_Table> {
public:
  /// \brief The AL table number.
  static constexpr TableId kId{2000000120};

  /// \brief The AL name.
  static constexpr std::string_view kName{"User"};

  detail::StateHandle State_Block;

  /// \brief The declared lengths, which are AL's and not this file's.
  static constexpr std::size_t kUserNameLength = 50;
  /// \brief The declared length of `Full Name`.
  static constexpr std::size_t kFullNameLength = 80;
  /// \brief The declared length of `Windows Security ID`.
  static constexpr std::size_t kSecurityIdLength = 119;
  /// \brief The declared length of the two e-mail fields and of `Exchange Identifier`.
  static constexpr std::size_t kEmailLength = 250;

  /// \brief AL `User."User Security ID"`.
  Guid UserSecurityID;
  /// \brief AL `User."User Name"`.
  Code<kUserNameLength> UserName;
  /// \brief AL `User."Full Name"`.
  Text<kFullNameLength> FullName;
  /// \brief AL `User.State`.
  Option<UserState> State;
  /// \brief AL `User."Expiry Date"`.
  DateTime ExpiryDate;
  /// \brief AL `User."Windows Security ID"`.
  Text<kSecurityIdLength> WindowsSecurityID;
  /// \brief AL `User."Change Password"`.
  Boolean ChangePassword{};
  /// \brief AL `User."License Type"`.
  Option<UserLicenseType> LicenseType;
  /// \brief AL `User."Authentication Email"`.
  Code<kEmailLength> AuthenticationEmail;
  /// \brief AL `User."Contact Email"`.
  Text<kEmailLength> ContactEmail;
  /// \brief AL `User."Exchange Identifier"`.
  Text<kEmailLength> ExchangeIdentifier;
  /// \brief AL `User."Application ID"`, the Entra application a service user authenticates as.
  Guid ApplicationID;

  /// \brief AL `User.SystemId`.
  Guid SystemId;
  /// \brief AL `User.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `User.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `User.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `User.SystemModifiedBy`.
  Guid SystemModifiedBy;

  /// \brief The field numbers, from the system symbols' declaration.
  struct Field_No {
    /// \brief The AL field number of `User Security ID`.
    static constexpr ::agiru::FieldNo UserSecurityID{1};
    /// \brief The AL field number of `User Name`.
    static constexpr ::agiru::FieldNo UserName{2};
    /// \brief The AL field number of `Full Name`.
    static constexpr ::agiru::FieldNo FullName{3};
    /// \brief The AL field number of `State`.
    static constexpr ::agiru::FieldNo State{4};
    /// \brief The AL field number of `Expiry Date`.
    static constexpr ::agiru::FieldNo ExpiryDate{5};
    /// \brief The AL field number of `Windows Security ID`.
    static constexpr ::agiru::FieldNo WindowsSecurityID{7};
    /// \brief The AL field number of `Change Password`.
    static constexpr ::agiru::FieldNo ChangePassword{8};
    /// \brief The AL field number of `License Type`.
    static constexpr ::agiru::FieldNo LicenseType{10};
    /// \brief The AL field number of `Authentication Email`.
    static constexpr ::agiru::FieldNo AuthenticationEmail{11};
    /// \brief The AL field number of `Contact Email`.
    static constexpr ::agiru::FieldNo ContactEmail{14};
    /// \brief The AL field number of `Exchange Identifier`.
    static constexpr ::agiru::FieldNo ExchangeIdentifier{15};
    /// \brief The AL field number of `Application ID`.
    static constexpr ::agiru::FieldNo ApplicationID{16};
  };

  /// \brief The primary key.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::UserSecurityID}};
  /// \brief The key on the user's name, which is how AL looks a user up.
  static constexpr std::array<::agiru::FieldNo, 1> kKey2{{Field_No::UserName}};
  /// \brief The key on the Windows security id.
  static constexpr std::array<::agiru::FieldNo, 1> kKey3{{Field_No::WindowsSecurityID}};
};

/// \brief AL `User`, under the name AL gives it.
using User = User_Table;

/// \brief The field table of the system `User` table.
inline constexpr std::array<FieldDef, 12> kUserFields{{
    Declare<&User::UserSecurityID>(User::Field_No::UserSecurityID,
                                   "User Security ID",
                                   "User Security ID",
                                   offsetof(User, UserSecurityID)),
    Declare<&User::UserName>(
        User::Field_No::UserName, "User Name", "User Name", offsetof(User, UserName)),
    Declare<&User::FullName>(
        User::Field_No::FullName, "Full Name", "Full Name", offsetof(User, FullName)),
    Declare<&User::State>(User::Field_No::State, "State", "State", offsetof(User, State)),
    Declare<&User::ExpiryDate>(
        User::Field_No::ExpiryDate, "Expiry Date", "Expiry Date", offsetof(User, ExpiryDate)),
    Declare<&User::WindowsSecurityID>(User::Field_No::WindowsSecurityID,
                                      "Windows Security ID",
                                      "Windows Security ID",
                                      offsetof(User, WindowsSecurityID)),
    Declare<&User::ChangePassword>(User::Field_No::ChangePassword,
                                   "Change Password",
                                   "Change Password",
                                   offsetof(User, ChangePassword)),
    Declare<&User::LicenseType>(
        User::Field_No::LicenseType, "License Type", "License Type", offsetof(User, LicenseType)),
    Declare<&User::AuthenticationEmail>(User::Field_No::AuthenticationEmail,
                                        "Authentication Email",
                                        "Authentication Email",
                                        offsetof(User, AuthenticationEmail)),
    Declare<&User::ContactEmail>(User::Field_No::ContactEmail,
                                 "Contact Email",
                                 "Contact Email",
                                 offsetof(User, ContactEmail)),
    Declare<&User::ExchangeIdentifier>(User::Field_No::ExchangeIdentifier,
                                       "Exchange Identifier",
                                       "Exchange Identifier",
                                       offsetof(User, ExchangeIdentifier)),
    Declare<&User::ApplicationID>(User::Field_No::ApplicationID,
                                  "Application ID",
                                  "Application ID",
                                  offsetof(User, ApplicationID)),
}};

/// \brief The keys of the system `User` table.
inline constexpr std::array<KeyDef, 3> kUserKeys{{
    KeyDef{.name = "Key1", .fields = User::kKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = User::kKey2, .clustered = false},
    KeyDef{.name = "Key3", .fields = User::kKey3, .clustered = false},
}};

/// \brief The declaration of the system `User` table.
inline constexpr TableDef kUserTable{
    .id = User::kId,
    .name = User::kName,
    .caption = User::kName,
    .fields = kUserFields,
    .keys = kUserKeys,
};

static_assert(FieldsAreSorted(kUserTable), "the field table is searched by number");

}

/// \brief What the runtime reaches the system `User` table through.
template <> struct agiru::TableTraits<agiru::platform::User> {
  /// \brief The table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kUserTable;
};

#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/Code.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <string_view>

/// \file
/// \brief The AL system table `User` (2000000120) -- who may sign in, and as what.

namespace agiru::platform {

/// \brief AL `User` -- the platform's own user table, which no `.al` file declares.
///
/// \note THE FIELD NUMBERS ARE THE PREDECESSOR'S, MEASURED RATHER THAN DOCUMENTED. No page in
///       `dev-itpro` tabulates them; `~/Git/openerp/openerp/runtime/base/system_tables.py` records
///       them and is 97 % green on the suite that reads them. Number 2 is absent there and absent
///       here: inventing one would put a number in the metadata that nothing can check.
///
/// \note `State` AND `License Type` CARRY NO VOCABULARY YET, and that is a refusal rather than an
///       omission. The AL source names their members -- `Enabled`, `Disabled`, `"Full User"`,
///       `"Limited User"`, `"Windows Group"`, `"AAD Group"`, `"External User"`, `Application`,
///       `Agent` -- and nowhere states their ORDER, so an ordinal here would be a guess wearing a
///       number. `Option<>` carries the ordinal without claiming a vocabulary (board:0032).
class User_Table : public Table<User_Table> {
public:
  /// \brief The AL table number.
  static constexpr TableId kId{2000000120};

  /// \brief The AL name.
  static constexpr std::string_view kName{"User"};

  /// \brief The declared lengths, which are AL's and not this file's.
  static constexpr std::size_t kUserNameLength = 50;
  /// \brief The declared length of `Full Name`.
  static constexpr std::size_t kFullNameLength = 80;
  /// \brief The declared length of `Windows Security ID`.
  static constexpr std::size_t kSecurityIdLength = 119;
  /// \brief The declared length of the two e-mail fields.
  static constexpr std::size_t kEmailLength = 250;

  /// \brief AL `User."User Security ID"`.
  Guid UserSecurityID;
  /// \brief AL `User."User Name"`.
  Code<kUserNameLength> UserName;
  /// \brief AL `User."Full Name"`.
  Text<kFullNameLength> FullName;
  /// \brief AL `User.State`.
  Option<void> State;
  /// \brief AL `User."Expiry Date"`.
  DateTime ExpiryDate;
  /// \brief AL `User."Windows Security ID"`.
  Text<kSecurityIdLength> WindowsSecurityID;
  /// \brief AL `User."Change Password"`.
  Boolean ChangePassword{};
  /// \brief AL `User."License Type"`.
  Option<void> LicenseType;
  /// \brief AL `User."Authentication Email"`.
  Code<kEmailLength> AuthenticationEmail;
  /// \brief AL `User."Contact Email"`.
  Text<kEmailLength> ContactEmail;
  /// \brief AL `User."Exchange Identifier"`.
  Text<kFullNameLength> ExchangeIdentifier;

  /// \brief The field numbers, from the predecessor's measured layout.
  struct Field_No {
    /// \brief The AL field number of `User Security ID`.
    static constexpr ::agiru::FieldNo UserSecurityID{1};
    /// \brief The AL field number of `User Name`.
    static constexpr ::agiru::FieldNo UserName{3};
    /// \brief The AL field number of `Full Name`.
    static constexpr ::agiru::FieldNo FullName{4};
    /// \brief The AL field number of `State`.
    static constexpr ::agiru::FieldNo State{5};
    /// \brief The AL field number of `Expiry Date`.
    static constexpr ::agiru::FieldNo ExpiryDate{6};
    /// \brief The AL field number of `Windows Security ID`.
    static constexpr ::agiru::FieldNo WindowsSecurityID{7};
    /// \brief The AL field number of `Change Password`.
    static constexpr ::agiru::FieldNo ChangePassword{8};
    /// \brief The AL field number of `License Type`.
    static constexpr ::agiru::FieldNo LicenseType{9};
    /// \brief The AL field number of `Authentication Email`.
    static constexpr ::agiru::FieldNo AuthenticationEmail{10};
    /// \brief The AL field number of `Contact Email`.
    static constexpr ::agiru::FieldNo ContactEmail{11};
    /// \brief The AL field number of `Exchange Identifier`.
    static constexpr ::agiru::FieldNo ExchangeIdentifier{12};
  };

  /// \brief The primary key.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::UserSecurityID}};
};

/// \brief AL `User`, under the name AL gives it.
using User = User_Table;

/// \brief The field table of the system `User` table.
inline constexpr std::array<FieldDef, 11> kUserFields{{
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
}};

/// \brief The keys of the system `User` table.
inline constexpr std::array<KeyDef, 1> kUserKeys{{
    KeyDef{.name = "PK", .fields = User::kKey1, .clustered = true},
}};

/// \brief The declaration of the system `User` table.
inline constexpr TableDef kUserTable{
    .id = User::kId,
    .name = User::kName,
    .caption = User::kName,
    .fields = kUserFields,
    .keys = kUserKeys,
};

}

/// \brief What the runtime reaches the system `User` table through.
template <> struct agiru::TableTraits<agiru::platform::User> {
  /// \brief The table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kUserTable;
};

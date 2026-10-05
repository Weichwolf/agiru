#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "platform/UserLicenseType.h"
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

/// \brief System `User Personalization."Customization Status"` members.
enum class CustomizationStatus : std::int32_t {
  Updated = 0,             ///< Current customizations.
  RecompilationNeeded = 1, ///< Customizations need compilation.
  RecompilationFailed = 2, ///< Compilation failed.
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

/// \brief Declared customization-state vocabulary.
template <> struct agiru::OptionTraits<agiru::platform::CustomizationStatus> {
  /// \brief The three System-symbol members.
  static constexpr std::array<agiru::EnumValueDef, 3> kValues{{
      {.ordinal = 0, .name = "Updated", .caption = "Updated"},
      {.ordinal = 1, .name = "Recompilation Needed", .caption = "Recompilation Needed"},
      {.ordinal = 2, .name = "Recompilation Failed", .caption = "Recompilation Failed"},
  }};
};

namespace agiru::platform {

/// \brief System `User Personalization`, declared by the pinned System symbols.
/// \note All nineteen declared fields are retained, including obsolete fields and six lookup
///       FlowFields. Runtime calculation uses their CalcFormula metadata, not table-specific code.
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
  /// \brief Full name, language name and region use Text[80] in System symbols.
  static constexpr std::size_t kDisplayNameLength = 80;
  /// \brief Declared Role caption length.
  static constexpr std::size_t kRoleLength = 100;

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
  /// \brief AL `Full Name`, a User lookup FlowField.
  Text<kDisplayNameLength> FullName;
  /// \brief AL `Language Name`, a Windows Language lookup FlowField.
  Text<kDisplayNameLength> LanguageName;
  /// \brief Obsolete debugger flag; InitValue is true in System symbols.
  Boolean DebuggerBreakOnError = true;
  /// \brief Obsolete debugger record-change flag.
  Boolean DebuggerBreakOnRecChanges{};
  /// \brief Obsolete debugger flag; InitValue is true in System symbols.
  Boolean DebuggerSkipSystemTriggers = true;
  /// \brief AL `Region`, a Windows Language lookup FlowField.
  Text<kDisplayNameLength> Region;
  /// \brief AL `License Type`, a User lookup FlowField with the shared license vocabulary.
  Option<UserLicenseType> LicenseType;
  /// \brief Internal customization-compilation state.
  Option<platform::CustomizationStatus> CustomizationStatus;
  /// \brief AL `Role`, an All Profile lookup FlowField.
  Text<kRoleLength> Role;
  /// \brief AL `Emit Version`.
  ::agiru::Integer EmitVersion{};

  /// \brief AL `UserPersonalization.SystemId`.
  Guid SystemId;
  /// \brief AL `UserPersonalization.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `UserPersonalization.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `UserPersonalization.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `UserPersonalization.SystemModifiedBy`.
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

  /// \brief The field numbers, from the system symbols' declaration.
  struct Field_No : SystemFieldNumbers {
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
    /// \brief AL field number of `Full Name`.
    static constexpr ::agiru::FieldNo FullName{7};
    /// \brief AL field number of `Language Name`.
    static constexpr ::agiru::FieldNo LanguageName{13};
    /// \brief AL field number of `Debugger Break On Error`.
    static constexpr ::agiru::FieldNo DebuggerBreakOnError{18};
    /// \brief AL field number of `Debugger Break On Rec Changes`.
    static constexpr ::agiru::FieldNo DebuggerBreakOnRecChanges{21};
    /// \brief AL field number of `Debugger Skip System Triggers`.
    static constexpr ::agiru::FieldNo DebuggerSkipSystemTriggers{24};
    /// \brief AL field number of `Region`.
    static constexpr ::agiru::FieldNo Region{28};
    /// \brief AL field number of `License Type`.
    static constexpr ::agiru::FieldNo LicenseType{31};
    /// \brief AL field number of `Customization Status`.
    static constexpr ::agiru::FieldNo CustomizationStatus{32};
    /// \brief AL field number of `Role`.
    static constexpr ::agiru::FieldNo Role{33};
    /// \brief AL field number of `Emit Version`.
    static constexpr ::agiru::FieldNo EmitVersion{34};
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
inline constexpr auto kUserPersonalizationFields =
    WithImplicitFields<UserPersonalization,
                       ::agiru::SystemFieldProfile::Runtime18,
                       ::agiru::TableType::Normal,
                       false>(std::array<FieldDef, 19>{{
        Declare<&UserPersonalization::UserSID>(
            UserPersonalization::Field_No::UserSID,
            "User SID",
            "User SID",
            offsetof(UserPersonalization, UserSID),
            {.relationTable = "User", .relationField = "User Security ID"}),
        Declare<&UserPersonalization::UserID>(
            UserPersonalization::Field_No::UserID,
            "User ID",
            "User ID",
            offsetof(UserPersonalization, UserID),
            {.fieldClass = ::agiru::FieldClass::FlowField,
             .calcFormula =
                 R"(Lookup(User."User Name" WHERE("User Security ID" = FIELD("User SID"))))"}),
        Declare<&UserPersonalization::FullName>(
            UserPersonalization::Field_No::FullName,
            "Full Name",
            "Full Name",
            offsetof(UserPersonalization, FullName),
            {.fieldClass = ::agiru::FieldClass::FlowField,
             .calcFormula = R"(Lookup(User."Full Name" WHERE("User Name" = FIELD("User ID"))))"}),
        Declare<&UserPersonalization::ProfileID>(
            UserPersonalization::Field_No::ProfileID,
            "Profile ID",
            "Profile ID",
            offsetof(UserPersonalization, ProfileID),
            {.relationTable = "All Profile", .relationField = "Profile ID"}),
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
        Declare<&UserPersonalization::LanguageName>(
            UserPersonalization::Field_No::LanguageName,
            "Language Name",
            "Language",
            offsetof(UserPersonalization, LanguageName),
            {.fieldClass = ::agiru::FieldClass::FlowField,
             .calcFormula =
                 R"(Lookup("Windows Language".Name WHERE("Language ID" = FIELD("Language ID"))))"}),
        Declare<&UserPersonalization::Company>(
            UserPersonalization::Field_No::Company,
            "Company",
            "Company",
            offsetof(UserPersonalization, Company),
            {.relationTable = "Company", .relationField = "Name"}),
        Declare<&UserPersonalization::DebuggerBreakOnError>(
            UserPersonalization::Field_No::DebuggerBreakOnError,
            "Debugger Break On Error",
            "Debugger Break On Error",
            offsetof(UserPersonalization, DebuggerBreakOnError),
            {.initValue = "true",
             .obsoleteState = "Removed",
             .obsoleteReason = "Support for the classic debugger engine has been removed."}),
        Declare<&UserPersonalization::DebuggerBreakOnRecChanges>(
            UserPersonalization::Field_No::DebuggerBreakOnRecChanges,
            "Debugger Break On Rec Changes",
            "Debugger Break On Rec Changes",
            offsetof(UserPersonalization, DebuggerBreakOnRecChanges),
            {.obsoleteState = "Removed",
             .obsoleteReason = "Support for the classic debugger engine has been removed."}),
        Declare<&UserPersonalization::DebuggerSkipSystemTriggers>(
            UserPersonalization::Field_No::DebuggerSkipSystemTriggers,
            "Debugger Skip System Triggers",
            "Debugger Skip System Triggers",
            offsetof(UserPersonalization, DebuggerSkipSystemTriggers),
            {.initValue = "true",
             .obsoleteState = "Removed",
             .obsoleteReason = "Support for the classic debugger engine has been removed."}),
        Declare<&UserPersonalization::LocaleID>(UserPersonalization::Field_No::LocaleID,
                                                "Locale ID",
                                                "Locale ID",
                                                offsetof(UserPersonalization, LocaleID)),
        Declare<&UserPersonalization::Region>(
            UserPersonalization::Field_No::Region,
            "Region",
            "Region",
            offsetof(UserPersonalization, Region),
            {.fieldClass = ::agiru::FieldClass::FlowField,
             .calcFormula =
                 R"(Lookup("Windows Language".Name WHERE("Language ID" = FIELD("Locale ID"))))"}),
        Declare<&UserPersonalization::TimeZone>(UserPersonalization::Field_No::TimeZone,
                                                "Time Zone",
                                                "Time Zone",
                                                offsetof(UserPersonalization, TimeZone)),
        Declare<&UserPersonalization::LicenseType>(
            UserPersonalization::Field_No::LicenseType,
            "License Type",
            "License Type",
            offsetof(UserPersonalization, LicenseType),
            {.fieldClass = ::agiru::FieldClass::FlowField,
             .calcFormula =
                 R"(Lookup(User."License Type" WHERE("User Security ID" = FIELD("User SID"))))"}),
        Declare<&UserPersonalization::CustomizationStatus>(
            UserPersonalization::Field_No::CustomizationStatus,
            "Customization Status",
            "Customization Status",
            offsetof(UserPersonalization, CustomizationStatus),
            {.access = "Internal"}),
        Declare<&UserPersonalization::Role>(
            UserPersonalization::Field_No::Role,
            "Role",
            "Role",
            offsetof(UserPersonalization, Role),
            {.fieldClass = ::agiru::FieldClass::FlowField,
             .calcFormula = "Lookup(\"All Profile\".Caption WHERE(\"Profile ID\" = FIELD(\"Profile "
                            "ID\"), Scope = FIELD(Scope), \"App ID\" = FIELD(\"App ID\")))"}),
        Declare<&UserPersonalization::EmitVersion>(UserPersonalization::Field_No::EmitVersion,
                                                   "Emit Version",
                                                   "Emit Version",
                                                   offsetof(UserPersonalization, EmitVersion)),
    }});

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
    .dataPerCompany = false,
    .replicateData = false,
};

static_assert(FieldsAreSorted(kUserPersonalizationTable), "the field table is searched by number");

}

/// \brief What the runtime reaches the system `User Personalization` table through.
template <> struct agiru::TableTraits<agiru::platform::UserPersonalization> {
  /// \brief The table declaration.
  static constexpr const agiru::TableDef &kTable = agiru::platform::kUserPersonalizationTable;
};

#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "runtime/Table.h"
#include "type/BigInteger.h"
#include "type/Blob.h"
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
/// \brief Saved object settings, declared by System.app 28.0.53152.0 in
///        `src/Tenant Database Tables/ObjectOptions.Table.al`.

namespace agiru::platform {

/// \brief The `Object Type` an option set belongs to, as the platform declares it.
enum class ObjectOptionsObjectType : std::int32_t {
  Report = 3,  ///< A report's request page.
  XMLport = 6, ///< An XMLport's request page.
  Page = 8,    ///< A page's settings.
};

}

/// \brief The `Object Type` members, as the platform names them.
template <> struct agiru::OptionTraits<agiru::platform::ObjectOptionsObjectType> {
  /// \brief All twenty source positions; unnamed positions remain empty, not object kinds.
  static constexpr auto kValues = [] {
    std::array<agiru::EnumValueDef, 20> values{};
    for (std::size_t i = 0; i < values.size(); ++i) {
      values[i].ordinal = static_cast<std::int32_t>(i);
    }
    const auto named = [&values](agiru::platform::ObjectOptionsObjectType kind,
                                 std::string_view name,
                                 std::string_view caption) {
      const auto ordinal = static_cast<std::int32_t>(kind);
      values[static_cast<std::size_t>(ordinal)] = {
          .ordinal = ordinal, .name = name, .caption = caption};
    };
    named(agiru::platform::ObjectOptionsObjectType::Report, "Report", "Report");
    named(agiru::platform::ObjectOptionsObjectType::XMLport, "XMLport", "XMLport");
    named(agiru::platform::ObjectOptionsObjectType::Page, "Page", "\"Page\"");
    return values;
  }();
};

namespace agiru::platform {

/// \brief Source-backed table 2000000196; declaration changes do not migrate populated schemas.
class ObjectOptions_Table : public Table<ObjectOptions_Table> {
public:
  /// \brief The table number.
  static constexpr TableId kId{2000000196};
  /// \brief The AL name.
  static constexpr std::string_view kName{"Object Options"};
  /// \brief Original extension availability; not a Microsoft service dependency.
  static constexpr std::string_view kScope{"Cloud"};

  /// \brief The record variable's state; first, so the runtime reaches it at offset 0.
  detail::StateHandle State_Block;

  /// \brief A parameter name is `Text[50]`.
  static constexpr std::size_t kNameLength = 50;
  /// \brief A company name is `Text[30]`.
  static constexpr std::size_t kCompanyLength = 30;
  /// \brief A user name is `Code[50]`.
  static constexpr std::size_t kUserLength = 50;

  /// \brief What the saved setting is called.
  Text<kNameLength> ParameterName;
  /// \brief The company it belongs to.
  Text<kCompanyLength> CompanyName;
  /// \brief The source's Report, XMLport or Page ordinal.
  Option<ObjectOptionsObjectType> ObjectType;
  /// \brief The object's number.
  ::agiru::Integer ObjectID{};
  /// \brief The user who saved it, empty when it is shared.
  Code<kUserLength> UserName;
  /// \brief Whether every user sees it.
  Boolean PublicVisible{};
  /// \brief The saved request page, as the platform writes it.
  Blob OptionData;
  /// \brief Whether the saved setting is temporary; distinct from record storage mode.
  Boolean Temporary{};
  /// \brief Who made it.
  Code<kUserLength> CreatedBy;

  /// \brief AL `Object Options.SystemId`.
  Guid SystemId;
  /// \brief AL `Object Options.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `Object Options.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `Object Options.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `Object Options.SystemModifiedBy`.
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

  /// \brief The field numbers.
  struct Field_No : SystemFieldNumbers {
    static constexpr ::agiru::FieldNo ParameterName{1};
    static constexpr ::agiru::FieldNo CompanyName{4};
    static constexpr ::agiru::FieldNo ObjectType{3};
    static constexpr ::agiru::FieldNo ObjectID{2};
    static constexpr ::agiru::FieldNo UserName{5};
    static constexpr ::agiru::FieldNo PublicVisible{7};
    static constexpr ::agiru::FieldNo OptionData{6};
    static constexpr ::agiru::FieldNo Temporary{8}; ///< Original saved-setting Boolean field.
    static constexpr ::agiru::FieldNo CreatedBy{9};
  };

  /// \brief The primary key.
  static constexpr std::array<::agiru::FieldNo, 5> kKey1{{Field_No::ParameterName,
                                                          Field_No::ObjectID,
                                                          Field_No::ObjectType,
                                                          Field_No::UserName,
                                                          Field_No::CompanyName}};
};

/// \brief The name the BaseApp uses.
using ObjectOptions = ObjectOptions_Table;

/// \brief The field table.
inline constexpr auto kObjectOptionsFields =
    WithImplicitFields<ObjectOptions,
                       ::agiru::SystemFieldProfile::Runtime18,
                       ::agiru::TableType::Normal,
                       false>(std::array<FieldDef, 9>{{
        Declare<&ObjectOptions::ParameterName>(ObjectOptions::Field_No::ParameterName,
                                               "Parameter Name",
                                               "Parameter Name",
                                               offsetof(ObjectOptions, ParameterName)),
        Declare<&ObjectOptions::ObjectID>(ObjectOptions::Field_No::ObjectID,
                                          "Object ID",
                                          "Object ID",
                                          offsetof(ObjectOptions, ObjectID)),
        Declare<&ObjectOptions::ObjectType>(ObjectOptions::Field_No::ObjectType,
                                            "Object Type",
                                            "Object Type",
                                            offsetof(ObjectOptions, ObjectType)),
        Declare<&ObjectOptions::CompanyName>(ObjectOptions::Field_No::CompanyName,
                                             "Company Name",
                                             "Company Name",
                                             offsetof(ObjectOptions, CompanyName),
                                             {.relationTable = "Company",
                                              .relationField = "Name",
                                              .relation = "System.Environment.Company.Name"}),
        Declare<&ObjectOptions::UserName>(ObjectOptions::Field_No::UserName,
                                          "User Name",
                                          "User Name",
                                          offsetof(ObjectOptions, UserName)),
        Declare<&ObjectOptions::OptionData>(ObjectOptions::Field_No::OptionData,
                                            "Option Data",
                                            "Option Data",
                                            offsetof(ObjectOptions, OptionData),
                                            {.subtype = "UserDefined"}),
        Declare<&ObjectOptions::PublicVisible>(ObjectOptions::Field_No::PublicVisible,
                                               "Public Visible",
                                               "Public Visible",
                                               offsetof(ObjectOptions, PublicVisible)),
        Declare<&ObjectOptions::Temporary>(ObjectOptions::Field_No::Temporary,
                                           "Temporary",
                                           "Temporary",
                                           offsetof(ObjectOptions, Temporary)),
        Declare<&ObjectOptions::CreatedBy>(ObjectOptions::Field_No::CreatedBy,
                                           "Created By",
                                           "Created By",
                                           offsetof(ObjectOptions, CreatedBy)),
    }});

/// \brief The keys.
inline constexpr std::array<KeyDef, 1> kObjectOptionsKeys{{
    KeyDef{.name = "Key1", .fields = ObjectOptions::kKey1, .clustered = true},
}};

/// \brief The table.
inline constexpr TableDef kObjectOptionsTable{
    .id = ObjectOptions::kId,
    .name = ObjectOptions::kName,
    .caption = "Object Options",
    .fields = kObjectOptionsFields,
    .keys = kObjectOptionsKeys,
    .dataPerCompany = false,
    .replicateData = false,
};

static_assert(FieldsAreSorted(kObjectOptionsTable), "the field table is sorted by number");
static_assert(offsetof(ObjectOptions, State_Block) == 0, "the state is the first member");

}

/// \brief The traits the runtime reaches the table through.
template <> struct agiru::TableTraits<agiru::platform::ObjectOptions_Table> {
  /// \brief The table.
  static constexpr const TableDef &kTable = agiru::platform::kObjectOptionsTable;
};

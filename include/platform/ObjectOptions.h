#pragma once

#include "meta/EnumDef.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Table.h"
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
#include <string_view>

/// \file
/// \brief The platform table `Object Options` (2000000225): the saved settings of a report's or a
///        page's request page, which the BaseApp writes when a user saves a view and reads when
///        one is run again.

namespace agiru::platform {

/// \brief The `Object Type` an option set belongs to, as the platform declares it.
enum class ObjectOptionsObjectType : std::int32_t {
  Report = 0, ///< A report's request page.
  Page = 1,   ///< A page's.
};

}

/// \brief The `Object Type` members, as the platform names them.
template <> struct agiru::OptionTraits<agiru::platform::ObjectOptionsObjectType> {
  /// \brief The members.
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      {.ordinal = 0, .name = "Report", .caption = "Report"},
      {.ordinal = 1, .name = "Page", .caption = "Page"},
  }};
};

namespace agiru::platform {

/// \brief The platform table `Object Options` (2000000225).
///
/// \note THE PLATFORM DECLARES IT AND NO `.al` FILE DOES, so the shape is written here from what
///       the BaseApp reads of it -- `Parameter Name`, `Company Name`, `Object Type`, `Object ID`,
///       `User Name`, `Public Visible`, `Option Data`, `Created By` -- and the primary key is the
///       four the `Object Options` page keys on.
class ObjectOptions_Table : public Table<ObjectOptions_Table> {
public:
  /// \brief The table number.
  static constexpr TableId kId{2000000225};
  /// \brief The AL name.
  static constexpr std::string_view kName{"Object Options"};

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
  /// \brief Report or page.
  Option<ObjectOptionsObjectType> ObjectType;
  /// \brief The object's number.
  ::agiru::Integer ObjectID{};
  /// \brief The user who saved it, empty when it is shared.
  Code<kUserLength> UserName;
  /// \brief Whether every user sees it.
  Boolean PublicVisible{};
  /// \brief The saved request page, as the platform writes it.
  Blob OptionData;
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

  /// \brief The field numbers.
  struct Field_No {
    static constexpr ::agiru::FieldNo ParameterName{1};
    static constexpr ::agiru::FieldNo CompanyName{2};
    static constexpr ::agiru::FieldNo ObjectType{3};
    static constexpr ::agiru::FieldNo ObjectID{4};
    static constexpr ::agiru::FieldNo UserName{5};
    static constexpr ::agiru::FieldNo PublicVisible{6};
    static constexpr ::agiru::FieldNo OptionData{7};
    static constexpr ::agiru::FieldNo CreatedBy{8};
  };

  /// \brief The primary key.
  static constexpr std::array<::agiru::FieldNo, 5> kKey1{{Field_No::ParameterName,
                                                          Field_No::CompanyName,
                                                          Field_No::ObjectType,
                                                          Field_No::ObjectID,
                                                          Field_No::UserName}};
};

/// \brief The name the BaseApp uses.
using ObjectOptions = ObjectOptions_Table;

/// \brief The field table.
inline constexpr auto kObjectOptionsFields =
    WithSystemFields<ObjectOptions>(std::array<FieldDef, 8>{{
        Declare<&ObjectOptions::ParameterName>(ObjectOptions::Field_No::ParameterName,
                                               "Parameter Name",
                                               "Parameter Name",
                                               offsetof(ObjectOptions, ParameterName)),
        Declare<&ObjectOptions::CompanyName>(ObjectOptions::Field_No::CompanyName,
                                             "Company Name",
                                             "Company Name",
                                             offsetof(ObjectOptions, CompanyName)),
        Declare<&ObjectOptions::ObjectType>(ObjectOptions::Field_No::ObjectType,
                                            "Object Type",
                                            "Object Type",
                                            offsetof(ObjectOptions, ObjectType)),
        Declare<&ObjectOptions::ObjectID>(ObjectOptions::Field_No::ObjectID,
                                          "Object ID",
                                          "Object ID",
                                          offsetof(ObjectOptions, ObjectID)),
        Declare<&ObjectOptions::UserName>(ObjectOptions::Field_No::UserName,
                                          "User Name",
                                          "User Name",
                                          offsetof(ObjectOptions, UserName)),
        Declare<&ObjectOptions::PublicVisible>(ObjectOptions::Field_No::PublicVisible,
                                               "Public Visible",
                                               "Public Visible",
                                               offsetof(ObjectOptions, PublicVisible)),
        Declare<&ObjectOptions::OptionData>(ObjectOptions::Field_No::OptionData,
                                            "Option Data",
                                            "Option Data",
                                            offsetof(ObjectOptions, OptionData)),
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
};

static_assert(FieldsAreSorted(kObjectOptionsTable), "the field table is sorted by number");
static_assert(offsetof(ObjectOptions, State_Block) == 0, "the state is the first member");

}

/// \brief The traits the runtime reaches the table through.
template <> struct agiru::TableTraits<agiru::platform::ObjectOptions_Table> {
  /// \brief The table.
  static constexpr const TableDef &kTable = agiru::platform::kObjectOptionsTable;
};

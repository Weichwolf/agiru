#pragma once

#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Option.h"
#include "type/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

/// \file
/// \brief The platform table `Feature Key` -- the feature switches an installation carries.

namespace agiru::platform {

/// \brief The `Enabled` option of `Feature Key`: `None` or `All Users`.
enum class FeatureKeyEnabled : std::int32_t {
  None = 0,
  AllUsers = 1,
};

}

template <> struct agiru::OptionTraits<agiru::platform::FeatureKeyEnabled> {
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      {.ordinal = 0, .name = "None", .caption = "None"},
      {.ordinal = 1, .name = "All Users", .caption = "All Users"},
  }};
};

namespace agiru::platform {

/// \brief AL table `Feature Key` (2000000211), a platform table with no `.al` source.
///
/// \note THE FIELD IMAGE IS `Feature Key Buffer` (table 2609) WORD FOR WORD, fields 1 to 8: the
///       System Application's buffer copies the platform table one to one, which is what makes
///       the image a reading and not a guess (openerp WI-1391, board:0636).
///
/// \note EMPTY, IT IS BEHAVIOUR-NEUTRAL. `Feature Management Impl.` reads a missing key as
///       `Enabled::None`, and `IsEnabled` answers false -- which is what a refused `Get` also
///       answered, except that the refusal stopped the test. Which features are on is DATA:
///       BC's `CreateDemonstrationData.EnableNewFeatures` writes the rows when it builds CRONUS,
///       and here they belong to the seed. A feature name in the runtime would be an AL object
///       name in generic code.
class FeatureKey_Table : public Table<FeatureKey_Table> {
public:
  /// \brief The table number.
  static constexpr TableId kId{2000000211};
  /// \brief The AL name.
  static constexpr std::string_view kName{"Feature Key"};
  /// \brief The record variable's state; first, so the runtime reaches it at offset 0.
  detail::StateHandle State_Block;
  /// \brief `ID` is `Text[50]`.
  static constexpr std::size_t kIdLength = 50;
  /// \brief The three descriptive texts are `Text[2048]`.
  static constexpr std::size_t kTextLength = 2048;
  /// \brief The feature's id, the primary key.
  Text<kIdLength> ID;
  /// \brief Whether the feature is on, and for whom.
  Option<FeatureKeyEnabled> Enabled;
  /// \brief What the feature does.
  Text<kTextLength> Description;
  /// \brief Where to read more.
  Text<kTextLength> LearnMoreLink;
  /// \brief When the feature becomes mandatory.
  Text<kTextLength> MandatoryBy;
  /// \brief Whether a user may try it.
  Boolean CanTry{};
  /// \brief Whether it cannot be switched off again.
  Boolean IsOneWay{};
  /// \brief Whether enabling it updates data.
  Boolean DataUpdateRequired{};

  /// \brief AL `FeatureKey.SystemId`.
  Guid SystemId;
  /// \brief AL `FeatureKey.SystemCreatedAt`.
  DateTime SystemCreatedAt;
  /// \brief AL `FeatureKey.SystemCreatedBy`.
  Guid SystemCreatedBy;
  /// \brief AL `FeatureKey.SystemModifiedAt`.
  DateTime SystemModifiedAt;
  /// \brief AL `FeatureKey.SystemModifiedBy`.
  Guid SystemModifiedBy;

  /// \brief The field numbers.
  struct Field_No {
    static constexpr ::agiru::FieldNo ID{1};
    static constexpr ::agiru::FieldNo Enabled{2};
    static constexpr ::agiru::FieldNo Description{3};
    static constexpr ::agiru::FieldNo LearnMoreLink{4};
    static constexpr ::agiru::FieldNo MandatoryBy{5};
    static constexpr ::agiru::FieldNo CanTry{6};
    static constexpr ::agiru::FieldNo IsOneWay{7};
    static constexpr ::agiru::FieldNo DataUpdateRequired{8};
  };

  /// \brief The primary key.
  static constexpr std::array<::agiru::FieldNo, 1> kKey1{{Field_No::ID}};
};

/// \brief The name the BaseApp uses.
using FeatureKey = FeatureKey_Table;

/// \brief The field table.
inline constexpr std::array<FieldDef, 8> kFeatureKeyFields{{
    Declare<&FeatureKey::ID>(FeatureKey::Field_No::ID, "ID", "ID", offsetof(FeatureKey, ID)),
    Declare<&FeatureKey::Enabled>(
        FeatureKey::Field_No::Enabled, "Enabled", "Enabled", offsetof(FeatureKey, Enabled)),
    Declare<&FeatureKey::Description>(FeatureKey::Field_No::Description,
                                      "Description",
                                      "Description",
                                      offsetof(FeatureKey, Description)),
    Declare<&FeatureKey::LearnMoreLink>(FeatureKey::Field_No::LearnMoreLink,
                                        "Learn More Link",
                                        "Learn more",
                                        offsetof(FeatureKey, LearnMoreLink)),
    Declare<&FeatureKey::MandatoryBy>(FeatureKey::Field_No::MandatoryBy,
                                      "Mandatory By",
                                      "Approximate mandatory date",
                                      offsetof(FeatureKey, MandatoryBy)),
    Declare<&FeatureKey::CanTry>(
        FeatureKey::Field_No::CanTry, "Can Try", "Get started", offsetof(FeatureKey, CanTry)),
    Declare<&FeatureKey::IsOneWay>(
        FeatureKey::Field_No::IsOneWay, "Is One Way", "Is One Way", offsetof(FeatureKey, IsOneWay)),
    Declare<&FeatureKey::DataUpdateRequired>(FeatureKey::Field_No::DataUpdateRequired,
                                             "Data Update Required",
                                             "Data Update Required",
                                             offsetof(FeatureKey, DataUpdateRequired)),
}};

/// \brief The keys.
inline constexpr std::array<KeyDef, 1> kFeatureKeyKeys{{
    KeyDef{.name = "Key1", .fields = FeatureKey::kKey1, .clustered = true},
}};

/// \brief The table definition.
inline constexpr TableDef kFeatureKeyTable{
    .id = FeatureKey::kId,
    .name = FeatureKey::kName,
    .caption = "Feature Key",
    .fields = kFeatureKeyFields,
    .keys = kFeatureKeyKeys,
    .dataPerCompany = false,
};

static_assert(FieldsAreSorted(kFeatureKeyTable), "the field table is sorted by number");
static_assert(offsetof(FeatureKey, State_Block) == 0, "the state is the first member");

}

template <> struct agiru::TableTraits<agiru::platform::FeatureKey_Table> {
  static constexpr const TableDef &kTable = agiru::platform::kFeatureKeyTable;
};

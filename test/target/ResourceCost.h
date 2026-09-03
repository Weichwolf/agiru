// Generated from Projects/Resources/Pricing/ResourceCost.Table.al. Do not edit.

#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Table.h"
#include "type/Code.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Guid.h"
#include "type/Option.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace agiru::app::tables {

enum class ResourceCostType : std::int32_t {
  Resource = 0,
  GroupResource = 1,
  All = 2,
};

enum class ResourceCostCostType : std::int32_t {
  Fixed = 0,
  PercentExtra = 1,
  LCYExtra = 2,
};

} // namespace agiru::app::tables

template <> struct agiru::OptionTraits<agiru::app::tables::ResourceCostType> {
  static constexpr std::array<EnumValueDef, 3> kValues{{
      EnumValueDef{.ordinal = 0, .name = "Resource", .caption = "Resource"},
      EnumValueDef{.ordinal = 1, .name = "Group(Resource)", .caption = "Group(Resource)"},
      EnumValueDef{.ordinal = 2, .name = "All", .caption = "All"},
  }};
};

template <> struct agiru::OptionTraits<agiru::app::tables::ResourceCostCostType> {
  static constexpr std::array<EnumValueDef, 3> kValues{{
      EnumValueDef{.ordinal = 0, .name = "Fixed", .caption = "Fixed"},
      EnumValueDef{.ordinal = 1, .name = "% Extra", .caption = "% Extra"},
      EnumValueDef{.ordinal = 2, .name = "LCY Extra", .caption = "LCY Extra"},
  }};
};

namespace agiru::app::tables {

class ResourceCost_Table;
using ResourceCost = ResourceCost_Table;

class ResourceCost_Table : public Table<ResourceCost_Table> {
public:
  static constexpr TableId kId{202};
  static constexpr std::string_view kName{"Resource Cost"};

  Option<ResourceCostType> Type{};
  ::agiru::Code<20> Code{};
  ::agiru::Code<10> WorkTypeCode{};
  Option<ResourceCostCostType> CostType{};
  Decimal DirectUnitCost{};
  Decimal UnitCost{};
  Guid SystemId{};
  DateTime SystemCreatedAt{};
  Guid SystemCreatedBy{};
  DateTime SystemModifiedAt{};
  Guid SystemModifiedBy{};

  struct Field_No : SystemFieldNumbers {
    static constexpr ::agiru::FieldNo Type{1};
    static constexpr ::agiru::FieldNo Code{2};
    static constexpr ::agiru::FieldNo WorkTypeCode{3};
    static constexpr ::agiru::FieldNo CostType{4};
    static constexpr ::agiru::FieldNo DirectUnitCost{5};
    static constexpr ::agiru::FieldNo UnitCost{6};
  };

  static constexpr std::array<::agiru::FieldNo, 3> kKey1{
      {Field_No::Type, Field_No::Code, Field_No::WorkTypeCode}};
  static constexpr std::array<::agiru::FieldNo, 3> kKey2{
      {Field_No::CostType, Field_No::Code, Field_No::WorkTypeCode}};

  static constexpr std::string_view Text000{"cannot be specified when %1 is %2"};

  void OnValidateCode();
  void OnValidateCostType();
};

inline constexpr auto kResourceCostFields = WithSystemFields<ResourceCost>(std::array<FieldDef, 6>{{
    Declare<&ResourceCost::Type>(
        ResourceCost::Field_No::Type, "Type", "Type", offsetof(ResourceCost, Type)),
    Declare<&ResourceCost::Code>(
        ResourceCost::Field_No::Code, "Code", "Code", offsetof(ResourceCost, Code)),
    Declare<&ResourceCost::WorkTypeCode>(ResourceCost::Field_No::WorkTypeCode,
                                         "Work Type Code",
                                         "Work Type Code",
                                         offsetof(ResourceCost, WorkTypeCode)),
    Declare<&ResourceCost::CostType>(ResourceCost::Field_No::CostType,
                                     "Cost Type",
                                     "Cost Type",
                                     offsetof(ResourceCost, CostType)),
    Declare<&ResourceCost::DirectUnitCost>(ResourceCost::Field_No::DirectUnitCost,
                                           "Direct Unit Cost",
                                           "Direct Unit Cost",
                                           offsetof(ResourceCost, DirectUnitCost)),
    Declare<&ResourceCost::UnitCost>(ResourceCost::Field_No::UnitCost,
                                     "Unit Cost",
                                     "Unit Cost",
                                     offsetof(ResourceCost, UnitCost)),
}});

inline constexpr std::array<KeyDef, 2> kResourceCostKeys{{
    KeyDef{.name = "Key1", .fields = ResourceCost::kKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = ResourceCost::kKey2, .clustered = false},
}};

inline constexpr TableDef kResourceCostTable{
    .id = ResourceCost::kId,
    .name = ResourceCost::kName,
    .caption = ResourceCost::kName,
    .fields = kResourceCostFields,
    .keys = kResourceCostKeys,
};

static_assert(FieldsAreSorted(kResourceCostTable),
              "the field table is emitted sorted by field number, which is what lets Field() "
              "binary-search it");
static_assert(std::is_standard_layout_v<ResourceCost>,
              "offsetof over the field table requires standard layout. The base carries NO data, "
              "which is what keeps it so");
static_assert(kResourceCostFields.size() == 6 + kSystemFieldCount,
              "table 202 declares 6 fields, and the platform adds its own");

} // namespace agiru::app::tables

template <> struct agiru::TableTraits<agiru::app::tables::ResourceCost> {
  static constexpr const TableDef &kTable = agiru::app::tables::kResourceCostTable;
  static constexpr std::array<agiru::OnValidateOf<agiru::app::tables::ResourceCost>, 2> kOnValidate{
      {
          {agiru::app::tables::ResourceCost::Field_No::Code,
           [](agiru::app::tables::ResourceCost &record) { record.OnValidateCode(); }},
          {agiru::app::tables::ResourceCost::Field_No::CostType,
           [](agiru::app::tables::ResourceCost &record) { record.OnValidateCostType(); }},
      }};
};

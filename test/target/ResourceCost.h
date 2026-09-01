// Generated from Projects/Resources/Pricing/ResourceCost.Table.al. Do not edit.
// Written by hand as the specification the generator must reproduce (board:0012).

#pragma once

#include "agiru/Decimal.h"
#include "agiru/Declare.h"
#include "agiru/Ids.h"
#include "agiru/Option.h"
#include "agiru/Table.h"
#include "agiru/TableDef.h"
#include "agiru/Text.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace agiru::app {

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

} // namespace agiru::app

template <> struct agiru::OptionTraits<agiru::app::ResourceCostType> {
  static constexpr std::array<std::string_view, 3> kMembers{"Resource", "Group(Resource)", "All"};
  static constexpr std::array<std::string_view, 3> kCaptions{"Resource", "Group(Resource)", "All"};
};

template <> struct agiru::OptionTraits<agiru::app::ResourceCostCostType> {
  static constexpr std::array<std::string_view, 3> kMembers{"Fixed", "% Extra", "LCY Extra"};
  static constexpr std::array<std::string_view, 3> kCaptions{"Fixed", "% Extra", "LCY Extra"};
};

namespace agiru::app {

class ResourceCost : public Table<ResourceCost> {
public:
  static constexpr TableId kId{202};
  static constexpr std::string_view kName{"Resource Cost"};

  Option<ResourceCostType> Type;
  ::agiru::Code<20> Code;
  ::agiru::Code<10> WorkTypeCode;
  Option<ResourceCostCostType> CostType;
  Decimal DirectUnitCost;
  Decimal UnitCost;

  struct FieldNumber {
    static constexpr FieldNo Type{1};
    static constexpr FieldNo Code{2};
    static constexpr FieldNo WorkTypeCode{3};
    static constexpr FieldNo CostType{4};
    static constexpr FieldNo DirectUnitCost{5};
    static constexpr FieldNo UnitCost{6};
  };

  static constexpr std::array kKey1{
      FieldNumber::Type, FieldNumber::Code, FieldNumber::WorkTypeCode};
  static constexpr std::array kKey2{
      FieldNumber::CostType, FieldNumber::Code, FieldNumber::WorkTypeCode};

  static constexpr std::string_view Text000{"cannot be specified when %1 is %2"};

  void OnValidateCode();

  void OnValidateCostType();
};

inline constexpr std::array kResourceCostFields{
    Declare<&ResourceCost::Type>(
        ResourceCost::FieldNumber::Type, "Type", "Type", offsetof(ResourceCost, Type)),
    Declare<&ResourceCost::Code>(
        ResourceCost::FieldNumber::Code, "Code", "Code", offsetof(ResourceCost, Code)),
    Declare<&ResourceCost::WorkTypeCode>(ResourceCost::FieldNumber::WorkTypeCode,
                                         "Work Type Code",
                                         "Work Type Code",
                                         offsetof(ResourceCost, WorkTypeCode)),
    Declare<&ResourceCost::CostType>(ResourceCost::FieldNumber::CostType,
                                     "Cost Type",
                                     "Cost Type",
                                     offsetof(ResourceCost, CostType)),
    Declare<&ResourceCost::DirectUnitCost>(ResourceCost::FieldNumber::DirectUnitCost,
                                           "Direct Unit Cost",
                                           "Direct Unit Cost",
                                           offsetof(ResourceCost, DirectUnitCost)),
    Declare<&ResourceCost::UnitCost>(ResourceCost::FieldNumber::UnitCost,
                                     "Unit Cost",
                                     "Unit Cost",
                                     offsetof(ResourceCost, UnitCost)),
};

inline constexpr std::array kResourceCostKeys{
    KeyDef{.name = "Key1", .fields = ResourceCost::kKey1, .clustered = true},
    KeyDef{.name = "Key2", .fields = ResourceCost::kKey2, .clustered = false},
};

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
static_assert(kResourceCostFields.size() == 6, "table 202 declares six fields");

} // namespace agiru::app

template <> struct agiru::TableTraits<agiru::app::ResourceCost> {
  static constexpr const TableDef &kTable = agiru::app::kResourceCostTable;
};

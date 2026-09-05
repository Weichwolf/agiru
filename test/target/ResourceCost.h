// Generated from Projects/Resources/Pricing/ResourceCost.Table.al. Do not edit.

#pragma once

#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/RecordState.h"
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

  detail::StateHandle State_Block;

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

extern const TableDef kResourceCostTable;

} // namespace agiru::app::tables

template <> struct agiru::TableTraits<agiru::app::tables::ResourceCost> {
  static constexpr const TableDef &kTable = agiru::app::tables::kResourceCostTable;
  static constexpr std::array<agiru::OnValidateOf<agiru::app::tables::ResourceCost>, 2> kOnValidate{
      {
          {.field = agiru::app::tables::ResourceCost::Field_No::Code,
           .run = [](agiru::app::tables::ResourceCost &record) { record.OnValidateCode(); }},
          {.field = agiru::app::tables::ResourceCost::Field_No::CostType,
           .run = [](agiru::app::tables::ResourceCost &record) { record.OnValidateCostType(); }},
      }};
};

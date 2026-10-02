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

#include "options/Types.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace agiru::Projects::Resources::Pricing {

class ResourceCost_Table;

class ResourceCost_Table : public Table<ResourceCost_Table> {
public:
  using Table<ResourceCost_Table>::operator=;

  static constexpr TableId kId{202};
  static constexpr std::string_view kName{"Resource Cost"};

  detail::StateHandle State_Block;

  Option<::agiru::options::OptionResourceGroupResourceAll> Type{};
  ::agiru::Code<20> Code{};
  ::agiru::Code<10> WorkTypeCode{};
  Option<::agiru::options::OptionFixedPercentExtraLCYExtra> CostType{};
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

} // namespace agiru::Projects::Resources::Pricing

template <> struct agiru::TableTraits<agiru::Projects::Resources::Pricing::ResourceCost_Table> {
  static constexpr const TableDef &kTable = agiru::Projects::Resources::Pricing::kResourceCostTable;
  static constexpr std::
      array<agiru::OnValidateOf<agiru::Projects::Resources::Pricing::ResourceCost_Table>, 2>
          kOnValidate{{
              {.field = agiru::Projects::Resources::Pricing::ResourceCost_Table::Field_No::Code,
               .run =
                   [](agiru::Projects::Resources::Pricing::ResourceCost_Table &record) {
                     record.OnValidateCode();
                   }},
              {.field = agiru::Projects::Resources::Pricing::ResourceCost_Table::Field_No::CostType,
               .run =
                   [](agiru::Projects::Resources::Pricing::ResourceCost_Table &record) {
                     record.OnValidateCostType();
                   }},
          }};
};

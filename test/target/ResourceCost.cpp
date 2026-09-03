// Generated from Projects/Resources/Pricing/ResourceCost.Table.al. Do not edit.

#include "ResourceCost.h"

#include "agiru.h"

namespace agiru::app::tables {

void ResourceCost::OnValidateCode() {
  if (Code != "" && Type == ResourceCostType::All) {
    FieldError(Code, StrSubstNo(Text000, FieldCaption(Type), Format(Type)));
  }
}

void ResourceCost::OnValidateCostType() {
  if (WorkTypeCode == "") { TestField(CostType, ResourceCostCostType::Fixed); }
}

namespace {
const RegisterTable<ResourceCost> kInCatalogue;
} // namespace

} // namespace agiru::app::tables

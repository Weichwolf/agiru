// Generated from Projects/Resources/Pricing/ResourceCost.Table.al. Do not edit.

#include "ResourceCost.h"

#include "agiru.h"

namespace agiru::app {

void ResourceCost::OnValidateCode() {
  if (Code != "" && Type == ResourceCostType::All) {
    FieldError(Code, StrSubstNo(Text000, FieldCaption(Type), Format(Type)));
  }
}

void ResourceCost::OnValidateCostType() {
  if (WorkTypeCode == "") { TestField(CostType, ResourceCostCostType::Fixed); }
}

} // namespace agiru::app

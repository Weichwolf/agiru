// THE TARGET IMAGE, definition half. Written BY HAND as the C++ the generator must emit from
// Projects/Resources/Pricing/ResourceCost.Table.al. See ResourceCost.h for what this file is.
//
// Only what AL puts in a `trigger` or a `procedure` is here. Compare each body with the AL above
// it: there is no connection, no table, no `this`, no row and no column, because a `.al` file
// contains none of those.

#include "ResourceCost.h"

#include "agiru/Record.h"

namespace agiru::app {

/// AL:
///     trigger OnValidate()
///     begin
///         if (Code <> '') and (Type = Type::All) then
///             FieldError(Code, StrSubstNo(Text000, FieldCaption(Type), Format(Type)));
///     end;
void ResourceCost::OnValidateCode() {
  if (Code != "" && Type == ResourceCostType::All) {
    FieldError(Code, StrSubstNo(Text000, FieldCaption(Type), Format(Type)));
  }
}

/// AL:
///     trigger OnValidate()
///     begin
///         if "Work Type Code" = '' then
///             TestField("Cost Type", "Cost Type"::Fixed);
///     end;
void ResourceCost::OnValidateCostType() {
  if (WorkTypeCode == "") { TestField(CostType, ResourceCostCostType::Fixed); }
}

} // namespace agiru::app

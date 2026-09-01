// THE TARGET IMAGE, implementation half. Written BY HAND as the C++ the generator must emit for the
// two OnValidate triggers of table 202 "Resource Cost". See ResourceCost.h for what this file is.

#include "ResourceCost.h"

#include "agiru/Option.h"
#include "agiru/Record.h"

namespace agiru::app {

/// AL:
///     trigger OnValidate()
///     begin
///         if (Code <> '') and (Type = Type::All) then
///             FieldError(Code, StrSubstNo(Text000, FieldCaption(Type), Format(Type)));
///     end;
///
/// `Format(Type)` renders the option's CAPTION. Table 202 declares the same text for members and
/// captions, so this file cannot tell them apart -- a table where they differ will (board:0007).
void ResourceCost::OnValidateCode() {
  if (!Code.IsEmpty() && Type == ::agiru::Option<ResourceCostType>{ResourceCostType::All}) {
    ::agiru::FieldError(
        this,
        kResourceCostTable,
        FieldNumber::Code,
        ::agiru::StrSubstNo(kText000,
                            ::agiru::FieldCaption(kResourceCostTable, FieldNumber::Type),
                            Type.Caption()));
  }
}

/// AL:
///     trigger OnValidate()
///     begin
///         if "Work Type Code" = '' then
///             TestField("Cost Type", "Cost Type"::Fixed);
///     end;
void ResourceCost::OnValidateCostType() {
  if (WorkTypeCode.IsEmpty()) {
    ::agiru::TestField(this,
                       kResourceCostTable,
                       FieldNumber::CostType,
                       ::agiru::Option<ResourceCostCostType>{ResourceCostCostType::Fixed});
  }
}

} // namespace agiru::app

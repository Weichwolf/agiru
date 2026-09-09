// Generated from Projects/Resources/Pricing/ResourceCost.Table.al. Do not edit.

#include "ResourceCost.h"

#include "Builtins.h"
#include "meta/Ids.h"
#include "runtime/Catalogue.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/Table.h"
#include "type/Code.h"

#include "BuiltinsWritten.h"

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
namespace ResourceCost_unit {
const RegisterTable<ResourceCost> kInCatalogue;
} // namespace ResourceCost_unit
} // namespace

} // namespace agiru::app::tables

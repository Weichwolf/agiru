// Generated from Projects/Resources/Pricing/ResourceCost.Table.al. Do not edit.

#include "ResourceCost.h"

#include "Builtins.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/Table.h"
#include "runtime/test/TestField.h"
#include "type/Option.h"

#include "BuiltinsWritten.h"
#include "options/Types.h"

namespace agiru::Projects::Resources::Pricing {

void ResourceCost_Table::OnValidateCode() {
  if (Code != "" && Type == ::agiru::Option<::agiru::options::OptionResourceGroupResourceAll>{
                                ::agiru::options::OptionResourceGroupResourceAll::All}) {
    FieldError(Code, StrSubstNo(Text000, FieldCaption(Type), Format(Type)));
  }
}

void ResourceCost_Table::OnValidateCostType() {
  if (WorkTypeCode == "") {
    TestField(CostType,
              ::agiru::Option<::agiru::options::OptionFixedPercentExtraLCYExtra>{
                  ::agiru::options::OptionFixedPercentExtraLCYExtra::Fixed});
  }
}

} // namespace agiru::Projects::Resources::Pricing

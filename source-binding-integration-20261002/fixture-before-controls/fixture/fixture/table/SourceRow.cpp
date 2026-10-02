// Generated from SourceRow.Table.al. Do not edit.

#include "SourceRow.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/Decimal.h"
#include "type/Option.h"
#include "type/Text.h"


#include "options/Types.h"

namespace agiru::Fixture {

void SourceRow_Table::Change_Proc(::agiru::Text<0> &Value, ::agiru::Text<20> Copy) {
  Value = Copy;
  Copy = "local copy";
}

void SourceRow_Table::ChangeOption(Option<::agiru::options::OptionBlankCD> &Value) {
  Value = ::agiru::Option<::agiru::options::OptionBlankCD>{::agiru::options::OptionBlankCD::D};
}

::agiru::Boolean SourceRow_Table::TryChange(Decimal &Value) {
  Value += 1;
  return true;
}

void SourceRow_Table::AddedChange(::agiru::Text<0> &Value) {
  Value = "extension";
}

} // namespace agiru::Fixture

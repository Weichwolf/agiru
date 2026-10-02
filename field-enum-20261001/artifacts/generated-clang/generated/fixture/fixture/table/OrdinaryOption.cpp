// Generated from Option.Table.al. Do not edit.

#include "OrdinaryOption.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/Table.h"
#include "type/Integer.h"
#include "type/Option.h"


#include "options/Types.h"
#include "fixture/table/OrdinaryOption.h"

namespace agiru::Fixture {

::agiru::Integer OrdinaryOption_Table::Exercise() {
  [[maybe_unused]] ::agiru::Integer Ordinal{};
  ::agiru::Fixture::OrdinaryOption_Table &XRec = this->StoredImage();


  State_2 = ::agiru::Option<::agiru::options::OptionNoneReady>{::agiru::options::OptionNoneReady::Ready};
  if (State_2 != ::agiru::Option<::agiru::options::OptionNoneReady>{::agiru::options::OptionNoneReady::Ready}) {
    ::agiru::RaiseOrCollect("Current option");
  }
  Ordinal = (*this).State_2;
  if (Ordinal != 1) {
    ::agiru::RaiseOrCollect("Ready ordinal");
  }
  if (XRec.State_2 != ::agiru::Option<::agiru::options::OptionNoneReady>{::agiru::options::OptionNoneReady::None}) {
    ::agiru::RaiseOrCollect("Previous option");
  }
  if (Format(State_2) != "Ready") {
    ::agiru::RaiseOrCollect("Ready format");
  }
  return 4;
}

} // namespace agiru::Fixture

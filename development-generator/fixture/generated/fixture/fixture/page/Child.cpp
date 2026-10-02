// Generated from Child.Page.al. Do not edit.

#include "Child.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "type/Integer.h"



namespace agiru::Fixture {

void Child_Page::Touch() {
  Calls += 1;
}

::agiru::Integer Child_Page::GetCalls() {
  return Calls;
}

void Child_Page::ClearAll() {
  Calls = decltype(Calls){};
}

} // namespace agiru::Fixture

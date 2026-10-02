// Generated from Host.Page.al. Do not edit.

#include "CollisionHost.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "type/Integer.h"



namespace agiru::Fixture {

::agiru::Integer CollisionHost_Page::DispatchWork() {
  return 7;
}

::agiru::Integer CollisionHost_Page::DispatchWork_2() {
  return 11;
}

::agiru::Integer CollisionHost_Page::Invoke() {
  (*this).DispatchWork_3.Page().Touch();
  (*this).DispatchWork_4.Page().Touch();
  return DispatchWork() + DispatchWork_2();
}

void CollisionHost_Page::ClearAll() {
}

} // namespace agiru::Fixture

#include "platform/AllProfile.h"
#include "runtime/Table.h"

#include "Check.h"

int main() {
  return gate::Run("UnindexedProfileSort", [] {
    agiru::Temporary<agiru::platform::AllProfile> rows;
    rows.ProfileID = "A";
    rows.RoleCenterID = 20;
    rows.Insert();
    rows.ProfileID = "B";
    rows.RoleCenterID = 10;
    rows.Insert();
    rows.Reset();
    CHECK_TRUE("a sortable source field does not require a declared matching key",
               rows.SetCurrentKey(rows.RoleCenterID));
    CHECK_TRUE("the requested sort is retained despite the wrong Boolean result",
               rows.FindFirst() && rows.RoleCenterID == 10);
  });
}

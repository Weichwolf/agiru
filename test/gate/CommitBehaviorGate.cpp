#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/Scopes.h"
#include "runtime/Session.h"
#include "runtime/Transaction.h"
#include "type/CommitBehavior.h"

#include "Check.h"

namespace {

void NestedRestrictionsCannotBeWeakened(agiru::CommitBehavior outerBehavior,
                                        agiru::CommitBehavior innerBehavior) {
  const agiru::CommitScope outer(outerBehavior);
  {
    const agiru::CommitScope inner(innerBehavior);
    CHECK_TRUE("Error dominates either nesting order",
               agiru::CommitScope::Standing() == agiru::CommitBehavior::Error);
    bool refused = false;
    try {
      agiru::Commit();
    } catch (const agiru::Error &) { refused = true; }
    CHECK_TRUE("an explicit nested Commit is refused", refused);
  }
  CHECK_TRUE("normal return restores the caller restriction",
             agiru::CommitScope::Standing() == outerBehavior);
}

void UnwindingRestoresTheCallerRestriction() {
  const agiru::CommitScope outer(agiru::CommitBehavior::Ignore);
  try {
    const agiru::CommitScope inner(agiru::CommitBehavior::Error);
    agiru::Commit();
  } catch (const agiru::Error &) {
    CHECK_TRUE("an AL error restores the caller restriction",
               agiru::CommitScope::Standing() == agiru::CommitBehavior::Ignore);
  }
  agiru::Commit();
  CHECK_TRUE("the restored Ignore permits the caller to continue",
             agiru::CommitScope::Standing() == agiru::CommitBehavior::Ignore);
}

void IgnoreRemainsActiveThroughNestedIgnore() {
  const agiru::CommitScope outer(agiru::CommitBehavior::Ignore);
  const agiru::CommitScope inner(agiru::CommitBehavior::Ignore);
  agiru::Commit();
  CHECK_TRUE("two Ignore scopes still ignore explicit Commit",
             agiru::CommitScope::Standing() == agiru::CommitBehavior::Ignore);
}

}

int main() {
  return gate::Run("CommitBehavior", [] {
    const agiru::Session session(AGIRU_TEST_DSN);
    CHECK_TRUE("a new call has no commit restriction", !agiru::CommitScope::Standing());
    NestedRestrictionsCannotBeWeakened(agiru::CommitBehavior::Error, agiru::CommitBehavior::Ignore);
    NestedRestrictionsCannotBeWeakened(agiru::CommitBehavior::Ignore, agiru::CommitBehavior::Error);
    UnwindingRestoresTheCallerRestriction();
    IgnoreRemainsActiveThroughNestedIgnore();
    CHECK_TRUE("all method scopes restore unrestricted behavior", !agiru::CommitScope::Standing());
  });
}

#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/Session.h"
#include "runtime/Transaction.h"

#include "Check.h"
#include "FailingAllocation.h"

#include <new>
#include <string>

using gate::failAllocation;

namespace {
void FailedErrorRecordingStillDiscardsWrites() {
  const agiru::Session session(AGIRU_TEST_DSN);
  const agiru::Connection &connection = agiru::Session::Current().Database();
  connection.Run("CREATE TEMP TABLE scope_discard_gate (value integer)");
  const std::string message(256, 'x');
  bool refused = false;
  {
    agiru::detail::Scope scope;
    connection.Run("INSERT INTO scope_discard_gate VALUES (1)");
    failAllocation = true;
    try {
      scope.Discard(message);
    } catch (const std::bad_alloc &) { refused = true; }
  }
  CHECK_TRUE("recording the error failed", refused);
  const agiru::Result result = connection.Execute("SELECT count(*) FROM scope_discard_gate");
  CHECK_TRUE("a failed error allocation still rolls back the method write",
             result.Value(0, 0) == "0");

  const agiru::Error coded(message, "Fixture");
  refused = false;
  {
    agiru::detail::Scope scope;
    connection.Run("INSERT INTO scope_discard_gate VALUES (2)");
    failAllocation = true;
    try {
      scope.Discard(coded);
    } catch (const std::bad_alloc &) { refused = true; }
  }
  CHECK_TRUE("recording a coded error failed", refused);
  const agiru::Result afterCoded = connection.Execute("SELECT count(*) FROM scope_discard_gate");
  CHECK_TRUE("a failed coded-error allocation still rolls back the method write",
             afterCoded.Value(0, 0) == "0");
}
}

int main() {
  return gate::Run("ScopeDiscard", [] { FailedErrorRecordingStillDiscardsWrites(); });
}

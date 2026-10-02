#include "runtime/Database.h"
#include "runtime/Session.h"
#include "runtime/Transaction.h"

#include "Check.h"

#include <string>

#include <unistd.h>

namespace {

std::size_t Rows(const agiru::Connection &connection, const std::string &table) {
  const agiru::Result result = connection.Execute("SELECT count(*) FROM " + table);
  return static_cast<std::size_t>(std::stoul(std::string(*result.Value(0, 0))));
}

void ACommitIsVisibleOutsideTheWritingConnection() {
  agiru::Connection observer(AGIRU_TEST_DSN);
  const std::string table = "agiru_commit_gate_" + std::to_string(getpid());
  observer.Run("CREATE TABLE " + table + " (value integer PRIMARY KEY)");
  {
    agiru::Connection writer(AGIRU_TEST_DSN);
    agiru::Boundaries boundaries;
    const std::size_t depth = boundaries.Open(writer);
    writer.Run("INSERT INTO " + table + " VALUES (1)");
    CHECK_TRUE("an uncommitted row is invisible to another connection", Rows(observer, table) == 0);
    boundaries.Commit(writer);
    CHECK_TRUE("a production Commit is visible to another connection", Rows(observer, table) == 1);
    writer.Run("INSERT INTO " + table + " VALUES (2)");
    boundaries.Rollback(writer, depth);
    CHECK_TRUE("a later rollback does not undo the committed row", Rows(observer, table) == 1);
  }
  CHECK_TRUE("the committed row survives closing the writer", Rows(observer, table) == 1);
  observer.Run("DROP TABLE " + table);
}

void AnAbortedTransactionCannotReportACommit() {
  agiru::Connection writer(AGIRU_TEST_DSN);
  writer.Run("CREATE TEMP TABLE aborted_commit_gate (value integer PRIMARY KEY)");
  writer.Run("BEGIN");
  writer.Run("INSERT INTO aborted_commit_gate VALUES (1)");
  bool statementFailed = false;
  try {
    writer.Run("INSERT INTO aborted_commit_gate VALUES (1)");
  } catch (const agiru::DatabaseError &) { statementFailed = true; }
  CHECK_TRUE("the control statement aborted the transaction", statementFailed);
  agiru::Boundaries boundaries;
  bool commitRefused = false;
  try {
    boundaries.Commit(writer);
  } catch (const agiru::Error &) { commitRefused = true; }
  CHECK_TRUE("Commit cannot report success for an aborted transaction", commitRefused);
  writer.Run("ROLLBACK");
  CHECK_TRUE("the failed transaction persisted no writes",
             Rows(writer, "aborted_commit_gate") == 0);
}

void ADeferredCommitFailureLeavesNoStaleSavepoints() {
  agiru::Connection writer(AGIRU_TEST_DSN);
  writer.Run("CREATE TEMP TABLE deferred_commit_gate "
             "(value integer UNIQUE DEFERRABLE INITIALLY DEFERRED)");
  agiru::Boundaries boundaries;
  const std::size_t depth = boundaries.Open(writer);
  writer.Run("INSERT INTO deferred_commit_gate VALUES (1), (1)");
  bool refused = false;
  try {
    boundaries.Commit(writer);
  } catch (const agiru::DatabaseError &) { refused = true; }
  CHECK_TRUE("the deferred constraint is checked at Commit", refused);
  CHECK_TRUE("the server ended the failed transaction", !writer.InTransaction());
  CHECK_TRUE("failed Commit invalidates savepoints the server discarded", boundaries.Depth() == 0);
  std::string cleanupError;
  try {
    boundaries.Rollback(writer, depth);
  } catch (const agiru::DatabaseError &error) { cleanupError = error.what(); }
  CHECK_SILENT("closing the old scope does not address a discarded savepoint", cleanupError);
  CHECK_TRUE("the deferred failure persisted no writes", Rows(writer, "deferred_commit_gate") == 0);
}

void ADeferredFailureUnwindsNestedScopesAndAllowsANewTransaction() {
  const agiru::Session session(AGIRU_TEST_DSN);
  const agiru::Connection &writer = agiru::Session::Current().Database();
  writer.Run("CREATE TEMP TABLE scope_commit_gate "
             "(value integer UNIQUE DEFERRABLE INITIALLY DEFERRED)");
  bool refused = false;
  try {
    const agiru::detail::Scope outer;
    const agiru::detail::Scope inner;
    writer.Run("INSERT INTO scope_commit_gate VALUES (1), (1)");
    agiru::Commit();
  } catch (const agiru::DatabaseError &) { refused = true; }
  CHECK_TRUE("a deferred Commit error survives both scope destructors", refused);
  CHECK_TRUE("all failed writes are gone after unwinding", Rows(writer, "scope_commit_gate") == 0);
  {
    agiru::detail::Scope recovered;
    writer.Run("INSERT INTO scope_commit_gate VALUES (2)");
    recovered.Keep();
  }
  agiru::Commit();
  CHECK_TRUE("the same session can commit a new transaction",
             Rows(writer, "scope_commit_gate") == 1);
}

}

int main() {
  return gate::Run("CommitDurability", [] {
    ACommitIsVisibleOutsideTheWritingConnection();
    AnAbortedTransactionCannotReportACommit();
    ADeferredCommitFailureLeavesNoStaleSavepoints();
    ADeferredFailureUnwindsNestedScopesAndAllowsANewTransaction();
  });
}

#include "runtime/Database.h"
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

}

int main() {
  return gate::Run("CommitDurability", ACommitIsVisibleOutsideTheWritingConnection);
}

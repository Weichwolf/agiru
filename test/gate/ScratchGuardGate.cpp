#include "runtime/Database.h"
#include "runtime/test/RunnerDatabase.h"

#include "Check.h"

#include <array>
#include <barrier>
#include <future>
#include <optional>
#include <string>
#include <string_view>

#include <unistd.h>

namespace {

std::string Identity(const agiru::Connection &maintenance, const std::string &name) {
  const std::array<std::optional<std::string>, 1> bind{name};
  const agiru::Result result =
      maintenance.Execute("SELECT oid::text FROM pg_database WHERE datname = $1", bind);
  if (result.Rows() == 0) { return {}; }
  const auto value = result.Value(0, 0);
  if (!value) { throw agiru::DatabaseError("The fixture database identity is null"); }
  return std::string(*value);
}

bool Refused(const std::string &connection, agiru::DatabaseName database, bool fresh) {
  try {
    const agiru::RunnerDatabase runner(connection, database.value, fresh);
  } catch (const agiru::DatabaseError &) { return true; }
  return false;
}

void UnownedDatabasesAndTheSourceAreProtected() {
  const agiru::Connection maintenance(
      agiru::PointedAt(AGIRU_TEST_DSN, agiru::DatabaseName{"postgres"}));
  const std::string suffix = std::to_string(getpid());
  const std::string source = "agiru_guard_source_" + suffix;
  const std::string target = "agiru_guard_target_" + suffix;
  maintenance.Run("CREATE DATABASE " + source);
  maintenance.Run("CREATE DATABASE " + target);
  const std::string sourceDsn = agiru::PointedAt(AGIRU_TEST_DSN, agiru::DatabaseName{source});
  const std::string sourceIdentity = Identity(maintenance, source);
  const std::string targetIdentity = Identity(maintenance, target);
  const std::string ownedName = "agiru_guard_owned_" + suffix;
  const agiru::RunnerDatabase owned(sourceDsn, ownedName);
  CHECK_TRUE("a new scratch database is cloned and marked", owned.Cloned());
  const agiru::RunnerDatabase reused(sourceDsn, ownedName);
  CHECK_TRUE("a marked clone can be reused", !reused.Cloned());
  const agiru::RunnerDatabase fresh(sourceDsn, ownedName, true);
  CHECK_TRUE("fresh can replace a marked clone", fresh.Cloned());
  const std::string freshIdentity = Identity(maintenance, ownedName);
  bool staleRefused = false;
  try {
    owned.Drop();
  } catch (const agiru::DatabaseError &) { staleRefused = true; }
  CHECK_TRUE("a stale handle cannot drop a replacement database", staleRefused);
  CHECK_TRUE("the replacement survives the stale cleanup",
             Identity(maintenance, ownedName) == freshIdentity);
  fresh.Drop();
  fresh.Drop();
  CHECK_TRUE("owned cleanup is idempotent", Identity(maintenance, ownedName).empty());
  const std::string concurrentName = "agiru_guard_parallel_" + suffix;
  constexpr int kWorkers = 2;
  std::barrier start(kWorkers);
  auto create = [&] {
    start.arrive_and_wait();
    const agiru::RunnerDatabase runner(sourceDsn, concurrentName);
    return runner.Cloned();
  };
  auto first = std::async(std::launch::async, create);
  auto second = std::async(std::launch::async, create);
  const int created = static_cast<int>(first.get()) + static_cast<int>(second.get());
  CHECK_TRUE("concurrent acquisition creates exactly one marked clone", created == 1);
  const agiru::RunnerDatabase concurrent(sourceDsn, concurrentName);
  concurrent.Drop();
  const agiru::Result limits =
      maintenance.Execute("SELECT pg_catalog.current_setting('max_identifier_length')");
  const auto limit = limits.Value(0, 0);
  if (!limit) { throw agiru::DatabaseError("The server identifier limit is absent"); }
  const std::string overlong(std::stoul(std::string(*limit)) + 1, 'x');
  CHECK_TRUE("identifier truncation cannot redirect scratch creation",
             Refused(sourceDsn, agiru::DatabaseName{overlong}, true));
  const std::string roleName = "agiru_guard_role_" + suffix;
  const std::string roleDatabaseName = "agiru_guard_owner_" + suffix;
  maintenance.Run("CREATE ROLE " + roleName);
  const agiru::RunnerDatabase roleDatabase(sourceDsn, roleDatabaseName);
  const std::string roleDatabaseIdentity = Identity(maintenance, roleDatabaseName);
  maintenance.Run("ALTER DATABASE " + roleDatabaseName + " OWNER TO " + roleName);
  CHECK_TRUE("a marker cannot authorize a clone owned by another role",
             Refused(sourceDsn, agiru::DatabaseName{roleDatabaseName}, true));
  CHECK_TRUE("owner refusal leaves the database identity unchanged",
             Identity(maintenance, roleDatabaseName) == roleDatabaseIdentity);
  maintenance.Run("ALTER DATABASE " + roleDatabaseName + " OWNER TO CURRENT_USER");
  roleDatabase.Drop();
  maintenance.Run("DROP ROLE " + roleName);
  CHECK_TRUE("an unowned existing database cannot be reused",
             Refused(sourceDsn, agiru::DatabaseName{target}, false));
  CHECK_TRUE("reuse refusal leaves the database identity unchanged",
             Identity(maintenance, target) == targetIdentity);
  CHECK_TRUE("fresh refuses an unowned existing database",
             Refused(sourceDsn, agiru::DatabaseName{target}, true));
  CHECK_TRUE("fresh refusal leaves the database identity unchanged",
             Identity(maintenance, target) == targetIdentity);
  maintenance.Run("ALTER DATABASE " + target + " IS_TEMPLATE true");
  const std::string templateIdentity = Identity(maintenance, target);
  CHECK_TRUE("fresh refuses a template target",
             Refused(sourceDsn, agiru::DatabaseName{target}, true));
  CHECK_TRUE("template refusal leaves its identity unchanged",
             Identity(maintenance, target) == templateIdentity);
  CHECK_TRUE("fresh refuses the source itself",
             Refused(sourceDsn, agiru::DatabaseName{source}, true));
  CHECK_TRUE("source refusal leaves its identity unchanged",
             Identity(maintenance, source) == sourceIdentity);
  maintenance.Run("ALTER DATABASE " + target + " IS_TEMPLATE false");
  maintenance.Run("DROP DATABASE " + target);
  maintenance.Run("DROP DATABASE IF EXISTS " + source);
}

}

int main() {
  return gate::Run("ScratchGuard", [] { UnownedDatabasesAndTheSourceAreProtected(); });
}

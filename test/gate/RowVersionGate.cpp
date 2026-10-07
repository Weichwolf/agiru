#include "runtime/Database.h"
#include "runtime/RowVersionStorage.h"

#include "Check.h"
#include "OwnedDatabase.h"

#include <array>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <future>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>

namespace {

using agiru::Connection;
using agiru::DatabaseError;
using agiru::ProvisionRowVersions;

using gate::OwnedDatabase;

constexpr auto kDisconnectTimeout = std::chrono::seconds(5);

std::int64_t Scalar(const Connection &connection,
                    std::string_view sql,
                    std::span<const std::optional<std::string>> parameters = {}) {
  const agiru::Result result = connection.Execute(sql, parameters);
  if (result.Rows() != 1 || result.Columns() != 1) {
    throw DatabaseError("RowVersion gate: a scalar must be one non-null value");
  }
  const auto cell = result.Value(0, 0);
  if (!cell) { throw DatabaseError("RowVersion gate: a scalar must not be null"); }
  const std::string_view text = *cell;
  std::int64_t value = 0;
  const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
  if (error != std::errc{} || end != text.data() + text.size()) {
    throw DatabaseError("RowVersion gate: an exact signed bigint is required");
  }
  return value;
}

std::int64_t Allocate(const Connection &connection) {
  return Scalar(connection, "SELECT agiru_platform.next_rowversion_v1()");
}

std::int64_t Last(const Connection &connection) {
  return Scalar(connection, "SELECT agiru_platform.last_rowversion_v1()");
}

std::int64_t Minimum(const Connection &connection) {
  return Scalar(connection, "SELECT agiru_platform.minimum_rowversion_v1()");
}

std::int64_t OwnFences(const Connection &connection) {
  return Scalar(connection,
                "SELECT count(*) FROM pg_catalog.pg_locks WHERE locktype = 'advisory' "
                "AND objsubid = 1 AND granted AND classid::bigint >= 2147483648 "
                "AND pid = pg_catalog.pg_backend_pid()");
}

std::string WriteToken(const Connection &connection) {
  const auto result = connection.Execute("SELECT agiru_platform.write_transaction_v1()");
  const auto value = result.Value(0, 0);
  if (result.Rows() != 1 || result.Columns() != 1 || !value) {
    throw DatabaseError("Write transaction gate requires an exact non-null token");
  }
  return std::string(*value);
}

void WriteTransactionIdentity() {
  const OwnedDatabase database("write_identity");
  const Connection writer(database.Dsn());
  const Connection independent(database.Dsn());
  ProvisionRowVersions(writer);
  writer.Run("BEGIN");
  independent.Run("BEGIN");
  const std::string original = WriteToken(writer);
  CHECK_TEXT(
      "repeated writes retain the top-level transaction token", WriteToken(writer), original);
  CHECK_TRUE("independent transactions have distinct own-write tokens",
             WriteToken(independent) != original);
  CHECK_TRUE("the transaction token does not allocate a rowversion", Last(writer) == 0);
  CHECK_TRUE("the transaction token does not acquire an allocator fence", OwnFences(writer) == 0);
  CHECK_TRUE("bulk token reads still use one transaction identity",
             Scalar(writer,
                    "SELECT count(DISTINCT agiru_platform.write_transaction_v1()) "
                    "FROM pg_catalog.generate_series(1, 1000)") == 1);
  CHECK_TRUE("the server token is a non-null UUID",
             Scalar(writer,
                    "SELECT (pg_typeof(agiru_platform.write_transaction_v1()) = "
                    "'uuid'::regtype AND agiru_platform.write_transaction_v1() <> "
                    "'00000000-0000-0000-0000-000000000000'::uuid)::integer") == 1);
  writer.Run("SAVEPOINT retained");
  CHECK_TEXT("a savepoint retains its parent's own-write token", WriteToken(writer), original);
  writer.Run("RELEASE SAVEPOINT retained");
  CHECK_TEXT("released children keep the top-level token", WriteToken(writer), original);
  writer.Run("SAVEPOINT discarded");
  writer.Run("SELECT set_config('agiru.write_transaction_v1', '', true)");
  CHECK_TRUE("a discarded cache replacement has a distinct token", WriteToken(writer) != original);
  writer.Run("ROLLBACK TO SAVEPOINT discarded");
  writer.Run("RELEASE SAVEPOINT discarded");
  CHECK_TEXT("rollback restores the parent's own-write token", WriteToken(writer), original);
  ProvisionRowVersions(writer);
  CHECK_TEXT("reprovisioning does not change an active write token", WriteToken(writer), original);
  writer.Run("COMMIT");
  CHECK_TRUE("Commit releases the connection-local token cache",
             Scalar(writer,
                    "SELECT (NULLIF(current_setting('agiru.write_transaction_v1', true), "
                    "'') IS NULL)::integer") == 1);
  writer.Run("BEGIN");
  const std::string committed = WriteToken(writer);
  CHECK_TRUE("Commit never grants the preceding transaction's exemption", committed != original);
  writer.Run("ROLLBACK");
  CHECK_TRUE("rollback releases the connection-local token cache",
             Scalar(writer,
                    "SELECT (NULLIF(current_setting('agiru.write_transaction_v1', true), "
                    "'') IS NULL)::integer") == 1);
  writer.Run("BEGIN");
  CHECK_TRUE("rollback never grants the preceding transaction's exemption",
             WriteToken(writer) != committed);
  writer.Run("ROLLBACK");
  independent.Run("ROLLBACK");
}

void FirstWriteInsideSavepoint() {
  const OwnedDatabase database("write_child");
  const Connection writer(database.Dsn());
  ProvisionRowVersions(writer);
  writer.Run("BEGIN");
  writer.Run("SAVEPOINT discarded");
  const std::string discarded = WriteToken(writer);
  writer.Run("ROLLBACK TO SAVEPOINT discarded");
  writer.Run("RELEASE SAVEPOINT discarded");
  CHECK_TRUE("a rolled-back first write token cannot authorize its parent",
             WriteToken(writer) != discarded);
  writer.Run("ROLLBACK");
  writer.Run("BEGIN");
  writer.Run("SAVEPOINT retained");
  const std::string retained = WriteToken(writer);
  writer.Run("RELEASE SAVEPOINT retained");
  CHECK_TEXT(
      "a released first write retains its token in the parent", WriteToken(writer), retained);
  writer.Run("COMMIT");
  CHECK_TRUE("autocommit cannot revive a released transaction token",
             WriteToken(writer) != retained);
  const std::string autocommitted = WriteToken(writer);
  CHECK_TRUE("separate autocommitted writes have distinct tokens",
             WriteToken(writer) != autocommitted);
}

void WaitForBackendExit(const Connection &observer,
                        std::int64_t backend,
                        std::chrono::milliseconds timeout = kDisconnectTimeout) {
  constexpr auto pollInterval = std::chrono::milliseconds(5);
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  const std::array<std::optional<std::string>, 1> parameters{std::to_string(backend)};
  while (Scalar(observer,
                "SELECT count(*) FROM pg_catalog.pg_stat_activity "
                "WHERE pid = $1::integer AND datname = pg_catalog.current_database()",
                parameters) != 0) {
    if (std::chrono::steady_clock::now() >= deadline) {
      throw DatabaseError("RowVersion gate: disconnected backend did not terminate");
    }
    std::this_thread::sleep_for(pollInterval);
  }
}

void AnActiveBackendCannotSatisfyDisconnect() {
  constexpr auto timeout = std::chrono::milliseconds(25);
  const OwnedDatabase database("disconnect_timeout");
  const Connection observer(database.Dsn());
  const Connection writer(database.Dsn());
  ProvisionRowVersions(observer);
  writer.Run("BEGIN");
  const auto pending = Allocate(writer);
  bool refused = false;
  try {
    WaitForBackendExit(observer, Scalar(writer, "SELECT pg_catalog.pg_backend_pid()"), timeout);
  } catch (const DatabaseError &error) {
    refused =
        std::string_view(error.what()) == "RowVersion gate: disconnected backend did not terminate";
  }
  CHECK_TRUE("a live backend cannot satisfy the disconnect observation", refused);
  CHECK_TRUE("observation timeout preserves the active transaction", writer.InTransaction());
  CHECK_TRUE("observation timeout cannot hide the active rowversion", Minimum(observer) == pending);
  writer.Run("ROLLBACK");
}

void InitialStateAndReprovisioning() {
  const OwnedDatabase database("provision");
  const Connection connection(database.Dsn());
  connection.Run("SET client_min_messages = warning");
  ProvisionRowVersions(connection);
  CHECK_TRUE("initial last used is zero, not the uncalled sequence start", Last(connection) == 0);
  CHECK_TRUE("without an allocator minimum is last plus one", Minimum(connection) == 1);
  CHECK_TRUE("provisioning does not leave a transaction open", !connection.InTransaction());
  CHECK_TRUE("the first autocommitted allocation is one", Allocate(connection) == 1);
  CHECK_TRUE("autocommit has no active allocator", Minimum(connection) == 2);
  ProvisionRowVersions(connection);
  CHECK_TRUE("reprovisioning preserves the database counter", Last(connection) == 1);
  CHECK_TRUE("reprovisioning does not reset subsequent allocations", Allocate(connection) == 2);
  connection.Run("BEGIN");
  ProvisionRowVersions(connection);
  CHECK_TRUE("caller provisioning keeps the caller transaction open", connection.InTransaction());
  connection.Run("ROLLBACK");
  CHECK_TRUE("provisioning rollback preserves an existing counter", Last(connection) == 2);
}

void SqlWritesAcrossTablesAndCommitOrder() {
  const OwnedDatabase database("writes");
  const Connection writer(database.Dsn());
  const Connection newer(database.Dsn());
  const Connection observer(database.Dsn());
  ProvisionRowVersions(writer);
  writer.Run("CREATE TABLE first_rows (id integer PRIMARY KEY, payload text NOT NULL, "
             "version bigint NOT NULL DEFAULT agiru_platform.next_rowversion_v1())");
  writer.Run("CREATE TABLE second_rows (LIKE first_rows INCLUDING ALL)");
  CHECK_TRUE(
      "INSERT's database default returns the first rowversion",
      Scalar(writer, "INSERT INTO first_rows VALUES (1, 'old', DEFAULT) RETURNING version") == 1);
  CHECK_TRUE("an UPDATE advances the version even without a value change",
             Scalar(writer,
                    "UPDATE first_rows SET version = agiru_platform.next_rowversion_v1() "
                    "WHERE id = 1 RETURNING version") == 2);
  writer.Run("BEGIN");
  CHECK_TRUE("the older transaction begins at version three",
             Scalar(writer,
                    "UPDATE first_rows SET payload = 'new', "
                    "version = agiru_platform.next_rowversion_v1() RETURNING version") == 3);
  CHECK_TRUE(
      "a different table shares the same database counter",
      Scalar(writer, "INSERT INTO second_rows VALUES (1, 'second', DEFAULT) RETURNING version") ==
          4);
  CHECK_TRUE("two writes in the same transaction get distinct versions",
             Scalar(writer,
                    "UPDATE first_rows SET version = agiru_platform.next_rowversion_v1() "
                    "RETURNING version") == 5);
  newer.Run("BEGIN");
  CHECK_TRUE(
      "another session continues the database sequence",
      Scalar(newer, "INSERT INTO first_rows VALUES (2, 'newer', DEFAULT) RETURNING version") == 6);
  CHECK_TRUE("last used includes uncommitted allocations", Last(observer) == 6);
  CHECK_TRUE("the active minimum retains the older writer's first allocation",
             Minimum(observer) == 3);
  CHECK_TRUE("a reader still sees the committed version",
             Scalar(observer, "SELECT version FROM first_rows") == 2);
  newer.Run("COMMIT");
  CHECK_TRUE("a newer commit cannot hide the older uncommitted writer", Minimum(observer) == 3);
  writer.Run("COMMIT");
  CHECK_TRUE("after both commits the minimum is last plus one", Minimum(observer) == 7);
  CHECK_TRUE("the older commit preserves its actual final version",
             Scalar(observer, "SELECT version FROM first_rows WHERE id = 1") == 5);
  CHECK_TRUE("a renamed row can consume a new version",
             Scalar(writer,
                    "UPDATE second_rows SET id = 2, "
                    "version = agiru_platform.next_rowversion_v1() RETURNING version") == 7);
  CHECK_TRUE("a committed database counter survives reconnect",
             Last(Connection(database.Dsn())) == 7);
}

void SavepointsAndBoundedFences() {
  constexpr std::int64_t allocationCount = 1000;
  const OwnedDatabase database("savepoints");
  const Connection writer(database.Dsn());
  const Connection observer(database.Dsn());
  ProvisionRowVersions(writer);
  writer.Run("BEGIN");
  writer.Run("SAVEPOINT child");
  const auto discarded = Allocate(writer);
  CHECK_TRUE("the first child allocation publishes a fence", Minimum(observer) == discarded);
  writer.Run("ROLLBACK TO SAVEPOINT child");
  CHECK_TRUE("rolling back the first allocation removes its fence", OwnFences(writer) == 0);
  CHECK_TRUE("a rolled-back allocation is never reused", Last(observer) == discarded);
  CHECK_TRUE("a rolled-back child is no longer active", Minimum(observer) == discarded + 1);
  const auto retained = Allocate(writer);
  CHECK_TRUE("the restored setting cannot skip a replacement fence", Minimum(observer) == retained);
  writer.Run("RELEASE SAVEPOINT child");
  CHECK_TRUE("releasing a child retains its fence in the parent", OwnFences(writer) == 1);
  writer.Run("SAVEPOINT later");
  CHECK_TRUE("bulk allocation consumes one distinct version per call",
             Scalar(writer,
                    "SELECT count(agiru_platform.next_rowversion_v1()) "
                    "FROM pg_catalog.generate_series(1, 1000)") == allocationCount);
  CHECK_TRUE("a thousand row writes retain only one transaction fence", OwnFences(writer) == 1);
  CHECK_TRUE("bulk allocation's last value is exact", Last(observer) == retained + allocationCount);
  CHECK_TRUE("bulk allocation cannot advance the active minimum", Minimum(observer) == retained);
  writer.Run("ROLLBACK TO SAVEPOINT later");
  writer.Run("RELEASE SAVEPOINT later");
  CHECK_TRUE("child rollback preserves the parent fence", OwnFences(writer) == 1);
  CHECK_TRUE("parent fence still bounds the minimum", Minimum(observer) == retained);
  writer.Run("ROLLBACK");
  CHECK_TRUE("transaction rollback releases every fence", OwnFences(writer) == 0);
  CHECK_TRUE("rollback preserves the consumed sequence values",
             Last(observer) == retained + allocationCount);
  CHECK_TRUE("a fully rolled-back writer no longer bounds the minimum",
             Minimum(observer) == Last(observer) + 1);
}

void StatementFailureAndDisconnect() {
  const OwnedDatabase database("failure");
  const Connection observer(database.Dsn());
  ProvisionRowVersions(observer);
  std::int64_t writerBackend = 0;
  {
    const Connection writer(database.Dsn());
    writerBackend = Scalar(writer, "SELECT pg_catalog.pg_backend_pid()");
    writer.Run("BEGIN");
    writer.Run("SAVEPOINT child");
    bool refused = false;
    try {
      writer.Run("DO $case$ BEGIN PERFORM agiru_platform.next_rowversion_v1(); "
                 "RAISE EXCEPTION 'rowversion fixture error'; END $case$");
    } catch (const DatabaseError &error) {
      refused = std::string_view(error.what()).contains("rowversion fixture error");
    }
    CHECK_TRUE("an error after allocation propagates", refused);
    writer.Run("ROLLBACK TO SAVEPOINT child");
    CHECK_TRUE("statement failure rolls back its fence", OwnFences(writer) == 0);
    CHECK_TRUE("statement failure does not rewind the counter", Last(observer) == 1);
    const auto pending = Allocate(writer);
    CHECK_TRUE("the next write after a caught error publishes a new fence",
               Minimum(observer) == pending);
  }
  WaitForBackendExit(observer, writerBackend);
  CHECK_TRUE("disconnect removes an uncommitted writer", Minimum(observer) == Last(observer) + 1);
  const Connection reconnected(database.Dsn());
  const auto before = Last(observer);
  reconnected.Run("SET agiru.rowversion_fence_v1 = 'stale:1'");
  reconnected.Run("BEGIN");
  CHECK_TRUE("a stale session setting cannot reuse another transaction's fence",
             Allocate(reconnected) == before + 1);
  CHECK_TRUE("the new transaction owns its own fence", OwnFences(reconnected) == 1);
  CHECK_TRUE("the stale setting cannot let the minimum skip a live writer",
             Minimum(observer) == before + 1);
  reconnected.Run("COMMIT");
}

void DatabaseIsolation() {
  const OwnedDatabase first("isolation_first");
  const OwnedDatabase second("isolation_second");
  const Connection writer(first.Dsn());
  const Connection observer(second.Dsn());
  ProvisionRowVersions(writer);
  ProvisionRowVersions(observer);
  CHECK_TRUE("each database has its own initial counter",
             Allocate(writer) == 1 && Allocate(observer) == 1);
  writer.Run("BEGIN");
  CHECK_TRUE("the first database has an active version two", Allocate(writer) == 2);
  CHECK_TRUE("another database continues independently", Allocate(observer) == 2);
  CHECK_TRUE("its next independent allocation advances once", Allocate(observer) == 3);
  CHECK_TRUE("cluster-wide locks do not lower another database's minimum", Minimum(observer) == 4);
  CHECK_TRUE("the first database still observes its own fence", Minimum(writer) == 2);
  writer.Run("ROLLBACK");
}

struct BatchResult {
  std::int64_t rows;
  std::int64_t fences;
};

void ConcurrentWritersOwnDistinctVersions() {
  constexpr std::size_t workerCount = 3;
  constexpr std::int64_t rowsPerWorker = 128;
  constexpr auto totalRows = rowsPerWorker * static_cast<std::int64_t>(workerCount);
  const OwnedDatabase database("concurrent");
  const Connection observer(database.Dsn());
  ProvisionRowVersions(observer);
  observer.Run(
      "CREATE TABLE concurrent_rows (worker integer, ordinal integer, "
      "version bigint DEFAULT agiru_platform.next_rowversion_v1(), PRIMARY KEY (worker, ordinal))");
  std::array<std::future<BatchResult>, workerCount> workers;
  for (std::size_t index = 0; index < workers.size(); ++index) {
    workers[index] = std::async(std::launch::async, [dsn = database.Dsn(), index] {
      const Connection writer(dsn);
      writer.Run("BEGIN");
      const std::array<std::optional<std::string>, 2> parameters{std::to_string(index),
                                                                 std::to_string(rowsPerWorker)};
      const auto rows =
          Scalar(writer,
                 "WITH written AS (INSERT INTO concurrent_rows (worker, ordinal) "
                 "SELECT $1::integer, ordinal FROM pg_catalog.generate_series(1, $2::integer) "
                 "AS ordinal RETURNING version) SELECT count(*) FROM written",
                 parameters);
      const auto fences = OwnFences(writer);
      writer.Run("COMMIT");
      return BatchResult{.rows = rows, .fences = fences};
    });
  }
  for (auto &worker : workers) {
    const auto result = worker.get();
    CHECK_TRUE("each concurrent session writes its complete batch", result.rows == rowsPerWorker);
    CHECK_TRUE("each concurrent batch owns only one transaction fence", result.fences == 1);
  }
  CHECK_TRUE("all concurrent writes persist",
             Scalar(observer, "SELECT count(*) FROM concurrent_rows") == totalRows);
  CHECK_TRUE("concurrent writers never reuse a version",
             Scalar(observer, "SELECT count(DISTINCT version) FROM concurrent_rows") == totalRows);
  CHECK_TRUE("concurrent writes start at the first database allocation",
             Scalar(observer, "SELECT min(version) FROM concurrent_rows") == 1);
  CHECK_TRUE("concurrent batches share one exact database-wide sequence",
             Last(observer) == totalRows);
  CHECK_TRUE("completed concurrent writers leave no active fence",
             Minimum(observer) == totalRows + 1);
}

void IncompatibleStorageIsRefused() {
  constexpr std::array changes{
      "ALTER SEQUENCE agiru_platform.rowversions_v1 CACHE 2",
      "ALTER SEQUENCE agiru_platform.rowversions_v1 INCREMENT BY 2",
      "ALTER SEQUENCE agiru_platform.rowversions_v1 MINVALUE 0",
      "ALTER SEQUENCE agiru_platform.rowversions_v1 MAXVALUE 2147483647",
      "ALTER SEQUENCE agiru_platform.rowversions_v1 START WITH 2",
      "ALTER SEQUENCE agiru_platform.rowversions_v1 CYCLE",
      "ALTER SEQUENCE agiru_platform.rowversions_v1 AS integer",
      "ALTER SEQUENCE agiru_platform.rowversions_v1 SET UNLOGGED",
      "ALTER SEQUENCE agiru_platform.rowversions_v1 OWNED BY agiru_platform.owner_fixture.id"};
  const OwnedDatabase database("incompatible");
  const Connection connection(database.Dsn());
  connection.Run("SET client_min_messages = warning");
  ProvisionRowVersions(connection);
  connection.Run("CREATE TABLE agiru_platform.owner_fixture (id bigint)");
  const auto before = Allocate(connection);
  for (const auto *const change : changes) {
    connection.Run("BEGIN");
    connection.Run(change);
    bool refused = false;
    try {
      ProvisionRowVersions(connection);
    } catch (const DatabaseError &error) {
      refused = std::string_view(error.what()).contains("incompatible sequence storage");
    }
    CHECK_TRUE("incompatible existing storage requires explicit migration", refused);
    CHECK_TRUE("a provisioning refusal does not abort the caller's transaction",
               connection.InTransaction() && !connection.InFailedTransaction());
    CHECK_TRUE("the caller remains usable after a provisioning refusal",
               Scalar(connection, "SELECT 1") == 1);
    connection.Run("ROLLBACK");
    ProvisionRowVersions(connection);
    CHECK_TRUE("failed provisioning never resets an existing counter", Last(connection) == before);
  }
}

void WrongRelationAndAtomicProvisioning() {
  const OwnedDatabase database("relation");
  const Connection connection(database.Dsn());
  connection.Run("SET client_min_messages = warning");
  connection.Run("CREATE SCHEMA agiru_platform");
  connection.Run("CREATE TABLE agiru_platform.rowversions_v1 (value bigint)");
  bool refused = false;
  try {
    ProvisionRowVersions(connection);
  } catch (const DatabaseError &error) {
    refused = std::string_view(error.what()).contains("incompatible sequence storage");
  }
  CHECK_TRUE("an existing non-sequence relation is explicitly refused", refused);
  CHECK_TRUE("a standalone provisioning refusal closes its own transaction",
             !connection.InTransaction());
  CHECK_TRUE("a refused install creates no operation functions",
             Scalar(connection,
                    "SELECT count(*) FROM pg_catalog.pg_proc WHERE "
                    "pronamespace = 'agiru_platform'::regnamespace") == 0);
  connection.Run("DROP TABLE agiru_platform.rowversions_v1");
  connection.Run("BEGIN");
  ProvisionRowVersions(connection);
  connection.Run("ROLLBACK");
  CHECK_TRUE("rolling back a new installation removes its sequence and functions",
             Scalar(connection,
                    "SELECT count(*) FROM pg_catalog.pg_class WHERE "
                    "relnamespace = 'agiru_platform'::regnamespace") == 0);
  ProvisionRowVersions(connection);
  CHECK_TRUE("a fresh install after rollback still starts uncalled", Last(connection) == 0);
}

void ExhaustionNeverWraps() {
  constexpr std::string_view lastValue = "9223372036854775807";
  const OwnedDatabase database("exhaustion");
  const Connection writer(database.Dsn());
  const Connection observer(database.Dsn());
  ProvisionRowVersions(writer);
  writer.Run("SELECT pg_catalog.setval('agiru_platform.rowversions_v1', 4294967295, true)");
  writer.Run("BEGIN");
  const auto wide = Allocate(writer);
  CHECK_TRUE("a fence crossing the 32-bit boundary decodes exactly", Minimum(observer) == wide);
  writer.Run("ROLLBACK");
  writer.Run(
      "SELECT pg_catalog.setval('agiru_platform.rowversions_v1', 9223372036854775806, true)");
  writer.Run("BEGIN");
  const agiru::Result final = writer.Execute("SELECT agiru_platform.next_rowversion_v1()");
  CHECK_TEXT(
      "the final representable allocation is exact", final.Value(0, 0).value_or(""), lastValue);
  const agiru::Result minimum = observer.Execute("SELECT agiru_platform.minimum_rowversion_v1()");
  CHECK_TEXT(
      "the largest active fence decodes exactly", minimum.Value(0, 0).value_or(""), lastValue);
  writer.Run("COMMIT");
  bool refused = false;
  try {
    (void)Allocate(writer);
  } catch (const DatabaseError &error) {
    refused = std::string_view(error.what()).contains("reached maximum value");
  }
  CHECK_TRUE("the counter refuses exhaustion instead of wrapping", refused);
  CHECK_TRUE("failed allocation leaks no transaction fence", OwnFences(writer) == 0);
  observer.Run("SET lock_timeout = '200ms'");
  bool overflow = false;
  try {
    (void)Minimum(observer);
  } catch (const DatabaseError &error) {
    overflow = std::string_view(error.what()).contains("bigint out of range");
  }
  CHECK_TRUE("an unrepresentable idle minimum refuses instead of overflowing", overflow);
  CHECK_TRUE(
      "overflow releases the publication lock",
      Scalar(writer, "SELECT pg_catalog.pg_try_advisory_lock(x'41475256'::integer, 1)::integer") ==
          1);
  writer.Run("SELECT pg_catalog.pg_advisory_unlock(x'41475256'::integer, 1)");
}

void UnrelatedAdvisoryNamespacesAreIgnored() {
  const OwnedDatabase database("lock_namespaces");
  const Connection writer(database.Dsn());
  const Connection observer(database.Dsn());
  ProvisionRowVersions(writer);
  CHECK_TRUE("a committed rowversion precedes unrelated locks", Allocate(writer) == 1);
  writer.Run("BEGIN");
  writer.Run("SELECT pg_catalog.pg_advisory_xact_lock(1::bigint)");
  writer.Run("SELECT pg_catalog.pg_advisory_xact_lock(-1, 1)");
  CHECK_TRUE("positive bigint and negative two-integer locks are not rowversion fences",
             Minimum(observer) == 2);
  CHECK_TRUE("unrelated locks do not consume rowversions", Last(observer) == 1);
  writer.Run("ROLLBACK");
}

void WaitForAllocation(const Connection &observer) {
  constexpr auto timeout = std::chrono::seconds(5);
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  while (Last(observer) == 0) {
    if (std::chrono::steady_clock::now() >= deadline) {
      throw DatabaseError("RowVersion gate: allocator did not reach its publication interval");
    }
    std::this_thread::yield();
  }
}

void PublicationIntervalIsProtected() {
  const OwnedDatabase database("publication");
  const Connection writer(database.Dsn());
  const Connection observer(database.Dsn());
  const Connection reader(database.Dsn());
  ProvisionRowVersions(writer);
  writer.Run("BEGIN");
  auto allocating = std::async(std::launch::async, [&] { return Allocate(writer); });
  WaitForAllocation(observer);
  auto reading = std::async(std::launch::async, [&] { return Minimum(reader); });
  constexpr auto observation = std::chrono::milliseconds(20);
  CHECK_TRUE("minimum cannot pass an unpublished allocation",
             reading.wait_for(observation) == std::future_status::timeout);
  const auto allocated = allocating.get();
  CHECK_TRUE("the reader observes the published transaction fence", reading.get() == allocated);
  writer.Run("ROLLBACK");
}

void CancellationReleasesPublicationLock() {
  const OwnedDatabase database("cancellation");
  const Connection writer(database.Dsn());
  const Connection observer(database.Dsn());
  ProvisionRowVersions(writer);
  writer.Run("SET statement_timeout = '20ms'");
  writer.Run("BEGIN");
  bool canceled = false;
  try {
    (void)Allocate(writer);
  } catch (const DatabaseError &error) {
    canceled = std::string_view(error.what()).contains("statement timeout");
  }
  CHECK_TRUE("the paused allocation is canceled inside the publication interval", canceled);
  writer.Run("ROLLBACK");
  CHECK_TRUE("cancellation after nextval preserves its consumed value", Last(observer) == 1);
  observer.Run("SET lock_timeout = '200ms'");
  CHECK_TRUE("cancellation releases the session publication lock", Minimum(observer) == 2);
  CHECK_TRUE("a new allocator can continue after cancellation", Allocate(observer) == 2);
}

void MinimumCancellationReleasesPublicationLock() {
  const OwnedDatabase database("minimum_cancellation");
  const Connection reader(database.Dsn());
  const Connection observer(database.Dsn());
  ProvisionRowVersions(reader);
  reader.Run("SET statement_timeout = '20ms'");
  bool canceled = false;
  try {
    (void)Minimum(reader);
  } catch (const DatabaseError &error) {
    canceled = std::string_view(error.what()).contains("statement timeout");
  }
  CHECK_TRUE("the paused minimum query is canceled inside the publication interval", canceled);
  observer.Run("SET lock_timeout = '200ms'");
  CHECK_TRUE("minimum cancellation releases the session publication lock", Allocate(observer) == 1);
}

}

int main(int argc, char **argv) {
  return gate::Run("RowVersion", [&] {
    if (argc == 2 && std::string_view(argv[1]) == "--disconnect") {
      AnActiveBackendCannotSatisfyDisconnect();
      StatementFailureAndDisconnect();
      return;
    }
    if (argc == 2 && std::string_view(argv[1]) == "--publication") {
      PublicationIntervalIsProtected();
      return;
    }
    if (argc == 2 && std::string_view(argv[1]) == "--cancellation") {
      CancellationReleasesPublicationLock();
      return;
    }
    if (argc == 2 && std::string_view(argv[1]) == "--minimum-cancellation") {
      MinimumCancellationReleasesPublicationLock();
      return;
    }
    if (argc != 1) { throw DatabaseError("RowVersion gate: unknown arguments"); }
    WriteTransactionIdentity();
    FirstWriteInsideSavepoint();
    InitialStateAndReprovisioning();
    SqlWritesAcrossTablesAndCommitOrder();
    SavepointsAndBoundedFences();
    AnActiveBackendCannotSatisfyDisconnect();
    StatementFailureAndDisconnect();
    DatabaseIsolation();
    ConcurrentWritersOwnDistinctVersions();
    IncompatibleStorageIsRefused();
    WrongRelationAndAtomicProvisioning();
    ExhaustionNeverWraps();
    UnrelatedAdvisoryNamespacesAreIgnored();
  });
}

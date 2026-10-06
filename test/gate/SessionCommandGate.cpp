#include "meta/Ids.h"
#include "platform/User.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/SessionCommand.h"
#include "runtime/SingleInstance.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/Transaction.h"
#include "type/Date.h"
#include "type/Guid.h"
#include "type/Language.h"

#include "Check.h"
#include "Cursor.h"
#include "OwnedDatabase.h"

#include <array>
#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

constexpr agiru::CodeunitId kSingle{50139};
constexpr int kPolishLanguage = 1045;
constexpr int kGermanLanguage = 1031;
constexpr int kPrivateNumber = 42;
constexpr int kChangedNumber = 43;
constexpr int kIdleSessions = 500;

agiru::Guid Identity(std::uint8_t suffix) {
  std::array<std::uint8_t, agiru::Guid::kSize> bytes{};
  bytes.back() = suffix;
  return agiru::Guid(bytes);
}

int &PrivateNumber() {
  return *static_cast<int *>(agiru::detail::SingleInstanceOf(
      kSingle,
      []() -> void * { return new int{}; },
      [](void *value) { delete static_cast<int *>(value); }));
}

void Seed(const std::string &dsn) {
  const agiru::Session seed(dsn);
  agiru::CreateTable(seed.Database(), agiru::TableTraits<agiru::platform::User>::kTable);
  for (std::uint8_t suffix = 1; suffix <= 2; ++suffix) {
    agiru::platform::User user;
    user.UserSecurityID = Identity(suffix);
    user.UserName = "User " + std::to_string(suffix);
    user.Insert();
  }
  seed.Database().Run("CREATE TABLE command_values (id integer PRIMARY KEY, value integer)");
  seed.Database().Run(
      "CREATE TABLE command_deferred (value integer UNIQUE DEFERRABLE INITIALLY DEFERRED)");
  agiru::Commit();
}

std::string Value(const agiru::Connection &connection, int identity) {
  const std::array<std::optional<std::string>, 1> arguments{std::to_string(identity)};
  const auto result =
      connection.Execute("SELECT value FROM command_values WHERE id = $1", arguments);
  return result.Rows() == 0 ? "missing" : std::string(result.Value(0, 0).value_or("null"));
}

void IdleStateBorrowsNoConnection(const std::string &dsn) {
  const agiru::Connection observer(dsn);
  std::vector<std::unique_ptr<agiru::Session>> sessions;
  sessions.reserve(kIdleSessions);
  for (int index = 0; index < kIdleSessions; ++index) {
    sessions.push_back(std::make_unique<agiru::Session>(Identity(1)));
  }
  CHECK_TRUE("idle sessions never become current", !agiru::Session::HasCurrent());
  const auto clients = observer.Execute("SELECT count(*) FROM pg_stat_activity "
                                        "WHERE datname = current_database()");
  CHECK_TRUE("five hundred idle contexts open no PostgreSQL connections",
             clients.Value(0, 0) == std::optional<std::string_view>{"1"});
  bool refused = false;
  try {
    static_cast<void>(sessions.front()->Database());
  } catch (const agiru::SessionError &) { refused = true; }
  CHECK_TRUE("idle state cannot accidentally execute SQL", refused);
  bool blank = false;
  try {
    const agiru::Session anonymous(agiru::Guid{});
  } catch (const agiru::SessionError &) { blank = true; }
  CHECK_TRUE("detached clients cannot use a blank principal", blank);
}

void StateSurvivesCommandsAndWorkers(const std::string &dsn) {
  agiru::Session persistent(Identity(1));
  const auto workDate = agiru::Date::FromYmd(2026, 1, 1);
  agiru::Temporary<agiru::platform::User> temporary;
  {
    agiru::Connection connection(dsn);
    agiru::SessionCommand command(persistent, connection);
    persistent.Language(kPolishLanguage);
    persistent.WorkDate(workDate);
    persistent.CompanyName("Private company");
    PrivateNumber() = kPrivateNumber;
    temporary.UserSecurityID = Identity(1);
    temporary.UserName = "Private temporary row";
    temporary.Insert();
    connection.Run("INSERT INTO command_values VALUES (1, 11)");
    command.Keep();
    CHECK_TRUE("Keep detaches immediately and returns an idle connection",
               !agiru::Session::HasCurrent() && !connection.InTransaction());
    bool repeated = false;
    try {
      command.Keep();
    } catch (const agiru::SessionError &) { repeated = true; }
    CHECK_TRUE("a completed command cannot commit again", repeated);
  }
  std::string error;
  bool privateState = false;
  std::thread worker([&] {
    try {
      agiru::Connection connection(dsn);
      agiru::SessionCommand command(persistent, connection);
      privateState = PrivateNumber() == kPrivateNumber && persistent.WorkDate() == workDate &&
                     persistent.CompanyName() == "Private company" &&
                     agiru::Language::Current() == kPolishLanguage && temporary.Get(Identity(1));
      PrivateNumber() = kChangedNumber;
      connection.Run("UPDATE command_values SET value = 22 WHERE id = 1");
      command.Keep();
    } catch (const std::exception &failure) { error = failure.what(); }
  });
  worker.join();
  CHECK_SILENT("the same context activates on another worker", error);
  CHECK_TRUE("language/workdate/company/temporary/SingleInstance survive detachment", privateState);
  agiru::Connection next(dsn);
  agiru::SessionCommand command(persistent, next);
  CHECK_TRUE("the second worker's private mutations survive", PrivateNumber() == kChangedNumber);
  CHECK_TEXT("next command sees the previous command's committed SQL", Value(next, 1), "22");
  command.Keep();
}

void DifferentUsersAndNestedParentsRemainPrivate(const std::string &dsn) {
  agiru::Session first(Identity(1));
  agiru::Session second(Identity(2));
  agiru::Connection shared(dsn);
  {
    agiru::SessionCommand command(first, shared);
    first.Language(kPolishLanguage);
    PrivateNumber() = kPrivateNumber;
    command.Keep();
  }
  {
    agiru::Session parent(dsn);
    parent.Language(kGermanLanguage);
    {
      agiru::SessionCommand command(second, shared);
      CHECK_TRUE("a reused connection carries the second user's identity",
                 agiru::Session::Current().UserSecurityId() == Identity(2));
      CHECK_TRUE("a second user has independent SingleInstances", PrivateNumber() == 0);
      CHECK_TRUE("a second user starts with its own language", agiru::Language::Current() == 1033);
      command.Keep();
    }
    CHECK_TRUE("command completion restores the parent's current session and language",
               &agiru::Session::Current() == &parent &&
                   agiru::Language::Current() == kGermanLanguage);
    { const agiru::Session idle(Identity(1)); }
    CHECK_TRUE("destroying idle contexts cannot clear another current session",
               &agiru::Session::Current() == &parent);
  }
  agiru::SessionCommand command(first, shared);
  CHECK_TRUE("the first user retains private state on the shared connection",
             PrivateNumber() == kPrivateNumber && agiru::Language::Current() == kPolishLanguage);
  command.Keep();
}

void AtomicFailureAndDurableCommit(const std::string &dsn) {
  agiru::Session session(Identity(1));
  agiru::Connection writer(dsn);
  const agiru::Connection observer(dsn);
  {
    const agiru::SessionCommand command(session, writer);
    writer.Run("INSERT INTO command_values VALUES (2, 22)");
  }
  CHECK_TEXT("unfinished commands roll back their writes", Value(observer, 2), "missing");
  {
    const agiru::SessionCommand command(session, writer);
    writer.Run("INSERT INTO command_values VALUES (2, 22)");
    agiru::Commit();
    writer.Run("INSERT INTO command_values VALUES (3, 33)");
  }
  CHECK_TEXT("explicit production Commit survives command failure", Value(observer, 2), "22");
  CHECK_TEXT("later work after Commit is rolled back", Value(observer, 3), "missing");
  CHECK_TRUE("command failure clears its logical savepoints", session.Transaction().Depth() == 0);
  CHECK_TRUE("command failure leaves the borrowed connection idle", !writer.InTransaction());
  {
    agiru::SessionCommand command(session, writer);
    writer.Run("INSERT INTO command_values VALUES (3, 33)");
    session.Transaction().MarkConsistent("Fixture", false);
    bool refused = false;
    try {
      command.Keep();
    } catch (const agiru::Error &) { refused = true; }
    CHECK_TRUE("automatic completion still enforces AL consistency", refused);
  }
  CHECK_TEXT("inconsistent completion commits nothing", Value(observer, 3), "missing");
  {
    agiru::SessionCommand command(session, writer);
    writer.Run("INSERT INTO command_values VALUES (3, 33)");
    command.Keep();
  }
  CHECK_TEXT("a recovered command commits normally", Value(observer, 3), "33");
}

void ConcurrentAndDirtyLeasesRefuse(const std::string &dsn) {
  agiru::Session session(Identity(1));
  agiru::Connection first(dsn);
  agiru::Connection second(dsn);
  {
    agiru::SessionCommand held(session, first);
    bool refused = false;
    std::string unexpected;
    std::thread worker([&] {
      try {
        const agiru::SessionCommand attempted(session, second);
      } catch (const agiru::SessionError &) {
        refused = true;
      } catch (const std::exception &error) { unexpected = error.what(); }
    });
    worker.join();
    CHECK_SILENT("overlapping activation has the expected refusal type", unexpected);
    CHECK_TRUE("one persistent session cannot run concurrently", refused);
    CHECK_TRUE("a refused overlap cannot change the active worker",
               &agiru::Session::Current() == &session);
    held.Keep();
  }
  second.Run("BEGIN");
  bool dirty = false;
  try {
    const agiru::SessionCommand attempted(session, second);
  } catch (const agiru::SessionError &) { dirty = true; }
  CHECK_TRUE("a caller-owned transaction refuses without being consumed",
             dirty && second.InTransaction());
  second.Run("ROLLBACK");
  second.Close();
  bool closed = false;
  try {
    const agiru::SessionCommand attempted(session, second);
  } catch (const agiru::DatabaseError &) { closed = true; }
  CHECK_TRUE("closed leases refuse without becoming current",
             closed && !agiru::Session::HasCurrent());
  CHECK_TRUE("closed transports are explicitly unhealthy", !second.IsOpen());
}

void RevocationAndAbortedSQLRestoreState(const std::string &dsn) {
  agiru::Session session(Identity(1));
  agiru::Connection connection(dsn);
  connection.Run(R"(UPDATE "User" SET "State" = 1)");
  bool revoked = false;
  try {
    const agiru::SessionCommand attempted(session, connection);
  } catch (const agiru::SessionError &) { revoked = true; }
  CHECK_TRUE("every command rechecks the current PostgreSQL account status", revoked);
  CHECK_TRUE("refused account cleanup leaves no current session or transaction",
             !agiru::Session::HasCurrent() && !connection.InTransaction() &&
                 session.Transaction().Depth() == 0);
  connection.Run(R"(UPDATE "User" SET "State" = 0)");
  {
    agiru::SessionCommand command(session, connection);
    bool sqlFailed = false;
    try {
      connection.Run("INSERT INTO command_values VALUES (1, 99)");
    } catch (const agiru::DatabaseError &) { sqlFailed = true; }
    CHECK_TRUE("fixture SQL enters a failed transaction",
               sqlFailed && connection.InFailedTransaction());
    bool refused = false;
    try {
      command.Keep();
    } catch (const agiru::Error &) { refused = true; }
    CHECK_TRUE("a failed SQL transaction cannot be reported as successful completion", refused);
  }
  CHECK_TRUE("an aborted command returns an idle connection", !connection.InTransaction());
  agiru::SessionCommand recovered(session, connection);
  recovered.Keep();
}

void ForeignCursorsCannotAbortAnotherUser(const std::string &dsn) {
  agiru::Session first(Identity(1));
  agiru::Session second(Identity(2));
  agiru::Connection reused(dsn);
  std::unique_ptr<agiru::detail::Cursor> stale;
  {
    agiru::SessionCommand command(first, reused);
    stale = std::make_unique<agiru::detail::Cursor>(
        reused, "SELECT 1", std::vector<std::optional<std::string>>{});
    CHECK_TRUE("a new cursor belongs to its command's active session", stale->Current());
    command.Keep();
  }
  agiru::SessionCommand command(second, reused);
  CHECK_TRUE("same connection and epoch do not grant another user's cursor ownership",
             !stale->Current());
  stale.reset();
  CHECK_TRUE("destroying a foreign stale cursor cannot abort the current user's transaction",
             !reused.InFailedTransaction());
  command.Keep();
}

void RollbackInvalidatesSameSessionCursors(const std::string &dsn) {
  agiru::Session session(Identity(1));
  agiru::Connection reused(dsn);
  std::unique_ptr<agiru::detail::Cursor> stale;
  {
    const agiru::SessionCommand command(session, reused);
    stale = std::make_unique<agiru::detail::Cursor>(
        reused, "SELECT 1", std::vector<std::optional<std::string>>{});
  }
  agiru::SessionCommand next(session, reused);
  CHECK_TRUE("host rollback invalidates a cursor even when session and connection are reused",
             !stale->Current());
  stale.reset();
  CHECK_TRUE("discarding a rolled-back cursor leaves the next command usable",
             !reused.InFailedTransaction());
  next.Keep();
}

void NestedAndDeferredFailuresRemainRecoverable(const std::string &dsn) {
  agiru::Session session(Identity(1));
  agiru::Connection connection(dsn);
  {
    agiru::SessionCommand command(session, connection);
    agiru::detail::Scope nested;
    bool refused = false;
    try {
      command.Keep();
    } catch (const agiru::SessionError &) { refused = true; }
    CHECK_TRUE("automatic completion cannot consume a live nested AL scope", refused);
    nested.Keep();
    command.Keep();
  }
  {
    agiru::SessionCommand command(session, connection);
    connection.Run("INSERT INTO command_deferred VALUES (1), (1)");
    bool refused = false;
    try {
      command.Keep();
    } catch (const agiru::DatabaseError &) { refused = true; }
    CHECK_TRUE("command completion checks deferred PostgreSQL constraints", refused);
  }
  CHECK_TRUE("a failed physical commit leaves no stale boundaries or transaction",
             session.Transaction().Depth() == 0 && !connection.InTransaction());
  agiru::SessionCommand recovered(session, connection);
  const auto count = connection.Execute("SELECT count(*) FROM command_deferred");
  CHECK_TRUE("deferred failure persists neither row",
             count.Value(0, 0) == std::optional<std::string_view>{"0"});
  recovered.Keep();
}

void BrokenConnectionsCannotReplaceTheOriginalError(const std::string &dsn) {
  agiru::Session session(Identity(1));
  agiru::Connection connection(dsn);
  {
    const agiru::SessionCommand command(session, connection);
    bool refused = false;
    try {
      const agiru::detail::Scope nested;
      connection.Run("ROLLBACK");
    } catch (const agiru::DatabaseError &) { refused = true; }
    CHECK_TRUE("normal scope cleanup failure propagates without terminating the process", refused);
  }
  std::string original;
  try {
    const agiru::SessionCommand command(session, connection);
    const agiru::detail::Scope nested;
    const agiru::detail::Cursor cursor(
        connection, "SELECT 1", std::vector<std::optional<std::string>>{});
    connection.Close();
    CHECK_TRUE("a closed lease invalidates its cursor without querying libpq", !cursor.Current());
    throw agiru::Error("original AL failure");
  } catch (const agiru::Error &error) { original = error.what(); }
  CHECK_TEXT("failed SQL cleanup retains the original AL error", original, "original AL failure");
  CHECK_TRUE("a broken command closes its lease and restores the worker",
             !connection.IsOpen() && !agiru::Session::HasCurrent() &&
                 session.Transaction().Depth() == 0);
}

}

int main() {
  return gate::Run("SessionCommand", [] {
    const gate::OwnedDatabase database("session_command");
    Seed(database.Dsn());
    IdleStateBorrowsNoConnection(database.Dsn());
    StateSurvivesCommandsAndWorkers(database.Dsn());
    DifferentUsersAndNestedParentsRemainPrivate(database.Dsn());
    AtomicFailureAndDurableCommit(database.Dsn());
    ConcurrentAndDirtyLeasesRefuse(database.Dsn());
    RevocationAndAbortedSQLRestoreState(database.Dsn());
    ForeignCursorsCannotAbortAnotherUser(database.Dsn());
    RollbackInvalidatesSameSessionCursors(database.Dsn());
    NestedAndDeferredFailuresRemainRecoverable(database.Dsn());
    BrokenConnectionsCannotReplaceTheOriginalError(database.Dsn());
    CHECK_TRUE("all command contexts leave the worker clear", !agiru::Session::HasCurrent());
  });
}

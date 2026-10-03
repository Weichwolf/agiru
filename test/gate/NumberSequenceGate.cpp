#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/NumberSequenceStorage.h"
#include "runtime/Session.h"
#include "type/BigInteger.h"
#include "type/Integer.h"
#include "type/NumberSequence.h"

#include "Check.h"

#include <algorithm>
#include <array>
#include <barrier>
#include <cstddef>
#include <exception>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

using agiru::BigInteger;
using agiru::NumberSequence;
using agiru::Session;

constexpr std::string_view kCompany = "number-sequence-gate";
constexpr std::string_view kConcurrent = "number-sequence-gate-concurrent";
constexpr std::size_t kWorkers = 3;
constexpr std::size_t kRounds = 100;
constexpr agiru::Integer kRangeSize = 100;
constexpr BigInteger kStep = 3;
constexpr BigInteger kLargeCount = 1000000000;

template <typename Body> std::string Refusal(Body body) {
  try {
    body();
  } catch (const agiru::Error &error) { return error.what(); }
  return {};
}

void Fresh(std::string_view name, BigInteger seed = 0, BigInteger increment = 1) {
  if (NumberSequence::Exists(name)) { NumberSequence::Delete(name); }
  NumberSequence::Insert(name, seed, increment);
}

void DefaultsAndOverloads() {
  constexpr std::string_view name = "number-sequence-gate-defaults";
  Fresh(name);
  CHECK_TRUE("Current exposes the initial seed", NumberSequence::Current(name) == 0);
  CHECK_TRUE("the first Next returns the seed, not seed plus step",
             NumberSequence::Next(name) == 0);
  CHECK_TRUE("the following Next increments", NumberSequence::Next(name) == 1);
  BigInteger increment = 0;
  CHECK_TRUE("the var overload returns the first value",
             NumberSequence::Range(name, 3, increment) == 2);
  CHECK_TRUE("the var overload writes the stored step", increment == 1);
  CHECK_TRUE("Current returns the last reserved value", NumberSequence::Current(name) == 4);
  NumberSequence::Restart(name);
  CHECK_TRUE("Restart defaults to zero", NumberSequence::Next(name) == 0);
  NumberSequence::Delete(name);
  CHECK_TRUE("Delete removes the logical identity", !NumberSequence::Exists(name));
  CHECK_TRUE("Next over a missing sequence refuses",
             !Refusal([&] { NumberSequence::Next(name); }).empty());
  CHECK_TRUE("Delete over a missing sequence refuses",
             !Refusal([&] { NumberSequence::Delete(name); }).empty());
  CHECK_TRUE("zero increment is not silently changed to one",
             !Refusal([&] { NumberSequence::Insert(name, 0, 0); }).empty());
  CHECK_TRUE("refused Insert leaves no registry entry", !NumberSequence::Exists(name));
}

void QuotedAndLongNames(bool cleanupOnly = false) {
  const std::array<std::string, 4> names{"q'\"; DROP SCHEMA agiru_platform; --",
                                         "number-sequence-gate-Ä東京",
                                         "number-sequence-gate-" + std::string(100, 'x') + "A",
                                         "number-sequence-gate-" + std::string(100, 'x') + "B"};
  for (const auto &name : names) {
    if (NumberSequence::Exists(name)) { NumberSequence::Delete(name); }
  }
  if (cleanupOnly) { return; }
  for (const auto &name : names) { NumberSequence::Insert(name, 10); }
  for (const auto &name : names) {
    CHECK_TRUE("arbitrary AL names are bound data, not SQL syntax",
               NumberSequence::Next(name) == 10);
    NumberSequence::Delete(name);
  }
}

void NamesAndCompanyIdentity() {
  QuotedAndLongNames();
  const std::string nullName = std::string("number-sequence-gate-null") + '\0' + "suffix";
  CHECK_TRUE("null bytes cannot truncate and alias a bound name",
             !Refusal([&] { NumberSequence::Insert(nullName); }).empty());
  constexpr std::string_view name = "number-sequence-gate-company";
  Fresh(name, 1);
  NumberSequence::Insert(name, 10, 1, false);
  {
    Session other(AGIRU_TEST_DSN);
    other.CompanyName("number-sequence-other-company");
    Fresh(name, kStep);
    CHECK_TRUE("another company has a distinct same-name sequence",
               NumberSequence::Next(name) == kStep);
    CHECK_TRUE("database-wide sequence is shared across companies",
               NumberSequence::Next(name, false) == 10);
    NumberSequence::Delete(name);
  }
  CHECK_TRUE("default company scope was not redirected", NumberSequence::Next(name) == 1);
  CHECK_TRUE("database-wide state is visible in the first company",
             NumberSequence::Next(name, false) == 11);
  NumberSequence::Delete(name);
  NumberSequence::Delete(name, false);
  Session::Current().CompanyName("number-sequence-A$B");
  Fresh("C");
  Session::Current().CompanyName("number-sequence-A");
  Fresh("B$C", 10);
  CHECK_TRUE("company/name delimiter ambiguity cannot alias identities",
             NumberSequence::Next("B$C") == 10);
  NumberSequence::Delete("B$C");
  Session::Current().CompanyName("number-sequence-A$B");
  CHECK_TRUE("the other framed identity is untouched", NumberSequence::Next("C") == 0);
  NumberSequence::Delete("C");
  Session::Current().CompanyName(kCompany);
}

void SignedStepsAndBounds() {
  constexpr std::string_view name = "number-sequence-gate-bounds";
  Fresh(name, 10, kStep);
  BigInteger increment = 0;
  CHECK_TRUE("Range returns the seed before first use",
             NumberSequence::Range(name, 4, increment, true) == 10);
  CHECK_TRUE("Range reports a non-unit increment", increment == kStep);
  CHECK_TRUE("Next skips the entire non-unit reservation", NumberSequence::Next(name) == 22);
  CHECK_TRUE("a duplicate cannot overwrite an existing sequence",
             !Refusal([&] { NumberSequence::Insert(name); }).empty());
  NumberSequence::Restart(name, -10);
  CHECK_TRUE("Restart may move below the original seed", NumberSequence::Next(name) == -10);
  NumberSequence::Delete(name);
  Fresh(name, -10, -kStep);
  CHECK_TRUE("descending range starts at its seed",
             NumberSequence::Range(name, 3, increment) == -10);
  CHECK_TRUE("the var step retains its sign", increment == -kStep);
  CHECK_TRUE("descending allocation has no reused values", NumberSequence::Next(name) == -19);
  increment = kStep;
  CHECK_TRUE("zero Count refuses",
             !Refusal([&] { NumberSequence::Range(name, 0, increment); }).empty());
  CHECK_TRUE("negative Count refuses", !Refusal([&] { NumberSequence::Range(name, -1); }).empty());
  CHECK_TRUE("var output is unchanged on failure", increment == kStep);
  CHECK_TRUE("invalid Count consumes no values", NumberSequence::Next(name) == -22);
  NumberSequence::Delete(name);
  Fresh(name, std::numeric_limits<BigInteger>::max() - 1);
  CHECK_TRUE("an overflowing range refuses without partial consumption",
             !Refusal([&] { NumberSequence::Range(name, 3); }).empty());
  CHECK_TRUE("the refused reservation retains the first value",
             NumberSequence::Next(name) == std::numeric_limits<BigInteger>::max() - 1);
  CHECK_TRUE("the last representable value works",
             NumberSequence::Next(name) == std::numeric_limits<BigInteger>::max());
  CHECK_TRUE("Next cannot wrap at the upper bound",
             !Refusal([&] { NumberSequence::Next(name); }).empty());
  NumberSequence::Delete(name);
  Fresh(name, std::numeric_limits<BigInteger>::min(), -1);
  CHECK_TRUE("the lower representable bound works",
             NumberSequence::Next(name) == std::numeric_limits<BigInteger>::min());
  CHECK_TRUE("descending Next cannot wrap", !Refusal([&] { NumberSequence::Next(name); }).empty());
  NumberSequence::Delete(name);
}

void TransactionBoundaries() {
  constexpr std::string_view name = "number-sequence-gate-transaction";
  Fresh(name);
  const auto &database = Session::Current().Database();
  database.Run("BEGIN");
  CHECK_TRUE("reservation inside a transaction returns its first value",
             NumberSequence::Range(name, 100) == 0);
  {
    Session other(AGIRU_TEST_DSN);
    other.CompanyName(kCompany);
    other.Database().Run("SET statement_timeout = '2s'");
    CHECK_TRUE("other sessions reserve without waiting for the posting transaction",
               NumberSequence::Next(name) == 100);
  }
  database.Run("ROLLBACK");
  CHECK_TRUE("caller rollback does not reissue consumed values", NumberSequence::Next(name) == 101);
  database.Run("BEGIN");
  NumberSequence::Restart(name, 10);
  CHECK_TRUE("the caller sees its transactional restart", NumberSequence::Next(name) == 10);
  database.Run("ROLLBACK");
  CHECK_TRUE("rolled-back restart restores the original sequence",
             NumberSequence::Next(name) == 102);
  database.Run("BEGIN");
  NumberSequence::Delete(name);
  CHECK_TRUE("Delete is visible inside its transaction", !NumberSequence::Exists(name));
  database.Run("ROLLBACK");
  CHECK_TRUE("rolled-back Delete restores sequence state", NumberSequence::Next(name) == 103);
  NumberSequence::Delete(name);
  database.Run("BEGIN");
  NumberSequence::Insert(name);
  NumberSequence::Next(name);
  database.Run("ROLLBACK");
  CHECK_TRUE("rolled-back creation leaves no sequence", !NumberSequence::Exists(name));
}

void ExistsDoesNotLockOutCreation() {
  constexpr std::string_view name = "number-sequence-gate-exists";
  if (NumberSequence::Exists(name)) { NumberSequence::Delete(name); }
  const auto &database = Session::Current().Database();
  database.Run("BEGIN");
  CHECK_TRUE("Exists observes an absent identity without reserving it",
             !NumberSequence::Exists(name));
  {
    Session other(AGIRU_TEST_DSN);
    other.CompanyName(kCompany);
    other.Database().Run("SET statement_timeout = '2s'");
    NumberSequence::Insert(name);
  }
  CHECK_TRUE("an earlier Exists does not block another transaction's Insert",
             NumberSequence::Exists(name));
  database.Run("ROLLBACK");
  CHECK_TRUE("the other transaction's sequence survives the observer's rollback",
             NumberSequence::Next(name) == 0);
  NumberSequence::Delete(name);
}

void MissingIdentityKeepsTransactionUsable() {
  constexpr std::string_view name = "number-sequence-gate-missing-transaction";
  if (NumberSequence::Exists(name)) { NumberSequence::Delete(name); }
  const auto &database = Session::Current().Database();
  database.Run("BEGIN");
  database.Run("CREATE TEMP TABLE sequence_gate_prior_write (value integer)");
  database.Run("INSERT INTO sequence_gate_prior_write VALUES (1)");
  for (const auto operation : {"current", "next", "delete", "restart"}) {
    const bool success = agiru::Tried([&] {
      if (operation == std::string_view{"current"}) { NumberSequence::Current(name); }
      if (operation == std::string_view{"next"}) { NumberSequence::Next(name); }
      if (operation == std::string_view{"delete"}) { NumberSequence::Delete(name); }
      if (operation == std::string_view{"restart"}) { NumberSequence::Restart(name); }
    });
    CHECK_TRUE("missing sequence remains an AL error", !success);
    CHECK_TRUE("caught missing sequence leaves PostgreSQL usable", !database.InFailedTransaction());
    CHECK_TRUE("missing sequence releases its session allocation lock",
               database.Execute("SELECT count(*) FROM pg_catalog.pg_locks "
                                "WHERE locktype = 'advisory' AND pid = pg_catalog.pg_backend_pid() "
                                "AND (objid::bigint & 1) = 1")
                       .Value(0, 0) == "0");
    CHECK_TRUE("the original missing identity remains in GetLastErrorText",
               agiru::GetLastErrorText() ==
                   "the number sequence " + std::string(name) + " does not exist");
  }
  CHECK_TRUE("TryFunction preserves writes made before the missing-sequence error",
             database.Execute("SELECT value FROM sequence_gate_prior_write").Value(0, 0) == "1");
  CHECK_TRUE("Exists remains callable after a caught Current error", !NumberSequence::Exists(name));
  NumberSequence::Insert(name, 10);
  CHECK_TRUE("original AL recovery can create and consume its missing sequence",
             NumberSequence::Next(name) == 10);
  database.Run("ROLLBACK");
  CHECK_TRUE("recovery does not commit the caller's writes", !NumberSequence::Exists(name));
}

struct Reservation {
  BigInteger first;
  agiru::Integer count;
};

struct WorkerResult {
  std::vector<Reservation> reservations;
  std::string error;
};

void ConcurrentReservations() {
  Fresh(kConcurrent, 0, kStep);
  std::array<WorkerResult, kWorkers> results;
  std::barrier ready(static_cast<std::ptrdiff_t>(kWorkers));
  std::array<std::thread, kWorkers> workers;
  for (std::size_t worker = 0; worker < kWorkers; ++worker) {
    workers[worker] = std::thread([&, worker] {
      bool arrived = false;
      try {
        Session session(AGIRU_TEST_DSN);
        session.CompanyName(kCompany);
        session.Database().Run("SET statement_timeout = '5s'");
        arrived = true;
        ready.arrive_and_wait();
        for (std::size_t round = 0; round < kRounds; ++round) {
          const agiru::Integer count = worker == 2 ? 1 : kRangeSize;
          const BigInteger first = worker == 2 ? NumberSequence::Next(kConcurrent)
                                               : NumberSequence::Range(kConcurrent, count);
          results[worker].reservations.push_back({first, count});
        }
      } catch (const std::exception &error) {
        results[worker].error = error.what();
        if (!arrived) { ready.arrive_and_drop(); }
      }
    });
  }
  for (auto &worker : workers) { worker.join(); }
  std::vector<Reservation> reservations;
  for (const auto &result : results) {
    CHECK_SILENT("concurrent session completed without error", result.error);
    CHECK_TRUE("no reservation disappeared from the denominator",
               result.reservations.size() == kRounds);
    reservations.insert(reservations.end(), result.reservations.begin(), result.reservations.end());
  }
  std::ranges::sort(reservations, {}, &Reservation::first);
  BigInteger expected = 0;
  for (const auto &reservation : reservations) {
    CHECK_TRUE("two Range sessions and Next never reserve overlapping values",
               reservation.first == expected);
    expected = reservation.first + reservation.count * kStep;
  }
  CHECK_TRUE("the next value follows all three sessions",
             NumberSequence::Next(kConcurrent) == expected);
  NumberSequence::Delete(kConcurrent);
}

void ErrorCleanupAndStoragePolicy() {
  constexpr std::string_view name = "number-sequence-gate-storage";
  Fresh(name);
  const auto &database = Session::Current().Database();
  const agiru::Result outstanding =
      database.Execute("SELECT count(*) FROM pg_catalog.pg_locks WHERE locktype = 'advisory' AND "
                       "pid = pg_catalog.pg_backend_pid()");
  CHECK_TRUE("no session advisory lock survives successful operations",
             outstanding.Value(0, 0) == "0");
  CHECK_TRUE("invalid reservation reports an error",
             !Refusal([&] { NumberSequence::Range(name, 0); }).empty());
  const agiru::Result afterError =
      database.Execute("SELECT count(*) FROM pg_catalog.pg_locks WHERE locktype = 'advisory' AND "
                       "pid = pg_catalog.pg_backend_pid()");
  CHECK_TRUE("no advisory lock leaks after errors", afterError.Value(0, 0) == "0");
  const agiru::Result relation = database.Execute(
      "SELECT 'agiru_platform.sequence_' || id FROM agiru_platform.number_sequences_v1 "
      "WHERE name = 'number-sequence-gate-storage' AND company = 'number-sequence-gate'");
  const auto physicalValue = relation.Value(0, 0);
  if (!physicalValue) { throw agiru::Error("the gate's sequence storage is missing"); }
  const std::string physical(*physicalValue);
  database.Run("ALTER SEQUENCE " + physical + " CACHE 10");
  CHECK_TRUE("incompatible cached sequences refuse instead of reusing cached numbers",
             !Refusal([&] { NumberSequence::Range(name, 3); }).empty());
  database.Run("ALTER SEQUENCE " + physical + " CACHE 1 CYCLE");
  CHECK_TRUE("cyclic storage is explicitly unsupported",
             !Refusal([&] { NumberSequence::Next(name); }).empty());
  database.Run("ALTER SEQUENCE " + physical + " NO CYCLE");
  CHECK_TRUE("refused storage checks consumed nothing", NumberSequence::Next(name) == 0);
  agiru::ProvisionNumberSequences(database);
  CHECK_TRUE("reprovisioning preserves existing consumption and registry identity",
             NumberSequence::Next(name) == 1);
  database.Run("CREATE SCHEMA number_sequence_gate_shadow");
  const std::string shadow =
      "number_sequence_gate_shadow." + physical.substr(physical.find('.') + 1);
  database.Run("CREATE SEQUENCE " + shadow + " START WITH 100");
  database.Run("SET search_path = number_sequence_gate_shadow, public");
  CHECK_TRUE("a search-path shadow cannot redirect the qualified sequence identity",
             NumberSequence::Next(name) == 2);
  database.Run("RESET search_path");
  database.Run("DROP SEQUENCE " + shadow);
  database.Run("DROP SCHEMA number_sequence_gate_shadow");
  NumberSequence::Delete(name);
  database.Run("CREATE SEQUENCE public.\"NumSeq$number-sequence-gate-legacy\"");
  const std::string migration = Refusal([&] { agiru::ProvisionNumberSequences(database); });
  database.Run("DROP SEQUENCE public.\"NumSeq$number-sequence-gate-legacy\"");
  CHECK_TRUE("provisioning never abandons legacy sequence identities",
             migration.find("migration") != std::string::npos);
}

void TransactionalLifecycleRaces() {
  constexpr std::string_view name = "number-sequence-gate-lifecycle";
  Fresh(name);
  const auto &database = Session::Current().Database();
  database.Run("BEGIN");
  NumberSequence::Restart(name, 100);
  std::string refused;
  {
    Session other(AGIRU_TEST_DSN);
    other.CompanyName(kCompany);
    other.Database().Run("SET statement_timeout = '100ms'");
    refused = Refusal([&] { NumberSequence::Next(name); });
  }
  CHECK_TRUE("other sessions cannot observe an uncommitted restart",
             refused.find("statement timeout") != std::string::npos);
  database.Run("COMMIT");
  CHECK_TRUE("committed restart is visible to subsequent reservations",
             NumberSequence::Next(name) == 100);
  database.Run("BEGIN");
  NumberSequence::Delete(name);
  {
    Session other(AGIRU_TEST_DSN);
    other.CompanyName(kCompany);
    other.Database().Run("SET statement_timeout = '100ms'");
    refused = Refusal([&] { NumberSequence::Range(name, 3); });
  }
  CHECK_TRUE("other sessions wait for a pending Delete rather than use orphaned storage",
             refused.find("statement timeout") != std::string::npos);
  database.Run("COMMIT");
  CHECK_TRUE("reservation after committed Delete reports the missing identity",
             Refusal([&] { NumberSequence::Next(name); }).find("does not exist") !=
                 std::string::npos);
  Fresh(name);
  database.Run("BEGIN");
  NumberSequence::Next(name);
  {
    Session other(AGIRU_TEST_DSN);
    other.CompanyName(kCompany);
    other.Database().Run("SET statement_timeout = '100ms'");
    CHECK_TRUE("a reservation's shared lifetime guard excludes concurrent Restart",
               Refusal([&] { NumberSequence::Restart(name); }).find("statement timeout") !=
                   std::string::npos);
    CHECK_TRUE("a pending posting transaction still permits other reservations",
               NumberSequence::Range(name, 3) == 1);
  }
  CHECK_TRUE("the original posting session can reserve again without a DDL lock cycle",
             NumberSequence::Next(name) == 4);
  database.Run("ROLLBACK");
  CHECK_TRUE("all sessions' consumed values survive that rollback",
             NumberSequence::Next(name) == 5);
  NumberSequence::Delete(name);
}

void RoundTripProbe() {
  constexpr std::string_view name = "number-sequence-gate-roundtrips";
  Fresh(name);
  CHECK_TRUE("a one-value reservation works", NumberSequence::Range(name, 1) == 0);
  CHECK_TRUE("a billion-value reservation works without a per-value loop",
             NumberSequence::Range(name, static_cast<agiru::Integer>(kLargeCount)) == 1);
  CHECK_TRUE("Current reflects the whole billion-value reservation",
             NumberSequence::Current(name) == kLargeCount);
  NumberSequence::Delete(name);
}

void CancellationCleanup() {
  constexpr std::string_view name = "number-sequence-gate-cancellation";
  Fresh(name);
  NumberSequence::Next(name);
  NumberSequence::Restart(name);
  const auto &database = Session::Current().Database();
  database.Run("SET statement_timeout = '10ms'");
  const std::string refused = Refusal([&] { NumberSequence::Range(name, 3); });
  database.Run("RESET statement_timeout");
  CHECK_TRUE("the controlled cancellation occurs inside the allocator after its lock",
             refused.find("pg_sleep") != std::string::npos);
  const agiru::Result locks =
      database.Execute("SELECT count(*) FROM pg_catalog.pg_locks WHERE locktype = 'advisory' AND "
                       "pid = pg_catalog.pg_backend_pid()");
  CHECK_TRUE("query cancellation releases session allocation locks", locks.Value(0, 0) == "0");
  CHECK_TRUE("cancellation before publication consumes no values", NumberSequence::Next(name) == 0);
  NumberSequence::Delete(name);
}

}

namespace {

bool RunMode(std::string_view mode) {
  if (mode == "--missing-transaction") {
    MissingIdentityKeepsTransactionUsable();
    return true;
  }
  if (mode == "--cancellation") {
    CancellationCleanup();
    return true;
  }
  if (mode == "--roundtrips") {
    RoundTripProbe();
    return true;
  }
  if (mode == "--concurrency") {
    ConcurrentReservations();
    return true;
  }
  if (mode == "--process-setup") {
    Fresh(kConcurrent, 0, kStep);
    return true;
  }
  if (mode == "--process-worker") {
    for (std::size_t round = 0; round < kRounds; ++round) {
      std::cout << "RESERVATION " << NumberSequence::Range(kConcurrent, kRangeSize) << '\n';
    }
    return true;
  }
  if (mode == "--process-cleanup") {
    if (NumberSequence::Exists(kConcurrent)) { NumberSequence::Delete(kConcurrent); }
    return true;
  }
  if (mode == "--names") {
    QuotedAndLongNames();
    return true;
  }
  if (mode == "--names-cleanup") {
    QuotedAndLongNames(true);
    return true;
  }
  return false;
}

}

int main(int argc, char **argv) {
  return gate::Run("NumberSequence", [&] {
    Session session(AGIRU_TEST_DSN);
    session.CompanyName(kCompany);
    const std::string_view mode = argc == 2 ? std::string_view(argv[1]) : std::string_view{};
    if (mode != "--process-worker" && mode != "--process-cleanup" && mode != "--names-cleanup") {
      agiru::ProvisionNumberSequences(session.Database());
    }
    if (RunMode(mode)) { return; }
    DefaultsAndOverloads();
    NamesAndCompanyIdentity();
    SignedStepsAndBounds();
    TransactionBoundaries();
    MissingIdentityKeepsTransactionUsable();
    ExistsDoesNotLockOutCreation();
    ConcurrentReservations();
    ErrorCleanupAndStoragePolicy();
    TransactionalLifecycleRaces();
  });
}

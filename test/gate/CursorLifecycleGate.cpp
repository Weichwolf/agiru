#include "runtime/Database.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/TableDefinition.h"
#include "runtime/Transaction.h"
#include "type/Decimal.h"

#include "Check.h"
#include "Cursor.h"
#include "ResourceCost.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <print>
#include <string>
#include <string_view>

namespace {

using Cost = agiru::Projects::Resources::Pricing::ResourceCost_Table;
constexpr std::size_t kRows = agiru::detail::kFetchBlock * 2;
constexpr std::size_t kFirstCode = 1000;
constexpr std::string_view kOriginalCost = "10";
constexpr std::string_view kChangedCost = "20";

std::string Code(std::size_t index) {
  return std::to_string(kFirstCode + index);
}

void Fill() {
  const auto &table = agiru::TableDefinition<Cost>();
  agiru::DropTable(agiru::Session::Current().Database(), table);
  agiru::CreateTable(agiru::Session::Current().Database(), table);
  Cost row;
  for (std::size_t index = 0; index < kRows; ++index) {
    row.Init();
    row.Code = Code(index);
    row.WorkTypeCode = "hours";
    row.DirectUnitCost = agiru::Decimal::FromInvariantString(kOriginalCost);
    row.Insert();
  }
}

template <bool Reflected> class Reader {
public:
  explicit Reader(Cost &row) : row_(row) {
    if constexpr (Reflected) { reference_.GetTable(row_); }
  }

  bool FindSet() {
    if constexpr (Reflected) { return reference_.FindSet(); }
    return row_.FindSet();
  }

  std::int32_t Next(std::int32_t steps) {
    if constexpr (Reflected) { return reference_.Next(steps); }
    return row_.Next(steps);
  }

  void Check(std::size_t index, std::string_view cost) {
    if constexpr (Reflected) { reference_.SetTable(row_); }
    CHECK_TEXT("navigation preserves the exact primary identity", row_.Code.Value(), Code(index));
    CHECK_TEXT(
        "the composite key survives transaction recovery", row_.WorkTypeCode.Value(), "HOURS");
    CHECK_TRUE("navigation does not serve an obsolete buffered value",
               row_.DirectUnitCost == agiru::Decimal::FromInvariantString(cost));
  }

private:
  Cost &row_;
  agiru::RecordRef reference_;
};

void TraceWalk(bool begin, std::int32_t steps, bool trace) {
  if (trace) { std::println(stderr, "cursor-walk: {} {}", begin ? "begin" : "end", steps); }
}

template <bool Reflected> void TransactionRecovery(bool rollback, std::int32_t steps, bool trace) {
  const agiru::Session session(AGIRU_TEST_DSN);
  const auto &connection = agiru::Session::Current().Database();
  auto &transaction = agiru::Session::Current().Transaction();
  Fill();
  const auto requested = static_cast<std::size_t>(steps);
  const std::size_t target = requested < kRows ? requested : kRows - 1;
  Cost writer;
  CHECK_TRUE("the future row exists", writer.Get(writer.Type, Code(target), "hours"));
  const std::size_t depth = rollback ? transaction.Open(connection) : 0;
  if (rollback) {
    writer.DirectUnitCost = agiru::Decimal::FromInvariantString(kChangedCost);
    writer.Modify();
  }
  Cost row;
  CHECK_TRUE("the source key is selected", row.SetCurrentKey(row.Code));
  Reader<Reflected> reader(row);
  CHECK_TRUE("the original result set opens", reader.FindSet());
  reader.Check(0, kOriginalCost);
  const auto before = transaction.CursorEpoch();
  if (rollback) {
    transaction.Rollback(connection, depth);
  } else {
    transaction.Commit(connection);
    writer.DirectUnitCost = agiru::Decimal::FromInvariantString(kChangedCost);
    writer.Modify();
  }
  CHECK_TRUE("the transaction generation changed", transaction.CursorEpoch() != before);
  CHECK_TRUE("Next zero does not move after a transaction boundary", reader.Next(0) == 0);
  reader.Check(0, kOriginalCost);
  std::string error;
  TraceWalk(true, steps, trace);
  try {
    CHECK_TRUE("the resumed result reports its actual movement",
               reader.Next(steps) == static_cast<std::int32_t>(target));
  } catch (const agiru::DatabaseError &refused) { error = refused.what(); }
  TraceWalk(false, steps, trace);
  CHECK_SILENT("Next must not fetch a destroyed portal", error);
  if (!error.empty()) { return; }
  reader.Check(target, rollback ? kOriginalCost : kChangedCost);
  const auto reverse = -static_cast<std::int32_t>(target + 1);
  TraceWalk(true, reverse, trace);
  CHECK_TRUE("reverse overshoot reports its actual signed movement",
             reader.Next(reverse) == -static_cast<std::int32_t>(target));
  TraceWalk(false, reverse, trace);
  reader.Check(0, kOriginalCost);
  CHECK_TRUE("reversal at the endpoint preserves exhaustion", reader.Next(-1) == 0);
  CHECK_TRUE("the endpoint can resume in the opposite direction", reader.Next(1) == 1);
  reader.Check(1, !rollback && target == 1 ? kChangedCost : kOriginalCost);
}

std::size_t Portals(const agiru::Connection &connection) {
  const auto result =
      connection.Execute("SELECT count(*) FROM pg_catalog.pg_cursors WHERE name LIKE 'agiru_%'");
  const auto value = result.Value(0, 0);
  CHECK_TRUE("the server reports its cursor population", value.has_value());
  return value.has_value() ? std::stoul(std::string(*value)) : 0;
}

void ReleasedBoundaryClosesItsCursor() {
  const agiru::Session session(AGIRU_TEST_DSN);
  const auto &connection = agiru::Session::Current().Database();
  auto &transaction = agiru::Session::Current().Transaction();
  const auto depth = transaction.Open(connection);
  {
    agiru::detail::Cursor cursor(connection, "SELECT 1", {});
    CHECK_TRUE("the scope owns one server cursor", Portals(connection) == 1);
    transaction.Release(connection, depth);
    CHECK_TRUE("a released boundary retains a usable cursor", cursor.Step());
  }
  CHECK_TRUE("destruction after release frees the server cursor", Portals(connection) == 0);
}

void ChildRollbackClosesTheSurvivingCursor() {
  const agiru::Session session(AGIRU_TEST_DSN);
  const auto &connection = agiru::Session::Current().Database();
  auto &transaction = agiru::Session::Current().Transaction();
  const auto outer = transaction.Open(connection);
  {
    const agiru::detail::Cursor cursor(connection, "SELECT 1", {});
    const auto child = transaction.Open(connection);
    transaction.Rollback(connection, child);
    CHECK_TRUE("a cursor born before the child survives its rollback", Portals(connection) == 1);
  }
  CHECK_TRUE("an epoch change does not leak a surviving portal", Portals(connection) == 0);
  transaction.Release(connection, outer);
}

void DestroyedCursorDoesNotPoisonTheConnection() {
  const agiru::Session session(AGIRU_TEST_DSN);
  const auto &connection = agiru::Session::Current().Database();
  auto &transaction = agiru::Session::Current().Transaction();
  const auto depth = transaction.Open(connection);
  {
    const agiru::detail::Cursor cursor(connection, "SELECT 1", {});
    transaction.Rollback(connection, depth);
    CHECK_TRUE("the rolled-back portal is absent", Portals(connection) == 0);
  }
  CHECK_TRUE("cleanup of an absent portal keeps the transaction usable",
             !connection.InFailedTransaction());
  CHECK_TRUE("the connection still executes SQL", connection.Execute("SELECT 1").Rows() == 1);
}

}

int main(int count, char **arguments) {
  const bool trace = count == 2 && std::string_view{arguments[1]} == "--trace-walks";
  if (count != 1 && !trace) { return 2; }
  return gate::Run("CursorLifecycle", [trace] {
    constexpr std::array<std::int32_t, 3> steps{
        1,
        static_cast<std::int32_t>(agiru::detail::kFetchBlock),
        static_cast<std::int32_t>(kRows + 1)};
    for (const auto count : steps) {
      TransactionRecovery<false>(false, count, trace);
      TransactionRecovery<false>(true, count, trace);
      TransactionRecovery<true>(false, count, trace);
      TransactionRecovery<true>(true, count, trace);
    }
    ReleasedBoundaryClosesItsCursor();
    ChildRollbackClosesTheSurvivingCursor();
    DestroyedCursorDoesNotPoisonTheConnection();
  });
}

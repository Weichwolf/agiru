#include "meta/Ids.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/RecordRefresh.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/TableDefinition.h"
#include "runtime/Transaction.h"
#include "type/Decimal.h"

#include "Check.h"
#include "OwnedDatabase.h"
#include "RecordChanges.h"
#include "ResourceCost.h"
#include "SessionState.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace {

using Cost = agiru::Projects::Resources::Pricing::ResourceCost_Table;
constexpr agiru::TableId kOther{50198};
constexpr std::size_t kFirstCode = 1000;

void Observations(const std::string &dsn) {
  const agiru::Session outer(dsn);
  {
    const agiru::detail::RecordRead read(Cost::kId);
    const agiru::detail::RecordRead locked(Cost::kId, true);
    const agiru::detail::RecordRead other(kOther);
    agiru::SelectLatestVersion(kOther.Value());
    CHECK_TRUE("a selected refresh leaves another table current", read.Current());
    CHECK_TRUE("a selected refresh invalidates its non-locked readers", !other.Current());
    CHECK_TRUE("cache lock metadata survives a selected refresh", locked.Current());
    {
      const agiru::Session nested(dsn);
      agiru::SelectLatestVersion();
    }
    CHECK_TRUE("a nested session cannot refresh the outer session", read.Current());
    agiru::SelectLatestVersion(0);
    CHECK_TRUE("table zero is not an all-table refresh alias", read.Current());
    agiru::SelectLatestVersion();
    CHECK_TRUE("an all-table refresh invalidates non-locked readers", !read.Current());
    CHECK_TRUE("cache lock metadata survives an all-table refresh", locked.Current());
    const agiru::detail::RecordRead fresh(Cost::kId);
    CHECK_TRUE("a post-refresh observation starts current", fresh.Current());
    agiru::detail::RecordWritten(outer.Database(), Cost::kId);
    CHECK_TRUE("own writes still invalidate cache-locked observations", !locked.Current());
    CHECK_TRUE("own writes still invalidate fresh observations", !fresh.Current());
  }
  CHECK_TRUE("refresh histories retire with their last readers",
             agiru::detail::SessionState::Current().recordChanges->ActiveTables() == 0);
  for (std::int32_t id = 1; id < 100; ++id) { agiru::SelectLatestVersion(id); }
  CHECK_TRUE("refreshing unobserved tables does not allocate historical counters",
             agiru::detail::SessionState::Current().recordChanges->ActiveTables() == 0);
}

void Fill(const std::string &dsn) {
  const agiru::Session session(dsn);
  const auto &table = agiru::TableDefinition<Cost>();
  agiru::DropTable(session.Database(), table);
  agiru::CreateTable(session.Database(), table);
  Cost row;
  for (std::size_t index = 0; index < 4; ++index) {
    row.Init();
    row.Code = std::to_string(kFirstCode + index);
    row.WorkTypeCode = "HOURS";
    row.DirectUnitCost = 10;
    row.Insert();
  }
  agiru::Session::Current().Transaction().Commit(session.Database());
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

  std::int32_t Next(std::int32_t steps = 1) {
    if constexpr (Reflected) { return reference_.Next(steps); }
    return row_.Next(steps);
  }

  void Copy() {
    if constexpr (Reflected) { reference_.SetTable(row_); }
  }

private:
  Cost &row_;
  agiru::RecordRef reference_;
};

template <bool Reflected> void FreshNavigation(const std::string &dsn, bool all, bool descending) {
  Fill(dsn);
  const agiru::Session session(dsn);
  const agiru::Connection writer(dsn);
  Cost row;
  CHECK_TRUE("refresh uses the selected AL key", row.SetCurrentKey(row.Code));
  row.Ascending(!descending);
  row.SetRange(row.Code, "1000", "1003");
  row.SetRange(row.WorkTypeCode, "HOURS");
  Cost stale;
  stale.Copy(row);
  Reader<Reflected> reader(row);
  Reader<Reflected> control(stale);
  CHECK_TRUE("the typed or reflected read buffers its filtered result", reader.FindSet());
  CHECK_TRUE("an independent cached control opens before the external write", control.FindSet());
  reader.Copy();
  const auto original = row.Code.Value();
  const std::string target = descending ? "1002" : "1001";
  const auto written = writer.Execute("UPDATE \"Resource Cost\" SET \"Direct Unit Cost\"=20 "
                                      "WHERE \"Code\"='" +
                                      target + "'");
  CHECK_TRUE("an independent committed connection changes exactly one future row",
             written.Affected() == 1);
  CHECK_TRUE("without refresh the original SQL cursor can still move", control.Next() == 1);
  control.Copy();
  CHECK_TRUE("the baseline really contains a stale buffered value", stale.DirectUnitCost == 10);
  const auto epoch = agiru::Session::Current().Transaction().CursorEpoch();
  if (all) {
    agiru::SelectLatestVersion();
  } else {
    agiru::SelectLatestVersion(Cost::kId.Value());
  }
  CHECK_TRUE("refresh never changes the transaction cursor epoch",
             agiru::Session::Current().Transaction().CursorEpoch() == epoch);
  CHECK_TRUE("refresh does not move the loaded record", reader.Next(0) == 0);
  reader.Copy();
  CHECK_TEXT("refresh preserves the loaded primary key", row.Code.Value(), original);
  CHECK_TRUE("refresh preserves the loaded non-key values", row.DirectUnitCost == 10);
  CHECK_TRUE("fresh navigation resumes from the current SQL key", reader.Next() == 1);
  reader.Copy();
  CHECK_TEXT("fresh navigation preserves ascending or descending order", row.Code.Value(), target);
  CHECK_TRUE("fresh navigation sees the independently committed amount", row.DirectUnitCost == 20);
  CHECK_TRUE("fresh navigation retains the range endpoint", reader.Next(10) == 2);
  CHECK_TRUE("fresh navigation cannot escape the filter", reader.Next() == 0);
}

void PendingWrite(const std::string &dsn) {
  const agiru::Session session(dsn);
  const auto &connection = session.Database();
  const agiru::Connection independent(dsn);
  connection.Run("CREATE TABLE refresh_pending (id integer PRIMARY KEY)");
  auto &boundary = agiru::Session::Current().Transaction();
  const auto depth = boundary.Open(connection);
  boundary.Write(connection);
  connection.Run("INSERT INTO refresh_pending VALUES(1)");
  const auto epoch = boundary.CursorEpoch();
  agiru::SelectLatestVersion();
  agiru::SelectLatestVersion(Cost::kId.Value());
  CHECK_TRUE("refresh keeps the active write phase and rollback depth",
             boundary.IsWriting() && boundary.Depth() == depth && boundary.CursorEpoch() == epoch);
  CHECK_TRUE("refresh cannot implicitly commit a pending write",
             independent.Execute("SELECT id FROM refresh_pending").Rows() == 0);
  CHECK_TRUE("refresh does not discard the caller's own pending write",
             connection.Execute("SELECT id FROM refresh_pending").Rows() == 1);
  boundary.Rollback(connection, depth);
  CHECK_TRUE("refresh preserves the caller's rollback boundary",
             independent.Execute("SELECT id FROM refresh_pending").Rows() == 0);
  const auto later = boundary.Open(connection);
  boundary.Write(connection);
  connection.Run("INSERT INTO refresh_pending VALUES(2)");
  boundary.Commit(connection);
  connection.Run("INSERT INTO refresh_pending VALUES(3)");
  agiru::SelectLatestVersion();
  boundary.Rollback(connection, later);
  CHECK_TRUE("refresh preserves prior explicit Commit and rolls back only later work",
             independent.Execute("SELECT id FROM refresh_pending WHERE id=2").Rows() == 1 &&
                 independent.Execute("SELECT id FROM refresh_pending WHERE id=3").Rows() == 0);
}

void FrozenSnapshot(const std::string &dsn, std::string_view level) {
  const agiru::Session session(dsn);
  session.Database().Run("BEGIN ISOLATION LEVEL " + std::string(level));
  const agiru::detail::RecordRead read(Cost::kId);
  for (const bool all : {false, true}) {
    bool refused = false;
    try {
      if (all) {
        agiru::SelectLatestVersion();
      } else {
        agiru::SelectLatestVersion(Cost::kId.Value());
      }
    } catch (const agiru::Error &error) { refused = error.Code() == "RecordRefreshIsolation"; }
    CHECK_TRUE("a frozen SQL snapshot refuses instead of claiming freshness", refused);
    CHECK_TRUE("snapshot refusal leaves the read observation unchanged", read.Current());
    CHECK_TRUE("snapshot refusal never ends the transaction", session.Database().InTransaction());
  }
  session.Database().Run("ROLLBACK");
}

}

int main() {
  return gate::Run("RecordRefresh", [] {
    const gate::OwnedDatabase database("refresh");
    Observations(database.Dsn());
    for (const bool all : {false, true}) {
      for (const bool descending : {false, true}) {
        FreshNavigation<false>(database.Dsn(), all, descending);
        FreshNavigation<true>(database.Dsn(), all, descending);
      }
    }
    PendingWrite(database.Dsn());
    FrozenSnapshot(database.Dsn(), "REPEATABLE READ");
    FrozenSnapshot(database.Dsn(), "SERIALIZABLE");
  });
}

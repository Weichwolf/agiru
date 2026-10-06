#include "meta/TableDef.h"
#include "platform/Integer.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/Transaction.h"

#include "Check.h"
#include "Cursor.h"
#include "Filter.h"
#include "Selection.h"
#include "Where.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using agiru::Connection;
using agiru::Error;
using agiru::Session;
using agiru::detail::Cursor;
using agiru::detail::kFetchBlock;
using agiru::detail::ParseFilter;
using agiru::detail::Where;

namespace {

constexpr std::size_t kRows = 200;

void ComputedRowsProjectTheirDeclaredSystemFields(const Connection &connection) {
  agiru::detail::RecordState state;
  state.filters.push_back(
      {.field = agiru::platform::Integer::Field_No::Number, .group = 0, .text = "1..3"});
  const auto &table = agiru::platform::kIntegerTable;
  const auto selection = agiru::detail::Select(&state, table);
  const auto select = "SELECT " + agiru::detail::Columns(table) + " FROM " + selection.from +
                      " WHERE " + selection.where + " ORDER BY " + selection.order;
  const auto rows = connection.Execute(select, selection.binds);
  CHECK_TRUE("the native series returns its bounded population", rows.Rows() == 3);
  CHECK_TRUE("the computed projection matches the declared row width", rows.Columns() == 7);
  CHECK_TRUE("the selected native field order places timestamp before Number",
             table.fields[0].no.Value() == 0 && table.fields[1].no.Value() == 1);
  for (std::size_t index = 0; index < rows.Rows(); ++index) {
    const auto value = [&](std::size_t column) {
      return rows.Value(index, column).value_or("<NULL>");
    };
    CHECK_TEXT("native numbers retain order", value(1), std::to_string(index + 1));
    CHECK_TEXT("native Integer rows use the virtual provider's timestamp", value(0), "1");
    CHECK_TEXT("an unpersisted virtual row has no stored system identity",
               value(2),
               "00000000-0000-0000-0000-000000000000");
    CHECK_TEXT("a virtual row has no insertion audit instant", value(3), "1753-01-01 00:00:00");
    CHECK_TEXT(
        "a virtual row has no creator identity", value(4), "00000000-0000-0000-0000-000000000000");
    CHECK_TEXT("a virtual row has no modification audit instant", value(5), "1753-01-01 00:00:00");
    CHECK_TEXT(
        "a virtual row has no modifier identity", value(6), "00000000-0000-0000-0000-000000000000");
  }
  auto invalid = select;
  const std::string column = "'00000000-0000-0000-0000-000000000000'::uuid AS \"SystemId\"";
  const auto start = invalid.find(", " + column);
  CHECK_TRUE("the control removes exactly the system identity projection",
             start != std::string::npos);
  if (start == std::string::npos) { return; }
  invalid.erase(start, column.size() + 2);
  agiru::detail::Scope control;
  bool refused = false;
  try {
    static_cast<void>(connection.Execute(invalid, selection.binds));
  } catch (const agiru::DatabaseError &) { refused = true; }
  control.Discard("");
  CHECK_TRUE("the previous narrow series shape fails actual SQL", refused);
}

void TypedAndReflectedIntegerReadsUseTheSameProvider() {
  agiru::platform::Integer row;
  row.SetRange(row.Number, -1, 1);
  CHECK_TRUE("typed Integer reads find a computed row", row.FindSet());
  CHECK_TRUE("typed Integer keeps exact Number and virtual timestamp",
             row.Number == -1 && row.SystemRowVersion == 1);
  CHECK_TRUE("typed Integer has blank system identity and audit fields",
             row.SystemId.IsNull() && row.SystemCreatedAt.IsUndefined() &&
                 row.SystemCreatedBy.IsNull() && row.SystemModifiedAt.IsUndefined() &&
                 row.SystemModifiedBy.IsNull());
  agiru::RecordRef reference;
  reference.GetTable(row);
  reference.Field(1).SetFilter("0..1");
  CHECK_TRUE("RecordRef finds the same native series", reference.FindFirst());
  agiru::platform::Integer reflected;
  reference.SetTable(reflected);
  CHECK_TRUE("RecordRef preserves exact Number and virtual timestamp",
             reflected.Number == 0 && reflected.SystemRowVersion == 1);
  CHECK_TRUE("reflected Integer preserves blank identity and audit fields",
             reflected.SystemId.IsNull() && reflected.SystemCreatedAt.IsUndefined() &&
                 reflected.SystemCreatedBy.IsNull() && reflected.SystemModifiedAt.IsUndefined() &&
                 reflected.SystemModifiedBy.IsNull());
  CHECK_TRUE("typed Integer counts its bounded view without implicit lookup columns",
             row.Count() == 3);
  CHECK_TRUE("RecordRef counts its own bounded view", reference.Count() == 2);
  CHECK_TRUE("typed Next advances the computed record", row.Next() == 1 && row.Number == 0);
  CHECK_TRUE("typed navigation retains the virtual timestamp", row.SystemRowVersion == 1);
  CHECK_TRUE("typed FindLast reaches the bounded upper endpoint",
             row.FindLast() && row.Number == 1);
  CHECK_TRUE("typed reverse navigation retains the virtual timestamp",
             row.Next(-1) == -1 && row.Number == 0 && row.SystemRowVersion == 1);
}

void Fill(const Connection &connection) {
  connection.Run("DROP TABLE IF EXISTS cursor_gate");
  connection.Run("CREATE TABLE cursor_gate (n integer NOT NULL, name text NOT NULL)");
  std::string values;
  for (std::size_t i = 1; i <= kRows; ++i) {
    if (i != 1) { values += ", "; }
    values += "(" + std::to_string(i) + ", 'row " + std::to_string(i) + "')";
  }
  connection.Run("INSERT INTO cursor_gate (n, name) VALUES " + values);
}

// A CURSOR HOLDS THE BLOCK AND NOT THE SET, which is the whole reason it exists: 10 000 sessions
// over a table of 100 million rows cannot each hold a result.
void ItWalksMoreRowsThanItHolds(const Connection &connection) {
  Cursor cursor(connection, "SELECT n FROM cursor_gate ORDER BY n", {});
  std::size_t seen = 0;
  std::size_t widest = 0;
  while (cursor.Step()) {
    ++seen;
    widest = widest > cursor.Held() ? widest : cursor.Held();
    const std::optional<std::string_view> value = cursor.Value(0);
    CHECK_TRUE("every row comes back in order",
               value.has_value() && std::stoul(std::string(*value)) == seen);
    if (seen > kRows) { break; }
  }
  CHECK_TRUE("it walks the whole set", seen == kRows);
  CHECK_TRUE("and never holds more than one block", widest <= kFetchBlock);
  CHECK_TRUE("which is smaller than the set", kFetchBlock < kRows);
  const std::optional<std::string_view> last = cursor.Value(0);
  CHECK_TRUE("exhaustion preserves the last successful row",
             last.has_value() && std::stoul(std::string(*last)) == kRows);
  CHECK_TRUE("a spent cursor takes no more steps", !cursor.Step());
  const std::optional<std::string_view> stillLast = cursor.Value(0);
  CHECK_TRUE("repeated exhaustion preserves the last successful row",
             stillLast.has_value() && std::stoul(std::string(*stillLast)) == kRows);
}

void AnEmptySetStepsNowhere(const Connection &connection) {
  Cursor cursor(connection, "SELECT n FROM cursor_gate WHERE n > 100000 ORDER BY n", {});
  CHECK_TRUE("a cursor over nothing steps nowhere", !cursor.Step());
  CHECK_TRUE("and stays there", !cursor.Step());
}

void AFullBlockPreservesItsLastRow(const Connection &connection) {
  Cursor cursor(connection,
                "SELECT n FROM cursor_gate WHERE n <= " + std::to_string(kFetchBlock) +
                    " ORDER BY n",
                {});
  std::size_t seen = 0;
  while (cursor.Step()) { ++seen; }
  CHECK_TRUE("an exact block is read completely", seen == kFetchBlock);
  CHECK_TRUE("an exact block remains bounded after exhaustion", cursor.Held() == kFetchBlock);
  const std::optional<std::string_view> last = cursor.Value(0);
  CHECK_TRUE("the empty fetch after a full block preserves its last row",
             last.has_value() && std::stoul(std::string(*last)) == kFetchBlock);
  CHECK_TRUE("the empty fetch after a full block is terminal", !cursor.Step());
}

// THE FILTER IS BOUND AND NEVER CONCATENATED. The value below carries a quote and a percent sign,
// either of which would end the statement or change the pattern if it were pasted in.
void TheFilterBindsItsValues(const Connection &connection) {
  const agiru::FieldDef def{.offset = 0,
                            .name = "n",
                            .caption = "n",
                            .values = {},
                            .initValue = {},
                            .no = agiru::FieldNo{1},
                            .length = 0,
                            .type = agiru::FieldType::Integer};
  const agiru::detail::Clause clause = Where(def, ParseFilter("10..20|>195"), 1);
  Cursor cursor(
      connection, "SELECT n FROM cursor_gate WHERE " + clause.sql + " ORDER BY n", clause.binds);
  std::size_t seen = 0;
  while (cursor.Step()) { ++seen; }
  CHECK_TRUE("a range and an alternative select what AL would", seen == 11 + 5);
}

void AWildcardBecomesALikeAndAnEmptyValueIsAValue(const Connection &connection) {
  connection.Run("INSERT INTO cursor_gate (n, name) VALUES (900, ''), (901, '100% sure')");
  const agiru::FieldDef def{.offset = 0,
                            .name = "name",
                            .caption = "name",
                            .values = {},
                            .initValue = {},
                            .no = agiru::FieldNo{2},
                            .length = 0,
                            .type = agiru::FieldType::Text};
  {
    const agiru::detail::Clause clause = Where(def, ParseFilter("row 1*"), 1);
    Cursor cursor(
        connection, "SELECT n FROM cursor_gate WHERE " + clause.sql + " ORDER BY n", clause.binds);
    std::size_t seen = 0;
    while (cursor.Step()) { ++seen; }
    CHECK_TRUE("`*` matches a run of characters", seen == 111);
  }
  {
    // THE BASEAPP WRITES THIS FILTER 332 TIMES. The AL literal carries the filter text `<>` plus an
    // empty quoted value, and that pair is the EMPTY value rather than an escaped apostrophe.
    const agiru::detail::Clause clause = Where(def, ParseFilter("<>''"), 1);
    Cursor cursor(
        connection, "SELECT n FROM cursor_gate WHERE " + clause.sql + " ORDER BY n", clause.binds);
    std::size_t seen = 0;
    while (cursor.Step()) { ++seen; }
    CHECK_TRUE("not-the-empty-value is every row that carries one", seen == kRows + 1);
  }
  {
    // A PERCENT SIGN IS DATA IN AL AND A WILDCARD IN SQL, so the translation escapes it -- and the
    // value is bound rather than pasted into the statement.
    const agiru::detail::Clause clause = Where(def, ParseFilter("100% sure"), 1);
    Cursor cursor(
        connection, "SELECT n FROM cursor_gate WHERE " + clause.sql + " ORDER BY n", clause.binds);
    CHECK_TRUE("a value carrying SQL's own wildcard finds itself", cursor.Step());
    const std::optional<std::string_view> found = cursor.Value(0);
    CHECK_TRUE("and it is the row it names", found.has_value() && *found == "901");
  }
}

} // namespace

int main() {
  return gate::Run("Cursor", [] {
    try {
      const Session session(AGIRU_TEST_DSN);
      const Connection &connection = Session::Current().Database();
      Fill(connection);
      // A CURSOR LIVES IN A TRANSACTION, which PostgreSQL says outright: "DECLARE CURSOR can only
      // be used in transaction blocks". A session is always inside one -- board:0012 pins the
      // connection for exactly that -- so the gate opens the same boundary the runtime does.
      agiru::detail::Scope boundary;
      ComputedRowsProjectTheirDeclaredSystemFields(connection);
      TypedAndReflectedIntegerReadsUseTheSameProvider();
      ItWalksMoreRowsThanItHolds(connection);
      AnEmptySetStepsNowhere(connection);
      AFullBlockPreservesItsLastRow(connection);
      TheFilterBindsItsValues(connection);
      AWildcardBecomesALikeAndAnEmptyValueIsAValue(connection);
      boundary.Discard("");
      connection.Run("DROP TABLE cursor_gate");
    } catch (const Error &e) { CHECK_TEXT("cursor operations must complete", e.what(), ""); }
  });
}

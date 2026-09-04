#include "meta/TableDef.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/Transaction.h"

#include "Check.h"
#include "Cursor.h"
#include "Filter.h"
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
}

void AnEmptySetStepsNowhere(const Connection &connection) {
  Cursor cursor(connection, "SELECT n FROM cursor_gate WHERE n > 100000 ORDER BY n", {});
  CHECK_TRUE("a cursor over nothing steps nowhere", !cursor.Step());
  CHECK_TRUE("and stays there", !cursor.Step());
}

// THE FILTER IS BOUND AND NEVER CONCATENATED. The value below carries a quote and a percent sign,
// either of which would end the statement or change the pattern if it were pasted in.
void TheFilterBindsItsValues(const Connection &connection) {
  const agiru::FieldDef def{.no = agiru::FieldNo{1},
                            .name = "n",
                            .caption = "n",
                            .type = agiru::FieldType::Integer,
                            .length = 0,
                            .offset = 0,
                            .values = {},
                            .initValue = {}};
  const agiru::detail::Clause clause = Where(def, ParseFilter("10..20|>195"), 1);
  Cursor cursor(
      connection, "SELECT n FROM cursor_gate WHERE " + clause.sql + " ORDER BY n", clause.binds);
  std::size_t seen = 0;
  while (cursor.Step()) { ++seen; }
  CHECK_TRUE("a range and an alternative select what AL would", seen == 11 + 5);
}

void AWildcardBecomesALikeAndAnEmptyValueIsAValue(const Connection &connection) {
  connection.Run("INSERT INTO cursor_gate (n, name) VALUES (900, ''), (901, '100% sure')");
  const agiru::FieldDef def{.no = agiru::FieldNo{2},
                            .name = "name",
                            .caption = "name",
                            .type = agiru::FieldType::Text,
                            .length = 0,
                            .offset = 0,
                            .values = {},
                            .initValue = {}};
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
      ItWalksMoreRowsThanItHolds(connection);
      AnEmptySetStepsNowhere(connection);
      TheFilterBindsItsValues(connection);
      AWildcardBecomesALikeAndAnEmptyValueIsAValue(connection);
      boundary.Discard("");
      connection.Run("DROP TABLE cursor_gate");
    } catch (const Error &e) { CHECK_TEXT("the gate needs a database", e.what(), "a database"); }
  });
}

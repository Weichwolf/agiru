#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "platform/User.h"
#include "runtime/Catalogue.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordState.h"
#include "runtime/RecordWindow.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/TablePermissions.h"
#include "runtime/Transaction.h"
#include "type/BigInteger.h"
#include "type/Code.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Text.h"

#include "Check.h"
#include "OwnedDatabase.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr std::size_t kLabelLength = 80;
constexpr std::size_t kCodeLength = 20;

struct Row : agiru::Table<Row> {
  static constexpr agiru::TableId kId{50195};
  static constexpr std::string_view kName = "Record Window Gate";
  agiru::detail::StateHandle State_Block;
  agiru::Integer ID{};
  agiru::Text<kLabelLength> Label;
  agiru::Code<kCodeLength> Code;
  agiru::Decimal Amount;
  agiru::BigInteger Exact{};
  agiru::BigInteger SystemRowVersion{};
  agiru::Guid SystemId;
  agiru::DateTime SystemCreatedAt;
  agiru::Guid SystemCreatedBy;
  agiru::DateTime SystemModifiedAt;
  agiru::Guid SystemModifiedBy;
};

constexpr agiru::FieldNo kId{1};
constexpr agiru::FieldNo kLabel{2};
constexpr agiru::FieldNo kCode{3};
constexpr agiru::FieldNo kAmount{4};
constexpr agiru::FieldNo kExact{5};
constexpr std::size_t kDefaultRows = 40;
constexpr std::size_t kSmallRows = 7;
constexpr std::size_t kLargeRows = 80;
constexpr agiru::Integer kPopulation = 151;
constexpr agiru::BigInteger kBeyondJsInteger = 9007199254740993;
constexpr std::string_view kAmountValue = "1.00000000000000000001";
constexpr std::string_view kExtendedAmount = "1.0000000000000000000000000001";
constexpr std::string_view kUser = "00000000-0000-0000-0000-000000000001";
constexpr std::array<std::string_view, 13> kLabels{
    "Å", "A", "a", "Ä", "é", "e", "Ζ", "😀", "漢字", "trail ", "trail", "0190", "100"};
constexpr std::array<std::string_view, 7> kCodes{"0190", "100", "0001", "20", "2", "10", "010"};
constexpr std::string_view kLabelOrder = R"("Label","Code","ID")";
using Position = agiru::RecordWindowPosition;

}

template <> struct agiru::TableTraits<Row> {
  static constexpr std::array kDeclared{
      agiru::Declare<&Row::ID>(kId, "ID", "ID", offsetof(Row, ID)),
      agiru::Declare<&Row::Label>(kLabel, "Label", "Label", offsetof(Row, Label)),
      agiru::Declare<&Row::Code>(kCode, "Code", "Code", offsetof(Row, Code)),
      agiru::Declare<&Row::Amount>(kAmount, "Amount", "Amount", offsetof(Row, Amount)),
      agiru::Declare<&Row::Exact>(kExact, "Exact", "Exact", offsetof(Row, Exact))};
  static constexpr auto kFields = agiru::WithImplicitFields<Row,
                                                            agiru::SystemFieldProfile::Runtime17,
                                                            agiru::TableType::Normal,
                                                            false>(kDeclared);
  static constexpr std::array kPrimary{kId};
  static constexpr std::array kLabelCode{kLabel, kCode};
  static constexpr std::array kCodeKey{kCode};
  static constexpr std::array kKeys{agiru::KeyDef{.name = "Primary", .fields = kPrimary},
                                    agiru::KeyDef{.name = "LabelCode", .fields = kLabelCode},
                                    agiru::KeyDef{.name = "Code", .fields = kCodeKey}};
  static constexpr agiru::TableDef kTable{
      .id = Row::kId, .name = Row::kName, .fields = kFields, .keys = kKeys};
};

namespace {

const agiru::RegisterTable<Row> kRegistration;

const agiru::TableDef &Declaration() {
  return agiru::TableTraits<Row>::kTable;
}

std::string_view Label(agiru::Integer id) {
  return kLabels[static_cast<std::size_t>(id - 1) % kLabels.size()];
}

void Fill(const agiru::Session &session) {
  agiru::CreateTable(session.Database(), Declaration());
  for (agiru::Integer id = kPopulation; id > 0; --id) {
    Row row;
    row.ID = id;
    row.Label = Label(id);
    row.Code = kCodes[static_cast<std::size_t>(id) % kCodes.size()];
    row.Amount = agiru::Decimal::FromInvariantString(kAmountValue);
    row.Exact = kBeyondJsInteger + id;
    row.Insert();
  }
  agiru::Session::Current().Transaction().Commit(session.Database());
}

std::vector<agiru::Integer>
Oracle(const agiru::Connection &connection, std::string_view order, std::string_view where = {}) {
  std::string sql = R"(SELECT "ID" FROM "Record Window Gate")";
  if (!where.empty()) { sql += " WHERE " + std::string(where); }
  const auto result = connection.Execute(sql + " ORDER BY " + std::string(order));
  std::vector<agiru::Integer> ids;
  for (std::size_t index = 0; index < result.Rows(); ++index) {
    const auto value = result.Value(index, 0);
    if (!value.has_value()) { throw agiru::Error("window oracle refuses null identity"); }
    ids.push_back(std::stoi(std::string(*value)));
  }
  return ids;
}

std::string Scalar(const agiru::Connection &connection, std::string_view sql) {
  const auto result = connection.Execute(sql);
  if (result.Rows() != 1 || result.Columns() != 1) {
    throw agiru::Error("window scalar requires one value");
  }
  const auto value = result.Value(0, 0);
  if (!value.has_value()) { throw agiru::Error("window scalar refuses null"); }
  return std::string(*value);
}

void Values(const Row &row, agiru::Integer expected) {
  CHECK_TRUE("window rows follow the independent SQL order without omissions", row.ID == expected);
  CHECK_TEXT(
      "window Unicode and whitespace values remain exact", row.Label.Value(), Label(expected));
  CHECK_TEXT(
      "window persisted Decimal value remains exact", row.Amount.ToInvariantString(), kAmountValue);
  CHECK_TRUE("window BigInteger remains exact beyond JavaScript precision",
             row.Exact == kBeyondJsInteger + expected);
}

void Walk(Row &anchor,
          const std::vector<agiru::Integer> &expected,
          std::size_t limit,
          bool backwards) {
  auto position = backwards ? Position::Last : Position::First;
  std::size_t seen = 0;
  for (;;) {
    const auto original = anchor.GetPosition();
    const auto filters = anchor.GetView();
    const auto window = agiru::ReadRecordWindow(&anchor, Declaration(), position, limit);
    CHECK_TRUE("window SQL transfers no more than the bound plus one probe",
               window.RowsRead() <= limit + 1);
    CHECK_TEXT("reading a window leaves the source key untouched", anchor.GetPosition(), original);
    CHECK_TEXT("reading a window retains every filter group and key", anchor.GetView(), filters);
    const std::size_t count = std::min(limit, expected.size() - seen);
    CHECK_TRUE("window size follows the requested bound", window.Size() == count);
    CHECK_TRUE("continuation discovery distinguishes exact and incomplete windows",
               window.HasMore() == (seen + count < expected.size()));
    const std::size_t start = backwards ? expected.size() - seen - count : seen;
    Row loaded;
    for (std::size_t index = 0; index < std::min(window.Size(), count); ++index) {
      window.Load(index, &loaded);
      Values(loaded, expected[start + index]);
    }
    seen += count;
    if (!window.HasMore()) { break; }
    if (window.Size() == 0 || count == 0 || seen >= expected.size()) {
      throw agiru::Error("window continuation exceeded the bounded oracle");
    }
    window.Load(backwards ? 0 : window.Size() - 1, &anchor);
    position = backwards ? Position::Before : Position::After;
  }
  CHECK_TRUE("every selected row is traversed exactly once", seen == expected.size());
}

void Bounds(const agiru::Connection &observer) {
  constexpr std::array<agiru::Integer, 5> counts{0, 1, 39, 40, 41};
  for (const auto count : counts) {
    Row row;
    row.SetRange(row.ID, 1, count);
    const auto expected = Oracle(observer, "\"ID\"", "\"ID\" <= " + std::to_string(count));
    Walk(row, expected, kDefaultRows, false);
    Walk(row, expected, kDefaultRows, true);
  }
  Row row;
  const auto expected = Oracle(observer, "\"ID\"");
  for (const auto limit : {kSmallRows, kDefaultRows, kLargeRows}) {
    Walk(row, expected, limit, false);
    Walk(row, expected, limit, true);
  }
}

void Orders(const agiru::Connection &observer) {
  for (const bool ascending : {false, true}) {
    for (const bool mixed : {false, true}) {
      Row row;
      CHECK_TRUE("window selects the declared secondary key",
                 row.SetCurrentKey(row.Label, row.Code));
      row.SetAscending(row.Code, !mixed);
      row.Ascending(ascending);
      const std::string first = ascending ? " ASC" : " DESC";
      const std::string second = ascending == !mixed ? " ASC" : " DESC";
      std::string order = R"("Label")";
      order.append(first).append(R"(,"Code")").append(second).append(R"(,"ID")").append(first);
      const auto expected = Oracle(observer, order);
      Walk(row, expected, kSmallRows, false);
      Walk(row, expected, kSmallRows, true);
      row.FilterGroup(2);
      row.SetRange(row.ID, kDefaultRows, static_cast<agiru::Integer>(kLargeRows));
      row.FilterGroup(0);
      row.SetFilter(row.Code, "0*|1*");
      const auto selected = Oracle(
          observer, order, R"("ID" BETWEEN 40 AND 80 AND ("Code" LIKE '0%' OR "Code" LIKE '1%'))");
      Walk(row, selected, kSmallRows, false);
      Walk(row, selected, kSmallRows, true);
    }
  }
}

void CollatedFilters(const agiru::Connection &observer) {
  Row row;
  CHECK_TRUE("digit-only Code windows select the declared text key", row.SetCurrentKey(row.Code));
  const auto byCode = Oracle(observer, R"("Code","ID")");
  Walk(row, byCode, kSmallRows, false);
  Walk(row, byCode, kSmallRows, true);
  row.Reset();
  CHECK_TRUE("text filter windows select the declared text key", row.SetCurrentKey(row.Label));
  row.SetRange(row.Label, "A");
  const auto equals = Oracle(observer, kLabelOrder, R"("Label"='A')");
  Walk(row, equals, kSmallRows, false);
  Walk(row, equals, kSmallRows, true);
  row.SetFilter(row.Label, "A..e");
  const auto range = Oracle(observer, kLabelOrder, R"("Label">='A' AND "Label"<='e')");
  Walk(row, range, kSmallRows, false);
  Walk(row, range, kSmallRows, true);
  row.SetFilter(row.Label, "");
  row.FilterGroup(2);
  row.SetRange(row.ID, kDefaultRows, static_cast<agiru::Integer>(kLargeRows));
  row.FilterGroup(-1);
  row.SetRange(row.Label, "A");
  row.SetRange(row.Code, "100");
  row.FilterGroup(0);
  const auto cross =
      Oracle(observer, kLabelOrder, R"("ID" BETWEEN 40 AND 80 AND ("Label"='A' OR "Code"='100'))");
  Walk(row, cross, kSmallRows, false);
  Walk(row, cross, kSmallRows, true);
}

void Refuses(std::string_view claim,
             const std::function<void()> &operation,
             std::string_view code) {
  bool refused = false;
  try {
    operation();
  } catch (const agiru::Error &error) { refused = error.Code() == code; }
  CHECK_TRUE(claim, refused);
}

void Invalid() {
  Row row;
  for (const auto limit : {std::size_t{0}, std::numeric_limits<std::size_t>::max()}) {
    Refuses(
        "window rejects zero or overflowing limits",
        [&] {
          static_cast<void>(agiru::ReadRecordWindow(&row, Declaration(), Position::First, limit));
        },
        "RecordWindowLimit");
  }
  Refuses(
      "unknown window positions never default to a read",
      [&] {
        static_cast<void>(
            agiru::ReadRecordWindow(&row, Declaration(), Position::Unknown, kDefaultRows));
      },
      "RecordWindowPosition");
  for (const auto position : {Position::After, Position::Before}) {
    Refuses(
        "relative windows require a positioned anchor",
        [&] {
          static_cast<void>(agiru::ReadRecordWindow(&row, Declaration(), position, kDefaultRows));
        },
        "RecordWindowAnchor");
  }
  const auto window = agiru::ReadRecordWindow(&row, Declaration(), Position::First, kDefaultRows);
  Refuses(
      "continuation probes cannot be loaded as display rows",
      [&] { window.Load(window.Size(), &row); },
      "RecordWindowIndex");
  Refuses(
      "null window storage explicitly refuses",
      [&] { window.Load(0, nullptr); },
      "RecordWindowInput");
  agiru::Temporary<Row> temporary;
  Refuses(
      "unqualified temporary collation does not silently substitute a SQL window",
      [&] {
        static_cast<void>(
            agiru::ReadRecordWindow(&temporary, Declaration(), Position::First, kDefaultRows));
      },
      "RecordWindowProvider");
}

void ExtendedValues(const agiru::Connection &connection) {
  connection.Run(R"(ALTER TABLE "Record Window Gate" ALTER COLUMN "Amount" TYPE numeric)");
  const std::array<std::optional<std::string>, 1> bind{std::string(kExtendedAmount)};
  connection.Run(R"(UPDATE "Record Window Gate" SET "Amount"=$1::numeric WHERE "ID"=1)", bind);
  Row row;
  const auto window = agiru::ReadRecordWindow(&row, Declaration(), Position::First, kSmallRows);
  window.Load(0, &row);
  CHECK_TEXT("window scale-28 SQL tokens never pass through floating point",
             row.Amount.ToInvariantString(),
             kExtendedAmount);
}

void Concurrent(const std::string &dsn) {
  const agiru::Session session(dsn);
  const agiru::Connection independent(dsn);
  Row row;
  auto window = agiru::ReadRecordWindow(&row, Declaration(), Position::First, kDefaultRows);
  window.Load(window.Size() - 1, &row);
  constexpr agiru::Integer kDeleted = 40;
  independent.Run(R"(DELETE FROM "Record Window Gate" WHERE "ID"=40)");
  const auto next = agiru::ReadRecordWindow(&row, Declaration(), Position::After, kSmallRows);
  Row observed;
  next.Load(0, &observed);
  CHECK_TRUE("deleted continuation anchors retain strict SQL positioning",
             observed.ID == kDeleted + 1);
  auto &boundary = agiru::Session::Current().Transaction();
  const auto depth = boundary.Open(session.Database());
  row.Get(1);
  row.Label = "pending";
  row.Modify();
  const auto epoch = boundary.CursorEpoch();
  const auto pending = agiru::ReadRecordWindow(&row, Declaration(), Position::First, kSmallRows);
  CHECK_TRUE("window reads preserve active transaction depth and epoch",
             boundary.Depth() == depth && boundary.CursorEpoch() == epoch);
  CHECK_TEXT("window reads do not implicitly commit pending writes",
             Scalar(independent, R"(SELECT "Label" FROM "Record Window Gate" WHERE "ID"=1)"),
             Label(1));
  boundary.Rollback(session.Database(), depth);
  pending.Load(0, &observed);
  CHECK_TEXT("window values own their SQL snapshot without a live lease",
             observed.Label.Value(),
             "pending");
}

class Deny final : public agiru::TablePermissionAuthority {
public:
  bool Allows([[maybe_unused]] const agiru::TableDef &table,
              [[maybe_unused]] agiru::TableOperation operation) const override {
    return false;
  }
};

void Permissions(const std::string &dsn) {
  const agiru::Session seed(dsn);
  agiru::CreateTable(seed.Database(), agiru::platform::kUserTable);
  agiru::platform::User user;
  user.UserSecurityID = agiru::Guid(kUser);
  user.UserName = "WINDOW";
  user.Insert();
  agiru::Session::Current().Transaction().Commit(seed.Database());
  Row row;
  const auto window = agiru::ReadRecordWindow(&row, Declaration(), Position::First, kSmallRows);
  agiru::Session denied(dsn, agiru::Guid(kUser));
  denied.TablePermissions(std::make_shared<Deny>());
  Refuses(
      "window reads cannot bypass authenticated permissions",
      [&] {
        static_cast<void>(
            agiru::ReadRecordWindow(&row, Declaration(), Position::First, kSmallRows));
      },
      "Permission");
  Refuses(
      "loading retained window values rechecks current permissions",
      [&] { window.Load(0, &row); },
      "Permission");
}

}

int main() {
  return gate::Run("RecordWindow", [] {
    const gate::OwnedDatabase database("record_window", gate::OwnedDatabase::Encoding::Utf8);
    {
      const agiru::Session session(database.Dsn());
      const agiru::Connection observer(database.Dsn());
      CHECK_TEXT("collation gates use actual UTF-8 storage",
                 Scalar(observer, "SHOW server_encoding"),
                 "UTF8");
      Fill(session);
      Bounds(observer);
      Orders(observer);
      CollatedFilters(observer);
      session.Database().Run("CREATE COLLATION window_compare "
                             "(provider=icu,locale='und-u-ks-level1',deterministic=false)");
      session.Database().Run("ALTER TABLE \"Record Window Gate\" ALTER COLUMN \"Label\" TYPE "
                             "varchar(80) COLLATE window_compare");
      CHECK_TEXT("the fixture collation equates case and accents",
                 Scalar(observer,
                        "SELECT ('A' COLLATE window_compare = 'a' AND "
                        "'A' COLLATE window_compare = 'Å')::text"),
                 "true");
      Orders(observer);
      CollatedFilters(observer);
      Invalid();
      ExtendedValues(session.Database());
    }
    Concurrent(database.Dsn());
    Permissions(database.Dsn());
  });
}

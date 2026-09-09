#include "meta/Ids.h"
#include "meta/QueryDef.h"
#include "runtime/Error.h"
#include "runtime/Query.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "type/Code.h"
#include "type/Decimal.h"
#include "type/Integer.h"

#include "Check.h"
#include "ResourceCost.h"

#include <array>
#include <cstddef>
#include <string>

using agiru::CreateTable;
using agiru::Decimal;
using agiru::DropTable;
using agiru::Error;
using agiru::Integer;
using agiru::QueryColumn;
using agiru::QueryDataItem;
using agiru::QueryDef;
using agiru::QueryJoin;
using agiru::QueryLink;
using agiru::QueryMethod;
using agiru::QueryOrder;
using agiru::Session;
using agiru::app::tables::ResourceCost;
using agiru::app::tables::ResourceCostCostType;
using agiru::app::tables::ResourceCostType;

using CodeValue = decltype(ResourceCost::Code);

namespace {

constexpr int kRows = 10;

std::string Named(int i) {
  return std::string("R") + (i < 10 ? "0" : "") + std::to_string(i);
}

// TEN ROWS, five of each Type, one unit cost each, so a sum per Type is 5 and a count is 5.
void Fill() {
  DropTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
  CreateTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
  for (int i = 0; i < kRows; ++i) {
    ResourceCost rec;
    rec.Type = i % 2 == 0 ? ResourceCostType::Resource : ResourceCostType::GroupResource;
    rec.Code = CodeValue(Named(i));
    rec.WorkTypeCode = "hours";
    rec.CostType = ResourceCostCostType::Fixed;
    rec.DirectUnitCost = Decimal::FromInvariantString("1.00");
    rec.UnitCost = Decimal::FromInvariantString("2.00");
    rec.Insert();
  }
}

/// THE SHAPE THE GENERATOR WRITES, by hand: a query over `Resource Cost` joined to ITSELF on
/// `Code`, the lower dataitem narrowed to one row by a `DataItemTableFilter`, so that nine upper
/// rows have no match below -- which is the case that tells a left outer join from an inner one
/// (`devenv-query-links-joins.md`, the worked example with the NULL salesperson).
extern const QueryDef kPairsQuery;

class Pairs_Query : public agiru::Query<Pairs_Query> {
public:
  [[maybe_unused]] static constexpr agiru::QueryId kId{50100};
  [[maybe_unused]] static constexpr std::string_view kName{"Pairs"};
  agiru::detail::QueryHandle State_Block;
  decltype(ResourceCost::Type) Type{};
  decltype(ResourceCost::Code) Code{};
  decltype(ResourceCost::Code) Matched_Code{};
  decltype(ResourceCost::DirectUnitCost) Matched_Cost{};
};

extern const QueryDef kInnerPairsQuery;

class InnerPairs_Query : public agiru::Query<InnerPairs_Query> {
public:
  [[maybe_unused]] static constexpr agiru::QueryId kId{50101};
  [[maybe_unused]] static constexpr std::string_view kName{"Inner Pairs"};
  agiru::detail::QueryHandle State_Block;
  decltype(ResourceCost::Code) Code{};
  decltype(ResourceCost::Code) Matched_Code{};
};

extern const QueryDef kTotalsQuery;

class Totals_Query : public agiru::Query<Totals_Query> {
public:
  [[maybe_unused]] static constexpr agiru::QueryId kId{50102};
  [[maybe_unused]] static constexpr std::string_view kName{"Totals"};
  agiru::detail::QueryHandle State_Block;
  decltype(ResourceCost::Type) Type{};
  decltype(ResourceCost::DirectUnitCost) Sum_Cost{};
  Integer Rows{};
};

constexpr std::array<QueryLink, 1> kPairLinks{{
    QueryLink{.field = ResourceCost::Field_No::Code, .dataItem = 0, .reference = ResourceCost::Field_No::Code},
}};

constexpr std::array<QueryDataItem, 2> kPairItems{{
    QueryDataItem{.name = "Upper", .table = &agiru::TableTraits<ResourceCost>::kTable, .join = QueryJoin::LeftOuter, .links = {}, .tableFilter = ""},
    QueryDataItem{.name = "Lower", .table = &agiru::TableTraits<ResourceCost>::kTable, .join = QueryJoin::LeftOuter, .links = kPairLinks, .tableFilter = "Code = const(R01)"},
}};

constexpr std::array<QueryColumn, 4> kPairColumns{{
    QueryColumn{.name = "Type", .caption = "Type", .offset = offsetof(Pairs_Query, Type), .dataItem = 0, .field = ResourceCost::Field_No::Type, .method = QueryMethod::None, .returned = true, .reverseSign = false, .columnFilter = ""},
    QueryColumn{.name = "Code", .caption = "Code", .offset = offsetof(Pairs_Query, Code), .dataItem = 0, .field = ResourceCost::Field_No::Code, .method = QueryMethod::None, .returned = true, .reverseSign = false, .columnFilter = ""},
    QueryColumn{.name = "Matched_Code", .caption = "Matched Code", .offset = offsetof(Pairs_Query, Matched_Code), .dataItem = 1, .field = ResourceCost::Field_No::Code, .method = QueryMethod::None, .returned = true, .reverseSign = false, .columnFilter = ""},
    QueryColumn{.name = "Matched_Cost", .caption = "Matched Cost", .offset = offsetof(Pairs_Query, Matched_Cost), .dataItem = 1, .field = ResourceCost::Field_No::DirectUnitCost, .method = QueryMethod::None, .returned = true, .reverseSign = false, .columnFilter = ""},
}};

constexpr std::array<QueryOrder, 1> kPairOrder{{QueryOrder{.column = "Code", .descending = false}}};

constexpr std::array<QueryDataItem, 2> kInnerItems{{
    QueryDataItem{.name = "Upper", .table = &agiru::TableTraits<ResourceCost>::kTable, .join = QueryJoin::LeftOuter, .links = {}, .tableFilter = ""},
    QueryDataItem{.name = "Lower", .table = &agiru::TableTraits<ResourceCost>::kTable, .join = QueryJoin::Inner, .links = kPairLinks, .tableFilter = "Code = const(R01)"},
}};

constexpr std::array<QueryColumn, 2> kInnerColumns{{
    QueryColumn{.name = "Code", .caption = "Code", .offset = offsetof(InnerPairs_Query, Code), .dataItem = 0, .field = ResourceCost::Field_No::Code, .method = QueryMethod::None, .returned = true, .reverseSign = false, .columnFilter = ""},
    QueryColumn{.name = "Matched_Code", .caption = "Matched Code", .offset = offsetof(InnerPairs_Query, Matched_Code), .dataItem = 1, .field = ResourceCost::Field_No::Code, .method = QueryMethod::None, .returned = true, .reverseSign = false, .columnFilter = ""},
}};

constexpr std::array<QueryDataItem, 1> kTotalItems{{
    QueryDataItem{.name = "Cost", .table = &agiru::TableTraits<ResourceCost>::kTable, .join = QueryJoin::LeftOuter, .links = {}, .tableFilter = ""},
}};

constexpr std::array<QueryColumn, 3> kTotalColumns{{
    QueryColumn{.name = "Type", .caption = "Type", .offset = offsetof(Totals_Query, Type), .dataItem = 0, .field = ResourceCost::Field_No::Type, .method = QueryMethod::None, .returned = true, .reverseSign = false, .columnFilter = ""},
    QueryColumn{.name = "Sum_Cost", .caption = "Sum Cost", .offset = offsetof(Totals_Query, Sum_Cost), .dataItem = 0, .field = ResourceCost::Field_No::DirectUnitCost, .method = QueryMethod::Sum, .returned = true, .reverseSign = false, .columnFilter = ""},
    QueryColumn{.name = "Rows", .caption = "Rows", .offset = offsetof(Totals_Query, Rows), .dataItem = 0, .field = ResourceCost::Field_No::Code, .method = QueryMethod::Count, .returned = true, .reverseSign = false, .columnFilter = ""},
}};

constexpr std::array<QueryOrder, 1> kTotalOrder{{QueryOrder{.column = "Type", .descending = false}}};

constexpr QueryDef kPairsQuery{.id = Pairs_Query::kId, .name = Pairs_Query::kName, .caption = "Pairs", .dataItems = kPairItems, .columns = kPairColumns, .orderBy = kPairOrder, .topNumberOfRows = 0};
constexpr QueryDef kInnerPairsQuery{.id = InnerPairs_Query::kId, .name = InnerPairs_Query::kName, .caption = "Inner Pairs", .dataItems = kInnerItems, .columns = kInnerColumns, .orderBy = {}, .topNumberOfRows = 0};
constexpr QueryDef kTotalsQuery{.id = Totals_Query::kId, .name = Totals_Query::kName, .caption = "Totals", .dataItems = kTotalItems, .columns = kTotalColumns, .orderBy = kTotalOrder, .topNumberOfRows = 0};

} // namespace

template <> struct agiru::QueryTraits<Pairs_Query> {
  [[maybe_unused]] static constexpr agiru::QueryId kId{50100};
  [[maybe_unused]] static constexpr std::string_view kName{"Pairs"};
  static constexpr const QueryDef &kQuery = kPairsQuery;
};

template <> struct agiru::QueryTraits<InnerPairs_Query> {
  [[maybe_unused]] static constexpr agiru::QueryId kId{50101};
  [[maybe_unused]] static constexpr std::string_view kName{"Inner Pairs"};
  static constexpr const QueryDef &kQuery = kInnerPairsQuery;
};

template <> struct agiru::QueryTraits<Totals_Query> {
  [[maybe_unused]] static constexpr agiru::QueryId kId{50102};
  [[maybe_unused]] static constexpr std::string_view kName{"Totals"};
  static constexpr const QueryDef &kQuery = kTotalsQuery;
};

namespace {

// THE DEFAULT JOIN IS LEFT OUTER, in the property page's own words, and the predecessor emitted
// INNER for an omitted `SqlJoinType` so a dataitem with no match deleted the whole result
// (openerp WI-1227). Nine upper rows have no lower match here; all ten come back.
void AnOmittedJoinIsLeftOuter() {
  Pairs_Query query;
  CHECK_TRUE("the query opens", static_cast<bool>(query.Open()));
  int rows = 0;
  int matched = 0;
  std::string first;
  while (query.Read()) {
    ++rows;
    if (rows == 1) { first = std::string(query.Code.Value()); }
    if (!std::string(query.Matched_Code.Value()).empty()) {
      ++matched;
      CHECK_TEXT("the matched row is the one the table filter kept",
                 std::string(query.Matched_Code.Value()),
                 "R01");
      // THE VALUE AND NOT ITS TEXT: a decimal that has been through the database carries the
      // storage scale, which StorageGate states on its own.
      CHECK_TRUE("and its cost came across",
                 query.Matched_Cost == Decimal::FromInvariantString("1.00"));
    }
  }
  CHECK_TRUE("every upper row comes back", rows == kRows);
  CHECK_TRUE("and exactly one of them found a lower row", matched == 1);
  CHECK_TEXT("OrderBy sorts the rows", first, "R00");
}

// THE NEGATIVE CONTROL: the same query declared INNER keeps only the matched row. A join that
// ignored `SqlJoinType` passes the case above and fails this one.
void AnInnerJoinDropsTheUnmatched() {
  InnerPairs_Query query;
  CHECK_TRUE("the query opens", static_cast<bool>(query.Open()));
  int rows = 0;
  while (query.Read()) { ++rows; }
  CHECK_TRUE("one row survives an inner join", rows == 1);
}

// A `Method = Sum` COLUMN GROUPS BY EVERY OTHER COLUMN (`devenv-query-totals-grouping.md`), and
// `Count` counts the group.
void AnAggregateGroupsByTheOtherColumns() {
  Totals_Query query;
  CHECK_TRUE("the query opens", static_cast<bool>(query.Open()));
  int groups = 0;
  while (query.Read()) {
    ++groups;
    CHECK_TRUE("the sum over a type is five",
               query.Sum_Cost == Decimal::FromInvariantString("5.00"));
    CHECK_TRUE("and the count is five", query.Rows == 5);
  }
  CHECK_TRUE("two types make two groups", groups == 2);
}

// A FILTER SET IN AL NARROWS THE DATASET, `TopNumberOfRows` LIMITS IT, and a `Read` before an
// `Open` refuses rather than answering false.
void FiltersAndTheRowLimitApply() {
  Pairs_Query query;
  query.SetRange(query.Type, ResourceCostType::Resource);
  CHECK_TRUE("the query opens", static_cast<bool>(query.Open()));
  int rows = 0;
  while (query.Read()) { ++rows; }
  CHECK_TRUE("SetRange keeps the five rows of one type", rows == 5);
  CHECK_TEXT("and GetFilter reads it back", std::string(query.GetFilter(query.Type).Value()), "0");

  query.SetRange(query.Type);
  query.TopNumberOfRows(3);
  CHECK_TRUE("the query opens again", static_cast<bool>(query.Open()));
  rows = 0;
  while (query.Read()) { ++rows; }
  CHECK_TRUE("TopNumberOfRows limits the dataset", rows == 3);

  std::string said;
  Pairs_Query closed;
  try {
    (void)closed.Read();
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("Read before Open refuses", !said.empty());

  // THE NEGATIVE CONTROL for the member rule: a member of ANOTHER object is no column of this one.
  said.clear();
  Integer stranger = 0;
  try {
    query.SetRange(stranger, 1);
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("a member of another object refuses", !said.empty());
}

} // namespace

int main() {
  return gate::Run("Query", [] {
    try {
      const Session session(AGIRU_TEST_DSN);
      Fill();
      AnOmittedJoinIsLeftOuter();
      AnInnerJoinDropsTheUnmatched();
      AnAggregateGroupsByTheOtherColumns();
      FiltersAndTheRowLimitApply();
      DropTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
    } catch (const Error &e) { CHECK_TEXT("the gate needs a database", e.what(), "a database"); }
  });
}

#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "type/Code.h"
#include "type/Decimal.h"

#include "Check.h"
#include "ResourceCost.h"

#include <string>

using agiru::CreateTable;
using agiru::Decimal;
using agiru::DropTable;
using agiru::Error;
using agiru::Session;
using agiru::app::tables::ResourceCost;
using agiru::app::tables::ResourceCostCostType;
using agiru::app::tables::ResourceCostType;

namespace {

// A SET LARGER THAN ONE FETCH BLOCK. `Cursor` fetches 64 rows at a time, so a walk that stops at 64
// or reads the same block twice is a defect this number is chosen to expose. A gate that inserted
// ten rows would be green over both.
constexpr int kRows = 150;
constexpr int kBlock = 64;

void Fill() {
  DropTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
  CreateTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
  for (int i = 0; i < kRows; ++i) {
    ResourceCost rec;
    rec.Type = i % 2 == 0 ? ResourceCostType::Resource : ResourceCostType::GroupResource;
    rec.Code = agiru::Code<20>(std::string("R") + (i < 10 ? "00" : (i < 100 ? "0" : "")) +
                               std::to_string(i));
    rec.WorkTypeCode = "hours";
    rec.CostType = ResourceCostCostType::Fixed;
    rec.DirectUnitCost = Decimal::FromInvariantString("1.00");
    rec.UnitCost = Decimal::FromInvariantString("2.00");
    rec.Insert();
  }
}

// `repeat ... until Next() = 0` IS THE LOOP THE BASEAPP WRITES, 120 049 times. It walks past the
// fetch block, it reads every row exactly once, and it stops.
void TheWalkCrossesTheFetchBlock() {
  Fill();
  ResourceCost rec;
  CHECK_TRUE("FindSet finds the set", static_cast<bool>(rec.FindSet()));
  int seen = 1;
  std::string first(rec.Code.Value());
  std::string last = first;
  while (rec.Next() != 0) {
    ++seen;
    last = std::string(rec.Code.Value());
  }
  CHECK_TRUE("every row is walked exactly once", seen == kRows);
  CHECK_TRUE("and the walk crosses the fetch block", kRows > kBlock);
  CHECK_TEXT("the first row is the key's first", first, "R000");
  CHECK_TEXT("the last row is the key's last", last, "R149");
  CHECK_TRUE("Count agrees with the walk", static_cast<int>(rec.Count()) == kRows);
  CHECK_TRUE("and the set is not empty", !static_cast<bool>(rec.IsEmpty()));
}

// A FILTER NARROWS THE SET AND NOT THE WALK. The predicate goes into the statement, so the rows the
// filter excludes never leave PostgreSQL -- which is the whole reason a filter is not a loop.
void AFilterNarrowsWhatTheCursorSelects() {
  ResourceCost rec;
  rec.SetRange(rec.Type, ResourceCostType::Resource);
  CHECK_TRUE("the filter halves the count", static_cast<int>(rec.Count()) == kRows / 2);
  CHECK_TRUE("FindSet finds the filtered set", static_cast<bool>(rec.FindSet()));
  int seen = 1;
  bool onlyResource = rec.Type == ResourceCostType::Resource;
  while (rec.Next() != 0) {
    ++seen;
    onlyResource = onlyResource && rec.Type == ResourceCostType::Resource;
  }
  CHECK_TRUE("and the walk sees exactly those rows", seen == kRows / 2);
  CHECK_TRUE("every one of which matches the filter", onlyResource);

  ResourceCost none;
  none.SetRange(none.Code, agiru::Code<20>("nothing at all"));
  CHECK_TRUE("a filter that matches nothing is empty", static_cast<bool>(none.IsEmpty()));
  CHECK_TRUE("and FindSet says so", !static_cast<bool>(none.FindSet()));
  CHECK_TRUE("and Next on an unpositioned record stays 0", static_cast<int>(none.Next()) == 0);
}

// A COPY DOES NOT SHARE THE POSITION. Two record variables are two positions in AL, and a shared
// cursor would step both at once.
void ACopyWalksItsOwnSet() {
  ResourceCost one;
  CHECK_TRUE("the first finds the set", static_cast<bool>(one.FindSet()));
  CHECK_TRUE("and steps once", static_cast<int>(one.Next()) == 1);
  // WHERE IT STANDS IS READ AND NOT ASSUMED. The primary key is (Type, Code), so the walk is not in
  // Code order at all -- R000, R002, R004 ... then R001. A gate that wrote "R001" here would be
  // asserting the wrong key and would go green the day the ordering broke in the other direction.
  const std::string stood(one.Code.Value());
  const ResourceCost two = one;
  ResourceCost three = two;
  CHECK_TRUE("the copy finds its own set", static_cast<bool>(three.FindSet()));
  CHECK_TEXT("which starts at the beginning", std::string(three.Code.Value()), "R000");
  CHECK_TEXT("and the original still stands where it stood", std::string(one.Code.Value()), stood);
  CHECK_TRUE("which is not the beginning", stood != "R000");
}

// AND `DeleteAll` IS ONE STATEMENT OVER THE FILTER, not a walk that deletes.
void DeleteAllTakesTheFilteredRows() {
  ResourceCost rec;
  rec.SetRange(rec.Type, ResourceCostType::GroupResource);
  rec.DeleteAll();
  CHECK_TRUE("the filtered rows are gone", static_cast<bool>(rec.IsEmpty()));
  ResourceCost rest;
  CHECK_TRUE("and only those", static_cast<int>(rest.Count()) == kRows / 2);

  bool refused = false;
  try {
    rest.DeleteAll(true);
  } catch (const Error &) { refused = true; }
  CHECK_TRUE("DeleteAll(true) refuses rather than skipping OnDelete", refused);
  DropTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
}

} // namespace

int main() {
  return gate::Run("Navigate", [] {
    try {
      const Session session(AGIRU_TEST_DSN);
      TheWalkCrossesTheFetchBlock();
      AFilterNarrowsWhatTheCursorSelects();
      ACopyWalksItsOwnSet();
      DeleteAllTakesTheFilteredRows();
    } catch (const Error &e) { CHECK_TEXT("the gate needs a database", e.what(), "a database"); }
  });
}

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

/// The type of `Resource Cost."Code"`, TAKEN FROM THE FIELD rather than written out. Its declared
/// length is a property of the AL table and repeating it here is a second place for it to be wrong.
using CodeValue = decltype(ResourceCost::Code);

namespace {

constexpr int kRows = 10;

std::string Named(int i) {
  return std::string("R") + (i < 10 ? "0" : "") + std::to_string(i);
}

// THE PRIMARY KEY IS (Type, Code) AND THE ROWS ARE WRITTEN SO THAT IT DISAGREES WITH BOTH THE
// INSERTION ORDER AND THE Code ORDER. Even rows are Resource (ordinal 0) and odd rows are
// GroupResource (1), so the key walk is R00, R02 ... R08, then R01, R03 ... R09. A reader that
// ordered by insertion, by Code, or by nothing at all is red on the first case.
//
// `Cost Type` runs the other way -- it is LCYExtra for the row the primary key puts FIRST -- so a
// second key over it names a different first row. That is what the negative control needs.
void Fill() {
  DropTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
  CreateTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
  for (int i = 0; i < kRows; ++i) {
    ResourceCost rec;
    rec.Type = i % 2 == 0 ? ResourceCostType::Resource : ResourceCostType::GroupResource;
    rec.Code = CodeValue(Named(i));
    rec.WorkTypeCode = "hours";
    rec.CostType = i == 0 ? ResourceCostCostType::LCYExtra : ResourceCostCostType::Fixed;
    rec.DirectUnitCost = Decimal::FromInvariantString("1.00");
    rec.UnitCost = Decimal::FromInvariantString("2.00");
    rec.Insert();
  }
}

void MinusIsTheFirstRowInKeyOrderAndPlusIsTheLast() {
  Fill();
  ResourceCost first;
  CHECK_TRUE("FindFirst finds a row", static_cast<bool>(first.FindFirst()));
  CHECK_TEXT("and it is the first in KEY order", std::string(first.Code.Value()), "R00");
  CHECK_TRUE("which is the low ordinal of Type", first.Type == ResourceCostType::Resource);

  ResourceCost last;
  CHECK_TRUE("FindLast finds a row", static_cast<bool>(last.FindLast()));
  CHECK_TEXT("and it is the last in KEY order, not the last inserted",
             std::string(last.Code.Value()),
             "R09");
  CHECK_TRUE("which is the high ordinal of Type", last.Type == ResourceCostType::GroupResource);
}

// `-` IS THE SET AND `+` IS ITS END, which is what decides whether `Next` may step afterwards.
void TheFirstRowOpensTheSetAndTheLastOneEndsIt() {
  ResourceCost walking;
  CHECK_TRUE("FindFirst finds the row", static_cast<bool>(walking.FindFirst()));
  CHECK_TRUE("and Next steps on from it", static_cast<int>(walking.Next()) == 1);
  CHECK_TEXT("to the second row in key order", std::string(walking.Code.Value()), "R02");

  ResourceCost ended;
  CHECK_TRUE("FindLast finds the row", static_cast<bool>(ended.FindLast()));
  CHECK_TRUE("and Next answers 0, because that IS the end", static_cast<int>(ended.Next()) == 0);
}

// `=`, `>` and `<` read the key values the record already carries, which is what
// record-find-method.md means by "you must assign value to all fields of the current and primary
// keys before you call FIND".
void EqualGreaterAndLessComparePositionsOnTheSortPath() {
  ResourceCost exact;
  exact.Type = ResourceCostType::Resource;
  exact.Code = CodeValue("R04");
  exact.WorkTypeCode = "hours";
  CHECK_TRUE("'=' finds the row that carries those key values", static_cast<bool>(exact.Find("=")));
  CHECK_TEXT("and reads it", std::string(exact.Code.Value()), "R04");

  ResourceCost after;
  after.Type = ResourceCostType::Resource;
  after.Code = CodeValue("R04");
  after.WorkTypeCode = "hours";
  CHECK_TRUE("'>' finds the next one along the sort path", static_cast<bool>(after.Find(">")));
  CHECK_TEXT("which is the next EVEN code, because Type comes first in the key",
             std::string(after.Code.Value()),
             "R06");

  ResourceCost before;
  before.Type = ResourceCostType::Resource;
  before.Code = CodeValue("R04");
  before.WorkTypeCode = "hours";
  CHECK_TRUE("'<' finds the one before it", static_cast<bool>(before.Find("<")));
  CHECK_TEXT(
      "and it is the LAST smaller one and not the first", std::string(before.Code.Value()), "R02");
}

// The combined form the BaseApp writes 81 times: try equal, then greater, then less.
void ACombinationIsTriedInTheWrittenOrder() {
  ResourceCost missing;
  missing.Type = ResourceCostType::Resource;
  missing.Code = CodeValue("R05");
  missing.WorkTypeCode = "hours";
  CHECK_TRUE("'=><' finds something for a key no row carries",
             static_cast<bool>(missing.Find("=><")));
  CHECK_TEXT("and it is the next one up, because '>' is tried before '<'",
             std::string(missing.Code.Value()),
             "R06");

  ResourceCost past;
  past.Type = ResourceCostType::GroupResource;
  past.Code = CodeValue("ZZZ");
  past.WorkTypeCode = "hours";
  CHECK_TRUE("past the end, '=><' falls through to '<'", static_cast<bool>(past.Find("=><")));
  CHECK_TEXT("and lands on the last row", std::string(past.Code.Value()), "R09");
}

// THE PAGE SAYS `-` AND `+` MAY ONLY BE USED ALONE. A reader that silently took the first character
// would answer something plausible for every one of these.
void MinusAndPlusRefuseToBeCombined() {
  ResourceCost rec;
  std::string said;
  try {
    static_cast<void>(rec.Find("-="));
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("'-=' is refused", said.find("only be used alone") != std::string::npos);
  said.clear();
  try {
    static_cast<void>(rec.Find("x"));
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("and a character the page does not declare is refused too",
             said.find("record-find-method.md") != std::string::npos);
}

// THE NEGATIVE CONTROL. A reader that ordered by the primary key whatever the current key says
// passes every case above, because the primary key is what they all use. This is the one that
// catches it: over the second key, `Cost Type` puts R00 LAST and R01 first.
void TheCurrentKeyDecidesWhichRowIsFirst() {
  // THE RECORDS ARE SCOPED AND THE DROP IS OUTSIDE, because `-` opens a cursor and an open cursor
  // holds the table. That is AL's own lifetime -- a record variable closes its set when it goes --
  // and a gate that dropped the table under a live one would fail on the DROP and say nothing
  // about Find.
  {
    ResourceCost rec;
    rec.SetCurrentKey(rec.CostType, rec.Code);
    CHECK_TRUE("FindFirst over the second key finds a row", static_cast<bool>(rec.FindFirst()));
    CHECK_TEXT("and it is the one the SECOND key puts first, not the primary key's",
               std::string(rec.Code.Value()),
               "R01");
    ResourceCost primary;
    CHECK_TRUE("while the primary key still answers its own",
               static_cast<bool>(primary.FindFirst()));
    CHECK_TEXT("which is a different row", std::string(primary.Code.Value()), "R00");
  }
  DropTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
}

// DETERMINISM IS COMPULSORY, AND A KEY THAT DOES NOT SEPARATE THE ROWS IS WHERE IT BREAKS.
// `record-find-method.md`: "If the current key is not the primary key, several records might have
// the same values in current key fields. If this occurs, the sort order defined by the primary key
// as the search path is used." Every row here carries the same `Work Type Code`, so a sort by that
// alone leaves the order to PostgreSQL -- and the same read twice may answer differently.
void AKeyThatSeparatesNothingStillOrdersByThePrimaryKey() {
  Fill();
  {
    ResourceCost rec;
    rec.SetCurrentKey(rec.WorkTypeCode);
    CHECK_TRUE("the first row is found", static_cast<bool>(rec.FindFirst()));
    CHECK_TEXT("and it is the primary key's first, because the current key ties",
               std::string(rec.Code.Value()),
               "R00");
    ResourceCost last;
    last.SetCurrentKey(last.WorkTypeCode);
    CHECK_TRUE("and the last one is found", static_cast<bool>(last.FindLast()));
    CHECK_TEXT("and it is the primary key's last", std::string(last.Code.Value()), "R09");
  }
  DropTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
}

} // namespace

int main() {
  return gate::Run("Find", [] {
    try {
      const Session session(AGIRU_TEST_DSN);
      MinusIsTheFirstRowInKeyOrderAndPlusIsTheLast();
      TheFirstRowOpensTheSetAndTheLastOneEndsIt();
      EqualGreaterAndLessComparePositionsOnTheSortPath();
      ACombinationIsTriedInTheWrittenOrder();
      MinusAndPlusRefuseToBeCombined();
      TheCurrentKeyDecidesWhichRowIsFirst();
      AKeyThatSeparatesNothingStillOrdersByThePrimaryKey();
    } catch (const Error &e) { CHECK_TEXT("the gate needs a database", e.what(), "a database"); }
  });
}

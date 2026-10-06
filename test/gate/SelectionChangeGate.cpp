#include "runtime/RecordRef.h"
#include "runtime/RecordState.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/TableDefinition.h"
#include "type/Decimal.h"
#include "type/Variant.h"

#include "Check.h"
#include "ResourceCost.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace {

using Cost = agiru::Projects::Resources::Pricing::ResourceCost_Table;
constexpr std::string_view codes = "ABCDEFGH";
constexpr std::size_t kCostGroupRows = 3;

void Fill(Cost &rows) {
  for (std::size_t i = 0; i < codes.size(); ++i) {
    rows.Init();
    rows.Code = codes.substr(i, 1);
    rows.WorkTypeCode = "hours";
    rows.DirectUnitCost = agiru::Decimal::FromInvariantString(i < kCostGroupRows       ? "10"
                                                              : i < kCostGroupRows * 2 ? "20"
                                                                                       : "30");
    rows.Insert();
  }
}

void Begin(Cost &rows) {
  rows.Reset();
  CHECK_TRUE("the source key is selected", rows.SetCurrentKey(rows.Code));
}

template <bool Reflected> class Reader {
public:
  explicit Reader(Cost &row) : row_(row) {
    if constexpr (Reflected) { reference_.GetTable(row_); }
  }

  bool Find(std::string_view which = "-") {
    if constexpr (Reflected) { return reference_.Find(which); }
    return row_.Find(which);
  }

  std::int32_t Next(std::int32_t steps = 1) {
    if constexpr (Reflected) { return reference_.Next(steps); }
    return row_.Next(steps);
  }

  void Range(std::string_view low, std::string_view high) {
    if constexpr (Reflected) {
      reference_.Field(Cost::Field_No::Code.Value())
          .SetRange(agiru::Variant{std::string(low)}, agiru::Variant{std::string(high)});
    } else {
      row_.SetRange(row_.Code, low, high);
    }
  }

  void ClearRange() {
    if constexpr (Reflected) {
      reference_.Field(Cost::Field_No::Code.Value()).SetRange();
    } else {
      row_.SetRange(row_.Code);
    }
  }

  void Descending() {
    if constexpr (Reflected) {
      reference_.Ascending(false);
    } else {
      row_.Ascending(false);
    }
  }

  void SortByValue() {
    if constexpr (Reflected) {
      reference_.SetView("SORTING(Direct Unit Cost) ORDER(Descending)");
    } else {
      CHECK_TRUE("the replacement key is selected", row_.SetCurrentKey(row_.DirectUnitCost));
      row_.Ascending(false);
    }
  }

  void View(std::string_view value) {
    if constexpr (Reflected) {
      reference_.SetView(value);
    } else {
      row_.SetView(value);
    }
  }

  void Position(std::string_view value) {
    if constexpr (Reflected) {
      reference_.SetPosition(value);
    } else {
      row_.SetPosition(value);
    }
  }

  void CostValue(std::string_view value) {
    const auto amount = agiru::Decimal::FromInvariantString(value);
    if constexpr (Reflected) {
      reference_.Field(Cost::Field_No::DirectUnitCost.Value()).Value(agiru::Variant{amount});
    } else {
      row_.DirectUnitCost = amount;
    }
  }

  void CheckCost(std::string_view value) {
    if constexpr (Reflected) { reference_.SetTable(row_); }
    CHECK_TEXT("SetPosition retains nonkeys without fetching the anchor",
               row_.DirectUnitCost.ToInvariantString(),
               value);
  }

  void Check(std::string_view code) {
    if constexpr (Reflected) { reference_.SetTable(row_); }
    CHECK_TEXT("the current selection determines the reached row", row_.Code.Value(), code);
    CHECK_TEXT("the composite primary key remains intact", row_.WorkTypeCode.Value(), "HOURS");
  }

private:
  Cost &row_;
  agiru::RecordRef reference_;
};

template <bool Reflected> void Filters(Cost &row) {
  Begin(row);
  Reader<Reflected> reader(row);
  CHECK_TRUE("Find opens the original selection", reader.Find());
  reader.Check("A");
  reader.Range("E", "G");
  CHECK_TRUE("Next zero after a filter change does not move", reader.Next(0) == 0);
  reader.Check("A");
  CHECK_TRUE("an excluded origin reaches the first admitted successor", reader.Next() == 1);
  reader.Check("E");
  CHECK_TRUE("partial forward movement respects the narrowed endpoint", reader.Next(100) == 2);
  reader.Check("G");
  reader.ClearRange();
  CHECK_TRUE("widening admits the next row", reader.Next() == 1);
  reader.Check("H");
  reader.Range("B", "D");
  CHECK_TRUE("an excluded origin reaches the last admitted predecessor", reader.Next(-1) == -1);
  reader.Check("D");
  reader.Range("F", "G");
  CHECK_TRUE("a missing predecessor reports no movement", reader.Next(-1) == 0);
  reader.Check("D");
  CHECK_TRUE("a failed step still permits the opposite direction", reader.Next() == 1);
  reader.Check("F");
  reader.Range("Z", "Z");
  CHECK_TRUE("an empty selection cannot move forward", reader.Next() == 0);
  CHECK_TRUE("an empty selection cannot move backward", reader.Next(-1) == 0);
  reader.Check("F");
  reader.ClearRange();
  CHECK_TRUE("removing an empty selection restores relative navigation", reader.Next() == 1);
  reader.Check("G");
  reader.Range("A", "C");
  CHECK_TRUE("the forecast-style narrow selection has an endpoint", reader.Find("+"));
  reader.Check("C");
  reader.ClearRange();
  CHECK_TRUE("forecast-style widening continues beyond the old endpoint", reader.Next() == 1);
  reader.Check("D");
}

template <bool Reflected> void Directions(Cost &row) {
  Begin(row);
  Reader<Reflected> reader(row);
  CHECK_TRUE("Find opens the ascending stream", reader.Find());
  CHECK_TRUE("the stream reaches its origin", reader.Next(2) == 2);
  reader.Check("C");
  reader.Descending();
  CHECK_TRUE("global reversal changes subsequent Next", reader.Next() == 1);
  reader.Check("B");
  CHECK_TRUE("global reversal changes negative Next", reader.Next(-1) == -1);
  reader.Check("C");
  reader.View("ORDER(Ascending)");
  CHECK_TRUE("an ORDER-only view replaces the active cursor selection", reader.Next() == 1);
  reader.Check("D");
  reader.SortByValue();
  CHECK_TRUE("replacement key and direction apply to the next step", reader.Next() == 1);
  reader.Check("C");
  reader.View("WHERE(Code=FILTER(F..H))");
  CHECK_TRUE("a WHERE-only view replaces the active selection", reader.Next() == 1);
  reader.Check("F");
}

template <bool Reflected> void Positions(Cost &row) {
  Begin(row);
  Reader<Reflected> reader(row);
  reader.Range("B", "G");
  CHECK_TRUE("position assignment starts with an existing cursor", reader.Find());
  reader.Check("B");
  reader.CostValue("777.1234");
  reader.Position("Field1=0(0),Field2=0(E),Field3=0(HOURS)");
  reader.Check("E");
  reader.CheckCost("777.1234");
  CHECK_TRUE("Next follows the assigned primary key, not the old SQL or temporary cursor",
             reader.Next() == 1);
  reader.Check("F");
  reader.Position("Field1=0(0),Field2=0(FF),Field3=0(HOURS)");
  CHECK_TRUE("a position without a stored row resumes relative navigation", reader.Next() == 1);
  reader.Check("G");
  reader.Position("Field1=0(0),Field2=0(H),Field3=0(HOURS)");
  CHECK_TRUE("positioning outside the filter preserves its upper bound", reader.Next() == 0);
  CHECK_TRUE("backward navigation still applies the original filter", reader.Next(-1) == -1);
  reader.Check("G");
}

void TypedChanges(Cost &row) {
  Begin(row);
  CHECK_TRUE("the field-direction walk starts", row.FindFirst());
  CHECK_TRUE("the field-direction origin is reached", row.Next(2) == 2);
  row.SetAscending(row.Code, false);
  CHECK_TRUE("field direction changes invalidate the old stream", row.Next() == 1);
  CHECK_TEXT("field reversal reaches its new successor", row.Code.Value(), "B");
  Begin(row);
  CHECK_TRUE("the copied-filter walk starts", row.FindFirst());
  Cost source;
  source.SetRange(source.Code, "E", "G");
  row.CopyFilters(source);
  CHECK_TRUE("CopyFilters affects subsequent Next", row.Next() == 1);
  CHECK_TEXT("copied filters admit the first row", row.Code.Value(), "E");
  source.SetRange(source.Code, "G", "H");
  source.CopyFilter(source.Code, row, row.Code);
  CHECK_TRUE("CopyFilter affects subsequent Next", row.Next() == 1);
  CHECK_TEXT("copied field filters skip excluded rows", row.Code.Value(), "G");
}

void MarksAndNoOps(Cost &row) {
  Begin(row);
  row.SetRange(row.Code, "A", "H");
  CHECK_TRUE("the stable selection opens", row.FindFirst());
  const auto *cursor = row.State_Block.Peek()->open.Held();
  row.SetRange(row.Code, "A", "H");
  row.Ascending(true);
  row.SetAscending(row.Code, true);
  row.MarkedOnly(false);
  row.Mark(true);
  row.Mark(false);
  row.ClearMarks();
  CHECK_TRUE("unchanged predicates and inactive marks keep the cursor",
             row.State_Block.Peek()->open.Held() == cursor);
  CHECK_TRUE("the unchanged selection still advances", row.Next() == 1);
  CHECK_TEXT("the unchanged selection reaches its successor", row.Code.Value(), "B");
  Begin(row);
  CHECK_TRUE("marking begins with the first row", row.FindFirst());
  row.Mark(true);
  CHECK_TRUE("the second mark can be reached", row.Next() == 1);
  row.Mark(true);
  CHECK_TRUE("the third mark can be reached", row.Next() == 1);
  row.Mark(true);
  row.MarkedOnly(true);
  CHECK_TRUE("the marked selection opens", row.FindFirst());
  row.Code = "B";
  row.Mark(false);
  row.Code = "E";
  row.Mark(true);
  row.Code = "A";
  CHECK_TRUE("same-cardinality mark replacement changes the selection", row.Next() == 1);
  CHECK_TEXT("an unmarked cached row is skipped", row.Code.Value(), "C");
  row.ClearMarks();
  CHECK_TRUE("clearing active marks empties the current selection", row.Next() == 0);
  CHECK_TEXT("empty marked selection preserves the origin", row.Code.Value(), "C");
  row.MarkedOnly(false);
  CHECK_TRUE("disabling marked-only resumes relative navigation", row.Next() == 1);
  CHECK_TEXT("the ordinary selection resumes after the origin", row.Code.Value(), "D");
}

void SharedTemporaryChanges(agiru::Temporary<Cost> &row) {
  Begin(row);
  row.SetRange(row.DirectUnitCost, agiru::Decimal::FromInvariantString("10"));
  CHECK_TRUE("the filtered temporary walk starts", row.FindFirst());
  agiru::Temporary<Cost> alias;
  alias.Copy(row, true);
  CHECK_TRUE("the shared future row is available", alias.Get(alias.Type, "B", "hours"));
  alias.DirectUnitCost = agiru::Decimal::FromInvariantString("99");
  alias.Modify();
  CHECK_TRUE("a modified future row is rechecked against the filter", row.Next() == 1);
  CHECK_TEXT("modified rows that no longer match are skipped", row.Code.Value(), "C");
  alias.DirectUnitCost = agiru::Decimal::FromInvariantString("10");
  alias.Modify();
  CHECK_TRUE("another future row is available", alias.Get(alias.Type, "D", "hours"));
  alias.DirectUnitCost = agiru::Decimal::FromInvariantString("10");
  alias.Modify();
  CHECK_TRUE("a newly admitted future row is visible", row.Next() == 1);
  CHECK_TEXT("the shared modification adds the next admitted row", row.Code.Value(), "D");
  alias.DirectUnitCost = agiru::Decimal::FromInvariantString("20");
  alias.Modify();
}

void Variants(Cost &row) {
  Filters<false>(row);
  Filters<true>(row);
  Directions<false>(row);
  Directions<true>(row);
  Positions<false>(row);
  Positions<true>(row);
  TypedChanges(row);
  MarksAndNoOps(row);
}

}

int main() {
  return gate::Run("SelectionChange", [] {
    agiru::Temporary<Cost> temporary;
    Fill(temporary);
    Variants(temporary);
    SharedTemporaryChanges(temporary);
    const agiru::Session session(AGIRU_TEST_DSN);
    const auto &table = agiru::TableDefinition<Cost>();
    agiru::DropTable(agiru::Session::Current().Database(), table);
    agiru::CreateTable(agiru::Session::Current().Database(), table);
    Cost stored;
    Fill(stored);
    Variants(stored);
  });
}

#include "meta/Ids.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/TableDefinition.h"
#include "type/Decimal.h"
#include "type/Variant.h"

#include "Check.h"
#include "Cursor.h"
#include "RecordChanges.h"
#include "ResourceCost.h"
#include "SessionState.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {

using Cost = agiru::Projects::Resources::Pricing::ResourceCost_Table;
constexpr std::size_t kRows = agiru::detail::kFetchBlock * 2 + 2;
constexpr std::size_t kFirstCode = 1000;
constexpr std::size_t kCodeStride = 2;
constexpr std::int32_t kOriginalCost = 10;
constexpr std::int32_t kChangedCost = 20;
constexpr std::int32_t kTemporaryCost = 30;
constexpr std::int32_t kFirstSyntheticTable = 61000;
constexpr std::int32_t kSyntheticVisits = 1000;
constexpr std::size_t kGroups = 3;

struct Order {
  bool mixed;
  bool ascending;
  bool backwards;
};

std::vector<std::size_t> Sequence(Order order) {
  std::vector<std::size_t> result;
  result.reserve(kRows);
  for (std::size_t index = 0; index < kRows; ++index) { result.push_back(index); }
  std::ranges::sort(result, [order](std::size_t left, std::size_t right) {
    if (order.mixed && left % kGroups != right % kGroups) {
      return left % kGroups < right % kGroups;
    }
    return order.mixed ? left > right : left < right;
  });
  if (!order.ascending) { std::ranges::reverse(result); }
  if (order.backwards) {
    std::ranges::reverse(result);
    result.erase(result.begin());
  }
  return result;
}

std::string Code(std::size_t index) {
  return std::to_string(kFirstCode + index * kCodeStride);
}

std::string GapCode(std::size_t index, bool ascending = true) {
  const auto value = kFirstCode + index * kCodeStride;
  return std::to_string(ascending ? value - 1 : value + 1);
}

void Fill(bool missingFromFilter, std::size_t target) {
  const auto &table = agiru::TableDefinition<Cost>();
  const auto &connection = agiru::Session::Current().Database();
  agiru::DropTable(connection, table);
  agiru::CreateTable(connection, table);
  Cost row;
  for (std::size_t index = 0; index < kRows; ++index) {
    row.Init();
    row.Code = Code(index);
    row.WorkTypeCode = "hours";
    row.DirectUnitCost = missingFromFilter && index == target ? kChangedCost : kOriginalCost;
    row.UnitCost = agiru::Decimal{static_cast<std::int32_t>(index % kGroups)};
    row.Insert();
  }
}

enum class Change { Modify, Insert, Delete, Rename, ModifyAll, DeleteAll, Exclude, Admit };

void Write(Change change, std::size_t target, bool ascending) {
  Cost writer;
  CHECK_TRUE("the changed future identity exists", writer.Get(writer.Type, Code(target), "hours"));
  switch (change) {
    case Change::Modify:
    case Change::Exclude:
    case Change::Admit:
      writer.DirectUnitCost = change == Change::Admit ? kOriginalCost : kChangedCost;
      writer.Modify();
      break;
    case Change::Insert:
      writer.Code = GapCode(target, ascending);
      writer.DirectUnitCost = kChangedCost;
      writer.Insert();
      break;
    case Change::Delete: writer.Delete(); break;
    case Change::Rename:
      writer.Rename(writer.Type, GapCode(target, ascending), writer.WorkTypeCode);
      break;
    case Change::ModifyAll:
      writer.SetRange(writer.Code, Code(target), Code(kRows - 1));
      writer.ModifyAll(writer.DirectUnitCost, agiru::Decimal{kChangedCost});
      break;
    case Change::DeleteAll:
      writer.SetRange(writer.Code, Code(target));
      writer.DeleteAll();
      break;
  }
}

class Reader {
public:
  Reader(Cost &row, bool reflected) : row_(row), reflected_(reflected) {
    if (reflected_) { reference_.GetTable(row_); }
  }

  bool FindSet() { return reflected_ ? reference_.FindSet() : row_.FindSet(); }

  bool FindLast() { return reflected_ ? reference_.FindLast() : row_.FindLast(); }

  std::int32_t Next(std::int32_t steps) {
    return reflected_ ? reference_.Next(steps) : row_.Next(steps);
  }

  void RefreshFrame() {
    if (reflected_) { reference_.SetTable(row_); }
  }

  void Modify() {
    if (reflected_) {
      reference_.Field(Cost::Field_No::DirectUnitCost.Value()).Value(agiru::Decimal{kChangedCost});
      reference_.Modify();
    } else {
      row_.DirectUnitCost = kChangedCost;
      row_.Modify();
    }
  }

  void Delete() {
    if (reflected_) {
      reference_.Delete();
    } else {
      row_.Delete();
    }
  }

  void Rename(std::string_view code) {
    const auto newCode = decltype(row_.Code){code};
    if (reflected_) {
      reference_.Rename(agiru::Variant(row_.Type.AsInteger()), newCode, row_.WorkTypeCode);
    } else {
      row_.Rename(row_.Type, newCode, row_.WorkTypeCode);
    }
  }

private:
  Cost &row_;
  bool reflected_;
  agiru::RecordRef reference_;
};

void DynamicWrite(bool reflected, Change change, std::size_t steps, Order order) {
  const agiru::Session session(AGIRU_TEST_DSN);
  const auto sequence = Sequence(order);
  const auto target = sequence[steps];
  Fill(change == Change::Admit, target);
  Cost row;
  CHECK_TRUE("the reader selects its declared order",
             order.mixed ? row.SetCurrentKey(row.UnitCost, row.Code) : row.SetCurrentKey(row.Code));
  if (order.mixed) { row.SetAscending(row.Code, false); }
  row.Ascending(order.ascending);
  if (change == Change::Exclude || change == Change::Admit) {
    row.SetRange(row.DirectUnitCost, agiru::Decimal{kOriginalCost});
  }
  Reader reader(row, reflected);
  CHECK_TRUE("the result set opens before the mutation",
             order.backwards ? reader.FindLast() : reader.FindSet());
  if (order.backwards) {
    CHECK_TRUE("a reverse cursor opens before the write", reader.Next(-1) == -1);
  }
  const bool codeAscending = ((!order.mixed) == order.ascending) != order.backwards;
  Write(change, target, codeAscending);
  CHECK_TRUE("zero does not consume the changed result", reader.Next(0) == 0);
  reader.RefreshFrame();
  CHECK_TEXT("the current buffer stays at its anchor", row.Code.Value(), Code(sequence.front()));
  const auto requested = static_cast<std::int32_t>(steps) * (order.backwards ? -1 : 1);
  CHECK_TRUE("the dynamic result retains the requested movement",
             reader.Next(requested) == requested);
  reader.RefreshFrame();
  const bool skipped =
      change == Change::Delete || change == Change::DeleteAll || change == Change::Exclude;
  const bool newIdentity = change == Change::Insert || change == Change::Rename;
  const std::string expected =
      newIdentity ? GapCode(target, codeAscending) : Code(sequence[steps + (skipped ? 1 : 0)]);
  CHECK_TEXT("Next reads the live future identity, not the old portal", row.Code.Value(), expected);
  CHECK_TEXT("the composite key stays exact", row.WorkTypeCode.Value(), "HOURS");
  const bool modified =
      change == Change::Modify || change == Change::ModifyAll || change == Change::Insert;
  CHECK_TRUE("Next reads the live exact amount",
             row.DirectUnitCost == agiru::Decimal{modified ? kChangedCost : kOriginalCost});
  CHECK_TRUE("the remaining result still supports reverse movement",
             reader.Next(-requested) == -requested);
  reader.RefreshFrame();
  CHECK_TEXT("reverse returns to the original anchor", row.Code.Value(), Code(sequence.front()));
}

void OwnCursorWrite(bool reflected, Change change) {
  const agiru::Session session(AGIRU_TEST_DSN);
  Fill(false, 0);
  Cost row;
  CHECK_TRUE("the own-write reader selects its key", row.SetCurrentKey(row.Code));
  Reader reader(row, reflected);
  CHECK_TRUE("the own-write result opens", reader.FindSet());
  reader.RefreshFrame();
  const auto systemId = row.SystemId;
  switch (change) {
    case Change::Modify: reader.Modify(); break;
    case Change::Delete: reader.Delete(); break;
    case Change::Rename: reader.Rename(GapCode(agiru::detail::kFetchBlock)); break;
    default: return;
  }
  CHECK_TRUE("zero preserves the written frame", reader.Next(0) == 0);
  reader.RefreshFrame();
  const bool renamed = change == Change::Rename;
  CHECK_TEXT("the modified frame retains its primary key",
             row.Code.Value(),
             renamed ? GapCode(agiru::detail::kFetchBlock) : Code(0));
  CHECK_TRUE("own writes retain the original system identity", row.SystemId == systemId);
  CHECK_TRUE("own modification preserves its exact frame value",
             row.DirectUnitCost ==
                 agiru::Decimal{change == Change::Modify ? kChangedCost : kOriginalCost});
  CHECK_TRUE("own writes resume from the current key", reader.Next(1) == 1);
  reader.RefreshFrame();
  CHECK_TEXT("rename does not resume from the old buffered ordinal",
             row.Code.Value(),
             Code(renamed ? agiru::detail::kFetchBlock : 1));
  CHECK_TRUE("the successor carries its own amount",
             row.DirectUnitCost == agiru::Decimal{kOriginalCost});
}

void RevisionIsolationAndRetirement() {
  const agiru::Session outer(AGIRU_TEST_DSN);
  const auto &connection = agiru::Session::Current().Database();
  constexpr agiru::TableId unrelated{kFirstSyntheticTable};
  {
    const agiru::detail::RecordRead read(Cost::kId);
    const agiru::detail::RecordRead other(unrelated);
    const auto &owner = agiru::detail::SessionState::Current().recordChanges;
    CHECK_TRUE("only actively observed tables consume revision storage",
               owner->ActiveTables() == 2);
    {
      const agiru::Session inner(AGIRU_TEST_DSN);
      const agiru::detail::RecordRead nested(Cost::kId);
      agiru::detail::RecordWritten(connection, Cost::kId);
      CHECK_TRUE("writes on another connection do not invalidate this session", nested.Current());
      agiru::detail::RecordWritten(agiru::Session::Current().Database(), Cost::kId);
      CHECK_TRUE("the nested session observes its own writes", !nested.Current());
    }
    CHECK_TRUE("another session's invalidation does not leak", read.Current());
    agiru::detail::RecordWritten(connection, Cost::kId);
    CHECK_TRUE("a matching session/table write changes the read revision", !read.Current());
    CHECK_TRUE("another table retains its buffered result", other.Current());
  }
  const auto &owner = agiru::detail::SessionState::Current().recordChanges;
  CHECK_TRUE("the final readers release every table counter", owner->ActiveTables() == 0);
  for (std::int32_t id = kFirstSyntheticTable; id < kFirstSyntheticTable + kSyntheticVisits; ++id) {
    {
      const agiru::detail::RecordRead read(agiru::TableId{id});
      CHECK_TRUE("historical table visits do not accumulate", owner->ActiveTables() == 1);
    }
    CHECK_TRUE("each ended observation retires its storage", owner->ActiveTables() == 0);
  }
}

void FailedAndTemporaryWrites() {
  const agiru::Session session(AGIRU_TEST_DSN);
  Fill(false, 0);
  const agiru::detail::RecordRead read(Cost::kId);
  Cost writer;
  CHECK_TRUE("the duplicate identity exists", writer.Get(writer.Type, Code(1), "hours"));
  CHECK_TRUE("duplicate insertion reports failure", !writer.Ok_Insert());
  CHECK_TRUE("failed insertion keeps the revision", read.Current());
  writer.Code = "absent";
  CHECK_TRUE("missing modification reports failure",
             !agiru::detail::RuntimeModify(&writer, agiru::TableDefinition<Cost>()));
  CHECK_TRUE("missing deletion reports failure",
             !agiru::detail::RuntimeDelete(&writer, agiru::TableDefinition<Cost>()));
  writer.SetRange(writer.Code, "absent");
  writer.DeleteAll();
  CHECK_TRUE("zero-row writes keep the revision", read.Current());
  agiru::Temporary<Cost> temporary;
  temporary.Code = "local";
  temporary.Insert();
  temporary.DirectUnitCost = kTemporaryCost;
  temporary.Modify();
  temporary.Delete();
  CHECK_TRUE("temporary writes do not invalidate SQL readers", read.Current());
}

void ReadOutlivesSession() {
  std::unique_ptr<agiru::detail::RecordRead> read;
  {
    const agiru::Session session(AGIRU_TEST_DSN);
    read = std::make_unique<agiru::detail::RecordRead>(Cost::kId);
  }
  CHECK_TRUE("a retained token owns its revision after session teardown", read->Current());
  read.reset();
}

}

int main() {
  return gate::Run("DynamicRecord", [] {
    RevisionIsolationAndRetirement();
    FailedAndTemporaryWrites();
    ReadOutlivesSession();
    constexpr std::array changes{Change::Modify,
                                 Change::Insert,
                                 Change::Delete,
                                 Change::Rename,
                                 Change::ModifyAll,
                                 Change::DeleteAll,
                                 Change::Exclude,
                                 Change::Admit};
    constexpr std::array<std::size_t, 2> steps{1, agiru::detail::kFetchBlock};
    constexpr std::array orders{Order{.mixed = false, .ascending = true, .backwards = false},
                                Order{.mixed = false, .ascending = false, .backwards = false},
                                Order{.mixed = true, .ascending = true, .backwards = false},
                                Order{.mixed = true, .ascending = false, .backwards = false},
                                Order{.mixed = false, .ascending = true, .backwards = true},
                                Order{.mixed = false, .ascending = false, .backwards = true},
                                Order{.mixed = true, .ascending = true, .backwards = true},
                                Order{.mixed = true, .ascending = false, .backwards = true}};
    for (const auto change : changes) {
      for (const auto count : steps) {
        for (const auto order : orders) {
          DynamicWrite(false, change, count, order);
          DynamicWrite(true, change, count, order);
        }
      }
    }
    for (const auto change : {Change::Modify, Change::Delete, Change::Rename}) {
      OwnCursorWrite(false, change);
      OwnCursorWrite(true, change);
    }
  });
}

#include <algorithm>

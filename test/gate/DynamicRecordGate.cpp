#include "meta/Ids.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/TableDefinition.h"
#include "type/Decimal.h"

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

std::string Code(std::size_t index) {
  return std::to_string(kFirstCode + index * kCodeStride);
}

std::string GapCode(std::size_t index) {
  return std::to_string(kFirstCode + index * kCodeStride - 1);
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
    row.Insert();
  }
}

enum class Change { Modify, Insert, Delete, Rename, ModifyAll, DeleteAll, Exclude, Admit };

void Write(Change change, std::size_t target) {
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
      writer.Code = GapCode(target);
      writer.DirectUnitCost = kChangedCost;
      writer.Insert();
      break;
    case Change::Delete: writer.Delete(); break;
    case Change::Rename: writer.Rename(writer.Type, GapCode(target), writer.WorkTypeCode); break;
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

void DynamicWrite(bool reflected, Change change, std::size_t steps) {
  const agiru::Session session(AGIRU_TEST_DSN);
  Fill(change == Change::Admit, steps);
  Cost row;
  CHECK_TRUE("the reader selects its code order", row.SetCurrentKey(row.Code));
  if (change == Change::Exclude || change == Change::Admit) {
    row.SetRange(row.DirectUnitCost, agiru::Decimal{kOriginalCost});
  }
  agiru::RecordRef reference;
  if (reflected) { reference.GetTable(row); }
  CHECK_TRUE("the result set opens before the mutation",
             reflected ? reference.FindSet() : row.FindSet());
  Write(change, steps);
  CHECK_TRUE("zero does not consume the changed result",
             (reflected ? reference.Next(0) : row.Next(0)) == 0);
  const auto requested = static_cast<std::int32_t>(steps);
  CHECK_TRUE("the dynamic result retains the requested movement",
             (reflected ? reference.Next(requested) : row.Next(requested)) == requested);
  if (reflected) { reference.SetTable(row); }
  const bool skipped =
      change == Change::Delete || change == Change::DeleteAll || change == Change::Exclude;
  const bool newIdentity = change == Change::Insert || change == Change::Rename;
  const std::string expected = newIdentity ? GapCode(steps) : Code(steps + (skipped ? 1 : 0));
  CHECK_TEXT("Next reads the live future identity, not the old portal", row.Code.Value(), expected);
  CHECK_TEXT("the composite key stays exact", row.WorkTypeCode.Value(), "HOURS");
  const bool modified =
      change == Change::Modify || change == Change::ModifyAll || change == Change::Insert;
  CHECK_TRUE("Next reads the live exact amount",
             row.DirectUnitCost == agiru::Decimal{modified ? kChangedCost : kOriginalCost});
  CHECK_TRUE("the remaining result still supports reverse movement",
             (reflected ? reference.Next(-requested) : row.Next(-requested)) == -requested);
  if (reflected) { reference.SetTable(row); }
  CHECK_TEXT("reverse returns to the original anchor", row.Code.Value(), Code(0));
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
    for (const auto change : changes) {
      for (const auto count : steps) {
        DynamicWrite(false, change, count);
        DynamicWrite(true, change, count);
      }
    }
  });
}

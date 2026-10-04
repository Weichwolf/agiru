#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/Transaction.h"
#include "type/Integer.h"

#include "Check.h"
#include "fixture/codeunit/RunCallForms.h"
#include "fixture/codeunit/RunWriter.h"
#include "fixture/table/RunBuffer.h"
#include "fixture/table/RunRow.h"

#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace {
using Row = agiru::Fixture::RunRow_Table;
using Buffer = agiru::Fixture::RunBuffer_Table;
using Writer = agiru::Fixture::RunWriter_Codeunit;
using Forms = agiru::Fixture::RunCallForms_Codeunit;
constexpr agiru::Integer kWriter = 50261;
constexpr agiru::Integer kSourceSave = 11;
constexpr agiru::Integer kCopySave = 22;
constexpr agiru::Integer kConsumedID = 31;
constexpr agiru::Integer kConsumedValue = 7;
constexpr agiru::Integer kStatementID = 41;
constexpr agiru::Integer kStatementValue = 9;
constexpr agiru::Integer kCalleeSave = 55;
constexpr agiru::Integer kCalleeID = 51;
constexpr agiru::Integer kHandleID = 52;
constexpr agiru::Integer kNestedID = 61;
constexpr agiru::Integer kNestedValue = 5;
constexpr agiru::Integer kNestedLastSave = 200;
constexpr agiru::Integer kCursorFirstID = 71;
constexpr agiru::Integer kCursorSecondID = 72;
static_assert(std::is_standard_layout_v<Row>);

void CopyIsNotVarPassing() {
  Row source;
  source.Save(kSourceSave);
  Row copy(source);
  CHECK_TRUE("ordinary copies start with their own table globals", copy.SavedCount() == 0);
  copy.Save(kCopySave);
  copy = source;
  CHECK_TRUE("ordinary assignment retains the destination's table globals",
             copy.SavedCount() == 1 && copy.SavedValue(1) == kCopySave);
  CHECK_TRUE("ordinary assignment does not change the source's table globals",
             source.SavedCount() == 1 && source.SavedValue(1) == kSourceSave);
}

template <typename Call> void Consumed(Call call) {
  Buffer stored;
  stored.DeleteAll();
  Row caller;
  caller.ID = kConsumedID;
  caller.Value = -kConsumedValue;
  CHECK_TRUE("a consumed Codeunit.Run catches the original blank error", !call(caller));
  CHECK_TEXT("the original blank error is retained", agiru::GetLastErrorText(), "");
  CHECK_TRUE("SQL writes before the failed run roll back", stored.Count() == 0);
  CHECK_TRUE("table globals saved before rollback survive in the caller",
             caller.SavedCount() == 1 && caller.SavedValue(1) == -kConsumedValue);
  caller.Restore();
  stored.Get(1);
  CHECK_TRUE("the caller restores its saved rows to SQL after rollback",
             stored.Count() == 1 && stored.Value == -kConsumedValue);
  stored.DeleteAll();
  caller.Value = kConsumedValue;
  CHECK_TRUE("a successful consumed call succeeds", call(caller));
  CHECK_TRUE("the successful run writes fields back", caller.Value == kConsumedValue + 1);
  CHECK_TRUE("successful runs use the same caller's table globals",
             caller.SavedCount() == 2 && caller.SavedValue(2) == kConsumedValue);
  CHECK_TRUE("successful SQL writes stay inside the enclosing test boundary", stored.Count() == 1);
}

template <typename Call> void Statement(Call call) {
  Buffer stored;
  stored.DeleteAll();
  Row caller;
  caller.ID = kStatementID;
  caller.Value = -kStatementValue;
  bool raised = false;
  try {
    call(caller);
  } catch (const agiru::Error &error) { raised = std::string_view(error.what()).empty(); }
  CHECK_TRUE("discarded Codeunit.Run propagates its original blank error", raised);
  CHECK_TRUE("discarded errors roll back their SQL writes", stored.Count() == 0);
  CHECK_TRUE("discarded errors do not destroy the caller's saved table globals",
             caller.SavedCount() == 1 && caller.SavedValue(1) == -kStatementValue);
  caller.Value = kStatementValue;
  call(caller);
  CHECK_TRUE("successful statements write fields back", caller.Value == kStatementValue + 1);
  CHECK_TRUE("successful statements preserve the shared table globals",
             caller.SavedCount() == 2 && caller.SavedValue(2) == kStatementValue);
}

void CalleeRestoration() {
  Buffer stored;
  stored.DeleteAll();
  Writer writer;
  writer.Rec.Save(kCalleeSave);
  Row caller;
  caller.ID = kCalleeID;
  caller.Value = -3;
  CHECK_TRUE("the typed callee reports its failed run", !writer.Ok_Run(caller));
  CHECK_TRUE("a failed call restores the callee's original globals",
             writer.Rec.SavedCount() == 1 && writer.Rec.SavedValue(1) == kCalleeSave);
  CHECK_TRUE("the failed caller keeps its separate saved value", caller.SavedValue(1) == -3);
  caller.Value = 3;
  CHECK_TRUE("the same typed callee can run again", writer.Ok_Run(caller));
  CHECK_TRUE("success also restores the callee's original globals",
             writer.Rec.SavedCount() == 1 && writer.Rec.SavedValue(1) == kCalleeSave);
  agiru::Instance<Row> held;
  held->ID = kHandleID;
  held->Value = -4;
  CHECK_TRUE("record handles use the same failed-run contract", !writer.Ok_Run(held));
  CHECK_TRUE("record handles receive their saved table globals", held->SavedValue(1) == -4);
}

void NestedAndCursor() {
  Buffer stored;
  stored.DeleteAll();
  Forms forms;
  Row caller;
  caller.ID = kNestedID;
  caller.Value = -kNestedValue;
  CHECK_TRUE("an outer run can handle an inner failed run", forms.Nested(caller));
  CHECK_TRUE("nested runs retain all three saves in the original caller",
             caller.SavedCount() == 3 && caller.SavedValue(1) == 100 &&
                 caller.SavedValue(2) == -kNestedValue && caller.SavedValue(3) == kNestedLastSave);
  CHECK_TRUE("the inner SQL writes still roll back", stored.Count() == 0);
  agiru::Temporary<Row> rows;
  rows.ID = kCursorFirstID;
  rows.Value = 1;
  rows.Insert();
  rows.ID = kCursorSecondID;
  rows.Value = 2;
  rows.Insert();
  rows.SetRange(rows.Value, 1, 2);
  CHECK_TRUE("the caller owns a positioned temporary cursor", rows.FindSet());
  CHECK_TRUE("a run on a temporary record succeeds", forms.Typed(rows));
  CHECK_TRUE("the caller retains its filters", rows.GetFilter(rows.Value) == "1..2");
  CHECK_TRUE("the caller retains its next-row cursor",
             rows.Next() == 1 && rows.ID == kCursorSecondID);
  CHECK_TRUE("temporary callers retain their own saved globals", rows.SavedValue(1) == 1);
}
}

int main(int argc, char **argv) {
  return gate::Run("Generated Codeunit Record", [argc, argv] {
    if (argc == 3 && std::string_view(argv[2]) == "--copy-only") {
      CopyIsNotVarPassing();
      return;
    }
    if (argc != 2) { throw std::runtime_error("expected dedicated gate database DSN"); }
    const agiru::Session session(argv[1]);
    const agiru::detail::Scope isolation;
    agiru::CreateTable(session.Database(), agiru::TableTraits<Buffer>::kTable);
    CopyIsNotVarPassing();
    Forms forms;
    Consumed([&](Row &row) { return forms.Typed(row); });
    Consumed([&](Row &row) { return forms.Static(row); });
    Consumed([&](Row &row) { return forms.Dynamic(row, kWriter); });
    Statement([&](Row &row) { forms.Statement(row); });
    Statement([&](Row &row) { forms.StaticStatement(row); });
    Statement([&](Row &row) { forms.DynamicStatement(row, kWriter); });
    CalleeRestoration();
    NestedAndCursor();
  });
}

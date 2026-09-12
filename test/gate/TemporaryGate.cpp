#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/Table.h"
#include "type/Decimal.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include "Check.h"
#include "LineNumberBuffer.h"
#include "ResourceCost.h"

#include <exception>
#include <string>
#include <vector>

using agiru::Decimal;
using agiru::Error;
using agiru::Temporary;
using agiru::app::tables::LineNumberBuffer;
using agiru::app::tables::ResourceCost;
using agiru::app::tables::ResourceCostType;

namespace {

constexpr agiru::Integer kTens = 10;

Temporary<LineNumberBuffer> With(const std::vector<agiru::Integer> &numbers) {
  Temporary<LineNumberBuffer> buffer;
  for (const agiru::Integer n : numbers) {
    buffer.OldLineNumber = n;
    buffer.NewLineNumber = n * kTens;
    buffer.Insert();
  }
  return buffer;
}

/// A KEY IS ORDERED BY ITS TYPE AND NEVER BY ITS TEXT, and this case is the one that catches the
/// difference: rendered as strings, 10 sorts before 9. Every buffer keyed on an entry number walks
/// in this order, and both orders look right until the numbers reach ten.
void RowsWalkInPrimaryKeyOrder() {
  // Out of order, and crossing ten twice: rendered as strings, 10 sorts before 9 and 100 before
  // 2. Nothing smaller than this catches a lexical comparison.
  constexpr agiru::Integer kNine = 9;
  constexpr agiru::Integer kHundred = 100;
  Temporary<LineNumberBuffer> buffer = With({kNine, kTens, 1, kHundred, 2});

  std::string walked;
  for (bool more = buffer.FindSet(); more; more = buffer.Next() != 0) {
    walked += std::to_string(buffer.OldLineNumber) + " ";
  }
  CHECK_TEXT("the walk is numeric and not lexical", walked, "1 2 9 10 100 ");
  CHECK_TRUE("and every row came back", buffer.Count() == 5);
}

/// AL refuses a duplicate primary key on a temporary record exactly as on a real one.
void ADuplicateKeyIsRefused() {
  constexpr agiru::Integer kOther = 999;
  Temporary<LineNumberBuffer> buffer = With({1});
  buffer.OldLineNumber = 1;
  buffer.NewLineNumber = kOther;

  std::string said;
  try {
    buffer.Insert();
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("inserting the same key twice refuses", !said.empty());
  CHECK_TRUE("and the store still holds one row", buffer.Count() == 1);
  CHECK_TRUE("with BC's own wording",
             said.find("already exists. Identification fields") != std::string::npos);
  // THE VALUE FORM ANSWERS FALSE INSTEAD, `record-insert--method.md`: "No run-time error occurs
  // if customer 1120 already exists."
  CHECK_TRUE("the value form answers false", !buffer.Ok_Insert());
  CHECK_TRUE("and writes nothing", buffer.Count() == 1);

  // THE NEGATIVE CONTROL. A store that refused every insert would pass the check above.
  buffer.OldLineNumber = 2;
  said.clear();
  try {
    buffer.Insert();
  } catch (const Error &e) { said = e.what(); }
  CHECK_SILENT("a different key inserts", said);
  CHECK_TRUE("and the store grows", buffer.Count() == 2);
}

constexpr agiru::Integer kReplacement = 222;

void GetsRowFinds() {
  Temporary<LineNumberBuffer> buffer = With({1, 2, 3});

  CHECK_TRUE("Get finds a row that is there", buffer.Get(2));
  CHECK_TRUE("and brings its other fields with it", buffer.NewLineNumber == 20);
  CHECK_TRUE("Get on a key that is not there answers false", !buffer.Get(4));

  (void)buffer.Get(2);
  buffer.NewLineNumber = kReplacement;
  CHECK_TRUE("Modify reports the row it replaced", buffer.Modify());
  (void)buffer.Get(2);
  CHECK_TRUE("and the replacement is what comes back", buffer.NewLineNumber == kReplacement);
  CHECK_TRUE("while the rows around it are untouched", buffer.Count() == 3);

  (void)buffer.Get(2);
  CHECK_TRUE("Delete reports the row it removed", buffer.Delete());
  CHECK_TRUE("the store is one shorter", buffer.Count() == 2);
  CHECK_TRUE("and the row is gone", !buffer.Get(2));
  // THE STATEMENT FORM RAISES, as AL's does: `Rec.Delete();` on a row that is not there is a
  // runtime error, and only `if Rec.Delete() then` answers false (board:0035 names the contexts).
  std::string said;
  try {
    static_cast<void>(buffer.Delete());
  } catch (const Error &e) { said = e.what(); }
  CHECK_TRUE("Delete on a key that is not there refuses, naming the key",
             said.find('2') != std::string::npos);
}

/// AL `Copy(From, true)` makes two variables share ONE set of rows. It is why the version rides on
/// the store rather than on the record, and it is how a codeunit hands its buffer out.
constexpr agiru::Integer kSeven = 7;

void CopyingWithShareGivesOneStoreAndWithoutGivesTwo() {
  Temporary<LineNumberBuffer> owner = With({1, 2});

  Temporary<LineNumberBuffer> shared;
  shared.Copy(owner, true);
  CHECK_TRUE("a shared copy sees the rows", shared.Count() == 2);
  owner.OldLineNumber = 3;
  owner.NewLineNumber = 3 * kTens;
  owner.Insert();
  CHECK_TRUE("and sees a row added through the other variable", shared.Count() == 3);
  shared.DeleteAll();
  CHECK_TRUE("and a clear through either empties both", owner.Count() == 0);

  // THE NEGATIVE CONTROL, and it is the whole meaning of the second argument.
  const Temporary<LineNumberBuffer> apart = With({kSeven});
  Temporary<LineNumberBuffer> separate;
  separate.Copy(apart, false);
  CHECK_TRUE("a copy WITHOUT sharing has its own empty store", separate.Count() == 0);
  CHECK_TRUE("while the one it copied from keeps its rows", apart.Count() == 1);
  CHECK_TRUE("though the current row came across", separate.OldLineNumber == kSeven);
}

/// A temporary record reaches no database, which is what lets a test build a result set with no
/// session open at all.
void ATemporaryRecordNeedsNoSession() {
  CHECK_TRUE("no session is open", !agiru::Session::HasCurrent());
  std::string said;
  try {
    Temporary<LineNumberBuffer> buffer = With({1, 2, 3});
    (void)buffer.Get(1);
  } catch (const Error &e) { said = e.what(); }
  CHECK_SILENT("and the whole store works anyway", said);
}

} // namespace

/// AL `var Buffer: Record "Line Number Buffer" temporary` is a `LineNumberBuffer &` here, and the
/// rows must follow the ARGUMENT: temporariness is state, not type (board:0583).
namespace {

void ThroughBaseReference(LineNumberBuffer &rec, agiru::Integer n) {
  rec.OldLineNumber = n;
  rec.NewLineNumber = n * kTens;
  rec.Insert();
}
}

namespace {

void ABaseReferenceKeepsATemporaryTemporary() {
  Temporary<LineNumberBuffer> buffer = With({1, 2});
  const LineNumberBuffer &asBase = buffer;
  CHECK_TRUE("the base reference says it is temporary", asBase.IsTemporary());
  ThroughBaseReference(buffer, 3);
  CHECK_TRUE("and an Insert through it lands in the variable's rows", buffer.Count() == 3);
  CHECK_TRUE("while a plain record is not temporary", !LineNumberBuffer{}.IsTemporary());
}
}

/// `SetRange` on a temporary record narrows the walk and the count, which the typed store never
/// did (board:0583 names that as the activation this carries).
namespace {

/// ASSIGNMENT COPIES FIELDS AND `Copy` COPIES FILTERS TOO (`record-copy-method.md` lists the
/// filters under `Copy`; openerp's `:=` was fields-only at 2 260 green). `Record Set Management`
/// does `TempFound := RecordSetTree; TempFound.Insert()` in a loop over a filtered database record,
/// and a `:=` that carried the filters left the buffer's `FindFirst` one node of ten (27 cases of
/// Record Set UT, 2026-09-10).
void AssignmentCopiesFieldsAndCopyCopiesFilters() {
  Temporary<LineNumberBuffer> source = With({1, 2, 3});
  source.SetRange(source.OldLineNumber, 2);
  CHECK_TRUE("the source is filtered", source.FindFirst() && source.Count() == 1);
  Temporary<LineNumberBuffer> assigned = With({7, 8});
  assigned = source;
  CHECK_TRUE("assignment brings the fields", assigned.OldLineNumber == 2);
  CHECK_TEXT("and none of the filters", assigned.GetFilters(), "");
  CHECK_TRUE("so the target still walks its own rows", assigned.Count() == 2);
  Temporary<LineNumberBuffer> copied = With({7, 8});
  copied.Copy(source);
  CHECK_TRUE("Copy brings the fields", copied.OldLineNumber == 2);
  CHECK_TEXT("and the filters", copied.GetFilters(), source.GetFilters());
  CHECK_TRUE("over the target's own rows, which the filter now narrows", copied.Count() == 0);
}

/// THE SHAPE SALES-POST WALKED INTO: `TempSalesLine := SalesLine` used to carry the source's
/// filters across, `"Document Type" = 2` among them, and the temporary rows render that field as
/// `Invoice`. The count must see the rows through an option filter, and a decimal filter must
/// compare the value and not its spelling.
void AnOptionAndADecimalFilterATemporaryRowByValue() {
  constexpr agiru::Integer kThree = 3;
  Temporary<ResourceCost> costs;
  for (int i = 0; i < kThree; ++i) {
    costs.Init();
    costs.Type = i == 1 ? ResourceCostType::GroupResource : ResourceCostType::Resource;
    costs.Code = std::string("R0") + std::to_string(i);
    costs.UnitCost = Decimal{i};
    costs.Insert();
  }
  costs.SetRange(costs.Type, ResourceCostType::Resource);
  CHECK_TRUE("an option SetRange sees its rows", costs.Count() == 2);
  costs.SetFilter(costs.Type, "Group(Resource)");
  CHECK_TRUE("and a member named in a SetFilter does too", costs.Count() == 1);
  costs.Reset();
  costs.SetFilter(costs.UnitCost, "<>0");
  CHECK_TRUE("a decimal <>0 keeps the non-zero rows", costs.Count() == 2);
  costs.SetRange(costs.UnitCost, Decimal{2});
  CHECK_TRUE("and a decimal SetRange finds its row", costs.FindFirst() && costs.Code == "R02");
}

/// TWO TEMPORARY GLOBALS HELD BY HANDLE ARE TWO STORES AFTER `A := B`. `Gen. Jnl.-Post Line`
/// writes `TempGLEntryPreview := TempGLEntryBuf; TempGLEntryPreview.Insert()` for every entry in
/// its buffer, and a handle assignment that cloned the other handle made the two share one store,
/// so the first `Insert` refused the entry as already there (chain 75, 29 UT cases).
void AssigningOneHandleToAnotherKeepsTheRowsApart() {
  agiru::Instance<Temporary<LineNumberBuffer>> buffer;
  agiru::Instance<Temporary<LineNumberBuffer>> preview;
  buffer->Init();
  buffer->OldLineNumber = 1;
  buffer->Insert();
  preview = buffer;
  CHECK_TRUE("the fields came across", preview->OldLineNumber == 1);
  CHECK_TRUE("and the preview's store is its own", preview->Count() == 0);
  CHECK_TRUE("so the same key inserts there", preview->Insert());
  CHECK_TRUE("while the buffer keeps one row", buffer->Count() == 1);
}

/// A RECORDREF OVER A TEMPORARY RECORD WALKS ITS ROWS AND NOT THE TABLE
/// (`recordref-gettable-method.md`: the reference is temporary afterwards). `Record Set
/// Management` hands its temporary set to a RecordRef through a Variant and counted the table
/// instead -- 159 customers where the set held 1 (Record Set UT, 57 cases, 2026-09-10).
void ARecordRefOverATemporaryRecordSeesItsRows() {
  Temporary<LineNumberBuffer> buffer = With({1, 2, 3});
  agiru::RecordRef reference;
  reference.GetTable(buffer);
  CHECK_TRUE("the reference is temporary", reference.IsTemporary());
  CHECK_TRUE("and counts the rows", reference.Count() == 3);
  agiru::Variant held(buffer);
  agiru::RecordRef fromVariant;
  fromVariant.GetTable(held);
  CHECK_TRUE("and one taken from a Variant is temporary", fromVariant.IsTemporary());
  CHECK_TRUE("and counts the rows too", fromVariant.Count() == 3);
}

void AFilterNarrowsATemporaryWalk() {
  constexpr agiru::Integer kFive = 5;
  Temporary<LineNumberBuffer> buffer = With({1, 2, 3, 4, kFive});
  buffer.SetRange(buffer.OldLineNumber, 2, 4);
  CHECK_TRUE("Count sees the filter", buffer.Count() == 3);
  std::string walked;
  for (bool more = buffer.FindSet(); more; more = buffer.Next() != 0) {
    walked += std::to_string(buffer.OldLineNumber) + " ";
  }
  CHECK_TEXT("and so does the walk", walked, "2 3 4 ");
  CHECK_TRUE("FindLast lands on the last in range", buffer.FindLast() && buffer.OldLineNumber == 4);
  buffer.Reset();
  CHECK_TRUE("Reset widens it again", buffer.Count() == kFive);
}

/// `MarkedOnly` OVER A TEMPORARY RECORD walks the marked rows and no other
/// (`record-markedonly-method.md`; `Whse.-Create Source Document` marks the receipt headers it
/// keeps and walks `MarkedOnly`, 34 UT cases refused with "not carried yet", 2026-09-12). The
/// marks are the VARIABLE's (`record-mark-method.md`), so a copy taken with `Copy` carries them
/// and a plain assignment does not.
void MarkedOnlyWalksTheMarkedTemporaryRows() {
  constexpr agiru::Integer kFive = 5;
  Temporary<LineNumberBuffer> buffer = With({1, 2, 3, 4, kFive});
  static_cast<void>(buffer.Get(2));
  buffer.Mark(true);
  static_cast<void>(buffer.Get(4));
  buffer.Mark(true);
  buffer.MarkedOnly(true);
  CHECK_TRUE("Count sees the marks", buffer.Count() == 2);
  std::string walked;
  for (bool more = buffer.FindSet(); more; more = buffer.Next() != 0) {
    walked += std::to_string(buffer.OldLineNumber) + " ";
  }
  CHECK_TEXT("and the walk keeps only the marked rows", walked, "2 4 ");
  static_cast<void>(buffer.Get(4));
  buffer.Mark(false);
  CHECK_TRUE("unmarking takes a row out of the walk", buffer.Count() == 1);
  buffer.MarkedOnly(false);
  CHECK_TRUE("MarkedOnly(false) widens it again", buffer.Count() == kFive);
  // THE NEGATIVE CONTROL: MarkedOnly with nothing marked walks nothing, which a filter that was
  // quietly dropped would answer with every row.
  buffer.ClearMarks();
  buffer.MarkedOnly(true);
  CHECK_TRUE("no marks, no rows", buffer.Count() == 0 && !buffer.FindFirst());
}

/// THE SHAPES THE BASEAPP SHARES WITH, which board:0620 counts at 141 UT failures: a global
/// reached through an `Instance`, a temporary handed on BY VALUE, and a `var` parameter whose
/// declared type is the base table and whose argument is a temporary.
void SharedThroughAnInstanceAndByValue() {
  std::string step = "session";
  try {
    const agiru::Session session(AGIRU_TEST_DSN);
    step = "instance";
    agiru::Instance<Temporary<LineNumberBuffer>> global;
    step = "made";
    Temporary<LineNumberBuffer> &owner = global;
    step = "insert";
    owner.OldLineNumber = 1;
    owner.NewLineNumber = kTens;
    owner.Insert();
    step = "copy-share";
    Temporary<LineNumberBuffer> shared;
    shared.Copy(global, true);
    CHECK_TRUE("a temporary reached through an Instance shares", shared.Count() == 1);
    step = "by-value";
    const LineNumberBuffer byValue = owner;
    CHECK_TRUE("a temporary copied BY VALUE is still temporary", byValue.IsTemporary());
    step = "share-from-by-value";
    Temporary<LineNumberBuffer> again;
    again.Copy(byValue, true);
    CHECK_TRUE("and shares with the same rows", again.Count() == 1);
    step = "var-parameter";
    const auto throughVar = [](LineNumberBuffer &viaVar, Temporary<LineNumberBuffer> &into) {
      into.Copy(viaVar, true);
    };
    Temporary<LineNumberBuffer> third;
    throughVar(owner, third);
    CHECK_TRUE("a temporary passed as a var base-typed parameter shares", third.Count() == 1);
    step = "reset";
    owner.Reset();
    CHECK_TRUE("a temporary stays temporary across a Reset", owner.IsTemporary());
    CHECK_TRUE("and keeps its rows", owner.Count() == 1);
    Temporary<LineNumberBuffer> fifth;
    fifth.Copy(owner, true);
    CHECK_TRUE("and still shares", fifth.Count() == 1);
    step = "scope-end";
  } catch (const std::exception &e) {
    CHECK_TEXT("no step throws (the step that did)", step, "scope-end");
    CHECK_TEXT("and what it said", std::string(e.what()), "");
  }
}
}

/// THE CODEUNIT SHAPE: a global that is MADE inside the argument conversion of the Copy, in a
/// session, because that is where the BaseApp does it (`GenJnlCheckLine.GetErrors`).
void SharedFromAGlobalMadeInTheCall() {
  std::string step = "session";
  try {
    const agiru::Session session(AGIRU_TEST_DSN);
    step = "holder";

    struct Holder {
      agiru::Instance<Temporary<LineNumberBuffer>> TempErrorMessage;

      void GetErrors(LineNumberBuffer &into) { into.Copy(TempErrorMessage, true); }
    };

    Holder holder;
    step = "made";
    Temporary<LineNumberBuffer> &made = holder.TempErrorMessage;
    CHECK_TRUE("the global made through the Instance is temporary", made.IsTemporary());
    step = "copy";
    Temporary<LineNumberBuffer> fourth;
    holder.GetErrors(fourth);
    CHECK_TRUE("and the copy is temporary afterwards", fourth.IsTemporary());
    step = "scope-end";
  } catch (const std::exception &e) {
    CHECK_TEXT("no step throws (the step that did)", step, "scope-end");
  }
}

/// A TEMPORARY RECORD HANDED TO A CODEUNIT IS THE CALLER'S OWN (board:0691): AL passes it `var`,
/// the codeunit fills it, and the caller reads the rows back -- the whole shape of
/// `CODEUNIT.RUN(CODEUNIT::"Get Bank Stmt. Line Candidates", TempProposal)`.
void RowsAddedByABorrowerAreTheOwnersRows() {
  Temporary<ResourceCost> owner;
  ResourceCost borrower;
  borrower.Copy(owner);
  agiru::detail::RuntimeBorrowTemporary(&borrower, &owner);
  borrower.Code = "SEEN";
  borrower.Insert();
  CHECK_TRUE("the owner finds what the borrower inserted", static_cast<bool>(owner.FindFirst()));
  CHECK_TEXT("with the value it was given", std::string(owner.Code.Value()), "SEEN");
  CHECK_TRUE("and counts it once", owner.Count() == 1);

  // THE NEGATIVE CONTROL: without the borrow the rows stay where they were written, which is what
  // `Copy` alone means -- a temporary record keeps its own.
  Temporary<ResourceCost> apart;
  ResourceCost copied;
  copied.Copy(apart);
  CHECK_TRUE("a copy alone shares nothing", apart.Count() == 0);
}

int main() {
  return gate::Run("Temporary", [] {
    RowsAddedByABorrowerAreTheOwnersRows();
    SharedThroughAnInstanceAndByValue();
    SharedFromAGlobalMadeInTheCall();
    ABaseReferenceKeepsATemporaryTemporary();
    AFilterNarrowsATemporaryWalk();
    MarkedOnlyWalksTheMarkedTemporaryRows();
    ARecordRefOverATemporaryRecordSeesItsRows();
    AssigningOneHandleToAnotherKeepsTheRowsApart();
    AnOptionAndADecimalFilterATemporaryRowByValue();
    RowsWalkInPrimaryKeyOrder();
    ADuplicateKeyIsRefused();
    GetsRowFinds();
    CopyingWithShareGivesOneStoreAndWithoutGivesTwo();
    AssignmentCopiesFieldsAndCopyCopiesFilters();
    ATemporaryRecordNeedsNoSession();
  });
}

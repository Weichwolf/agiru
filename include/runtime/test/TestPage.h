#pragma once

#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/test/TestAction.h"
#include "runtime/test/TestField.h"
#include "runtime/test/TestFilter.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"

/// \file
/// \brief AL `TestPage` -- a page driven without a screen.

namespace agiru {

/// \brief AL `TestPage <Page>` -- the page, headless, with its controls reachable by name.
///
/// \tparam P The generated page class.
///
/// \note IT DERIVES FROM THE PAGE, and that is what makes `SalesOrder."No.".SetValue('X')` work at
///       all. AL reaches a control THROUGH the TestPage, so the controls have to be members of it;
///       the page already declares every one of them as a `TestField`, and inheriting is how C++
///       says "and all of those too" without the generator writing them a second time.
///
/// \warning A CONTROL WHOSE AL NAME IS ONE OF THE METHODS BELOW IS HIDDEN BY IT. C++ resolves the
///          derived name first, so a page with an action called `Close` would find this `Close`.
///          The failure is loud -- a wrong signature at the call site -- rather than silent, and no
///          page in the read roots has one today.
/// \brief A page this run never translated, so the controls are not there to reach.
///
/// \note IT IS NOT AN EMPTY PAGE. A `TestPage` over a page the transpiler has not seen must refuse
///       every control by NAME rather than silently having none, so what stands here is a class
///       with no members at all: the call site fails to compile and says which control it wanted.
class UnknownPage {};

}

/// \brief The controls a `TestPage` over an untranslated page has, which is none.
template <> struct agiru::PageTraits<agiru::UnknownPage> {
  /// \brief No controls at all.
  /// \tparam Field  How a test reaches a control.
  /// \tparam Action How a test reaches an action.
  template <typename Field, typename Action> using Controls = agiru::UnknownPage;
};

namespace agiru {

/// \brief AL `TestPage <Page>` -- the page, headless, with its controls reachable by name.
/// \tparam P The generated page class.
template <typename P = UnknownPage>
class TestPage : public PageTraits<P>::template Controls<TestField, TestAction> {
public:
  /// \brief AL `TestPage.OpenNew()` -- opens the page ready to insert.
  /// \throws Error until a page can be opened (board:0030).
  void OpenNew() { Unopened(); }

  /// \brief AL `TestPage.OpenEdit()` -- opens the page on its source table for editing.
  /// \throws Error until a page can be opened (board:0030).
  void OpenEdit() { Unopened(); }

  /// \brief AL `TestPage.OpenView()` -- opens the page read-only.
  /// \throws Error until a page can be opened (board:0030).
  void OpenView() { Unopened(); }

  /// \brief AL `TestPage.Close()`.
  /// \throws Error until a page can be opened (board:0030).
  void Close() { Unopened(); }

  /// \brief AL `TestPage.First()` -- moves to the first record.
  /// \return Whether there was one.
  /// \throws Error until a page can be opened (board:0030).
  Boolean First() { Unopened(); }

  /// \brief AL `TestPage.Next()` -- moves to the next record.
  /// \return Whether there was one.
  /// \throws Error until a page can be opened (board:0030).
  Boolean Next() { Unopened(); }

  /// \brief AL `TestPage.New()` -- starts a new record on the page.
  /// \throws Error until a page can be opened (board:0030).
  void New() { Unopened(); }

  /// \brief AL `TestPage.Last()` -- moves to the last record.
  /// \return Whether a record was there.
  /// \throws Error until a page runs (board:0030).
  Boolean Last() { Unopened(); }

  /// \brief AL `TestPage.Previous()` -- moves to the record before this one.
  /// \return Whether a record was there.
  /// \throws Error until a page runs (board:0030).
  Boolean Previous() { Unopened(); }

  /// \brief AL `TestPage.Prev()` -- the older spelling of `Previous`.
  /// \return Whether a record was there.
  /// \throws Error until a page runs (board:0030).
  Boolean Prev() { Unopened(); }

  /// \brief AL `TestPage.GotoRecord(Record)` -- positions the page on a record.
  /// \tparam R The record's type.
  /// \param Record The record to stand on.
  /// \return Whether the page could.
  /// \throws Error until a page runs (board:0030).
  template <typename R> Boolean GotoRecord(const R &Record) {
    static_cast<void>(Record);
    Unopened();
  }

  /// \brief AL `TestPage.GoToKey(...)` -- positions the page by primary key.
  /// \tparam Values The key field types.
  /// \param values The key values, in key order.
  /// \return Whether a record carried that key.
  /// \throws Error until a page runs (board:0030).
  template <typename... Values> Boolean GoToKey(const Values &...values) {
    (static_cast<void>(values), ...);
    Unopened();
  }

  /// \brief AL `TestPage.Trap()` -- catches the next page this one opens.
  ///
  /// \return This page, which the caught page is then read through.
  /// \throws Error until a page runs (board:0030).
  /// \note IT IS ARMED BEFORE THE ACTION, not after. A test writes `Other.Trap(); Page.Action.
  ///       Invoke();` and reads `Other` afterwards, so `Trap` records an intention and the page
  ///       that opens is bound to it.
  TestPage &Trap() { Unopened(); }

  /// \brief AL `TestPage.OK()` -- the OK system action.
  /// \return The action.
  /// \throws Error until a page runs (board:0030).
  TestAction OK() { Unopened(); }

  /// \brief AL `TestPage.Cancel()` -- the Cancel system action.
  /// \return The action.
  /// \throws Error until a page runs (board:0030).
  TestAction Cancel() { Unopened(); }

  /// \brief AL `TestPage.Yes()` -- the Yes system action.
  /// \return The action.
  /// \throws Error until a page runs (board:0030).
  TestAction Yes() { Unopened(); }

  /// \brief AL `TestPage.No()` -- the No system action.
  /// \return The action.
  /// \throws Error until a page runs (board:0030).
  TestAction No() { Unopened(); }

  /// \brief AL `TestPage.Caption()` -- the page's caption.
  /// \return The caption.
  /// \throws Error until a page runs (board:0030).
  [[nodiscard]] Text<0> Caption() const { Unopened(); }

  /// \brief AL `TestPage.Editable()` -- whether the page takes input.
  /// \return Whether it does.
  /// \throws Error until a page runs (board:0030).
  [[nodiscard]] Boolean Editable() const { Unopened(); }

  /// \brief AL `TestPage.Expand(Expand)` -- expands or collapses the current row.
  /// \param Expand Whether to expand.
  /// \throws Error until a page runs (board:0030).
  void Expand(Boolean Expand) {
    static_cast<void>(Expand);
    Unopened();
  }

  /// \brief AL `TestPage.IsExpanded()` -- whether the current row is expanded.
  /// \return Whether it is.
  /// \throws Error until a page runs (board:0030).
  [[nodiscard]] Boolean IsExpanded() const { Unopened(); }

  /// \brief AL `TestPage.GetValidationError()` -- the validation error standing on the page.
  /// \return The message.
  /// \throws Error until a page runs (board:0030).
  [[nodiscard]] Text<0> GetValidationError() const { Unopened(); }

  /// \brief AL `TestPage.ValidationErrorCount()` -- how many validation errors stand.
  /// \return The count.
  /// \throws Error until a page runs (board:0030).
  [[nodiscard]] Integer ValidationErrorCount() const { Unopened(); }

  /// \brief AL `TestPage.FindFirstField()` -- the first control that takes input.
  /// \return The control.
  /// \throws Error until a page runs (board:0030).
  TestField FindFirstField() { Unopened(); }

  /// \brief AL `TestPage.FindNextField()` -- the control after the current one.
  /// \return The control.
  /// \throws Error until a page runs (board:0030).
  TestField FindNextField() { Unopened(); }

  /// \brief AL `TestPage.FindPreviousField()` -- the control before the current one.
  /// \return The control.
  /// \throws Error until a page runs (board:0030).
  TestField FindPreviousField() { Unopened(); }

  /// \brief AL `TestPage.GetField(No)` -- a control by its field number.
  /// \param No The field number.
  /// \return The control.
  /// \throws Error until a page runs (board:0030).
  TestField GetField(Integer No) {
    static_cast<void>(No);
    Unopened();
  }

  /// \brief AL `TestPage.Filter` -- the page's filters.
  ///
  /// \note IT IS A MEMBER AND NOT A METHOD, because AL writes `Page.FILTER.SETFILTER(...)` with no
  ///       parentheses on `FILTER`. `methods-auto/testfilter/` is its own type for the same reason.
  TestFilter Filter{};

private:
  [[noreturn]] static void Unopened() {
    throw Error("a TestPage needs a running page (board:0030)");
  }
};

}

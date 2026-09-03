#pragma once

#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/test/TestAction.h"
#include "runtime/test/TestField.h"
#include "type/Boolean.h"

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

private:
  [[noreturn]] static void Unopened() {
    throw Error("a TestPage needs a running page (board:0030)");
  }
};

}

#pragma once

#include "runtime/ErrorValue.h"
#include "runtime/PageSession.h"
#include "runtime/test/PageTraps.h"
#include "runtime/test/TestAction.h"
#include "runtime/test/TestField.h"
#include "runtime/test/TestFilter.h"
#include "type/Dictionary.h"
#include "type/Integer.h"
#include "type/StringValue.h"

#include <string>
#include <string_view>
#include <utility>

/// \file
/// \brief AL TestPage presentation adapter over the shared production page session.

namespace agiru {

/// \brief An untranslated page whose requested controls must fail to compile.
class UnknownPage {};

}

/// \brief The controls a `TestPage` over an untranslated page has, which is none.
template <> struct agiru::PageTraits<agiru::UnknownPage> {
  /// \brief No controls at all.
  /// \tparam Field_Kind  How a test reaches a control.
  /// \tparam Action_Kind How a test reaches an action.
  /// \tparam Part_Kind   How a test reaches an embedded page.
  ///
  /// \note THE INTERIOR UNDERSCORE IS THE SEAM. `Field`, `Action` and `Part` are names a page
  ///       control may carry, and a control of that name shadows the template parameter -- 40
  ///       headers failed on `declaration of 'Field' shadows template parameter`. No AL name
  ///       reaches an interior underscore.
  template <typename Field_Kind, typename Action_Kind, template <typename> class Part_Kind>
  using Controls = agiru::UnknownPage;
};

namespace agiru {

/// \brief AL test diagnostics, distinct from production command errors.
struct TestPageDiagnostics {
  static constexpr std::string_view kNotOpen = "The TestPage is not open."; ///< AL test message.
  static constexpr std::string_view kNotOpenCode{}; ///< Preserve the uncoded AL test diagnostic.
  static constexpr std::string_view kAlreadyOpen =
      "The TestPage is already open and cannot be opened again."; ///< AL test double-open message.
  static constexpr std::string_view
      kAlreadyOpenCode{}; ///< Preserve the uncoded AL test diagnostic.
  static constexpr std::string_view kValidationCode =
      "TestValidation";                            ///< AL test validation code.
  static constexpr bool kCollectSaveErrors = true; ///< Retain TestPage row-save error inspection.
};

/// \brief AL TestPage: generated test controls and traps over one shared typed page kernel.
/// \tparam P The generated AL page or report request-page class.
template <typename P = UnknownPage>
class TestPage : public PageTraits<P>::template Controls<TestField, TestAction, ::agiru::TestPage>,
                 public PageSession<P, TestPageDiagnostics> {
  using Session_ = PageSession<P, TestPageDiagnostics>;

public:
  /// \brief Identifies the adapter for AL page handler binding.
  using IsTestPage = void;

  /// \brief Allows the request-page test adapter to bind its test-only actions.
  template <typename> friend class TestRequestPage;

  using Session_::Adopt;
  using Session_::AttachPart;
  using Session_::BindPart;
  using Session_::Caption;
  using Session_::Close;
  using Session_::Computed_;
  using Session_::ControlCaption;
  using Session_::ControlEditable;
  using Session_::ControlEnabled;
  using Session_::ControlFilterText;
  using Session_::ControlOrdinal;
  using Session_::ControlText;
  using Session_::ControlVisible;
  using Session_::Editable;
  using Session_::First;
  using Session_::GetValidationError;
  using Session_::GoToKey;
  using Session_::GoToRecord;
  using Session_::KeyOf_;
  using Session_::Last;
  using Session_::LinkPart;
  using Session_::New;
  using Session_::Next;
  using Session_::OpenEdit;
  using Session_::OpenNew;
  using Session_::OpenView;
  using Session_::OwnState_;
  using Session_::OwnVisible_;
  using Session_::PartInstance;
  using Session_::PresentNewRow_;
  using Session_::Prev;
  using Session_::Previous;
  using Session_::RowLeft;
  using Session_::RunControlTrigger;
  using Session_::SetControlFilter;
  using Session_::SetControlText;
  using Session_::SplitField_;
  using Session_::StandingKey_;
  using Session_::StateWithin_;
  using Session_::ValidationErrorCount;
  using Session_::VisibleWithin_;

  TestPage() = default;

  /// \brief Copies an AL test handle, retaining the existing shared-page semantics.
  /// \param other The handle to borrow.
  TestPage(const TestPage &other) : Session_(other) { BindTestControls_(); }

  /// \brief Assigns an AL test handle.
  /// \param other The handle to borrow.
  /// \return This adapter.
  TestPage &operator=(const TestPage &other) {
    Session_::operator=(other);
    return *this;
  }

  /// \brief Moves the page ownership to this AL test handle.
  /// \param other The source handle.
  TestPage(TestPage &&other) noexcept : Session_(std::move(other)) { BindTestControls_(); }

  /// \brief Moves an AL test handle.
  /// \param other The source handle.
  /// \return This adapter.
  TestPage &operator=(TestPage &&other) noexcept {
    Session_::operator=(std::move(other));
    return *this;
  }

  ~TestPage() override { detail::WithdrawTraps(this); }

  /// \brief AL `TestPage.Trap()` -- the next non-modal run of this page lands here.
  /// \return This.
  TestPage &Trap() {
    if constexpr (requires { PageTraits<P>::kId; }) {
      detail::TrapPage(PageTraits<P>::kId.Value(), this, &TestPage::AdoptOwned_);
      return *this;
    } else {
      this->NotOpen();
    }
  }

  /// \brief AL `TestPage.OK()` -- the page's OK action.
  /// \return The action, to `Invoke`.
  TestAction OK() { return Bound_("OK"); }

  /// \brief AL `TestPage.Cancel()` -- the page's Cancel action.
  /// \return The action, to `Invoke`.
  TestAction Cancel() { return Bound_("Cancel"); }

  /// \brief AL `TestPage.Yes()` -- a confirmation's Yes.
  /// \return The action, to `Invoke`.
  TestAction Yes() { return Bound_("Yes"); }

  /// \brief AL `TestPage.No()` -- a confirmation's No.
  /// \return The action, to `Invoke`.
  TestAction No() { return Bound_("No"); }

  /// \brief AL `TestPage.Edit()` -- edits the selected row in a list's declared card,
  /// or switches a standalone page to editing. Explicit actions take precedence.
  /// \return The action, to `Invoke`.
  TestAction Edit() { return Bound_("Edit"); }

  /// \brief AL `TestPage.View()` -- switches an editing page to viewing, the system action
  ///        `testpage-view-method.md` documents beside `Edit`.
  /// \return The action, to `Invoke`.
  TestAction View() { return Bound_("View"); }

  /// \brief AL `TestPage.Expand(Boolean)` -- expands or collapses the current row of a tree.
  /// \param Expand True to expand.
  void Expand(Boolean Expand) { static_cast<void>(Expand); }

  /// \brief AL `TestPage.IsExpanded()`.
  /// \return False; rows do not nest here yet.
  [[nodiscard]] Boolean IsExpanded() const { return false; }

  /// \brief AL `TestPage.FindFirstField(Field, Value)` -- the first row whose control shows it.
  /// \tparam V The value's type.
  /// \param field The control.
  /// \param value What it must show.
  /// \return Whether a row does.
  template <typename V> Boolean FindFirstField(const TestField &field, const V &value) {
    if (!First()) { return false; }
    return Matches_(field, value) || FindNextField(field, value);
  }

  /// \brief AL `TestPage.FindNextField(Field, Value)`.
  /// \tparam V The value's type.
  /// \param field The control.
  /// \param value What it must show.
  /// \return Whether a later row does.
  template <typename V> Boolean FindNextField(const TestField &field, const V &value) {
    while (Next()) {
      if (Matches_(field, value)) { return true; }
    }
    return false;
  }

  /// \brief AL `TestPage.FindPreviousField(Field, Value)`.
  /// \tparam V The value's type.
  /// \param field The control.
  /// \param value What it must show.
  /// \return Whether an earlier row does.
  template <typename V> Boolean FindPreviousField(const TestField &field, const V &value) {
    while (Previous()) {
      if (Matches_(field, value)) { return true; }
    }
    return false;
  }

  /// \brief AL `TestPage.GetField(No)` -- a control by its field number.
  /// \param No The field number.
  /// \return The control, bound.
  TestField GetField(Integer No) {
    const ControlDef *control = this->FindFieldControl(::agiru::FieldNo{No});
    if (control == nullptr) {
      throw Error("no control on the page shows field " + std::to_string(No));
    }
    TestField field{control->name};
    field.Bind(*this);
    return field;
  }

  /// \brief AL `TestPage.RunPageBackgroundTask(...)`.
  /// \param CodeunitId            The codeunit.
  /// \param Parameters            Its parameters.
  /// \param RunCompletionTriggers Whether to run the completion triggers.
  /// \return Never.
  /// \throws Error always -- background tasks have no runner yet (board:0030).
  Dictionary<::agiru::Text<0>, ::agiru::Text<0>>
  RunPageBackgroundTask(Integer CodeunitId,
                        Dictionary<::agiru::Text<0>, ::agiru::Text<0>> &Parameters,
                        Boolean RunCompletionTriggers = {}) {
    static_cast<void>(CodeunitId);
    static_cast<void>(Parameters);
    static_cast<void>(RunCompletionTriggers);
    this->NotOpen();
  }

  /// \brief AL `TestPage.Filter` -- the page's filter pane, bound when the page opens.
  TestFilter Filter{}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
  void BindPresentation() override { BindTestControls_(); }

  void BindTestControls_() {
    if constexpr (requires { this->BindControls(*static_cast<PageCore *>(this)); }) {
      this->BindControls(*static_cast<PageCore *>(this));
    }
    Filter.Bind(*this);
  }

  static void AdoptOwned_(void *harness, void *page, bool owned) {
    static_cast<TestPage *>(harness)->AdoptObject(page, owned);
  }

  TestAction Bound_(std::string_view name) {
    TestAction action{name};
    action.Bind(*this);
    return action;
  }

  template <typename V> bool Matches_(const TestField &field, const V &value) {
    return this->ControlText(field.Name()) == AsText(value);
  }
};

}

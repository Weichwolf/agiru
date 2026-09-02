#pragma once

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"

#include <string>
#include <string_view>

/// \file
/// \brief The base every generated AL page stands on, and the controls a page is made of.

namespace agiru {

/// \brief The declaration belonging to a generated page.
///
/// \tparam T The generated page class.
///
/// The generator specialises this beside the class, so the class itself carries nothing but what AL
/// wrote -- its controls, its actions, its variables and its procedures. The number, the name and
/// the page type live here, the way a table's field table does.
template <typename T> struct PageTraits;

/// \brief AL `TestField` -- one control of a page, reached by the name AL gave it.
///
/// \note A CONTROL IS NOT TYPED HERE, AND THAT IS AL'S OWN SHAPE RATHER THAN A SIMPLIFICATION.
///       `TestField.SetValue` takes `Any` and `TestField.Value` returns `Text`: the platform moves
///       the value through its string form and the CONTROL decides how to read it. The typed
///       readers are the `AsX()` family, which is exactly the set the page documents.
///
/// \warning WHAT IS BEHIND IT IS NOT CONNECTED YET. A control that is not bound to a running page
///          refuses rather than answering with a blank, because a test that reads `Value()` and
///          gets `""` from an unopened page is green for the wrong reason (board:0030).
class TestField {
public:
  /// \brief A control with the name AL gave it.
  /// \param name The control name.
  explicit constexpr TestField(std::string_view name) : name_(name) {}

  /// \brief The control's AL name.
  /// \return The name.
  [[nodiscard]] constexpr std::string_view Name() const { return name_; }

  /// \brief AL `TestField.SetValue(Value)`.
  /// \param value The value, in the string form the platform moves it in.
  /// \throws Error until a page can be opened.
  void SetValue(std::string_view value);

  /// \brief AL `TestField.Value()`.
  /// \return The control's value as text.
  /// \throws Error until a page can be opened.
  [[nodiscard]] std::string Value() const;

  /// \brief AL `TestField.AssertEquals(Expected)`.
  /// \param expected What the control should hold.
  /// \throws Error when it holds something else, and until a page can be opened.
  void AssertEquals(std::string_view expected) const;

  /// \brief AL `TestField.AsInteger()`. \return The value as an Integer. \throws Error as Value does.
  [[nodiscard]] Integer AsInteger() const;

  /// \brief AL `TestField.AsBoolean()`. \return The value as a Boolean. \throws Error as Value does.
  [[nodiscard]] Boolean AsBoolean() const;

  /// \brief AL `TestField.Activate()` -- puts the focus on the control.
  /// \throws Error until a page can be opened.
  void Activate();

  /// \brief AL `TestField.Lookup()` -- opens the control's lookup.
  /// \throws Error until a page can be opened.
  void Lookup();

  /// \brief AL `TestField.DrillDown()` -- follows the control's drilldown.
  /// \throws Error until a page can be opened.
  void DrillDown();

  /// \brief AL `TestField.Editable()`. \return Whether the control takes input.
  /// \throws Error until a page can be opened.
  [[nodiscard]] Boolean Editable() const;

  /// \brief AL `TestField.Enabled()`. \return Whether the control is enabled.
  /// \throws Error until a page can be opened.
  [[nodiscard]] Boolean Enabled() const;

  /// \brief AL `TestField.Visible()`. \return Whether the control is shown.
  /// \throws Error until a page can be opened.
  [[nodiscard]] Boolean Visible() const;

private:
  [[noreturn]] void Unbound() const;

  std::string_view name_;
};

/// \brief AL `TestAction` -- one action of a page, reached by the name AL gave it.
class TestAction {
public:
  /// \brief An action with the name AL gave it.
  /// \param name The action name.
  explicit constexpr TestAction(std::string_view name) : name_(name) {}

  /// \brief The action's AL name.
  /// \return The name.
  [[nodiscard]] constexpr std::string_view Name() const { return name_; }

  /// \brief AL `TestAction.Invoke()` -- runs the action's `OnAction` trigger.
  /// \throws Error until a page can be opened.
  void Invoke();

  /// \brief AL `TestAction.Enabled()`. \return Whether the action is enabled.
  /// \throws Error until a page can be opened.
  [[nodiscard]] Boolean Enabled() const;

  /// \brief AL `TestAction.Visible()`. \return Whether the action is shown.
  /// \throws Error until a page can be opened.
  [[nodiscard]] Boolean Visible() const;

private:
  [[noreturn]] void Unbound() const;

  std::string_view name_;
};

/// \brief What every AL page can do, without the generated class saying any of it.
///
/// \tparam Derived The generated page class.
///
/// A page is the object BC's user actually works in, and AL code drives it from two sides: the
/// application calls `Page.Run` and `Page.RunModal`, and a test drives every control through
/// `TestPage`. Both halves are the platform's, so both live here.
/// \brief AL `TestRequestPage <Report>` -- a report's request page, driven without a screen.
///
/// \tparam R The generated report class, once reports are translated (board:0034).
template <typename R = void> class TestRequestPage {
public:
  /// \brief AL `TestRequestPage.OK()` -- closes the request page and runs the report.
  /// \return Whether it ran.
  /// \throws Error until a report can be run (board:0034).
  Boolean OK() { throw Error("a TestRequestPage needs a report (board:0034)"); }

  /// \brief AL `TestRequestPage.Cancel()` -- closes the request page without running.
  /// \return Whether it closed.
  /// \throws Error until a report can be run (board:0034).
  Boolean Cancel() { throw Error("a TestRequestPage needs a report (board:0034)"); }
};

template <typename Derived> class Page {
public:
  /// \brief The page's AL number.
  /// \return The number AL declared.
  [[nodiscard]] static constexpr PageId Id() { return PageTraits<Derived>::kId; }

  /// \brief The page's AL name.
  /// \return The name AL declared.
  [[nodiscard]] static constexpr std::string_view Name() { return PageTraits<Derived>::kName; }

  /// \brief AL `Page.Run(PageId, Record)` -- shows the page.
  /// \throws Error until the UI runs (board:0030).
  static void Run() { throw Error("Page.Run needs a running UI (board:0030)"); }

  /// \brief AL `Page.RunModal(PageId, Record)` -- shows the page and waits for it.
  /// \return The action the user closed it with.
  /// \throws Error until the UI runs (board:0030).
  static Integer RunModal() { throw Error("Page.RunModal needs a running UI (board:0030)"); }

protected:
  ~Page() = default;
};

} // namespace agiru

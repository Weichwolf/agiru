#pragma once

#include "runtime/Error.h"
#include "type/Boolean.h"

#include <string_view>

/// \file
/// \brief AL `TestField` -- one control of a page, driven by a test.
///
/// \note IT IS NOT PART OF A PAGE. A page has CONTROLS; `TestField` is how a TEST reaches one, and
/// the
///       generated page declares its controls as a class TEMPLATE so that nothing test-shaped
///       lands in `apps/` outside the test app.

namespace agiru {

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

  /// \brief AL `TestField.AsInteger()`. \return The value as an Integer. \throws Error as Value
  /// does.
  [[nodiscard]] Integer AsInteger() const;

  /// \brief AL `TestField.AsBoolean()`. \return The value as a Boolean. \throws Error as Value
  /// does.
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

  /// \brief AL `TestField.AsDate()`. Converts the value in a field on a test page to a Date data
  /// type.
  /// \return The AL `Date`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Date AsDate() const { Unbound(); }

  /// \brief AL `TestField.AsDateTime()`. Converts the value in a field on a test page to a DateTime
  /// data type.
  /// \return The AL `DateTime`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::DateTime AsDateTime() const { Unbound(); }

  /// \brief AL `TestField.AsDecimal()`. Converts the value in a field on a test page to a Date data
  /// type.
  /// \return The AL `Decimal`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Decimal AsDecimal() const { Unbound(); }

  /// \brief AL `TestField.AssistEdit()`. Provides assist-edit functionality to a field on a test
  /// page.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void AssistEdit() const { Unbound(); }

  /// \brief AL `TestField.AsTime()`. Converts the value in a field on a test page to a Time data
  /// type.
  /// \return The AL `Time`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Time AsTime() const { Unbound(); }

  /// \brief AL `TestField.Caption()`. Gets the current caption of the field as a String.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] std::string Caption() const { Unbound(); }

  /// \brief AL `TestField.Drilldown()`. Applies drill-down capability for a field on a test page.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Drilldown() const { Unbound(); }

  /// \brief AL `TestField.GetOption(Integer)`. Gets the options for a field on a test page.
  /// \param Index The AL `Integer`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] std::string GetOption(::agiru::Integer Index = {}) const {
    static_cast<void>(Index);
    Unbound();
  }

  /// \brief AL `TestField.GetValidationError(Integer)`. Gets the validation error that occurred on
  /// a test page.
  /// \param Index The AL `Integer`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] std::string GetValidationError(::agiru::Integer Index = {}) const {
    static_cast<void>(Index);
    Unbound();
  }

  /// \brief AL `TestField.HideValue()`. Gets the hide value state for the field.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Boolean HideValue() const { Unbound(); }

  /// \brief AL `TestField.Invoke()`. Invokes the default action on the field.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Invoke() const { Unbound(); }

  /// \brief AL `TestField.OptionCount()`. Gets the number of options in a field on a test page.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Integer OptionCount() const { Unbound(); }

  /// \brief AL `TestField.ShowMandatory()`. Gets the ShowMandatory state for the field.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Boolean ShowMandatory() const { Unbound(); }

  /// \brief AL `TestField.ValidationErrorCount()`. Gets the number of validation errors that
  /// occurred on the test page.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  [[nodiscard]] ::agiru::Integer ValidationErrorCount() const { Unbound(); }

private:
  [[noreturn]] void Unbound() const;

  std::string_view name_;
};

} // namespace agiru

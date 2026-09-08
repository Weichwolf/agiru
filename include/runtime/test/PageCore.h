#pragma once

#include "type/Boolean.h"

#include <cstdint>
#include <string>
#include <string_view>

/// \file
/// \brief What a control of a headless page reaches into: the page under test, type-erased.

namespace agiru {

/// \brief The triggers a control carries, named the way AL names them.
enum class ControlTriggerKind : std::uint8_t {
  Validate,   ///< `OnValidate`, after the record's own.
  Action,     ///< `OnAction`.
  DrillDown,  ///< `OnDrillDown`.
  AssistEdit, ///< `OnAssistEdit`.
  Lookup,     ///< `OnLookup`.
};

/// \brief The page a `TestField`, `TestAction` or `TestFilter` drives, without its type.
///
/// \note A CONTROL IS A NAME AND A PAGE IS A TYPE. The generated `Controls` class names every
///       control once, as `TestField Name{"Name"}`; what the control does with a `SetValue` is
///       decided by the page it sits on -- which record, which field, which trigger -- and that
///       page is a template argument two classes up. This interface is the seam: `TestPage<P>`
///       implements it, and `BindControls` hands it to every control after the page is made.
class PageCore {
public:
  virtual ~PageCore() = default;

  /// \brief AL `TestField.SetValue(Value)`: evaluates the text into the control's field and
  ///        runs the record's `OnValidate`, then the control's.
  /// \param control The control's AL name.
  /// \param text    What the user typed.
  /// \throws Error whatever the validation raises, and when the control has no field.
  virtual void SetControlText(std::string_view control, std::string_view text) = 0;

  /// \brief AL `TestField.Value`: the control's field, formatted.
  /// \param control The control's AL name.
  /// \return The text a user would read.
  /// \throws Error when the control has no field.
  [[nodiscard]] virtual std::string ControlText(std::string_view control) const = 0;

  /// \brief Runs one of a control's triggers.
  /// \param control The control's AL name; `OK`, `Cancel`, `Yes` and `No` are the page's own.
  /// \param kind    Which trigger.
  /// \throws Error whatever the trigger raises.
  virtual void RunControlTrigger(std::string_view control, ControlTriggerKind kind) = 0;

  /// \brief AL `TestFilter.SetFilter(Field, Text)`: narrows the page's record on the control's
  ///        field.
  /// \param control The control's AL name.
  /// \param filter  The filter expression.
  virtual void SetControlFilter(std::string_view control, std::string_view filter) = 0;

  /// \brief AL `TestField.Visible()`.
  /// \param control The control's AL name.
  /// \return What the control's `Visible` property says; true when it says nothing.
  [[nodiscard]] virtual Boolean ControlVisible(std::string_view control) const = 0;

  /// \brief AL `TestField.Editable()`.
  /// \param control The control's AL name.
  /// \return What the control's `Editable` property and the page's mode say.
  [[nodiscard]] virtual Boolean ControlEditable(std::string_view control) const = 0;

  /// \brief AL `TestField.Enabled()`.
  /// \param control The control's AL name.
  /// \return What the control's `Enabled` property says; true when it says nothing.
  [[nodiscard]] virtual Boolean ControlEnabled(std::string_view control) const = 0;

  /// \brief AL `TestField.Caption()`.
  /// \param control The control's AL name.
  /// \return The control's caption, or the field's.
  [[nodiscard]] virtual std::string ControlCaption(std::string_view control) const = 0;
};

namespace detail {

/// \brief AL `TestPage.Trap()`: the next non-modal run of the page lands in this harness.
/// \param page    The page's number.
/// \param harness The `TestPage` that trapped it.
/// \param adopt   Hands the page object, which the harness then owns, to the harness.
void TrapPage(std::int32_t page, void *harness, void (*adopt)(void *harness, void *page));

/// \brief Gives a page just run to the harness that trapped it, if one did.
/// \param page   The page's number.
/// \param object The page object; the harness owns it afterwards when this returns true.
/// \return Whether a trap took it.
[[nodiscard]] bool ReleaseTrap(std::int32_t page, void *object);

/// \brief Forgets every trap; the runner does this between cases.
void ClearTraps();

}

}

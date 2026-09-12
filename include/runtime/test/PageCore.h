#pragma once

#include "meta/TableDef.h"
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

  /// \brief The ordinal behind an Option or Enum control, which `AssertEquals(1)` names where
  ///        `ControlText` shows the caption.
  /// \param control The control's AL name.
  /// \return The ordinal as text; empty for a control that is not an option.
  [[nodiscard]] virtual std::string ControlOrdinal(std::string_view control) const = 0;

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

  /// \brief AL `TestPage.Filter.GetFilter(Field)`: the filter standing on the control's field in
  ///        the page's record, in the record's own filter group.
  /// \param control The control's AL name.
  /// \return The filter text, empty for none.
  [[nodiscard]] virtual std::string ControlFilterText(std::string_view control) const = 0;

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

  /// \brief The running page instance behind a `part` control, for the nested test page.
  /// \param control The part's AL name.
  /// \return The subpage instance, or nothing when the page is not running or has no such part.
  [[nodiscard]] virtual void *PartInstance(std::string_view control) = 0;

  /// \brief Applies a part's `SubPageLink` to the subpage's record from this page's current record,
  ///        which is what keeps `SalesInvoice.SalesLines` on the invoice's own lines.
  /// \param control The part's AL name.
  /// \param subRecord The subpage's record.
  /// \param subTable Its declaration.
  virtual void LinkPart(std::string_view control, void *subRecord, const TableDef &subTable) = 0;

  /// \brief The row the page stands on is LEFT -- focus went to another row, another control,
  ///        an action, the parent, or the page closed -- so a new row the user edited is inserted
  ///        (`devenv-delayedinsert-property.md`) and one nobody edited is dropped.
  virtual void RowLeft() = 0;

  /// \brief A part attaches to this page, so that leaving the page leaves the part's row too.
  /// \param part The part's harness, which outlives this page's use of it.
  virtual void AttachPart(PageCore &part) = 0;
};

namespace detail {

/// \brief AL `TestPage.Trap()`: the next non-modal run of the page lands in this harness.
/// \param page    The page's number.
/// \param harness The `TestPage` that trapped it.
/// \param adopt   Hands the page object to the harness, saying whether the harness owns it now
///                (a page `Page.Run` made) or only drives it (a page VARIABLE's `Run()`, which
///                the variable keeps).
void TrapPage(std::int32_t page,
              void *harness,
              void (*adopt)(void *harness, void *page, bool owned));

/// \brief Gives a page just run to the harness that trapped it, if one did.
/// \param page   The page's number.
/// \param object The page object.
/// \param owned  Whether the harness owns it afterwards; false for a page variable's own object.
/// \return Whether a trap took it.
[[nodiscard]] bool ReleaseTrap(std::int32_t page, void *object, bool owned);

/// \brief Whether a `Trap` is waiting for the page, so a runner can decide what to hand it.
/// \param page The page's number.
/// \return True when the next non-modal run of that page would be taken.
[[nodiscard]] bool TrapPending(std::int32_t page);

/// \brief Forgets every trap; the runner does this between cases.
void ClearTraps();

}

}

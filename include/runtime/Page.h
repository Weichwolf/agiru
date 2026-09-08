#pragma once

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/test/Handlers.h"
#include "runtime/test/PageCore.h"
#include "runtime/test/TestAction.h"
#include "type/Action.h"
#include "type/Boolean.h"
#include "type/Dictionary.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/PageBackgroundTaskErrorLevel.h"
#include "type/PageStyle.h"
#include "type/PromptMode.h"
#include "type/Text.h"
#include "type/Variant.h"

#include <concepts>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

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

/// \brief A call through a control whose object this build does not carry: a `usercontrol`'s
///        add-in, or a `part` whose page is outside the translated scope.
/// \param what The call as AL wrote it, `Part.Page().Method`.
/// \return Never; the type exists so the call can stand in an expression.
/// \throws Error always, naming the call -- a HOLE WITH A NAME rather than a compile error in a
///         page every test of that table needs (board:0034).
[[noreturn]] inline RefusedOptionValue RefusedControl(std::string_view what) {
  throw Error("the control " + std::string(what) + " has no object behind it in this build " +
              "(board:0034)");
}

/// \brief One control's triggers, as a page's `PageTraits` tabulates them.
///
/// \tparam P The generated page class.
///
/// \note IT IS STATIC DATA BESIDE THE PAGE, the way a table's `kOnValidate` is: the generator
///       emits one row per control that declares a trigger, with a member pointer per trigger
///       it declares and `nullptr` for the rest. A headless page (`TestPage`) finds the control
///       by its AL name and calls through the pointer; nothing is looked up by string at run
///       time beyond the one name the test wrote.
template <typename P> struct ControlTrigger {
  std::string_view control;          ///< The control's AL name.
  void (P::*validate)() = nullptr;   ///< `OnValidate`.
  void (P::*action)() = nullptr;     ///< `OnAction`.
  void (P::*drillDown)() = nullptr;  ///< `OnDrillDown`.
  void (P::*assistEdit)() = nullptr; ///< `OnAssistEdit`.
};

/// \brief What every AL page can do, without the generated class saying any of it.
///
/// \tparam Derived The generated page class.
///
/// A page is the object BC's user actually works in, and AL code drives it from two sides: the
/// application calls `Page.Run` and `Page.RunModal`, and a test drives every control through
/// `TestPage`. Both halves are the platform's, so both live here.
/// \brief What every AL page can do, without the generated class saying any of it.
/// \tparam Derived The generated page class.
/// \note THE PARAMETER DEFAULTS TO `void`, WHICH IS WHAT LETS AL'S OWN SPELLING THROUGH. AL writes
///       `Page.RunModal(Id, Rec)` -- a static on the TYPE -- and it writes `page 21 "Customer
///       Card"` whose generated class derives from `Page<Customer_Card>`. A class template and a
///       class of the same name cannot both exist in C++, so the statics live on the template and
///       the generator spells the static form `Page<>::RunModal`. `Id()` and `Name()` are the two
///       that need a Derived, and asking for them on `Page<>` is a compile error rather than a
///       wrong number.
/// \brief A PART control on a page: AL reaches the sub-page through it -- `CurrPage.Matrix.PAGE`.
///
/// \tparam P The sub-page's generated class.
///
/// \note THE PART IS A CONTROL AND THE SUB-PAGE IS AN OBJECT, and AL keeps them apart with
///       `.PAGE`. The part itself carries what the platform offers on a control (`Visible`,
///       `Editable`); everything on the other side of `.PAGE` is the sub-page's own surface, and
///       reaching it needs a running UI (board:0030).
template <typename P> class PartRef {
public:
  /// \brief AL `CurrPage.<Part>.PAGE` -- the sub-page behind the part.
  /// \return Never.
  /// \throws Error always -- a part's page needs a running UI (board:0030).
  [[nodiscard]] P &Page() const { throw Error("A part's PAGE needs a running UI (board:0030)"); }

  /// \brief AL `CurrPage.<Part>.Visible(Boolean)`.
  /// \param NewVisible Whether it shows.
  /// \return Never.
  /// \throws Error always -- a part needs a running UI (board:0030).
  ::agiru::Boolean Visible(::agiru::Boolean NewVisible) const {
    static_cast<void>(NewVisible);
    throw Error("A part's Visible needs a running UI (board:0030)");
  }

  /// \brief AL `CurrPage.<Part>.Editable(Boolean)`.
  /// \param NewEditable Whether it takes input.
  /// \return Never.
  /// \throws Error always -- a part needs a running UI (board:0030).
  ::agiru::Boolean Editable(::agiru::Boolean NewEditable) const {
    static_cast<void>(NewEditable);
    throw Error("A part's Editable needs a running UI (board:0030)");
  }
};

namespace detail {

/// \brief Runs the triggers a page owes after landing on a record: `OnAfterGetRecord`, then
///        `OnAfterGetCurrRecord`.
/// \tparam P The generated page class.
/// \param page The page.
template <typename P> void AfterGetRecord(P &page) {
  if constexpr (requires { page.OnAfterGetRecord(); }) { page.OnAfterGetRecord(); }
  if constexpr (requires { page.OnAfterGetCurrRecord(); }) { page.OnAfterGetCurrRecord(); }
}

/// \brief Opens a page the way the platform does: `OnInit`, the record positioned (or a new one
///        with `OnNewRecord`), `OnOpenPage`, then the after-get triggers.
/// \tparam P The generated page class.
/// \param page     The page.
/// \param editable Whether it opens for editing.
/// \param isNew    Whether it opens on a new record (`OpenNew`).
template <typename P> void OpenPage(P &page, bool editable, bool isNew) {
  page.OpenedAs(editable);
  if constexpr (requires { page.OnInit(); }) { page.OnInit(); }
  bool found = false;
  if constexpr (requires { page.Rec; }) {
    if (isNew) {
      page.Rec.Init();
      if constexpr (requires { page.OnNewRecord(::agiru::Boolean{}); }) { page.OnNewRecord(false); }
    } else {
      found = static_cast<bool>(page.Rec.FindFirst());
    }
  }
  if constexpr (requires { page.OnOpenPage(); }) { page.OnOpenPage(); }
  if (found || isNew) { AfterGetRecord(page); }
}

/// \brief Closes a page the way the platform does: `OnQueryClosePage`, then `OnClosePage`.
/// \tparam P The generated page class.
/// \param page The page.
/// \throws Error when `OnQueryClosePage` refuses.
template <typename P> void ClosePage(P &page) {
  if constexpr (requires(::agiru::Action action) {
                  { page.OnQueryClosePage(action) } -> std::convertible_to<bool>;
                }) {
    if (!page.OnQueryClosePage(page.ClosedWith())) {
      throw Error("the page refused to close (OnQueryClosePage)");
    }
  }
  if constexpr (requires { page.OnClosePage(); }) { page.OnClosePage(); }
}

/// \brief Hands the record a `Page.Run(Rec)` names to the page: its filters, and its position.
/// \tparam P      The generated page class.
/// \tparam Record What was passed; only the page's own source table is taken.
/// \param page   The page.
/// \param record The argument.
template <typename P, typename Record> void AdoptRecord(P &page, const Record &record) {
  if constexpr (requires { page.Rec.Copy(record); }) {
    page.Rec.Copy(record);
  } else {
    static_cast<void>(record);
  }
}

/// \brief AL `Page.Run(Rec)` / `Page.RunModal(Rec)` on a generated page, headless.
///
/// \tparam P         The generated page class.
/// \tparam Arguments The record, when one was passed.
/// \param modal     Whether it is `RunModal`.
/// \param arguments The record, when one was passed.
/// \return The action the page closed with.
/// \throws Error when no test harness answers: AL's `Unhandled UI` for a page a test did not
///         trap or declare a handler for.
///
/// \note THE PAGE RUNS FOR WHOEVER CATCHES IT. A non-modal run goes to a `TestPage.Trap()` first,
///       and the harness then owns the page and drives it; otherwise a `[PageHandler]` or
///       `[ModalPageHandler]` for this page number is invoked with the page, already opened, and
///       the page closes when the handler returns. There is no third case yet: a page nobody
///       waits for is an unhandled UI, which is what AL says too (board:0030).
template <typename P, typename... Arguments>
::agiru::Action RunPage(bool modal, const Arguments &...arguments) {
  auto page = std::make_unique<P>();
  (AdoptRecord(*page, arguments), ...);
  OpenPage(*page, true, false);
  const std::int32_t id = PageTraits<P>::kId.Value();
  if (!modal && ReleaseTrap(id, page.get())) {
    static_cast<void>(page.release());
    return ::agiru::Action::OK;
  }
  const TestHandler *handler =
      HandlerTable::For(modal ? HandlerKind::ModalPage : HandlerKind::Page, id);
  if (handler == nullptr) {
    throw Error(std::string("Unhandled UI: ") + (modal ? "ModalPage " : "Page ") +
                std::string(PageTraits<P>::kName));
  }
  handler->invoke(PageTraits<P>::kName, page.get());
  HandlerTable::Ran(*handler);
  ClosePage(*page);
  return page->ClosedWith();
}

}

template <typename Derived = void> class Page {
public:
  /// \brief Marks the page opened, in the mode a runner chose.
  /// \param editable Whether `OpenEdit`/`OpenNew` (true) or `OpenView` (false).
  void OpenedAs(bool editable) { editable_ = editable; }

  /// \brief Whether the page was opened for editing.
  /// \return True after `OpenEdit` or `OpenNew`.
  [[nodiscard]] bool OpenedEditable() const { return editable_; }

  /// \brief Records the action the page closed with (`OK`, `Cancel`, `Yes`, `No`, `LookupOK`).
  /// \param action The action.
  void CloseWith(::agiru::Action action) { closeAction_ = action; }

  /// \brief What `Page.RunModal` answers: the action the page closed with.
  /// \return The action; `OK` when nothing said otherwise.
  [[nodiscard]] ::agiru::Action ClosedWith() const { return closeAction_; }

  /// \brief The page's AL number.
  /// \return The number AL declared.
  [[nodiscard]] static constexpr PageId Id() { return PageTraits<Derived>::kId; }

  /// \brief The page's AL name.
  /// \return The name AL declared.
  [[nodiscard]] static constexpr std::string_view Name() { return PageTraits<Derived>::kName; }

  /// \brief AL `Page.Run(PageId [, Record])` -- shows the page.
  ///
  /// \tparam Arguments What AL handed it: nothing, or the record to open on.
  /// \param arguments The record, when there is one.
  /// \throws Error until the UI runs (board:0030).
  ///
  /// \note AL NAMES THE KIND TWICE AND THE GENERATED FORM ONCE. `Page.Run(Page::"X", Rec)` becomes
  ///       `pages::X::Run(Rec)`: the object is the receiver, which is what the call means.
  template <typename... Arguments> static void Run(const Arguments &...arguments) {
    if constexpr (std::is_void_v<Derived>) {
      (static_cast<void>(arguments), ...);
      throw Error("Page.Run by number needs the page catalogue (board:0030)");
    } else {
      static_cast<void>(detail::RunPage<Derived>(false, arguments...));
    }
  }

  /// \brief AL `Page.RunModal(PageId [, Record])` -- shows the page and waits for it.
  /// \tparam Arguments What AL handed it.
  /// \param arguments The record, when there is one.
  /// \return The action the user closed it with.
  /// \throws Error until the UI runs (board:0030).
  /// \note IT RETURNS AN `Action` AND NOT AN `Integer`. Every `RunModal` page names
  ///       `Type: Action` as its return, and AL compares it against `Action::LookupOK` --
  ///       `if Page.RunModal(...) = Action::LookupOK then` is how the whole BaseApp reads a
  ///       lookup. An `Integer` there is not comparable to an `Action` and never was.
  /// \warning A CONTROL HAS ITS OWN TRIGGERS AND THEY ARE THE FIELD'S, NOT THE PAGE'S: a page
  ///          field runs `OnLookup`, `OnDrillDown`, `OnAssistEdit` and `OnControlAddIn`, and around
  ///          a write `OnBeforeValidate` then `OnAfterValidate`; a `pagefieldextension` adds
  ///          `OnAfterAfterLookup` beside them, an `actionextension` `OnBeforeAction` and
  ///          `OnAfterAction`. A table field's own `OnLookup` is the one AL declares in the TABLE
  ///          and the page's control inherits. Each is one page under `triggers-auto/`.

  /// \warning THE PLATFORM EVENTS FIRE WHETHER OR NOT THE PAGE DECLARES THE TRIGGER, which is why
  ///          they are the RUNTIME's and never the generated object's: `OnOpenPageEvent`,
  ///          `OnClosePageEvent`, `OnQueryClosePageEvent`, `OnAfterGetRecordEvent`,
  ///          `OnAfterGetCurrRecordEvent`, `OnNewRecordEvent`, `OnInsertRecordEvent`,
  ///          `OnModifyRecordEvent`, `OnDeleteRecordEvent`, and around an action
  ///          `OnBeforeActionEvent` then `OnAfterActionEvent`. A page that declares none of them
  ///          still raises all of them, the way `Table::Insert` raises `OnBeforeInsertEvent`
  ///          without asking the table.
  ///
  /// \warning THE TRIGGER ORDER IS OWED AND NAMED HERE, because a page that runs must run them in
  ///          the platform's order and not in the generator's: `OnInit`, then `OnOpenPage`, then
  ///          per record `OnFindRecord`, `OnAfterGetRecord` and `OnAfterGetCurrRecord`; a write
  ///          runs `OnInsertRecord`, `OnModifyRecord` or `OnDeleteRecord`; the close runs
  ///          `OnQueryClosePage` and then `OnClosePage`. Each is one page under `triggers-auto/`,
  ///          and naming them here is what keeps them from being a silent hole while the UI is
  ///          board:0030's work -- nothing fires until there is a page to fire it on.
  template <typename... Arguments> static ::agiru::Action RunModal(const Arguments &...arguments) {
    if constexpr (std::is_void_v<Derived>) {
      (static_cast<void>(arguments), ...);
      throw Error("Page.RunModal by number needs the page catalogue (board:0030)");
    } else {
      return detail::RunPage<Derived>(true, arguments...);
    }
  }

  /// \brief AL `Page.Activate(Boolean)`. Activates the current page on the client if possible. The
  /// data on the page will not be refreshed.
  /// \param Refresh The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
  /// \brief AL `Page.Activate()` -- the READING form, which the documentation brackets:
  ///        `[X := ] Page.Activate([NewX])`.
  /// \return Never.
  /// \throws Error always -- a page property needs a running UI (board:0030).
  [[nodiscard]] ::agiru::Boolean Activate() const {
    throw Error("Page.Activate() needs a running UI (board:0030)");
  }

  ::agiru::Boolean Activate(::agiru::Boolean Refresh) {
    static_cast<void>(Refresh);
    throw Error("Page.Activate(Boolean) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.CancelBackgroundTask(Integer)`. Attempt to cancel a page background task.
  /// \param TaskId The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::Boolean CancelBackgroundTask(::agiru::Integer TaskId) {
    static_cast<void>(TaskId);
    throw Error("Page.CancelBackgroundTask(Integer) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.Caption()` -- the READING form, which the documentation's syntax block
  /// brackets: `[X := ] Page.Caption([NewCaption])`.
  /// \return The caption the page shows.
  /// \throws Error until the UI runs (board:0030).
  std::string Caption() const { throw Error("Page.Caption() needs a running UI (board:0030)"); }

  /// \brief AL `Page.Caption(Text)`. The caption shown in the title bar. For example, the default
  /// value in English (United States) is the same as the name of the page.
  /// \param NewCaption The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error until the UI runs (board:0030).
  std::string Caption(std::string_view NewCaption) {
    static_cast<void>(NewCaption);
    throw Error("Page.Caption(Text) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.Close()`. Closes the current page.
  /// \throws Error until the UI runs (board:0030).
  void Close() { throw Error("Page.Close() needs a running UI (board:0030)"); }

  /// \brief AL `Page.Editable(Boolean)`. Gets or sets the default editability of the page.
  /// \param NewEditable The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
  /// \brief AL `Page.Editable()` -- the READING form, which the documentation brackets:
  ///        `[X := ] Page.Editable([NewX])`.
  /// \return Never.
  /// \throws Error always -- a page property needs a running UI (board:0030).
  [[nodiscard]] ::agiru::Boolean Editable() const {
    throw Error("Page.Editable() needs a running UI (board:0030)");
  }

  ::agiru::Boolean Editable(::agiru::Boolean NewEditable) {
    static_cast<void>(NewEditable);
    throw Error("Page.Editable(Boolean) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.EnqueueBackgroundTask(Integer, Integer, Dictionary of [Text, Text], Integer,
  /// PageBackgroundTaskErrorLevel)`. Creates and queues a background task that runs the specified
  /// codeunit (without a UI) in a read-only child session of the page session. If the task
  /// completes successfully, the **OnPageBackgroundTaskCompleted** trigger is invoked. If an error
  /// occurs, the **OnPageBackgroundTaskError** trigger is invoked. If the page is closed before the
  /// task completes, or the page record ID on the task changed, the task is cancelled.
  /// \param TaskId The AL `Integer`.
  /// \param CodeunitId The AL `Integer`.
  /// \param Parameters The AL `Dictionary of [Text, Text]`.
  /// \param Timeout The AL `Integer`.
  /// \param ErrorLevel The AL `PageBackgroundTaskErrorLevel`.
  /// \return The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::Boolean
  EnqueueBackgroundTask(::agiru::Integer &TaskId,
                        ::agiru::Integer CodeunitId,
                        ::agiru::Dictionary<::agiru::Text<0>, ::agiru::Text<0>> &Parameters,
                        ::agiru::Integer Timeout = {},
                        const ::agiru::PageBackgroundTaskErrorLevel &ErrorLevel =
                            ::agiru::PageBackgroundTaskErrorLevel{}) {
    static_cast<void>(TaskId);
    static_cast<void>(CodeunitId);
    static_cast<void>(Parameters);
    static_cast<void>(Timeout);
    static_cast<void>(ErrorLevel);
    throw Error("Page.EnqueueBackgroundTask(Integer, Integer, Dictionary of [Text, Text], Integer, "
                "PageBackgroundTaskErrorLevel) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.GetBackgroundParameters()`. Gets the page background task input parameters.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::Dictionary<::agiru::Text<0>, ::agiru::Text<0>> GetBackgroundParameters() {
    throw Error("Page.GetBackgroundParameters() needs a running UI (board:0030)");
  }

  /// \brief AL `Page.GetRecord(Record)`. Gets the current record of the page.
  /// \tparam Record The table the caller hands over.
  /// \param record The record.
  /// \throws Error until the UI runs (board:0030).
  /// \note A BARE `Record` PARAMETER TAKES ANY TABLE, which is what makes it a template here.
  ///       `RecordRef` is a different AL type -- a record reached by NUMBER -- and using it
  ///       would refuse every call that hands over a record it has.
  template <typename Record> void GetRecord(Record &record) {
    static_cast<void>(record);
    throw Error("Page.GetRecord(Record) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.LookupMode()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] Page.LookupMode([NewX])`.
  /// \return The value it holds.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::Boolean LookupMode() const {
    throw Error("Page.LookupMode() needs a running UI (board:0030)");
  }

  /// \brief AL `Page.LookupMode(Boolean)`. Gets or sets the default lookup mode for the page.
  /// \param NewLookupMode The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::Boolean LookupMode(::agiru::Boolean NewLookupMode) {
    static_cast<void>(NewLookupMode);
    throw Error("Page.LookupMode(Boolean) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.ObjectId()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] Page.ObjectId([NewX])`.
  /// \return The value it holds.
  /// \throws Error until the UI runs (board:0030).
  [[nodiscard]] std::string ObjectId() const {
    throw Error("Page.ObjectId() needs a running UI (board:0030)");
  }

  /// \brief AL `Page.ObjectId(Boolean)`. Returns a string in the "Page xxx" format, where xxx is
  /// the caption or ID of the application object.
  /// \param UseNames The AL `Boolean`.
  /// \return The AL `Text`.
  /// \throws Error until the UI runs (board:0030).
  std::string ObjectId(::agiru::Boolean UseNames) {
    static_cast<void>(UseNames);
    throw Error("Page.ObjectId(Boolean) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.PromptMode()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] Page.PromptMode([NewX])`.
  /// \return The value it holds.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::PromptMode PromptMode() const {
    throw Error("Page.PromptMode() needs a running UI (board:0030)");
  }

  /// \brief AL `Page.PromptMode(PromptMode)`. The mode of a PromptDialog page that prompts the user
  /// for input and shows the output of a copilot interaction.
  /// \param NewPromptMode The AL `PromptMode`.
  /// \return The AL `PromptMode`.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::PromptMode PromptMode(const ::agiru::PromptMode &NewPromptMode) {
    static_cast<void>(NewPromptMode);
    throw Error("Page.PromptMode(PromptMode) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.SaveRecord()`. Saves the current record as if performed by the client. If the
  /// record does not exist it is inserted, otherwise it is modified.
  /// \throws Error until the UI runs (board:0030).
  void SaveRecord() { throw Error("Page.SaveRecord() needs a running UI (board:0030)"); }

  /// \brief AL `Page.SetBackgroundTaskResult(Dictionary of [Text, Text])`. Sets the page background
  /// task result as a dictionary. When the task is completed, the OnPageBackgroundCompleted trigger
  /// will be invoked on the page with this result dictionary.
  /// \param Results The AL `Dictionary of [Text, Text]`.
  /// \throws Error until the UI runs (board:0030).
  void
  SetBackgroundTaskResult(const ::agiru::Dictionary<::agiru::Text<0>, ::agiru::Text<0>> &Results) {
    static_cast<void>(Results);
    throw Error(
        "Page.SetBackgroundTaskResult(Dictionary of [Text, Text]) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.SetRecord(Record)`. Sets the current record for the page.
  /// \tparam Record The table the caller hands over.
  /// \param record The record.
  /// \throws Error until the UI runs (board:0030).
  /// \note A BARE `Record` PARAMETER TAKES ANY TABLE, which is what makes it a template here.
  ///       `RecordRef` is a different AL type -- a record reached by NUMBER -- and using it
  ///       would refuse every call that hands over a record it has.
  template <typename Record> void SetRecord(Record &record) {
    static_cast<void>(record);
    throw Error("Page.SetRecord(Record) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.SetSelectionFilter(Record)`. Notes the records that the user has selected on
  /// the page, marks those records in the table specified, and sets the filter to "marked only".
  /// \tparam Record The table the caller hands over.
  /// \param record The record.
  /// \throws Error until the UI runs (board:0030).
  /// \note A BARE `Record` PARAMETER TAKES ANY TABLE, which is what makes it a template here.
  ///       `RecordRef` is a different AL type -- a record reached by NUMBER -- and using it
  ///       would refuse every call that hands over a record it has.
  template <typename Record> void SetSelectionFilter(Record &record) {
    static_cast<void>(record);
    throw Error("Page.SetSelectionFilter(Record) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.SetTableView(Record)`. Applies the table view on the current record as the
  /// table view for the page, report, or XmlPort.
  /// \tparam Record The table the caller hands over.
  /// \param record The record.
  /// \throws Error until the UI runs (board:0030).
  /// \note A BARE `Record` PARAMETER TAKES ANY TABLE, which is what makes it a template here.
  ///       `RecordRef` is a different AL type -- a record reached by NUMBER -- and using it
  ///       would refuse every call that hands over a record it has.
  template <typename Record> void SetTableView(Record &record) {
    static_cast<void>(record);
    throw Error("Page.SetTableView(Record) needs a running UI (board:0030)");
  }

  /// \brief AL `Page.Update(Boolean)`. Saves the current record and then updates the controls on
  /// the page. If you set the SaveRecord parameter to false, this method will not save the record
  /// before the page is updated.
  /// \param SaveRecord The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
  void Update(::agiru::Boolean SaveRecord = true) {
    static_cast<void>(SaveRecord);
    throw Error("Page.Update(Boolean) needs a running UI (board:0030)");
  }

  /// \note NO PROTECTED DESTRUCTOR AND NO PRIVATE CONSTRUCTOR, for the reason `Table` gives: a
  ///       generated class has no user-declared constructor, so `pages::X P{}` is aggregate
  ///       initialisation and both of those make it fail from the caller's context.

private:
  bool editable_ = true;
  ::agiru::Action closeAction_ = ::agiru::Action::OK;
};

/// \brief AL `Page.Run(Number, ...)` and `Page.RunModal(Number, ...)` by object NUMBER, the way
///        `Codeunit<void>` carries `Codeunit.Run(Number)`: refused until the page catalogue exists
///        (board:0038).
template <> class Page<void> {
public:
  /// \brief AL `PAGE.GetBackgroundParameters(...)` -- the parameters a page background task was
  /// started with.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- there is no running page yet (board:0030).
  template <typename... Arguments>
  static ::agiru::Text<0> GetBackgroundParameters(Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Page.GetBackgroundParameters is declared and needs a running UI (board:0030)");
  }

  /// \brief AL `PAGE.SetFilterToMultipleValues(...)` -- a filter naming several values at once.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- there is no running page yet (board:0030).
  template <typename... Arguments> static void SetFilterToMultipleValues(Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Page.SetFilterToMultipleValues is declared and needs a running UI (board:0030)");
  }

  template <typename... Arguments>
  static void Run(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Page.Run(" + std::to_string(Number) +
                ") by number needs the page catalogue (board:0038)");
  }

  template <typename... Arguments>
  static ::agiru::Action RunModal(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Page.RunModal(" + std::to_string(Number) +
                ") by number needs the page catalogue (board:0038)");
  }
};

}

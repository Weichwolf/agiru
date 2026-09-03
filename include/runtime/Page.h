#pragma once

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "type/Boolean.h"
#include "type/Dictionary.h"
#include "type/Integer.h"
#include "type/PageBackgroundTaskErrorLevel.h"
#include "type/PageStyle.h"
#include "type/PromptMode.h"
#include "type/Variant.h"

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

/// \brief What every AL page can do, without the generated class saying any of it.
/// \tparam Derived The generated page class.
template <typename Derived> class Page {
public:
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
  template <typename... Arguments> static void Run(Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Page.Run needs a running UI (board:0030)");
  }

  /// \brief AL `Page.RunModal(PageId [, Record])` -- shows the page and waits for it.
  /// \tparam Arguments What AL handed it.
  /// \param arguments The record, when there is one.
  /// \return The action the user closed it with.
  /// \throws Error until the UI runs (board:0030).
  template <typename... Arguments> static Integer RunModal(Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Page.RunModal needs a running UI (board:0030)");
  }

  // WHAT A PAGE VARIABLE CAN DO, from `methods-auto/page/`. AL writes `P.SetTableView(Rec)` and
  // `P.GetRecord(Rec)` on a page it is about to run, so these are the page's own surface and not
  // the test framework's -- `TestPage` is a different type with a different one.
  /// \brief AL `Page.Activate(Boolean)`. Activates the current page on the client if possible. The
  /// data on the page will not be refreshed.
  /// \param Refresh The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
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
  ::agiru::Boolean EnqueueBackgroundTask(::agiru::Integer &TaskId,
                                         ::agiru::Integer CodeunitId,
                                         ::agiru::Dictionary<std::string, std::string> &Parameters,
                                         ::agiru::Integer Timeout,
                                         const ::agiru::PageBackgroundTaskErrorLevel &ErrorLevel) {
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
  void GetBackgroundParameters() {
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

  /// \brief AL `Page.LookupMode(Boolean)`. Gets or sets the default lookup mode for the page.
  /// \param NewLookupMode The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error until the UI runs (board:0030).
  ::agiru::Boolean LookupMode(::agiru::Boolean NewLookupMode) {
    static_cast<void>(NewLookupMode);
    throw Error("Page.LookupMode(Boolean) needs a running UI (board:0030)");
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
  void SetBackgroundTaskResult(const ::agiru::Dictionary<std::string, std::string> &Results) {
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
  void Update(::agiru::Boolean SaveRecord) {
    static_cast<void>(SaveRecord);
    throw Error("Page.Update(Boolean) needs a running UI (board:0030)");
  }

  /// \note NO PROTECTED DESTRUCTOR AND NO PRIVATE CONSTRUCTOR, for the reason `Table` gives: a
  ///       generated class has no user-declared constructor, so `pages::X P{}` is aggregate
  ///       initialisation and both of those make it fail from the caller's context.
};

} // namespace agiru

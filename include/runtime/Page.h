#pragma once

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/test/TestAction.h"
#include "type/Action.h"
#include "type/Boolean.h"
#include "type/Dictionary.h"
#include "type/Integer.h"
#include "type/PageBackgroundTaskErrorLevel.h"
#include "type/PageStyle.h"
#include "type/PromptMode.h"
#include "type/Text.h"
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
  TestAction OK() { throw Error("a TestRequestPage needs a report (board:0034)"); }

  /// \brief AL `TestRequestPage.Cancel()` -- closes the request page without running.
  /// \return Whether it closed.
  /// \throws Error until a report can be run (board:0034).
  TestAction Cancel() { throw Error("a TestRequestPage needs a report (board:0034)"); }

  /// \brief AL `TestRequestPage.Schedule()` -- queues the report on the job queue.
  /// \return The AL `TestAction` the page's own documentation names as the return
  ///         (`testrequestpage-schedule-method.md`).
  /// \throws Error until a report can be run (board:0034).
  TestAction Schedule() { throw Error("a TestRequestPage needs a report (board:0034)"); }

  /// \brief AL `TestRequestPage.GoToRecord(Record)` -- positions the request page on a record.
  /// \tparam Source The record's type.
  /// \param Record The record to stand on.
  /// \return Whether the request page could.
  /// \throws Error until a report runs (board:0034).
  template <typename Source> Boolean GoToRecord(const Source &Record) {
    static_cast<void>(Record);
    throw Error("a TestRequestPage needs a report (board:0034)");
  }

  /// \brief AL `TestRequestPage.Caption()` -- the caption the request page shows.
  /// \return The caption.
  /// \throws Error until a report can be run (board:0034).
  [[nodiscard]] ::agiru::Text<0> Caption() const { Unrun(); }

  /// \brief AL `TestRequestPage.Editable()` -- whether the request page may be typed into.
  /// \return Whether it is editable.
  /// \throws Error until a report can be run (board:0034).
  [[nodiscard]] Boolean Editable() const { Unrun(); }

  /// \brief AL `TestRequestPage.Expand(Boolean)` -- expands or collapses the current row.
  /// \param Expand True to expand, false to collapse.
  /// \throws Error until a report can be run (board:0034).
  void Expand(Boolean Expand) {
    static_cast<void>(Expand);
    Unrun();
  }

  /// \brief AL `TestRequestPage.IsExpanded()` -- whether the current row is expanded.
  /// \return Whether it is.
  /// \throws Error until a report can be run (board:0034).
  [[nodiscard]] Boolean IsExpanded() const { Unrun(); }

  /// \brief AL `TestRequestPage.FindFirstField(TestField, Any)` -- the first row whose field
  ///        carries the value.
  /// \tparam Value What the field is compared against.
  /// \param Field The control to look in.
  /// \param value The value to find.
  /// \return Whether a row carries it.
  /// \throws Error until a report can be run (board:0034).
  template <typename Field, typename Value> Boolean FindFirstField(const Field &, const Value &) {
    Unrun();
  }

  /// \brief AL `TestRequestPage.FindNextField(TestField, Any)`.
  /// \tparam Value What the field is compared against.
  /// \return Whether a later row carries it.
  /// \throws Error until a report can be run (board:0034).
  template <typename Field, typename Value> Boolean FindNextField(const Field &, const Value &) {
    Unrun();
  }

  /// \brief AL `TestRequestPage.FindPreviousField(TestField, Any)`.
  /// \tparam Value What the field is compared against.
  /// \return Whether an earlier row carries it.
  /// \throws Error until a report can be run (board:0034).
  template <typename Field, typename Value>
  Boolean FindPreviousField(const Field &, const Value &) {
    Unrun();
  }

  /// \brief AL `TestRequestPage.First()` -- moves to the first row.
  /// \return Whether there is one.
  /// \throws Error until a report can be run (board:0034).
  Boolean First() { Unrun(); }

  /// \brief AL `TestRequestPage.Next()` -- moves to the next row.
  /// \return Whether there is one.
  /// \throws Error until a report can be run (board:0034).
  Boolean Next() { Unrun(); }

  /// \brief AL `TestRequestPage.Previous()` -- moves to the previous row.
  /// \return Whether there is one.
  /// \throws Error until a report can be run (board:0034).
  Boolean Previous() { Unrun(); }

  /// \brief AL `TestRequestPage.Last()` -- moves to the last row.
  /// \return Whether there is one.
  /// \throws Error until a report can be run (board:0034).
  Boolean Last() { Unrun(); }

  /// \brief AL `TestRequestPage.New()` -- starts a new row.
  /// \throws Error until a report can be run (board:0034).
  void New() { Unrun(); }

  /// \brief AL `TestRequestPage.GoToKey(Value, ...)` -- positions on a row by its key.
  /// \tparam Values The key's fields, in the key's order.
  /// \return Whether a row carries that key.
  /// \throws Error until a report can be run (board:0034).
  ///
  /// \note VARIADIC BECAUSE THE KEY IS. `testrequestpage-gotokey-method.md` writes
  ///       `GoToKey([Value: Any,...])`, and a primary key is up to sixteen fields.
  template <typename... Values> Boolean GoToKey(const Values &...) { Unrun(); }

  /// \brief AL `TestRequestPage.GetValidationError([Integer])` -- one of the errors the request
  ///        page collected.
  /// \param Index Which one, one-based; the first when omitted.
  /// \return The error text.
  /// \throws Error until a report can be run (board:0034).
  [[nodiscard]] ::agiru::Text<0> GetValidationError(Integer Index = {}) const {
    static_cast<void>(Index);
    Unrun();
  }

  /// \brief AL `TestRequestPage.ValidationErrorCount()` -- how many it collected.
  /// \return The count.
  /// \throws Error until a report can be run (board:0034).
  [[nodiscard]] Integer ValidationErrorCount() const { Unrun(); }

  /// \brief AL `TestRequestPage.Preview()` -- closes the request page and previews the report.
  /// \return What the page answered.
  /// \throws Error until a report can be run (board:0034).
  TestAction Preview() { Unrun(); }

  /// \brief AL `TestRequestPage.Print()` -- closes the request page and prints the report.
  /// \return What the page answered.
  /// \throws Error until a report can be run (board:0034).
  TestAction Print() { Unrun(); }

  /// \brief AL `TestRequestPage.SaveAsExcel(Text)` -- runs the report into a workbook.
  /// \param FileName Where it goes.
  /// \throws Error until a report can be run (board:0034).
  void SaveAsExcel(std::string_view FileName) {
    static_cast<void>(FileName);
    Unrun();
  }

  /// \brief AL `TestRequestPage.SaveAsPdf(Text)` -- runs the report into a PDF.
  /// \param FileName Where it goes.
  /// \throws Error until a report can be run (board:0034).
  void SaveAsPdf(std::string_view FileName) {
    static_cast<void>(FileName);
    Unrun();
  }

  /// \brief AL `TestRequestPage.SaveAsWord(Text)` -- runs the report into a document.
  /// \param FileName Where it goes.
  /// \throws Error until a report can be run (board:0034).
  void SaveAsWord(std::string_view FileName) {
    static_cast<void>(FileName);
    Unrun();
  }

  /// \brief AL `TestRequestPage.SaveAsXml(Text, Text)` -- writes the parameters and the data set.
  /// \param ParameterFileName Where the request page's parameters go.
  /// \param DataSetFileName   Where the report's data set goes.
  /// \throws Error until a report can be run (board:0034).
  void SaveAsXml(std::string_view ParameterFileName, std::string_view DataSetFileName) {
    static_cast<void>(ParameterFileName);
    static_cast<void>(DataSetFileName);
    Unrun();
  }

private:
  /// \brief The one refusal every method above raises.
  /// \throws Error always -- a request page needs a report, and reports are board:0034.
  [[noreturn]] static void Unrun() { throw Error("a TestRequestPage needs a report (board:0034)"); }
};

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

template <typename Derived = void> class Page {
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
  template <typename... Arguments> static ::agiru::Action RunModal(Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Page.RunModal needs a running UI (board:0030). When it runs it owes OnInit, "
                "OnOpenPage, OnFindRecord, OnNextRecord, OnNewRecord, OnAfterGetRecord, "
                "OnAfterGetCurrRecord, and on close OnQueryClosePage then OnClosePage");
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
  ::agiru::Dictionary<std::string, std::string> GetBackgroundParameters() {
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
  void Update(::agiru::Boolean SaveRecord = true) {
    static_cast<void>(SaveRecord);
    throw Error("Page.Update(Boolean) needs a running UI (board:0030)");
  }

  /// \note NO PROTECTED DESTRUCTOR AND NO PRIVATE CONSTRUCTOR, for the reason `Table` gives: a
  ///       generated class has no user-declared constructor, so `pages::X P{}` is aggregate
  ///       initialisation and both of those make it fail from the caller's context.
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

#pragma once

#include "runtime/Error.h"
#include "runtime/Report.h"
#include "runtime/test/TestAction.h"
#include "runtime/test/TestField.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"

#include <string_view>

/// \file
/// \brief AL `TestRequestPage` -- a report's request page, driven by a test.

namespace agiru {

/// \brief The stand-in for a report the transpiler has not translated.
class UnknownReport {};

/// \brief A data item on a request page is the record of its table.
/// \tparam T The table's generated class.
template <typename T> using DataItem = T;

}

/// \brief The controls a `TestRequestPage` over an untranslated report has, which is none.
template <> struct agiru::ReportTraits<agiru::UnknownReport> {
  /// \brief No controls at all.
  /// \tparam Field_Kind  How a test reaches a request-page field.
  /// \tparam Filter_Kind How a test reaches a data item's filters.
  template <typename Field_Kind, template <typename> class Filter_Kind>
  using Controls = agiru::UnknownReport;
};

namespace agiru {

/// \brief AL `TestRequestPage <Report>` -- a report's request page, driven without a screen.
///
/// \tparam R The generated report class.
///
/// \note A DATA ITEM IS A RECORD, ON THE REQUEST PAGE AS IN THE REPORT. AL writes
///       `RequestPage.Vendor.SetFilter("No.", '10000')`: the tab named after the data item carries
///       the filters of ITS TABLE, and the field named inside it is that table's field. So the
///       control is the table's own record class, whose `SetFilter`, `GetFilter` and
///       `SetCurrentKey` are the platform's, and the field is its member. A request-page `field`
///       is a `TestField`, the way it is on a `TestPage`.
template <typename R = UnknownReport>
class TestRequestPage : public ReportTraits<R>::template Controls<TestField, DataItem> {
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
  template <typename Field, typename Value>
  Boolean FindFirstField(const Field &field, const Value &value) {
    static_cast<void>(field);
    static_cast<void>(value);
    Unrun();
  }

  /// \brief AL `TestRequestPage.FindNextField(TestField, Any)`.
  /// \tparam Value What the field is compared against.
  /// \return Whether a later row carries it.
  /// \throws Error until a report can be run (board:0034).
  template <typename Field, typename Value>
  Boolean FindNextField(const Field &field, const Value &value) {
    static_cast<void>(field);
    static_cast<void>(value);
    Unrun();
  }

  /// \brief AL `TestRequestPage.FindPreviousField(TestField, Any)`.
  /// \tparam Value What the field is compared against.
  /// \return Whether an earlier row carries it.
  /// \throws Error until a report can be run (board:0034).
  template <typename Field, typename Value>
  Boolean FindPreviousField(const Field &field, const Value &value) {
    static_cast<void>(field);
    static_cast<void>(value);
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
  template <typename... Values> Boolean GoToKey(const Values &...values) {
    (static_cast<void>(values), ...);
    Unrun();
  }

  /// \brief AL `TestRequestPage.GetValidationError([Integer])` -- one of the errors the request
  ///        page collected.
  /// \param Index Which one, one-based; the first when omitted.
  /// \return The error text.
  /// \throws Error until a report can be run (board:0034).
  [[nodiscard]] ::agiru::Text<0> GetValidationError(::agiru::Integer Index = {}) const {
    static_cast<void>(Index);
    Unrun();
  }

  /// \brief AL `TestRequestPage.ValidationErrorCount()` -- how many it collected.
  /// \return The count.
  /// \throws Error until a report can be run (board:0034).
  [[nodiscard]] ::agiru::Integer ValidationErrorCount() const { Unrun(); }

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

}

#pragma once

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/DefaultLayout.h"
#include "type/Integer.h"
#include "type/ReportFormat.h"
#include "type/SecurityFilter.h"
#include "type/Text.h"
#include "type/TextEncoding.h"

#include <string>

/// \file
/// \brief AL `REPORT` and `XMLPORT` -- the platform objects a body reaches BY NUMBER.

namespace agiru {

/// \brief What the runtime knows about a generated report: its number and its name.
/// \tparam T The report's generated class.
template <typename T> struct ReportTraits;

/// \brief What the runtime knows about a generated xmlport: its number and its name.
/// \tparam T The xmlport's generated class.
template <typename T> struct XmlPortTraits;

/// \brief AL's `REPORT` object: `REPORT.Run(Number)`, `REPORT.RunModal(Number)`.
///
/// \tparam Derived The report's generated class, or `void` for the platform object AL spells
///         `REPORT`. A report's own class is generated under `apps/`, carries its number and its
///         name, and refuses its members the same way (board:0034).
///
/// \note IT IS THE PLATFORM HALF AND NOT THE REPORT. A report's BODY is not translated yet, so
///       running one by number has nothing to run; refusing here names the number the body asked
///       for, which a missing symbol never would.
template <typename Derived = void> class Report {
public:
  /// \brief The report's AL number.
  /// \return The number AL declared.
  [[nodiscard]] static constexpr ReportId Id() { return ReportTraits<Derived>::kId; }

  /// \brief AL `Report.Break()`. Stops processing the current data item.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments> void Break(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.Break is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.CreateTotals(...)`. Names the variables the platform totals.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments> void CreateTotals(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.CreateTotals is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.DefaultLayout()`. The built-in layout the report is rendered with.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>
  ::agiru::DefaultLayout DefaultLayout(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.DefaultLayout is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.ExcelLayout(var InStream)`. Reads the report's Excel layout.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Boolean ExcelLayout(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.ExcelLayout is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.Execute(...)`. Runs the report without its request page.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments> void Execute(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.Execute is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.FormatRegion([FormatRegion])`. The format region the run uses.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Text<0> FormatRegion(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.FormatRegion is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.IsReadOnly()`. Whether the report reads at a read-only intent.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Boolean IsReadOnly(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.IsReadOnly is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.Language([Language])`. The language the run renders in.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Integer Language(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.Language is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.NewPage()`. Starts a new page in the output.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments> void NewPage(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.NewPage is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.NewPagePerRecord([Set])`. Whether every record starts a page.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>
  ::agiru::Boolean NewPagePerRecord(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.NewPagePerRecord is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.ObjectId([UseNames])`. The object's identifier as text.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Text<0> ObjectId(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.ObjectId is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.PageNo([NewPageNo])`. The page number the run stands on.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Integer PageNo(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.PageNo is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.PaperSource(PaperBinNo [, PhysicalPage])`. Picks the printer's tray.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments> void PaperSource(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.PaperSource is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.Preview()`. Whether the run is a preview rather than an output.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Boolean Preview(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.Preview is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.Print(...)`. Sends the report to a printer.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments> void Print(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.Print is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.PrintOnlyIfDetail([Set])`. Whether a section prints only with detail under
  /// it.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>
  ::agiru::Boolean PrintOnlyIfDetail(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.PrintOnlyIfDetail is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.Quit()`. Ends the run without producing output.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments> void Quit(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.Quit is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.RDLCLayout(var InStream)`. Reads the report's RDLC layout.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Boolean RDLCLayout(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.RDLCLayout is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.Run()`. Runs the report.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments> void Run(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.Run is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.RunModal()`. Runs the report and waits for it.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments> void RunModal(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.RunModal is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.RunRequestPage([PageParameters])`. Opens the request page and returns its
  /// parameters.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Text<0> RunRequestPage(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.RunRequestPage is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.SaveAs(Parameters, Format, var OutStream [, RecordRef])`. Renders into a
  /// stream.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Boolean SaveAs(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAs is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.SaveAsExcel(FileName)`. Renders the report as a workbook.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Boolean SaveAsExcel(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.SaveAsExcel is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.SaveAsHtml(FileName)`. Renders the report as HTML.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Boolean SaveAsHtml(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAsHtml is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.SaveAsPdf(FileName)`. Renders the report as a PDF.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Boolean SaveAsPdf(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAsPdf is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.SaveAsWord(FileName)`. Renders the report as a Word document.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Boolean SaveAsWord(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAsWord is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.SaveAsXml(FileName)`. Renders the report's dataset as XML.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Boolean SaveAsXml(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAsXml is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.SetTableView(var Record)`. Gives a data item the record's filters and key.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments> void SetTableView(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.SetTableView is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.ShowOutput([Value])`. Whether the current section is printed.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Boolean ShowOutput(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.ShowOutput is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.Skip()`. Leaves the current record out of the dataset.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments> void Skip(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.Skip is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.TargetFormat()`. The format the run renders into.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>
  ::agiru::ReportFormat TargetFormat(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.TargetFormat is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.TotalsCausedBy()`. The field number whose change caused the total.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Integer TotalsCausedBy(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.TotalsCausedBy is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.UseRequestPage([Set])`. Whether the run opens its request page.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Boolean UseRequestPage(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.UseRequestPage is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.ValidateAndPrepareLayout(LayoutStream, var PreparedLayoutStream,
  /// ReportLayoutType)`. Checks a layout before it is used.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>
  ::agiru::Boolean ValidateAndPrepareLayout(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.ValidateAndPrepareLayout is declared and has no translated report body yet "
                "(board:0063)");
  }

  /// \brief AL `Report.WordLayout(var InStream)`. Reads the report's Word layout.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Boolean WordLayout(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.WordLayout is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.WordXmlPart([ExtendedFormat])`. The report's Word XML part.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>::agiru::Text<0> WordXmlPart(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Report.WordXmlPart is declared and has no translated report body yet (board:0063)");
  }

  /// \brief AL `Report.GetSubstituteReportId(...)`. The report an event substituted for this one.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- report bodies are not translated yet (board:0063).
  template <typename... Arguments>
  ::agiru::Boolean GetSubstituteReportId(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Report.GetSubstituteReportId is declared and has no translated report body yet "
                "(board:0063)");
  }
};

/// \brief AL `REPORT` reached by NUMBER, which is what a body writes.
template <> class Report<void> {
public:
  /// \brief AL `REPORT.Run(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static void Run(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.Run(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.RunModal(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static void RunModal(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.RunModal(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.Execute(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static void Execute(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.Execute(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.RunRequestPage(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static ::agiru::Text<0> RunRequestPage(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.RunRequestPage(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.SaveAs(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static ::agiru::Boolean SaveAs(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAs(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.SaveAsExcel(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static ::agiru::Boolean SaveAsExcel(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAsExcel(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.SaveAsWord(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static ::agiru::Boolean SaveAsWord(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAsWord(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.SaveAsHtml(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static ::agiru::Boolean SaveAsHtml(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAsHtml(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.SaveAsXml(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static ::agiru::Boolean SaveAsXml(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAsXml(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.Print(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static ::agiru::Boolean Print(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.Print(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.ObjectId(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static ::agiru::Text<0> ObjectId(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.ObjectId(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.Language(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static ::agiru::Text<0> Language(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.Language(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.FormatRegion(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static ::agiru::Text<0> FormatRegion(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.FormatRegion(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.GetSubstituteReportId(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static ::agiru::Boolean GetSubstituteReportId(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.GetSubstituteReportId(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }

  /// \brief AL `REPORT.SaveAsPdf(Number, ...)` and its siblings.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The report's number.
  /// \param arguments The rest, read only to be discarded.
  /// \throws Error always -- a report has no translated body yet (board:0034).
  template <typename... Arguments>
  static void SaveAsPdf(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Report.SaveAsPdf(" + std::to_string(Number) +
                ") has no translated report body yet (board:0034)");
  }
};

/// \brief AL's `XMLPORT` object, reached by NUMBER.
///
/// \tparam Derived The xmlport's generated class, or `void` for the platform object AL spells
///         `XMLPORT`.
template <typename Derived = void> class XmlPort {
public:
  /// \brief The xmlport's AL number.
  /// \return The number AL declared.
  [[nodiscard]] static constexpr XmlPortId Id() { return XmlPortTraits<Derived>::kId; }

  /// \brief AL `XmlPort.Break()`. Stops processing the current table element.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void Break(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("XmlPort.Break is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.BreakUnbound()`. Ends an unbound element's repetition.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void BreakUnbound(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.BreakUnbound is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.CurrentPath()`. The node path the transfer stands on.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments>::agiru::Text<0> CurrentPath(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.CurrentPath is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.Export()`. Writes the port's data to its destination.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void Export(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("XmlPort.Export is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.FieldDelimiter([Delimiter])`. The text a field is wrapped in.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments>::agiru::Text<0> FieldDelimiter(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.FieldDelimiter is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.FieldSeparator([Separator])`. The text between two fields.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments>::agiru::Text<0> FieldSeparator(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.FieldSeparator is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.Filename([Name])`. The file the transfer reads or writes.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments>::agiru::Text<0> Filename(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("XmlPort.Filename is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.Import()`. Reads the port's data from its source.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void Import(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("XmlPort.Import is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.ImportFile()`. Reads the port's data from a named file.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void ImportFile(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.ImportFile is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.Quit()`. Ends the transfer without finishing it.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void Quit(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("XmlPort.Quit is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.RecordSeparator([Separator])`. The text between two records.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments>::agiru::Text<0> RecordSeparator(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.RecordSeparator is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.Run()`. Runs the port in its declared direction.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void Run(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("XmlPort.Run is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.SetDestination(var OutStream)`. Names the stream an export writes to.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void SetDestination(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.SetDestination is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.SetSource(var InStream)`. Names the stream an import reads from.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void SetSource(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.SetSource is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.SetTableView(var Record)`. Gives a table element the record's filters and
  /// key.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void SetTableView(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.SetTableView is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.Skip()`. Leaves the current record out of the transfer.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void Skip(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("XmlPort.Skip is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.TableSeparator([Separator])`. The text between two tables.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments>::agiru::Text<0> TableSeparator(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.TableSeparator is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.TextEncoding([Encoding])`. The encoding the transfer reads or writes in.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments>
  ::agiru::TextEncoding TextEncoding(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.TextEncoding is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.FilterGroup([Group])`. The filter group a table element's filters go into.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments>::agiru::Integer FilterGroup(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.FilterGroup is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.FormatRegion([FormatRegion])`. The format region the transfer uses.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments>::agiru::Text<0> FormatRegion(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.FormatRegion is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.GetJsonDocument(var Document)`. Reads what a JSON export produced.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void GetJsonDocument(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.GetJsonDocument is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.GetTableView(var Record)`. The view a table element stands on.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments>::agiru::Text<0> GetTableView(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.GetTableView is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.GetXmlDocument(var Document)`. Reads what an XML export produced.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void GetXmlDocument(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.GetXmlDocument is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.Language([Language])`. The language the transfer runs in.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments>::agiru::Integer Language(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("XmlPort.Language is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.ObjectId([UseNames])`. The object's identifier as text.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments>::agiru::Text<0> ObjectId(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("XmlPort.ObjectId is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.SetJsonDocument(Document)`. Names the JSON an import reads.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void SetJsonDocument(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.SetJsonDocument is declared and has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XmlPort.SetXmlDocument(Document)`. Names the XML an import reads.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- xmlport bodies are not translated yet (board:0065).
  template <typename... Arguments> void SetXmlDocument(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "XmlPort.SetXmlDocument is declared and has no translated xmlport body yet (board:0065)");
  }
};

/// \brief What the runtime knows about a generated query: its number and its name.
/// \tparam T The query's generated class.
template <typename T> struct QueryTraits;

/// \brief AL's `QUERY` object: a generated query's own class derives from this.
///
/// \tparam Derived The query's generated class, or `void` for the platform object AL spells
///         `QUERY`.
template <typename Derived = void> class Query {
public:
  /// \brief The query's AL number.
  /// \return The number AL declared.
  [[nodiscard]] static constexpr QueryId Id() { return QueryTraits<Derived>::kId; }

  /// \brief AL `Query.Close()`. Closes the dataset.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments> void Close(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.Close is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.ColumnCaption(ColumnNo)`. A column's caption.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>::agiru::Text<0> ColumnCaption(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Query.ColumnCaption is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.ColumnName(ColumnNo)`. A column's AL name.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>::agiru::Text<0> ColumnName(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.ColumnName is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.ColumnNo(ColumnName)`. A column's number.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>::agiru::Integer ColumnNo(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.ColumnNo is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.GetFilter(Column)`. The filter standing on a column.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>::agiru::Text<0> GetFilter(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.GetFilter is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.GetFilters()`. Every filter the query carries, as text.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>::agiru::Text<0> GetFilters(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.GetFilters is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.Open()`. Generates the dataset.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>::agiru::Boolean Open(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.Open is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.Read()`. Reads the next row of the dataset.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>::agiru::Boolean Read(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.Read is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.SaveAsCsv(...)`. Writes the dataset as CSV.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>::agiru::Boolean SaveAsCsv(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SaveAsCsv is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.SaveAsJson(...)`. Writes the dataset as JSON.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>::agiru::Boolean SaveAsJson(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SaveAsJson is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.SaveAsXml(...)`. Writes the dataset as XML.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>::agiru::Boolean SaveAsXml(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SaveAsXml is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.SecurityFiltering([Filtering])`. The security filtering mode the query reads
  /// at.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>
  ::agiru::SecurityFilter SecurityFiltering(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Query.SecurityFiltering is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.SetFilter(Column, Filter [, Value, ...])`. Filters a column.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments> void SetFilter(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SetFilter is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.SetRange(Column [, From [, To]])`. Ranges a column.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments> void SetRange(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SetRange is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.TopNumberOfRows([Rows])`. How many rows the dataset holds at most.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>::agiru::Integer TopNumberOfRows(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Query.TopNumberOfRows is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.ColumnFilter(Column)`. The filter standing on one column.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>::agiru::Text<0> ColumnFilter(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.ColumnFilter is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.ObjectId([UseNames])`. The object's identifier as text.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments>::agiru::Text<0> ObjectId(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.ObjectId is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.Run()`. Runs the query.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments> void Run(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.Run is declared and has no translated query body yet (board:0064)");
  }

  /// \brief AL `Query.SetCurrentKey(...)`. Names the columns the dataset is ordered by.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- query bodies are not translated yet (board:0064).
  template <typename... Arguments> void SetCurrentKey(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error(
        "Query.SetCurrentKey is declared and has no translated query body yet (board:0064)");
  }
};

/// \brief AL `QUERY` reached by NUMBER.
template <> class Query<void> {
public:
  /// \brief AL `QUERY.Run(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The query's number.
  /// \param arguments The rest, read only to be discarded.
  /// \throws Error always -- a query has no translated body yet (board:0064).
  template <typename... Arguments>
  static void Run(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Query.Run(" + std::to_string(Number) +
                ") has no translated query body yet (board:0064)");
  }

  /// \brief AL `QUERY.SaveAsCsv(Number, ...)` and its siblings.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The query's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a query has no translated body yet (board:0064).
  template <typename... Arguments>
  static ::agiru::Boolean SaveAsCsv(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SaveAsCsv(" + std::to_string(Number) +
                ") has no translated query body yet (board:0064)");
  }

  /// \brief AL `QUERY.SaveAsJson(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The query's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a query has no translated body yet (board:0064).
  template <typename... Arguments>
  static ::agiru::Boolean SaveAsJson(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SaveAsJson(" + std::to_string(Number) +
                ") has no translated query body yet (board:0064)");
  }

  /// \brief AL `QUERY.SaveAsXml(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The query's number.
  /// \param arguments The rest, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- a query has no translated body yet (board:0064).
  template <typename... Arguments>
  static ::agiru::Boolean SaveAsXml(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SaveAsXml(" + std::to_string(Number) +
                ") has no translated query body yet (board:0064)");
  }
};

/// \brief AL `XMLPORT` reached by NUMBER.
template <> class XmlPort<void> {
public:
  /// \brief AL `XMLPORT.Export(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The xmlport's number.
  /// \param arguments The rest, read only to be discarded.
  /// \throws Error always -- an xmlport has no translated body yet (board:0065).
  template <typename... Arguments>
  static void Export(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("XmlPort.Export(" + std::to_string(Number) +
                ") has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XMLPORT.Import(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The xmlport's number.
  /// \param arguments The rest, read only to be discarded.
  /// \throws Error always -- an xmlport has no translated body yet (board:0065).
  template <typename... Arguments>
  static void Import(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("XmlPort.Import(" + std::to_string(Number) +
                ") has no translated xmlport body yet (board:0065)");
  }

  /// \brief AL `XMLPORT.Run(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The xmlport's number.
  /// \param arguments The rest, read only to be discarded.
  /// \throws Error always -- an xmlport has no translated body yet (board:0065).
  template <typename... Arguments>
  static void Run(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("XmlPort.Run(" + std::to_string(Number) +
                ") has no translated xmlport body yet (board:0065)");
  }
};

}

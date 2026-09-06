#pragma once

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"

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

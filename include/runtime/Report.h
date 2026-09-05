#pragma once

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "type/Integer.h"

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

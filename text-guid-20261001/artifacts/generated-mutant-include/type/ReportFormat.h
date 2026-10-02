#pragma once

#include <cstdint>

/// \file
/// \brief AL `ReportFormat` -- Specifies the format of the report.
///
/// The members and their order come from `methods-auto/reportformat/reportformat-option.md`, which
/// is the specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `ReportFormat`. Specifies the format of the report.
enum class ReportFormat : std::int32_t {
  Excel, ///< Saves the report as an Excel file.
  Html,  ///< Saves the report in HTML format.
  Pdf,   ///< Saves the report in PDF format.
  Word,  ///< Saves the report in Word format.
  Xml,   ///< Saves the report in XML format.
};

}

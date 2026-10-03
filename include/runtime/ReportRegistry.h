#pragma once

#include "meta/Ids.h"

#include <string_view>

/// \file
/// \brief Generated report registration and lookup without dataset or page dependencies.

namespace agiru {

/// \brief Report execution request, defined by runtime/Report.h.
struct ReportRequest;

/// \brief What the runtime knows about a generated report: its number, its name and how to run it.
struct ReportEntry {
  ReportId id;           ///< The AL report number.
  std::string_view name; ///< The AL name.
  /// \brief Constructs the report and runs the request. \param request The request.
  void (*run)(const ReportRequest &request);
};

/// \brief Puts a report in the catalogue, once per generated report, at load time.
/// \param entry The entry, which lives for the program.
void RegisterReportEntry(const ReportEntry *entry);

/// \brief Finds a report by its number.
/// \param id The number.
/// \return The entry, or nullptr when this build carries no such report.
[[nodiscard]] const ReportEntry *FindReport(ReportId id);

}

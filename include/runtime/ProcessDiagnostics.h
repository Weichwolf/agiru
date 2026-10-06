#pragma once

/// \file
/// \brief Immutable startup diagnostics shared by database and runtime workers.

namespace agiru::detail {

/// \brief Whether AGIRU_TRACE_SQL was present at process startup, including an empty value.
/// \return Whether SQL statements and their binds are traced.
[[nodiscard]] bool TraceSql() noexcept;

/// \brief Whether AGIRU_TRACE_SQL was exactly 2 at process startup.
/// \return Whether SQL result summaries are traced.
[[nodiscard]] bool TraceSqlRows() noexcept;

/// \brief Whether AGIRU_TRACE_ERRORS was present at process startup.
/// \return Whether error and allocation backtraces are enabled.
[[nodiscard]] bool TraceErrors() noexcept;

/// \brief Whether AGIRU_TRACE_UI was present at process startup.
/// \return Whether test UI-handler selection is traced.
[[nodiscard]] bool TraceUi() noexcept;

/// \brief The owned, immutable AGIRU_TEST_PROCEDURE startup selection.
/// \return A process-lifetime CSV pointer; null means unset, empty means select no procedures.
[[nodiscard]] const char *SelectedTestProcedures() noexcept;

}

#pragma once

#include "type/Integer.h"

/// \file
/// \brief Session-local AL read-cache refresh, independent of transaction boundaries.

namespace agiru {

/// \brief AL Database.SelectLatestVersion(): invalidates all non-locked record read buffers.
/// Preserves loaded record values, filters, keys, locked buffers and pending writes.
/// No caption cache exists while CaptionClassTranslate remains explicitly unsupported.
/// \throws Error if the physical SQL snapshot cannot supply fresh committed reads.
/// \note Does not commit, roll back or grant permission; subsequent reads reauthorize.
void SelectLatestVersion();

/// \brief AL Database.SelectLatestVersion(Integer): refreshes only the named table's buffers.
/// \param table Exact AL table number, not an alias for an all-table refresh.
/// \throws Error if the physical SQL snapshot cannot supply fresh committed reads.
/// \note Uses the same non-locked, session-local policy as the parameterless overload.
void SelectLatestVersion(Integer table);

}

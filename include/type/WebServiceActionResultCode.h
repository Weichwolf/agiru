#pragma once

#include <cstdint>

/// \file
/// \brief AL `WebServiceActionResultCode` -- Represents a web service action status code.
///
/// The members and their order come from
/// `methods-auto/webserviceactionresultcode/webserviceactionresultcode-option.md`, which is the
/// specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `WebServiceActionResultCode`. Represents a web service action status code.
enum class WebServiceActionResultCode : std::int32_t {
  None,    ///< No status code.
  Get,     ///< Item read.
  Created, ///< Item created.
  Updated, ///< Item updated.
  Deleted, ///< Item deleted.
};

} // namespace agiru

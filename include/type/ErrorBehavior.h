#pragma once

#include <cstdint>

/// \file
/// \brief AL `ErrorBehavior` -- Specifies whether errors will be collected within the scope of the
/// method.
///
/// The members and their order come from `methods-auto/errorbehavior/errorbehavior-option.md`,
/// which is the specification: an AL option is zero-based and sequential in the order the page
/// lists.

namespace agiru {

/// \brief AL `ErrorBehavior`. Specifies whether errors will be collected within the scope of the
/// method.
enum class ErrorBehavior : std::int32_t {
  Collect, ///< Collectable errors will be gathered and code execution will be continued until the
           ///< end of the ErrorBehavior scope. If errors are left unhandled at the end of the
           ///< ErrorBehavior scope, execution will stop with an aggregated error.
};

}

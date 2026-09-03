#pragma once

#include <cstdint>

/// \file
/// \brief AL `ExecutionContext` -- Represents the context in which a session is running.
///
/// The members and their order come from
/// `methods-auto/executioncontext/executioncontext-option.md`, which is the specification: an AL
/// option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `ExecutionContext`. Represents the context in which a session is running.
enum class ExecutionContext : std::int32_t {
  Normal,    ///< The normal execution context.
  Install,   ///< An application is being installed.
  Uninstall, ///< An application is being uninstalled.
  Upgrade,   ///< An application is being upgraded.
};

}

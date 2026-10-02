#pragma once

#include <cstdint>

/// \file
/// \brief AL `ExecutionMode` -- The execution mode of the current session.
///
/// The members and their order come from `methods-auto/executionmode/executionmode-option.md`,
/// which is the specification: an AL option is zero-based and sequential in the order the page
/// lists.

namespace agiru {

/// \brief AL `ExecutionMode`. The execution mode of the current session.
enum class ExecutionMode : std::int32_t {
  Standard, ///< The session is executing in standard mode.
  Debug,    ///< The session is executing in debug mode.
};

}

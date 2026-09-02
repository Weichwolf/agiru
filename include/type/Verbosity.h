#pragma once

#include <cstdint>

/// \file
/// \brief AL `Verbosity` -- Represents the security level of events.
///
/// The members and their order come from `methods-auto/verbosity/verbosity-option.md`, which is the
/// specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `Verbosity`. Represents the security level of events.
enum class Verbosity : std::int32_t {
  Critical, ///< Identifies an abnormal exit or termination event.
  Error,    ///< Identifies a severe error event.
  Warning,  ///< Identifies a warning event such as an allocation failure.
  Normal,   ///< Identifies a non-error event such as an entry or exit event.
  Verbose,  ///< Identifies a detailed trace event.
};

} // namespace agiru

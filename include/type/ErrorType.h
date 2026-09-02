#pragma once

#include <cstdint>

/// \file
/// \brief AL `ErrorType` -- Represents the type of error.
///
/// The members and their order come from `methods-auto/errortype/errortype-option.md`, which is the
/// specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `ErrorType`. Represents the type of error.
enum class ErrorType : std::int32_t {
  Client,   ///< Identifies a client error. The specified message will be shown in the client to the
            ///< user and sent to telemetry.
  Internal, ///< Identifies an internal, the message specified will be sent to telemetry and a
            ///< generic error will be displayed to the user.
};

} // namespace agiru

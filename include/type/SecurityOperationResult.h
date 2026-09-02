#pragma once

#include <cstdint>

/// \file
/// \brief AL `SecurityOperationResult` -- Represents security audit operation result.
///
/// The members and their order come from
/// `methods-auto/securityoperationresult/securityoperationresult-option.md`, which is the
/// specification: an AL option is zero-based and sequential in the order the page lists.

namespace agiru {

/// \brief AL `SecurityOperationResult`. Represents security audit operation result.
enum class SecurityOperationResult : std::int32_t {
  Success,     ///< Identifies operation success.
  ClientError, ///< Identifies client error in the operation.
  Failure,     ///< Identifies operation failure.
  Timeout,     ///< Identifies operation timeout.
};

} // namespace agiru

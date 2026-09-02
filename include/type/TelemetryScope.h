#pragma once

#include <cstdint>

/// \file
/// \brief AL `TelemetryScope` -- Represents the emission scope of the telemetry signal.
///
/// The members and their order come from `methods-auto/telemetryscope/telemetryscope-option.md`,
/// which is the specification: an AL option is zero-based and sequential in the order the page
/// lists.

namespace agiru {

/// \brief AL `TelemetryScope`. Represents the emission scope of the telemetry signal.
enum class TelemetryScope : std::int32_t {
  ExtensionPublisher, ///< Emit telemetry to extensions publisher's account.
  All,                ///< Emit telemetry to extension publisher's and partner's telemetry account .
};

} // namespace agiru

#pragma once

#include <cstddef>
#include <cstdint>

/// \file
/// \brief Native transport configuration without request, handler or executor state.
namespace agiru {

/// \brief Default native executor size from the Linux process affinity mask.
/// \return One worker per available CPU, at least one and at most the declared worker ceiling.
/// Workers are not pinned: the OS distributes ready jobs across available CPUs.
[[nodiscard]] std::size_t DefaultHttpWorkers();

/// \brief Explicit resource ceilings, not measured production scale guarantees.
struct HttpServerOptions {
  static constexpr std::size_t kMaxWorkers = 256; ///< Explicit native executor resource ceiling.
  static constexpr std::uint16_t kBackendPort = 18080;      ///< Development Caddy upstream port.
  static constexpr std::size_t kDefaultQueue = 32;          ///< Initial bounded admission profile.
  static constexpr unsigned kDefaultConnections = 256;      ///< Initial native connection ceiling.
  static constexpr unsigned kDefaultTimeoutSeconds = 15;    ///< Initial network inactivity timeout.
  static constexpr std::size_t kDefaultBodyBytes = 1048576; ///< One-MiB request safety default.
  static constexpr std::size_t kDefaultTotalBodyBytes = 33554432; ///< Thirty-two-MiB body quota.
  std::uint16_t port = kBackendPort; ///< Private native port; zero requests an ephemeral port.
  bool loopback = true;              ///< Bind loopback; false explicitly binds all IPv4 interfaces.
  std::size_t workers = DefaultHttpWorkers(); ///< Bounded executor, not per-user threads.
  std::size_t queue = kDefaultQueue; ///< Maximum pending executor jobs; overflow returns 503.
  unsigned connections = kDefaultConnections;          ///< Maximum simultaneous native connections.
  unsigned timeoutSeconds = kDefaultTimeoutSeconds;    ///< Network inactivity, not AL cancellation.
  std::size_t bodyBytes = kDefaultBodyBytes;           ///< Per-request body ceiling.
  std::size_t totalBodyBytes = kDefaultTotalBodyBytes; ///< Aggregate owned request-body ceiling.
  std::size_t responseBytes = kDefaultBodyBytes;       ///< Per-response byte ceiling.
};

/// \brief Validates resource bounds without creating workers or opening a listener.
/// \param options Native transport limits; port zero remains valid for standalone fixtures.
/// \throws Error with HttpServerOptions when a resource bound is invalid.
void ValidateHttpServerOptions(const HttpServerOptions &options);

}

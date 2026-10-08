#pragma once

#include <chrono>
#include <cstddef>

/// \file
/// \brief Trusted browser-session limits, independent of HTTP and credential storage.
namespace agiru {

/// \brief Trusted deployment limits, never values accepted from an HTTP request.
struct BrowserSessionOptions {
  /// \brief BC's documented initial web idle limit.
  static constexpr auto kDefaultIdle = std::chrono::minutes(20);
  /// \brief Initial deployment absolute ceiling, not a BC platform guarantee.
  static constexpr auto kDefaultLifetime = std::chrono::hours(8);
  /// \brief Initial concurrent browser identity ceiling, not scale proof.
  static constexpr std::size_t kDefaultSessionsPerUser = 8;
  std::chrono::seconds idle = kDefaultIdle;              ///< Trusted idle deadline policy.
  std::chrono::seconds lifetime = kDefaultLifetime;      ///< Trusted absolute deadline policy.
  std::size_t sessionsPerUser = kDefaultSessionsPerUser; ///< Trusted live identity limit.
};

}

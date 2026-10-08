#pragma once

#include "runtime/ClientCredentials.h"

#include <chrono>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

/// \file
/// \brief PostgreSQL browser-session authority, separate from agent bearer credentials.
namespace agiru {

class Connection;

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

/// \brief Newly issued secrets, returned only to the trusted HTTPS transport adapter.
struct BrowserSessionGrant {
  std::string secret; ///< Cookie value only; never URLs, HTML, logs or browser storage.
  std::string csrf;   ///< Session-bound synchronizer token; not an authentication credential.
};

/// \brief Authenticated browser identity and its session-bound synchronizer token.
struct BrowserSessionIdentity {
  ClientCredentialIdentity client; ///< Original User GUID plus non-secret session verifier.
  std::string csrf; ///< For same-origin bootstrap/header validation, never URLs or logs.
};

/// \brief Validates trusted positive limits: idle <= lifetime <= one day, sessions <= 1024.
/// \param options Immutable trusted deployment policy.
/// \throws Error with BrowserSessionConfiguration for invalid policy.
void ValidateBrowserSessionOptions(const BrowserSessionOptions &options);

/// \brief Installs browser-session storage using trusted migration authority.
/// \param connection Database already containing User and agiru_client.credentials.
/// \note Creates no identities or grants. Caller owns commit/rollback; deleting a User or
/// source credential cascades its browser sessions. Bearer and browser secrets are distinct.
void InstallBrowserSessions(const Connection &connection);

/// \brief Issues a new browser identity from an already verified development credential.
/// \param connection Explicit READ COMMITTED transaction; caller owns commit/rollback.
/// \param source Original User and credential verifier, rechecked against SQL before issuance.
/// \param options Trusted server policy; never HTTP/CLI request parameters.
/// \return CSPRNG cookie and domain-separated HMAC CSRF token; only their SHA-256 reaches SQL.
/// \note A User row lock serializes admission across hosts. Both deadlines are capped by
/// source expiry; revoking the source invalidates all its derived browser sessions.
/// \warning This is not password login or permission. The transport must check account state,
/// enforce HTTPS/Origin/CSRF and bind handles to the returned browser identity, not source.
/// \throws Error for invalid identity/policy/transaction, expired source or capacity refusal.
[[nodiscard]] BrowserSessionGrant IssueBrowserSession(const Connection &connection,
                                                      const ClientCredentialIdentity &source,
                                                      const BrowserSessionOptions &options = {});

/// \brief Resolves an exact browser cookie value using SQL expiry/revocation authority.
/// \param connection Database authority; no retained lease or process-local session cache.
/// \param secret Raw cookie value from the permitted transport, never a URL or Authorization.
/// \return Identity and CSRF, or nothing for malformed/unknown/expired/revoked sessions.
/// \note Passive lookup never renews idle expiry. Account/ERP permissions need independent
/// SessionCommand checks. Cookie syntax and origin validation belong to the transport adapter.
/// \throws Error for storage/crypto failures; never authenticates an anonymous fallback.
[[nodiscard]] std::optional<BrowserSessionIdentity>
LookupBrowserSession(const Connection &connection, std::string_view secret);

/// \brief Renews idle expiry only after an authorized, fresh client operation.
/// \param connection SQL authority; caller owns commit/rollback.
/// \param identity Previously resolved exact user/verifier/CSRF identity, rechecked in SQL.
/// \return True when still authorized, false for stale/forged/revoked/expired identity.
/// \note Never extends absolute/source expiry. Polling, passive reads and replay must not call it.
[[nodiscard]] bool RenewBrowserSession(const Connection &connection,
                                       const BrowserSessionIdentity &identity);

/// \brief Atomically replaces a browser identity and invalidates its old secrets.
/// \param connection Explicit READ COMMITTED transaction; caller owns commit/rollback.
/// \param identity Exact live browser identity; stale rotations refuse without creating rows.
/// \return Fresh cookie and CSRF tokens, preserving the original absolute/source deadline.
/// \note No grace reuse or adoption of old handles. A rollback restores the old identity;
/// a failed insertion cannot leave the old identity revoked without a replacement.
/// \throws Error for invalid transaction/identity, expiry, revocation or provider/storage failure.
[[nodiscard]] BrowserSessionGrant RotateBrowserSession(const Connection &connection,
                                                       const BrowserSessionIdentity &identity);

/// \brief Revokes a browser identity under trusted operator/logout authority.
/// \param connection SQL authority; caller owns commit/rollback.
/// \param verifier Canonical non-secret SHA-256 identity, never the cookie secret.
/// \return True once, false for unknown/already revoked identity; retains its audit row.
/// \warning Does not cancel an already executing AL stack; transport cancellation remains separate.
[[nodiscard]] bool RevokeBrowserSession(const Connection &connection, std::string_view verifier);

}

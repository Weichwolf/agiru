#pragma once

#include "type/Guid.h"

#include <chrono>
#include <optional>
#include <string>
#include <string_view>

/// \file
/// \brief PostgreSQL authority for native opaque agent credentials, not ERP permissions.

namespace agiru {

class Connection;

/// \brief Authenticated user and non-secret credential identity for client-owned state.
/// \note Separate credentials for one user are separate clients, not interchangeable sessions.
struct ClientCredentialIdentity {
  Guid user;            ///< Original system User security GUID.
  std::string verifier; ///< Canonical SHA-256 verifier; never the bearer secret.
};

/// \brief Installs the agiru-owned credential store; requires trusted migration authority.
/// \param connection Database containing the original system User table.
/// \note No users, credentials or permissions are granted. Caller owns commit/rollback.
/// Deleting a system User removes its credentials; this store must not block account deletion.
/// \throws DatabaseError for failed schema creation or missing User storage.
void InstallClientCredentials(const Connection &connection);

/// \brief Issues a random bearer credential for an existing system User GUID.
/// \param connection Trusted operator connection; never expose issuance as an anonymous endpoint.
/// \param user Original User security GUID, not a username or caller-supplied identity header.
/// \param lifetime Positive expiry duration, at most the initial one-day issuance ceiling.
/// \return Plaintext once, prefixed ag1_; store it privately and never log it or put it in a URL.
/// \note Only a SHA-256 verifier reaches SQL, including SQL tracing. Caller owns commit/rollback.
/// Account state is rechecked by SessionCommand before execution; issuance is not permission.
/// \throws Error for blank identity, invalid duration, unavailable crypto or failed SQL.
[[nodiscard]] std::string IssueClientCredential(const Connection &connection,
                                                const Guid &user,
                                                std::chrono::seconds lifetime);

/// \brief Resolves a canonical opaque bearer credential from an HTTP Authorization value.
/// \param connection Database authority; PostgreSQL time owns expiry and revocation.
/// \param authorization Case-insensitive Bearer scheme followed by the exact issued secret.
/// \return The credential's system User GUID, or nothing for invalid/expired/revoked credentials.
/// \throws Error for SQL/crypto/provider failures, never converted to an anonymous success.
/// \warning This authenticates a credential only. SessionCommand must recheck User state/name/
/// expiry; the host must independently authorize every ERP operation in company context.
/// Cookies, identity headers, URL credentials and Basic/password sign-in are not accepted here.
[[nodiscard]] std::optional<Guid> LookupClientCredential(const Connection &connection,
                                                         std::string_view authorization);

/// \brief Resolves the same bearer policy while retaining its exact client identity.
/// \param connection PostgreSQL authority for expiry, revocation and user ownership.
/// \param authorization Case-insensitive Bearer scheme followed by the exact issued secret.
/// \return User and verifier, or nothing for invalid/expired/revoked credentials.
/// \note Bind retained handles to both fields; a user GUID alone cannot isolate clients.
/// \throws Error for SQL/crypto/provider failures; never authenticates an anonymous fallback.
[[nodiscard]] std::optional<ClientCredentialIdentity>
LookupClientCredentialIdentity(const Connection &connection, std::string_view authorization);

/// \brief Revokes a credential by its public verifier identity; requires trusted operator
/// authority.
/// \param connection Database authority; caller owns commit/rollback.
/// \param digest Canonical lowercase SHA-256 verifier, never the plaintext secret.
/// \return True for newly revoked identity, false for unknown/already revoked identities.
/// \throws Error for a malformed verifier or failed SQL; retains the revocation audit row.
[[nodiscard]] bool RevokeClientCredential(const Connection &connection, std::string_view digest);

}

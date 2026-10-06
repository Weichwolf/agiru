#include "runtime/ClientCredentials.h"

#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/SecureToken.h"
#include "type/Guid.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace agiru {

namespace {

constexpr auto kMaximumLifetime = std::chrono::hours(24);
constexpr std::string_view kPrefix = "ag1_";
constexpr std::size_t kDigestCharacters = 64;

bool CanonicalDigest(std::string_view text) {
  return text.size() == kDigestCharacters && std::ranges::all_of(text, [](char unit) {
           return (unit >= '0' && unit <= '9') || (unit >= 'a' && unit <= 'f');
         });
}

bool BearerScheme(std::string_view text) {
  constexpr std::string_view expected = "bearer";
  if (text.size() != expected.size()) { return false; }
  for (std::size_t at = 0; at < text.size(); ++at) {
    const char unit = text[at];
    const char lower = unit >= 'A' && unit <= 'Z' ? static_cast<char>(unit + ('a' - 'A')) : unit;
    if (lower != expected[at]) { return false; }
  }
  return true;
}

}

void InstallClientCredentials(const Connection &connection) {
  connection.Run("CREATE SCHEMA IF NOT EXISTS agiru_client");
  connection.Run(R"(CREATE TABLE IF NOT EXISTS agiru_client.credentials (
    digest text PRIMARY KEY CHECK (digest ~ '^[0-9a-f]{64}$'),
    user_security_id uuid NOT NULL REFERENCES "User"("User Security ID") ON DELETE CASCADE,
    issued_at timestamptz NOT NULL DEFAULT clock_timestamp(),
    expires_at timestamptz NOT NULL,
    revoked_at timestamptz,
    CHECK (expires_at > issued_at)
  ))");
}

std::string IssueClientCredential(const Connection &connection,
                                  const Guid &user,
                                  std::chrono::seconds lifetime) {
  if (user.IsNull() || lifetime.count() <= 0 || lifetime > kMaximumLifetime) {
    throw Error("invalid client credential identity or duration", "ClientCredentialInput");
  }
  const std::string secret = std::string(kPrefix) + GenerateSecureToken();
  const std::array<std::optional<std::string>, 3> binds{
      SecureTokenDigest(secret), user.ToStorageText(), std::to_string(lifetime.count())};
  connection.Run("INSERT INTO agiru_client.credentials(digest, user_security_id, expires_at) "
                 "VALUES ($1, $2::uuid, clock_timestamp() + $3::integer * interval '1 second')",
                 binds);
  return secret;
}

std::optional<Guid> LookupClientCredential(const Connection &connection,
                                           std::string_view authorization) {
  constexpr std::size_t kMaximumAuthorization = 128;
  const auto separator = authorization.find(' ');
  if (authorization.size() > kMaximumAuthorization || separator == std::string_view::npos ||
      !BearerScheme(authorization.substr(0, separator))) {
    return std::nullopt;
  }
  auto secret = authorization.substr(separator);
  secret.remove_prefix(secret.find_first_not_of(' ') == std::string_view::npos
                           ? secret.size()
                           : secret.find_first_not_of(' '));
  if (!secret.starts_with(kPrefix) || !CanonicalDigest(secret.substr(kPrefix.size()))) {
    return std::nullopt;
  }
  const std::array<std::optional<std::string>, 1> binds{SecureTokenDigest(secret)};
  const auto rows = connection.Execute(
      "SELECT user_security_id::text FROM agiru_client.credentials "
      "WHERE digest = $1 AND revoked_at IS NULL AND expires_at > clock_timestamp()",
      binds);
  if (rows.Rows() == 0) { return std::nullopt; }
  if (rows.Rows() != 1) {
    throw Error("invalid client credential authority", "ClientCredentialStorage");
  }
  const auto identity = rows.Value(0, 0);
  if (!identity) { throw Error("invalid client credential authority", "ClientCredentialStorage"); }
  const Guid user(*identity);
  if (user.IsNull()) {
    throw Error("invalid client credential authority", "ClientCredentialStorage");
  }
  return user;
}

bool RevokeClientCredential(const Connection &connection, std::string_view digest) {
  if (!CanonicalDigest(digest)) {
    throw Error("invalid client credential verifier", "ClientCredentialInput");
  }
  const std::array<std::optional<std::string>, 1> binds{std::string(digest)};
  return connection
             .Execute("UPDATE agiru_client.credentials SET revoked_at = clock_timestamp() "
                      "WHERE digest = $1 AND revoked_at IS NULL",
                      binds)
             .Affected() == 1;
}

}

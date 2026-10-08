#include "runtime/BrowserSession.h"

#include "runtime/BrowserSessionOptions.h"
#include "runtime/ClientCredentials.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/SecureToken.h"
#include "type/Guid.h"

#include "CredentialFormat.h"

#include <array>
#include <chrono>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace agiru {
namespace {

constexpr std::string_view kPrefix = "agb1_";
constexpr std::string_view kCsrfDomain = "agiru.browser.csrf.v1";
constexpr auto kMaximumLifetime = std::chrono::hours(24);
constexpr std::size_t kMaximumSessions = 1024;
constexpr std::string_view kLive =
    "b.revoked_at IS NULL AND b.expires_at>clock_timestamp() "
    "AND b.idle_expires_at>clock_timestamp() AND c.revoked_at IS NULL "
    "AND c.expires_at>clock_timestamp() AND c.user_security_id=b.user_security_id";

[[noreturn]] void Invalid(std::string_view code) {
  throw Error("browser session refused", std::string(code));
}

void Source(const ClientCredentialIdentity &identity) {
  if (identity.user.IsNull() || !detail::CanonicalCredentialDigest(identity.verifier)) {
    Invalid("BrowserSessionInput");
  }
}

void Identity(const BrowserSessionIdentity &identity) {
  Source(identity.client);
  if (!detail::CanonicalCredentialDigest(identity.csrf)) { Invalid("BrowserSessionInput"); }
}

void Admission(const Connection &connection, const Guid &user) {
  if (!connection.InTransaction()) { Invalid("BrowserSessionTransaction"); }
  const auto isolation = connection.Execute("SHOW transaction_isolation");
  if (isolation.Rows() != 1 || isolation.Value(0, 0) != "read committed") {
    Invalid("BrowserSessionTransaction");
  }
  const std::array<std::optional<std::string>, 1> binds{user.ToStorageText()};
  if (connection
          .Execute(R"(SELECT 1 FROM "User" WHERE "User Security ID"=$1::uuid FOR UPDATE)", binds)
          .Rows() != 1) {
    Invalid("BrowserSessionAuthentication");
  }
}

BrowserSessionGrant Generate() {
  auto secret = std::string(kPrefix) + GenerateSecureToken();
  auto csrf = SecureTokenMac(secret, kCsrfDomain);
  return {.secret = std::move(secret), .csrf = std::move(csrf)};
}

std::array<std::optional<std::string>, 3> Binds(const BrowserSessionIdentity &identity) {
  Identity(identity);
  return {identity.client.verifier,
          identity.client.user.ToStorageText(),
          SecureTokenDigest(identity.csrf)};
}

}

void ValidateBrowserSessionOptions(const BrowserSessionOptions &options) {
  if (options.idle.count() <= 0 || options.lifetime.count() <= 0 ||
      options.idle > options.lifetime || options.lifetime > kMaximumLifetime ||
      options.sessionsPerUser == 0 || options.sessionsPerUser > kMaximumSessions) {
    Invalid("BrowserSessionConfiguration");
  }
}

void InstallBrowserSessions(const Connection &connection) {
  connection.Run(R"(CREATE TABLE IF NOT EXISTS agiru_client.browser_sessions (
    digest text PRIMARY KEY CHECK (digest ~ '^[0-9a-f]{64}$'),
    source_digest text NOT NULL REFERENCES agiru_client.credentials(digest) ON DELETE CASCADE,
    user_security_id uuid NOT NULL REFERENCES "User"("User Security ID") ON DELETE CASCADE,
    csrf_digest text NOT NULL CHECK (csrf_digest ~ '^[0-9a-f]{64}$'),
    idle_seconds integer NOT NULL CHECK (idle_seconds > 0 AND idle_seconds <= 86400),
    issued_at timestamptz NOT NULL,
    last_activity_at timestamptz NOT NULL,
    idle_expires_at timestamptz NOT NULL,
    expires_at timestamptz NOT NULL,
    revoked_at timestamptz,
    CHECK (expires_at > issued_at),
    CHECK (idle_expires_at <= expires_at)
  ))");
  connection.Run("CREATE INDEX IF NOT EXISTS browser_sessions_live_user "
                 "ON agiru_client.browser_sessions(user_security_id,expires_at,idle_expires_at) "
                 "WHERE revoked_at IS NULL");
}

BrowserSessionGrant IssueBrowserSession(const Connection &connection,
                                        const ClientCredentialIdentity &source,
                                        const BrowserSessionOptions &options) {
  ValidateBrowserSessionOptions(options);
  Source(source);
  Admission(connection, source.user);
  const std::array<std::optional<std::string>, 2> user{source.user.ToStorageText(),
                                                       std::to_string(options.sessionsPerUser)};
  const auto population =
      connection.Execute("SELECT count(*)>=$2::integer FROM agiru_client.browser_sessions b JOIN "
                         "agiru_client.credentials c "
                         "ON c.digest=b.source_digest WHERE b.user_security_id=$1::uuid AND " +
                             std::string(kLive),
                         user);
  if (population.Rows() != 1 || population.Value(0, 0) != "f") {
    Invalid("BrowserSessionCapacity");
  }
  auto grant = Generate();
  const std::array<std::optional<std::string>, 6> binds{SecureTokenDigest(grant.secret),
                                                        source.verifier,
                                                        source.user.ToStorageText(),
                                                        SecureTokenDigest(grant.csrf),
                                                        std::to_string(options.idle.count()),
                                                        std::to_string(options.lifetime.count())};
  const auto inserted = connection.Execute(
      "WITH stamp AS MATERIALIZED (SELECT clock_timestamp() AS at) "
      "INSERT INTO agiru_client.browser_sessions "
      "(digest,source_digest,user_security_id,csrf_digest,idle_seconds,issued_at,last_activity_at,"
      "idle_expires_at,expires_at) SELECT $1,c.digest,c.user_security_id,$4,$5::integer,"
      "stamp.at,stamp.at,LEAST(c.expires_at,stamp.at+$5::integer*interval '1 second'),"
      "LEAST(c.expires_at,stamp.at+$6::integer*interval '1 second') "
      "FROM agiru_client.credentials c CROSS JOIN stamp "
      "WHERE c.digest=$2 AND c.user_security_id=$3::uuid AND c.revoked_at IS NULL "
      "AND c.expires_at>stamp.at RETURNING digest",
      binds);
  if (inserted.Rows() != 1) { Invalid("BrowserSessionAuthentication"); }
  return grant;
}

std::optional<BrowserSessionIdentity> LookupBrowserSession(const Connection &connection,
                                                           std::string_view secret) {
  if (!secret.starts_with(kPrefix) ||
      !detail::CanonicalCredentialDigest(secret.substr(kPrefix.size()))) {
    return std::nullopt;
  }
  const auto verifier = SecureTokenDigest(secret);
  const auto csrf = SecureTokenMac(secret, kCsrfDomain);
  const std::array<std::optional<std::string>, 1> binds{verifier};
  const auto rows = connection.Execute(
      "SELECT b.user_security_id::text,b.csrf_digest FROM agiru_client.browser_sessions b "
      "JOIN agiru_client.credentials c ON c.digest=b.source_digest WHERE b.digest=$1 AND " +
          std::string(kLive),
      binds);
  if (rows.Rows() == 0) { return std::nullopt; }
  const auto user = rows.Value(0, 0);
  const auto storedCsrf = rows.Value(0, 1);
  if (rows.Rows() != 1 || !user || !storedCsrf || *storedCsrf != SecureTokenDigest(csrf)) {
    Invalid("BrowserSessionStorage");
  }
  const Guid identity(*user);
  if (identity.IsNull()) { Invalid("BrowserSessionStorage"); }
  return BrowserSessionIdentity{.client = {.user = identity, .verifier = verifier}, .csrf = csrf};
}

bool RenewBrowserSession(const Connection &connection, const BrowserSessionIdentity &identity) {
  const auto binds = Binds(identity);
  return connection
             .Execute(
                 "UPDATE agiru_client.browser_sessions b SET last_activity_at=clock_timestamp(),"
                 "idle_expires_at=LEAST(b.expires_at,c.expires_at,clock_timestamp()+b.idle_seconds*"
                 "interval '1 second') "
                 "FROM agiru_client.credentials c WHERE b.source_digest=c.digest AND b.digest=$1 "
                 "AND b.user_security_id=$2::uuid AND b.csrf_digest=$3 AND " +
                     std::string(kLive) + " RETURNING b.digest",
                 binds)
             .Rows() == 1;
}

BrowserSessionGrant RotateBrowserSession(const Connection &connection,
                                         const BrowserSessionIdentity &identity) {
  Identity(identity);
  Admission(connection, identity.client.user);
  auto grant = Generate();
  const std::array<std::optional<std::string>, 5> binds{identity.client.verifier,
                                                        identity.client.user.ToStorageText(),
                                                        SecureTokenDigest(identity.csrf),
                                                        SecureTokenDigest(grant.secret),
                                                        SecureTokenDigest(grant.csrf)};
  const auto rotated = connection.Execute(
      "WITH old AS (UPDATE agiru_client.browser_sessions b SET revoked_at=clock_timestamp() "
      "FROM agiru_client.credentials c WHERE b.source_digest=c.digest AND b.digest=$1 "
      "AND b.user_security_id=$2::uuid AND b.csrf_digest=$3 AND " +
          std::string(kLive) +
          " RETURNING b.*,c.expires_at AS source_expires_at), stamp AS MATERIALIZED (SELECT "
          "clock_timestamp() AS at) "
          "INSERT INTO agiru_client.browser_sessions "
          "(digest,source_digest,user_security_id,csrf_digest,idle_seconds,issued_at,last_activity_"
          "at,"
          "idle_expires_at,expires_at) SELECT $4,old.source_digest,old.user_security_id,$5,"
          "old.idle_seconds,stamp.at,stamp.at,LEAST(old.expires_at,old.source_expires_at,stamp.at+"
          "old.idle_seconds*"
          "interval '1 second'),LEAST(old.expires_at,old.source_expires_at) "
          "FROM old CROSS JOIN stamp RETURNING digest",
      binds);
  if (rotated.Rows() != 1) { Invalid("BrowserSessionAuthentication"); }
  return grant;
}

bool RevokeBrowserSession(const Connection &connection, std::string_view verifier) {
  if (!detail::CanonicalCredentialDigest(verifier)) { Invalid("BrowserSessionInput"); }
  const std::array<std::optional<std::string>, 1> binds{std::string(verifier)};
  return connection
             .Execute("UPDATE agiru_client.browser_sessions SET revoked_at=clock_timestamp() "
                      "WHERE digest=$1 AND revoked_at IS NULL RETURNING digest",
                      binds)
             .Rows() == 1;
}

}

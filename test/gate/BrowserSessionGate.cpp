#include "platform/User.h"
#include "runtime/BrowserSession.h"
#include "runtime/ClientCredentials.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/SecureToken.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "type/Guid.h"

#include "Check.h"
#include "OwnedDatabase.h"
#include "PrivateAuthFile.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <exception>
#include <future>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

namespace {

constexpr std::string_view kUser = "00000000-0000-0000-0000-000000000001";
constexpr std::string_view kOtherUser = "00000000-0000-0000-0000-000000000002";

std::string Value(const agiru::Connection &connection,
                  std::string_view sql,
                  std::span<const std::optional<std::string>> binds = {}) {
  const auto rows = connection.Execute(sql, binds);
  if (rows.Rows() != 1) { throw std::runtime_error("missing gate SQL row"); }
  const auto value = rows.Value(0, 0);
  if (!value) { throw std::runtime_error("missing gate SQL value"); }
  return std::string(*value);
}

template <class Body> void Refused(std::string_view claim, std::string_view code, Body body) {
  bool refused = false;
  try {
    body();
  } catch (const agiru::Error &error) { refused = error.Code() == code; }
  CHECK_TRUE(claim, refused);
}

class Fixture {
public:
  Fixture() {
    const agiru::Session session(database.Dsn());
    agiru::CreateTable(session.Database(), agiru::TableTraits<agiru::platform::User>::kTable);
    for (const auto identity : {kUser, kOtherUser}) {
      agiru::platform::User user;
      user.UserSecurityID = agiru::Guid(identity);
      user.UserName = identity == kUser ? "FIRST USER" : "SECOND USER";
      user.Insert();
    }
    agiru::InstallClientCredentials(session.Database());
    agiru::InstallBrowserSessions(session.Database());
    agiru::InstallBrowserSessions(session.Database());
    agiru::Commit();
    sourceSecret =
        agiru::IssueClientCredential(connection, agiru::Guid(kUser), std::chrono::hours(1));
    const auto found = agiru::LookupClientCredentialIdentity(connection, "Bearer " + sourceSecret);
    if (!found) { throw std::runtime_error("missing gate source credential"); }
    source = *found;
  }

  gate::OwnedDatabase database{"browser_sessions"};
  agiru::Connection connection{database.Dsn()};
  agiru::ClientCredentialIdentity source;
  std::string sourceSecret;
};

agiru::BrowserSessionGrant Committed(Fixture &fixture, agiru::BrowserSessionOptions options = {}) {
  fixture.connection.Run("BEGIN");
  auto grant = agiru::IssueBrowserSession(fixture.connection, fixture.source, options);
  fixture.connection.Run("COMMIT");
  return grant;
}

agiru::BrowserSessionIdentity Identity(const agiru::Connection &connection,
                                       const agiru::BrowserSessionGrant &grant) {
  const auto found = agiru::LookupBrowserSession(connection, grant.secret);
  if (!found) { throw std::runtime_error("missing gate browser identity"); }
  return *found;
}

std::string Expires(const agiru::Connection &connection, std::string_view verifier) {
  const std::array<std::optional<std::string>, 1> binds{std::string(verifier)};
  return Value(connection,
               "SELECT expires_at::text FROM agiru_client.browser_sessions WHERE digest=$1",
               binds);
}

void MacContracts() {
  CHECK_TEXT("HMAC SHA-256 preserves RFC 4231 test case 1",
             agiru::SecureTokenMac(std::string(20, '\x0b'), "Hi There"),
             "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7");
  CHECK_TEXT("HMAC SHA-256 preserves RFC 4231 test case 2",
             agiru::SecureTokenMac("Jefe", "what do ya want for nothing?"),
             "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843");
  CHECK_TRUE("MAC key and message retain exact NUL bytes",
             agiru::SecureTokenMac(std::string_view("a\0b", 3), "d") !=
                     agiru::SecureTokenMac("a", "d") &&
                 agiru::SecureTokenMac("a", std::string_view("d\0e", 3)) !=
                     agiru::SecureTokenMac("a", "d"));
}

void IdentityContracts() {
  Fixture fixture;
  const auto first = Committed(fixture);
  const auto peer = Committed(fixture);
  const agiru::Connection observer(fixture.database.Dsn());
  const auto identity = Identity(observer, first);
  const auto other = Identity(observer, peer);
  CHECK_TRUE("same-user browsers have separate identities from each other and their agent source",
             identity.client.user == fixture.source.user &&
                 other.client.user == fixture.source.user &&
                 identity.client.verifier != other.client.verifier &&
                 identity.client.verifier != fixture.source.verifier);
  CHECK_TRUE("browser token and synchronizer token are canonical independent secrets",
             first.secret.starts_with("agb1_") && first.secret.size() == 69 &&
                 first.csrf.size() == 64 && first.csrf != peer.csrf &&
                 first.csrf != first.secret.substr(5) && first.csrf == identity.csrf);
  CHECK_TRUE("browser verifier is exactly its cookie hash, never its source credential hash",
             identity.client.verifier == agiru::SecureTokenDigest(first.secret));
  const auto rows =
      observer.Execute("SELECT digest,csrf_digest FROM agiru_client.browser_sessions");
  CHECK_TRUE("independent SQL sees both committed browser identities", rows.Rows() == 2);
  for (std::size_t row = 0; row < rows.Rows(); ++row) {
    CHECK_TRUE("SQL and SQL tracing contain only cookie and CSRF verifiers",
               (rows.Value(row, 0) == agiru::SecureTokenDigest(first.secret) &&
                rows.Value(row, 1) == agiru::SecureTokenDigest(first.csrf)) ||
                   (rows.Value(row, 0) == agiru::SecureTokenDigest(peer.secret) &&
                    rows.Value(row, 1) == agiru::SecureTokenDigest(peer.csrf)));
  }
  CHECK_TRUE("browser cookies cannot authenticate as agent bearer credentials",
             !agiru::LookupClientCredential(observer, "Bearer " + first.secret));
  CHECK_TRUE("agent bearer secrets cannot authenticate as browser sessions",
             !agiru::LookupBrowserSession(observer, fixture.sourceSecret));
  for (const auto &invalid : {std::string{},
                              "__Host-agiru=" + first.secret,
                              "Bearer " + first.secret,
                              first.secret + " ",
                              first.secret + std::string(1, '\0'),
                              "agb1_" + agiru::GenerateSecureToken()}) {
    CHECK_TRUE("malformed framed and unknown browser secrets cannot authenticate",
               !agiru::LookupBrowserSession(observer, invalid));
  }
  const auto before = Value(
      observer,
      "SELECT string_agg(last_activity_at::text||'|'||idle_expires_at::text,';' ORDER BY digest) "
      "FROM agiru_client.browser_sessions");
  static_cast<void>(agiru::LookupBrowserSession(observer, first.secret));
  CHECK_TEXT("passive session lookup never renews activity or idle expiry",
             Value(observer,
                   "SELECT string_agg(last_activity_at::text||'|'||idle_expires_at::text,"
                   "';' ORDER BY digest) FROM agiru_client.browser_sessions"),
             before);
}

void RenewalContracts() {
  Fixture fixture;
  const auto grant = Committed(fixture);
  const auto identity = Identity(fixture.connection, grant);
  const auto absolute = Expires(fixture.connection, identity.client.verifier);
  fixture.connection.Run("UPDATE agiru_client.browser_sessions SET "
                         "idle_expires_at=clock_timestamp()+interval '30 seconds'");
  auto forged = identity;
  forged.csrf = agiru::GenerateSecureToken();
  CHECK_TRUE("foreign CSRF cannot renew another browser identity",
             !agiru::RenewBrowserSession(fixture.connection, forged));
  forged = identity;
  forged.client.user = agiru::Guid(kOtherUser);
  CHECK_TRUE("foreign User GUID cannot renew a browser identity",
             !agiru::RenewBrowserSession(fixture.connection, forged));
  CHECK_TRUE("authorized fresh activity renews the idle deadline",
             agiru::RenewBrowserSession(fixture.connection, identity));
  CHECK_TEXT("idle renewal does not extend the absolute deadline",
             Expires(fixture.connection, identity.client.verifier),
             absolute);
  CHECK_TEXT("renewal is independently visible and bounded by its absolute deadline",
             Value(fixture.connection,
                   "SELECT idle_expires_at>clock_timestamp()+interval '10 minutes' "
                   "AND idle_expires_at<=expires_at FROM agiru_client.browser_sessions"),
             "t");
  fixture.connection.Run(
      "UPDATE agiru_client.credentials SET expires_at=clock_timestamp()+interval '1 minute'");
  CHECK_TRUE("renewal rechecks a shortened source deadline",
             agiru::RenewBrowserSession(fixture.connection, identity));
  CHECK_TEXT("renewed idle expiry cannot exceed a shortened source lifetime",
             Value(fixture.connection,
                   "SELECT b.idle_expires_at<=c.expires_at FROM agiru_client.browser_sessions b "
                   "JOIN agiru_client.credentials c ON c.digest=b.source_digest"),
             "t");
  fixture.connection.Run("UPDATE agiru_client.browser_sessions SET "
                         "idle_expires_at=clock_timestamp()-interval '1 second'");
  CHECK_TRUE("an expired idle session cannot authenticate or resurrect through renewal",
             !agiru::LookupBrowserSession(fixture.connection, grant.secret) &&
                 !agiru::RenewBrowserSession(fixture.connection, identity));
}

void RotationContracts() {
  Fixture fixture;
  const auto old = Committed(fixture);
  const auto identity = Identity(fixture.connection, old);
  const auto absolute = Expires(fixture.connection, identity.client.verifier);
  const agiru::Connection observer(fixture.database.Dsn());
  fixture.connection.Run("BEGIN");
  const auto aborted = agiru::RotateBrowserSession(fixture.connection, identity);
  CHECK_TRUE("uncommitted rotation cannot publish a new identity to another connection",
             agiru::LookupBrowserSession(observer, old.secret) &&
                 !agiru::LookupBrowserSession(observer, aborted.secret));
  CHECK_TRUE("the rotating transaction sees its exact replacement and denies its old token",
             !agiru::LookupBrowserSession(fixture.connection, old.secret) &&
                 agiru::LookupBrowserSession(fixture.connection, aborted.secret));
  fixture.connection.Run("ROLLBACK");
  CHECK_TRUE("rollback restores the old identity and grants no aborted replacement",
             agiru::LookupBrowserSession(observer, old.secret) &&
                 !agiru::LookupBrowserSession(observer, aborted.secret));
  fixture.connection.Run("BEGIN");
  const auto rotated = agiru::RotateBrowserSession(fixture.connection, identity);
  fixture.connection.Run("COMMIT");
  const auto fresh = Identity(observer, rotated);
  CHECK_TRUE("durable rotation replaces both cookie and synchronizer without grace reuse",
             old.secret != rotated.secret && old.csrf != rotated.csrf &&
                 fresh.client.user == identity.client.user &&
                 fresh.client.verifier != identity.client.verifier &&
                 !agiru::LookupBrowserSession(observer, old.secret));
  CHECK_TEXT("rotation preserves the original absolute deadline",
             Expires(observer, fresh.client.verifier),
             absolute);
  fixture.connection.Run("BEGIN");
  Refused("a stale rotation cannot create an extra replacement",
          "BrowserSessionAuthentication",
          [&] { static_cast<void>(agiru::RotateBrowserSession(fixture.connection, identity)); });
  fixture.connection.Run("ROLLBACK");
  CHECK_TEXT("rotation retains one revoked audit identity and one live replacement",
             Value(observer,
                   "SELECT count(*)::text||'|'||count(revoked_at)::text "
                   "FROM agiru_client.browser_sessions"),
             "2|1");
}

void RotationFailure() {
  Fixture fixture;
  const auto grant = Committed(fixture);
  const auto identity = Identity(fixture.connection, grant);
  fixture.connection.Run("CREATE FUNCTION refuse_browser_insert() RETURNS trigger LANGUAGE plpgsql "
                         "AS 'BEGIN RAISE EXCEPTION ''fixture insertion failure''; END'");
  fixture.connection.Run(
      "CREATE TRIGGER refuse_browser_insert BEFORE INSERT ON "
      "agiru_client.browser_sessions FOR EACH ROW EXECUTE FUNCTION refuse_browser_insert()");
  fixture.connection.Run("BEGIN");
  fixture.connection.Run("SAVEPOINT rotate_failure");
  bool failed = false;
  try {
    static_cast<void>(agiru::RotateBrowserSession(fixture.connection, identity));
  } catch (const agiru::DatabaseError &) { failed = true; }
  CHECK_TRUE("an injected replacement insertion error cannot become rotation success", failed);
  fixture.connection.Run("ROLLBACK TO SAVEPOINT rotate_failure");
  CHECK_TRUE("failed replacement insertion restores its old identity at the SQL boundary",
             agiru::LookupBrowserSession(fixture.connection, grant.secret).has_value());
  fixture.connection.Run("COMMIT");
  const agiru::Connection observer(fixture.database.Dsn());
  CHECK_TRUE("failed rotation preserves the old identity independently after commit",
             agiru::LookupBrowserSession(observer, grant.secret).has_value());
  CHECK_TEXT("failed rotation creates no replacement or revoked audit row",
             Value(observer,
                   "SELECT count(*)::text||'|'||count(revoked_at)::text "
                   "FROM agiru_client.browser_sessions"),
             "1|0");
}

void ExpiryAndRevocation() {
  Fixture fixture;
  const auto first = Committed(fixture);
  const auto peer = Committed(fixture);
  const auto identity = Identity(fixture.connection, first);
  const agiru::Connection observer(fixture.database.Dsn());
  CHECK_TRUE("browser revocation changes authority once",
             agiru::RevokeBrowserSession(fixture.connection, identity.client.verifier));
  CHECK_TRUE("browser revocation preserves its audit identity idempotently",
             !agiru::RevokeBrowserSession(fixture.connection, identity.client.verifier));
  CHECK_TRUE("revocation refuses lookup and renewal without disabling a peer browser",
             !agiru::LookupBrowserSession(observer, first.secret) &&
                 !agiru::RenewBrowserSession(fixture.connection, identity) &&
                 agiru::LookupBrowserSession(observer, peer.secret));
  CHECK_TRUE("source expiry caps the configured eight-hour browser lifetime",
             Value(observer,
                   "SELECT bool_and(b.expires_at<=c.expires_at)::text FROM "
                   "agiru_client.browser_sessions b JOIN agiru_client.credentials c "
                   "ON c.digest=b.source_digest") == "true");
  fixture.connection.Run(
      "UPDATE agiru_client.credentials SET issued_at=statement_timestamp()-interval '2 hours',"
      "expires_at=statement_timestamp()-interval '1 hour'");
  CHECK_TRUE("a source expired after issuance invalidates a still-live browser deadline",
             !agiru::LookupBrowserSession(observer, peer.secret));
  fixture.connection.Run(
      "UPDATE agiru_client.credentials SET expires_at=clock_timestamp()+interval '1 hour'");
  CHECK_TRUE("revoking the source credential invalidates all derived browser sessions",
             agiru::RevokeClientCredential(fixture.connection, fixture.source.verifier) &&
                 !agiru::LookupBrowserSession(observer, peer.secret));
  fixture.connection.Run("BEGIN");
  Refused(
      "a revoked source cannot issue another browser identity",
      "BrowserSessionAuthentication",
      [&] { static_cast<void>(agiru::IssueBrowserSession(fixture.connection, fixture.source)); });
  fixture.connection.Run("ROLLBACK");
}

void AbsoluteExpiry() {
  Fixture fixture;
  const auto grant = Committed(fixture);
  const auto identity = Identity(fixture.connection, grant);
  fixture.connection.Run("UPDATE agiru_client.browser_sessions SET "
                         "issued_at=statement_timestamp()-interval '2 hours',"
                         "expires_at=statement_timestamp()-interval '1 hour',"
                         "idle_expires_at=statement_timestamp()-interval '1 hour'");
  CHECK_TRUE("absolute expiry refuses both lookup and renewal",
             !agiru::LookupBrowserSession(fixture.connection, grant.secret) &&
                 !agiru::RenewBrowserSession(fixture.connection, identity));
  fixture.connection.Run("BEGIN");
  Refused("absolute expiry cannot be extended by rotation", "BrowserSessionAuthentication", [&] {
    static_cast<void>(agiru::RotateBrowserSession(fixture.connection, identity));
  });
  fixture.connection.Run("ROLLBACK");
  fixture.connection.Run("UPDATE agiru_client.credentials SET "
                         "issued_at=clock_timestamp()-interval '2 hours',"
                         "expires_at=clock_timestamp()-interval '1 hour'");
  fixture.connection.Run("BEGIN");
  Refused(
      "an expired source cannot issue a new browser identity", "BrowserSessionAuthentication", [&] {
        static_cast<void>(agiru::IssueBrowserSession(fixture.connection, fixture.source));
      });
  fixture.connection.Run("ROLLBACK");
}

void InputAndAdmission() {
  Fixture fixture;
  Refused(
      "issuance requires an explicit caller-owned transaction", "BrowserSessionTransaction", [&] {
        static_cast<void>(agiru::IssueBrowserSession(fixture.connection, fixture.source));
      });
  fixture.connection.Run("BEGIN ISOLATION LEVEL REPEATABLE READ");
  Refused("stale repeatable-read admission snapshots refuse", "BrowserSessionTransaction", [&] {
    static_cast<void>(agiru::IssueBrowserSession(fixture.connection, fixture.source));
  });
  fixture.connection.Run("ROLLBACK");
  for (const auto options : {agiru::BrowserSessionOptions{.idle = std::chrono::seconds(0)},
                             agiru::BrowserSessionOptions{.lifetime = std::chrono::hours(25)},
                             agiru::BrowserSessionOptions{.idle = std::chrono::hours(9)},
                             agiru::BrowserSessionOptions{.sessionsPerUser = 0},
                             agiru::BrowserSessionOptions{.sessionsPerUser = 1025}}) {
    Refused("invalid trusted browser policy refuses before SQL admission",
            "BrowserSessionConfiguration",
            [&] {
              static_cast<void>(
                  agiru::IssueBrowserSession(fixture.connection, fixture.source, options));
            });
  }
  const auto grant = Committed(fixture, {.sessionsPerUser = 1});
  fixture.connection.Run("BEGIN");
  Refused(
      "live same-user browser admission obeys the trusted capacity", "BrowserSessionCapacity", [&] {
        static_cast<void>(
            agiru::IssueBrowserSession(fixture.connection, fixture.source, {.sessionsPerUser = 1}));
      });
  fixture.connection.Run("ROLLBACK");
  CHECK_TEXT("capacity refusal adds no session identity",
             Value(fixture.connection, "SELECT count(*) FROM agiru_client.browser_sessions"),
             "1");
  fixture.connection.Run(
      R"(DELETE FROM "User" WHERE "User Security ID"='00000000-0000-0000-0000-000000000001')");
  CHECK_TRUE("User deletion cascades both credential and browser authority without blocking",
             !agiru::LookupBrowserSession(fixture.connection, grant.secret));
  CHECK_TEXT("deleted-user browser rows are removed",
             Value(fixture.connection, "SELECT count(*) FROM agiru_client.browser_sessions"),
             "0");
}

void ConcurrentAdmission() {
  Fixture fixture;
  const agiru::Connection observer(fixture.database.Dsn());
  fixture.connection.Run("BEGIN");
  static_cast<void>(
      agiru::IssueBrowserSession(fixture.connection, fixture.source, {.sessionsPerUser = 1}));
  std::atomic<int> peerPid = 0;
  std::atomic<bool> finished = false;
  std::string outcome;
  auto peer = std::async(std::launch::async, [&] {
    try {
      const agiru::Connection connection(fixture.database.Dsn());
      connection.Run("SET statement_timeout='5s'");
      peerPid = std::stoi(Value(connection, "SELECT pg_backend_pid()"));
      connection.Run("BEGIN");
      try {
        static_cast<void>(
            agiru::IssueBrowserSession(connection, fixture.source, {.sessionsPerUser = 1}));
        connection.Run("COMMIT");
        outcome = "admitted";
      } catch (const agiru::Error &error) {
        connection.Run("ROLLBACK");
        outcome = error.Code();
      }
    } catch (const std::exception &error) { outcome = error.what(); }
    finished = true;
  });
  bool locked = false;
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!finished && std::chrono::steady_clock::now() < deadline) {
    const auto pid = peerPid.load();
    if (pid != 0) {
      locked =
          Value(observer,
                "SELECT EXISTS(SELECT 1 FROM pg_stat_activity WHERE pid=" + std::to_string(pid) +
                    " AND wait_event_type='Lock')") == "t";
    }
    if (locked) { break; }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  fixture.connection.Run("COMMIT");
  peer.get();
  CHECK_TRUE("cross-connection admission waits on the same authoritative User row", locked);
  CHECK_TEXT("the waiting issuer observes the prior commit before counting capacity",
             outcome,
             "BrowserSessionCapacity");
  CHECK_TEXT("competing issuers cannot overrun one live browser identity",
             Value(observer, "SELECT count(*) FROM agiru_client.browser_sessions"),
             "1");
}

void ProviderFailure() {
  Refused("MAC provider failure refuses without a weak CSRF fallback", "SecureTokenProvider", [] {
    static_cast<void>(agiru::SecureTokenMac("secret", "domain"));
  });
}

void TraceSecrets(const std::string &path) {
  Fixture fixture;
  const auto grant = Committed(fixture);
  gate::PrivateAuthFile(path + ".source", fixture.sourceSecret);
  gate::PrivateAuthFile(path + ".cookie", grant.secret);
  gate::PrivateAuthFile(path + ".csrf", grant.csrf);
  const auto identity = Identity(fixture.connection, grant);
  CHECK_TRUE("trace fixture renews a live session",
             agiru::RenewBrowserSession(fixture.connection, identity));
  fixture.connection.Run("BEGIN");
  const auto fresh = agiru::RotateBrowserSession(fixture.connection, identity);
  fixture.connection.Run("COMMIT");
  gate::PrivateAuthFile(path + ".rotated-cookie", fresh.secret);
  gate::PrivateAuthFile(path + ".rotated-csrf", fresh.csrf);
  CHECK_TRUE("trace fixture resolves the replacement independently",
             agiru::LookupBrowserSession(fixture.connection, fresh.secret).has_value());
}

}

int main(int argc, char **argv) {
  return gate::Run("BrowserSession", [&] {
    if (argc == 2 && std::string_view(argv[1]) == "--mac-provider") {
      ProviderFailure();
      return;
    }
    MacContracts();
    if (argc == 3 && std::string_view(argv[1]) == "--trace-secrets") {
      TraceSecrets(argv[2]);
      return;
    }
    IdentityContracts();
    RenewalContracts();
    RotationContracts();
    RotationFailure();
    ExpiryAndRevocation();
    AbsoluteExpiry();
    InputAndAdmission();
    ConcurrentAdmission();
  });
}

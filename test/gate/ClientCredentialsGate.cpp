#include "platform/User.h"
#include "runtime/ClientCredentials.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/HttpServer.h"
#include "runtime/SecureToken.h"
#include "runtime/Session.h"
#include "runtime/SessionCommand.h"
#include "runtime/Storage.h"
#include "runtime/Transaction.h"
#include "type/Guid.h"

#include "Check.h"
#include "OwnedDatabase.h"
#include "PrivateAuthFile.h"

#include <array>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <ios>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {

constexpr std::string_view kUser = "00000000-0000-0000-0000-000000000001";
constexpr std::string_view kOtherUser = "00000000-0000-0000-0000-000000000002";
constexpr unsigned kAuthenticationRequired = 401;
constexpr unsigned kMethodNotAllowed = 405;

void Seed(const std::string &dsn) {
  const agiru::Session session(dsn);
  agiru::CreateTable(session.Database(), agiru::TableTraits<agiru::platform::User>::kTable);
  for (const auto identity : {kUser, kOtherUser}) {
    agiru::platform::User user;
    user.UserSecurityID = agiru::Guid(identity);
    user.UserName = identity == kUser ? "FIRST USER" : "SECOND USER";
    user.Insert();
  }
  agiru::InstallClientCredentials(session.Database());
  agiru::Commit();
}

void CryptoContracts() {
  CHECK_TEXT("SHA-256 retains the published empty-input vector",
             agiru::SecureTokenDigest(""),
             "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
  CHECK_TEXT("SHA-256 retains the published abc vector",
             agiru::SecureTokenDigest("abc"),
             "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
  CHECK_TRUE("digest includes NUL bytes",
             agiru::SecureTokenDigest(std::string_view("a\0b", 3)) !=
                 agiru::SecureTokenDigest("a"));
  const auto first = agiru::GenerateSecureToken();
  const auto second = agiru::GenerateSecureToken();
  CHECK_TRUE("secure tokens use the declared bounded hexadecimal representation",
             first.size() == 64 &&
                 first.find_first_not_of("0123456789abcdef") == std::string::npos);
  CHECK_TRUE("independent secure token generations do not repeat the sample", first != second);
}

void CredentialContracts() {
  const gate::OwnedDatabase database("client_credentials");
  Seed(database.Dsn());
  const agiru::Connection writer(database.Dsn());
  const agiru::Connection observer(database.Dsn());
  const auto first =
      agiru::IssueClientCredential(writer, agiru::Guid(kUser), std::chrono::hours(1));
  const auto second =
      agiru::IssueClientCredential(writer, agiru::Guid(kOtherUser), std::chrono::hours(1));
  CHECK_TRUE("independent credentials retain exact system GUID ownership",
             agiru::LookupClientCredential(writer, "Bearer " + first) == agiru::Guid(kUser) &&
                 agiru::LookupClientCredential(writer, "bEaReR  " + second) ==
                     agiru::Guid(kOtherUser));
  const auto rows =
      observer.Execute("SELECT digest, user_security_id::text FROM agiru_client.credentials");
  CHECK_TRUE("independent SQL sees both durable verifiers, never plaintext credentials",
             rows.Rows() == 2);
  for (std::size_t at = 0; at < rows.Rows(); ++at) {
    const auto digest = rows.Value(at, 0).value_or("");
    CHECK_TRUE("stored verifier is SHA-256, not its secret",
               digest == agiru::SecureTokenDigest(first) ||
                   digest == agiru::SecureTokenDigest(second));
  }
  for (const auto *const invalid :
       {"", "Bearer", "Bearer ", "Basic abc", "Bearer ag1_bad", "User "}) {
    CHECK_TRUE("malformed credentials refuse without success",
               !agiru::LookupClientCredential(writer, invalid));
  }
  for (const auto &invalid : {first,
                              "Bearer " + first + " ",
                              "Bearer\t" + first,
                              "Bearer " + first + "\r\nX-Test: forged",
                              "Bearer " + std::string(200, 'a'),
                              "Bearer ag1_" + agiru::GenerateSecureToken()}) {
    CHECK_TRUE("missing/unknown/framed credentials cannot authenticate",
               !agiru::LookupClientCredential(writer, invalid));
  }
  CHECK_TRUE("revocation changes authoritative SQL once",
             agiru::RevokeClientCredential(writer, agiru::SecureTokenDigest(first)));
  CHECK_TRUE("revocation survives a different connection",
             !agiru::LookupClientCredential(observer, "Bearer " + first));
  CHECK_TRUE("revocation retains an idempotent audit identity",
             !agiru::RevokeClientCredential(writer, agiru::SecureTokenDigest(first)));
  writer.Run(
      "UPDATE agiru_client.credentials SET issued_at = clock_timestamp() - interval '2 hours', "
      "expires_at = clock_timestamp() - interval '1 hour' WHERE revoked_at IS NULL");
  CHECK_TRUE("expiry is evaluated against PostgreSQL time",
             !agiru::LookupClientCredential(observer, "Bearer " + second));
  for (const auto duration :
       {std::chrono::seconds(0), std::chrono::seconds(-1), std::chrono::seconds(86401)}) {
    bool refused = false;
    try {
      static_cast<void>(agiru::IssueClientCredential(writer, agiru::Guid(kUser), duration));
    } catch (const agiru::Error &error) { refused = error.Code() == "ClientCredentialInput"; }
    CHECK_TRUE("invalid credential lifetimes refuse before issuance", refused);
  }
  bool blank = false;
  try {
    static_cast<void>(agiru::IssueClientCredential(writer, agiru::Guid{}, std::chrono::hours(1)));
  } catch (const agiru::Error &error) { blank = error.Code() == "ClientCredentialInput"; }
  CHECK_TRUE("blank credential identities cannot reach SQL", blank);
  bool missing = false;
  try {
    static_cast<void>(
        agiru::IssueClientCredential(writer, agiru::Guid::Create(), std::chrono::hours(1)));
  } catch (const agiru::DatabaseError &) { missing = true; }
  CHECK_TRUE("foreign-key ownership rejects invented users", missing);
  writer.Run("BEGIN");
  const auto aborted =
      agiru::IssueClientCredential(writer, agiru::Guid(kUser), std::chrono::hours(1));
  writer.Run("ROLLBACK");
  CHECK_TRUE("rolled-back issuance grants no credential",
             !agiru::LookupClientCredential(observer, "Bearer " + aborted));
  const auto deletedUser =
      agiru::IssueClientCredential(writer, agiru::Guid(kOtherUser), std::chrono::hours(1));
  const std::array<std::optional<std::string>, 1> identity{std::string(kOtherUser)};
  writer.Run(R"(DELETE FROM "User" WHERE "User Security ID" = $1::uuid)", identity);
  CHECK_TRUE("account deletion revokes credentials without being blocked by their foreign keys",
             !agiru::LookupClientCredential(observer, "Bearer " + deletedUser));
}

void ProviderFailure(std::string_view provider) {
  bool refused = false;
  try {
    if (provider == "random") {
      static_cast<void>(agiru::GenerateSecureToken());
    } else {
      static_cast<void>(agiru::SecureTokenDigest("secret"));
    }
  } catch (const agiru::Error &error) { refused = error.Code() == "SecureTokenProvider"; }
  CHECK_TRUE("provider failure explicitly refuses without fallback", refused);
}

class AuthenticationFixture {
public:
  AuthenticationFixture(std::string html, const std::string &authPath)
      : database_("http_credentials"), html_(std::move(html)) {
    Seed(database_.Dsn());
    const agiru::Connection connection(database_.Dsn());
    connection.Run("CREATE TABLE authenticated_receipts(user_security_id uuid, method text)");
    gate::PrivateAuthFile(
        authPath,
        agiru::IssueClientCredential(connection, agiru::Guid(kUser), std::chrono::hours(1)));
    gate::PrivateAuthFile(
        authPath + ".second",
        agiru::IssueClientCredential(connection, agiru::Guid(kOtherUser), std::chrono::hours(1)));
    const auto name = connection.Execute("SELECT current_database()");
    std::fputs("DATABASE ", stdout);
    std::fputs(std::string(name.Value(0, 0).value_or("")).c_str(), stdout);
    std::fputc('\n', stdout);
  }

  agiru::ServerHttpResponse Handle(const agiru::ServerHttpRequest &request) const {
    agiru::Connection connection(database_.Dsn());
    const auto user = agiru::LookupClientCredential(connection, request.Header("Authorization"));
    if (!user) { return Refused(); }
    if (request.method != "GET") {
      return {.status = kMethodNotAllowed,
              .body = "<p>Only authored fixture reads are supported</p>",
              .headers = {{.name = "Allow", .value = "GET"}}};
    }
    try {
      agiru::Session session(*user);
      agiru::SessionCommand command(session, connection);
      const std::array<std::optional<std::string>, 2> binds{
          session.UserSecurityId().ToStorageText(), request.method};
      connection.Run("INSERT INTO authenticated_receipts VALUES ($1::uuid, $2)", binds);
      agiru::ServerHttpResponse response{
          .body = html_,
          .headers = {{.name = "X-Fixture-User", .value = std::string(session.UserId())}}};
      command.Keep();
      return response;
    } catch (const agiru::SessionError &) { return Refused(); }
  }

private:
  static agiru::ServerHttpResponse Refused() {
    return {.status = kAuthenticationRequired,
            .body = "<p>Authentication required</p>",
            .headers = {{.name = "WWW-Authenticate", .value = "Bearer realm=\"agiru\""}}};
  }

  gate::OwnedDatabase database_;
  std::string html_;
};

void Serve(const char *htmlPath, const char *authPath) {
  std::ifstream input(htmlPath, std::ios::binary);
  if (!input) { throw std::runtime_error("missing authored HTML fixture"); }
  const AuthenticationFixture fixture(
      std::string{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()},
      authPath);
  const agiru::HttpServer server([&](const auto &request) { return fixture.Handle(request); });
  std::fputs("READY\n", stdout);
  std::fflush(stdout);
  while (true) {
    const int command = std::getchar();
    if (command == 'Q' || command == EOF) { break; }
  }
}

}

int main(int argc, char **argv) {
  return gate::Run("ClientCredentials", [=] {
    if (argc == 1) {
      CryptoContracts();
      CredentialContracts();
    } else if (argc == 3 && std::string_view(argv[1]) == "--provider-failure") {
      ProviderFailure(argv[2]);
    } else if (argc == 4 && std::string_view(argv[1]) == "--serve") {
      Serve(argv[2], argv[3]);
    } else {
      throw std::runtime_error("invalid ClientCredentialsGate mode");
    }
  });
}

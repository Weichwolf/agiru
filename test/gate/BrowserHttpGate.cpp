#include "platform/User.h"
#include "runtime/BrowserSession.h"
#include "runtime/ClientCredentials.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/HttpServer.h"
#include "runtime/PageCommandHost.h"
#include "runtime/PageHostOptions.h"
#include "runtime/SecureToken.h"
#include "runtime/Session.h"
#include "runtime/SessionCommand.h"
#include "runtime/Storage.h"
#include "runtime/TablePermissions.h"
#include "type/Guid.h"

#include "BrowserHttp.h"
#include "Check.h"
#include "JsonEngine.h"
#include "OwnedDatabase.h"
#include "PrivateAuthFile.h"

#include <chrono>
#include <cstdio>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {

constexpr std::string_view kUser = "00000000-0000-0000-0000-000000000001";
constexpr std::string_view kOrigin = "https://localhost:8443";
constexpr std::string_view kCookie = "__Host-agiru=";
constexpr std::string_view kHttpsScheme = "https://";

void Header(agiru::ServerHttpRequest &request, std::string_view name, std::string_view value) {
  for (auto &header : request.headers) {
    if (header.name == name) {
      header.value = value;
      return;
    }
  }
  request.headers.push_back({.name = std::string(name), .value = std::string(value)});
}

std::string Header(const agiru::ServerHttpResponse &response, std::string_view name) {
  for (const auto &header : response.headers) {
    if (header.name == name) { return header.value; }
  }
  return {};
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
  explicit Fixture(std::string origin = std::string(kOrigin)) {
    const agiru::Session seed(database.Dsn());
    agiru::CreateTable(seed.Database(), agiru::TableTraits<agiru::platform::User>::kTable);
    agiru::platform::User user;
    user.UserSecurityID = agiru::Guid(kUser);
    user.UserName = "COOKIE USER";
    user.Insert();
    agiru::InstallClientCredentials(seed.Database());
    agiru::InstallBrowserSessions(seed.Database());
    seed.Database().Run("CREATE TABLE cookie_probe(user_security_id uuid)");
    agiru::Commit();
    source = agiru::IssueClientCredential(connection, agiru::Guid(kUser), std::chrono::hours(1));
    options.database = database.Dsn();
    options.company = "Cookie fixture";
    options.origin = std::move(origin);
    options.browserCookies = true;
    agiru::detail::ValidateBrowserHttpOptions(options);
  }

  agiru::ServerHttpRequest Request(std::string_view method = "POST",
                                   std::string_view target = "/session") const {
    return {.method = std::string(method),
            .target = std::string(target),
            .headers = {{.name = "Accept", .value = "application/json"},
                        {.name = "Content-Type", .value = "application/json"},
                        {.name = "Origin", .value = options.origin},
                        {.name = "Sec-Fetch-Site", .value = "same-origin"},
                        {.name = "Sec-Fetch-Mode", .value = "cors"},
                        {.name = "Sec-Fetch-Dest", .value = "empty"},
                        {.name = "X-Agiru-Client", .value = "browser"},
                        {.name = "X-Forwarded-Proto", .value = "https"},
                        {.name = "X-Forwarded-Host",
                         .value = options.origin.substr(kHttpsScheme.size())}},
            .body = {}};
  }

  agiru::ServerHttpResponse Login() const {
    auto request = Request();
    Header(request, "Authorization", "Bearer " + source);
    const auto response = agiru::detail::BrowserSessionEndpoint(connection, request, options);
    if (!response) { throw std::runtime_error("missing session endpoint response"); }
    return *response;
  }

  agiru::ServerHttpRequest Authenticated(const agiru::ServerHttpResponse &grant,
                                         std::string_view method = "GET",
                                         std::string_view target = "/protected") const {
    auto request = Request(method, target);
    const auto cookie = Header(grant, "Set-Cookie");
    Header(request, "Cookie", std::string_view(cookie).substr(0, cookie.find(';')));
    Header(request, "X-Agiru-CSRF", agiru::detail::JsonNode::parse(grant.body)["csrf"].Text());
    return request;
  }

  gate::OwnedDatabase database{"browser_http"};
  mutable agiru::Connection connection{database.Dsn()};
  agiru::PageHostOptions options;
  std::string source;
};

void CookieContracts() {
  const Fixture fixture;
  const auto grant = fixture.Login();
  const auto cookie = Header(grant, "Set-Cookie");
  CHECK_TRUE("cookie is host-only secure HttpOnly Strict root-path and not persistent",
             cookie.starts_with(kCookie) &&
                 cookie.ends_with("; Secure; HttpOnly; SameSite=Strict; Path=/") &&
                 !cookie.contains("Domain") && !cookie.contains("Max-Age"));
  const auto secret =
      std::string_view(cookie).substr(kCookie.size(), cookie.find(';') - kCookie.size());
  CHECK_TRUE("session response exposes CSRF only, never the source or cookie credential",
             !grant.body.contains(secret) && !grant.body.contains(fixture.source));
  auto request = fixture.Authenticated(grant);
  const auto client =
      agiru::detail::AuthenticatePageClient(fixture.connection, request, fixture.options);
  const auto bearer =
      agiru::LookupClientCredentialIdentity(fixture.connection, "Bearer " + fixture.source);
  CHECK_TRUE("browser routes use their independent verifier and original AL User GUID",
             bearer && client.identity.user == bearer->user &&
                 client.identity.verifier != bearer->verifier && !client.csrf.empty());
  const auto retained = agiru::detail::RetainedPageRequest(request, client.csrf);
  CHECK_TRUE("retained AL work contains no Cookie Authorization or forwarding authority",
             retained.Header("Cookie").empty() && retained.Header("Authorization").empty() &&
                 retained.Header("X-Forwarded-Host").empty() &&
                 retained.Header("X-Agiru-CSRF") == client.csrf);
  request = fixture.Request("GET", "/protected");
  Header(request, "Authorization", "Bearer " + fixture.source);
  Header(request, "X-Agiru-CSRF", "untrusted agent header");
  const auto agent =
      agiru::detail::AuthenticatePageClient(fixture.connection, request, fixture.options);
  CHECK_TRUE("agent bearers remain separate and cannot inject browser renewal authority",
             bearer && agent.identity.verifier == bearer->verifier && agent.csrf.empty());
  CHECK_TRUE("constant-time adapter compares exact equal-length bytes including NUL",
             agiru::SecureTokenEqual(std::string_view("a\0b", 3), std::string_view("a\0b", 3)) &&
                 !agiru::SecureTokenEqual(std::string_view("a\0b", 3), "a") &&
                 !agiru::SecureTokenEqual("abcd", "abce"));
}

void BrowserRefusals() {
  Fixture fixture;
  const auto grant = fixture.Login();
  const auto original = fixture.Authenticated(grant);
  for (const auto &[name, value] : {std::pair{"X-Forwarded-Proto", "http"},
                                    {"X-Forwarded-Host", "elsewhere.invalid"},
                                    {"Sec-Fetch-Site", "cross-site"},
                                    {"Sec-Fetch-Site", "same-site"},
                                    {"Sec-Fetch-Site", ""},
                                    {"Sec-Fetch-Mode", "navigate"},
                                    {"Sec-Fetch-Dest", "document"},
                                    {"Sec-Fetch-User", "?1"},
                                    {"Purpose", "prefetch"},
                                    {"Sec-Purpose", "prefetch"},
                                    {"X-Agiru-Client", ""},
                                    {"Origin", "https://elsewhere.invalid"},
                                    {"X-Agiru-CSRF", ""},
                                    {"X-Agiru-CSRF", "forged"}}) {
    auto request = original;
    Header(request, name, value);
    Refused("invalid browser context refuses before protected GET execution",
            "PageHostPermission",
            [&] {
              static_cast<void>(agiru::detail::AuthenticatePageClient(
                  fixture.connection, request, fixture.options));
            });
  }
  auto request = original;
  Header(request, "Origin", "");
  CHECK_TRUE("same-origin GET accepts absent Origin only with metadata and session CSRF",
             !agiru::detail::AuthenticatePageClient(fixture.connection, request, fixture.options)
                  .csrf.empty());
  request.method = "POST";
  Refused("cookie POST requires exact trusted Origin", "PageHostPermission", [&] {
    static_cast<void>(
        agiru::detail::AuthenticatePageClient(fixture.connection, request, fixture.options));
  });
  request = original;
  Header(request, "Authorization", "Bearer " + fixture.source);
  Refused("cookie and bearer ambiguity refuses rather than adopting either client",
          "PageHostAuthentication",
          [&] {
            static_cast<void>(agiru::detail::AuthenticatePageClient(
                fixture.connection, request, fixture.options));
          });
  request = original;
  Header(request,
         "Cookie",
         std::string(request.Header("Cookie")) + "; " + std::string(request.Header("Cookie")));
  Refused("repeated session cookies refuse even when their values agree",
          "PageHostAuthentication",
          [&] {
            static_cast<void>(agiru::detail::AuthenticatePageClient(
                fixture.connection, request, fixture.options));
          });
  fixture.options.browserCookies = false;
  Refused("HTTP development profiles never accept browser cookies", "PageHostPermission", [&] {
    static_cast<void>(
        agiru::detail::AuthenticatePageClient(fixture.connection, original, fixture.options));
  });
}

void Lifecycle() {
  Fixture fixture;
  const auto grant = fixture.Login();
  const auto original = fixture.Authenticated(grant, "GET", "/session");
  const auto bootstrap =
      agiru::detail::BrowserSessionEndpoint(fixture.connection, original, fixture.options);
  CHECK_TRUE("same-origin bootstrap restores CSRF without rotating cookies or exposing credentials",
             bootstrap && bootstrap->body == grant.body &&
                 Header(*bootstrap, "Set-Cookie").empty());
  auto request = fixture.Authenticated(grant, "POST", "/session/rotate");
  const auto rotated =
      agiru::detail::BrowserSessionEndpoint(fixture.connection, request, fixture.options);
  CHECK_TRUE("HTTP rotation replaces both exact secrets only after durable SQL commit",
             rotated && Header(*rotated, "Set-Cookie") != Header(grant, "Set-Cookie") &&
                 rotated->body != grant.body);
  Refused("old cookie cannot authenticate after rotation", "PageHostAuthentication", [&] {
    static_cast<void>(
        agiru::detail::AuthenticatePageClient(fixture.connection, original, fixture.options));
  });
  if (!rotated) { throw std::runtime_error("rotation did not return a grant"); }
  request = fixture.Authenticated(*rotated, "POST", "/session/logout");
  const auto logout =
      agiru::detail::BrowserSessionEndpoint(fixture.connection, request, fixture.options);
  CHECK_TRUE("logout durably revokes SQL authority and deletes the protected host cookie",
             logout && logout->body == "{}" &&
                 Header(*logout, "Set-Cookie") ==
                     "__Host-agiru=; Secure; HttpOnly; SameSite=Strict; Path=/; Max-Age=0");
  Refused("a logged-out cookie cannot resume an old browser client", "PageHostAuthentication", [&] {
    static_cast<void>(
        agiru::detail::AuthenticatePageClient(fixture.connection, request, fixture.options));
  });
}

void AccountAndIssuance() {
  Fixture fixture;
  auto request = fixture.Request();
  Header(request, "Authorization", "Bearer " + fixture.source);
  Header(request, "Origin", "https://elsewhere.invalid");
  Refused("cross-origin source credential exchange cannot create a browser identity",
          "PageHostPermission",
          [&] {
            static_cast<void>(agiru::detail::BrowserSessionEndpoint(
                fixture.connection, request, fixture.options));
          });
  fixture.connection.Run(R"(UPDATE "User" SET "State"=1)");
  bool disabled = false;
  try {
    static_cast<void>(fixture.Login());
  } catch (const agiru::SessionError &) { disabled = true; }
  CHECK_TRUE("disabled User cannot exchange a valid source credential", disabled);
  const auto rows =
      fixture.connection.Execute("SELECT count(*) FROM agiru_client.browser_sessions");
  CHECK_TRUE("refused exchange leaves no browser identity in independently visible SQL",
             rows.Value(0, 0) == "0");
}

class DeniedTables final : public agiru::TablePermissionAuthority {
public:
  bool Allows(const agiru::TableDef &table, agiru::TableOperation operation) const override {
    static_cast<void>(table);
    static_cast<void>(operation);
    return false;
  }
};

void ProductionHost() {
  const Fixture fixture;
  agiru::InstallPageCommandHost(fixture.connection);
  agiru::PageCommandHost host(
      fixture.options,
      [](const auto &, auto, const auto &) { throw agiru::Error("no ERP grant", "Permission"); },
      std::make_shared<DeniedTables>());
  auto request = fixture.Request();
  Header(request, "Authorization", "Bearer " + fixture.source);
  const auto grant = host.Handle(request);
  constexpr unsigned kSuccess = 200;
  CHECK_TRUE("production page host exposes the durable session endpoint without ERP grants",
             grant.status == kSuccess && Header(grant, "Set-Cookie").starts_with(kCookie));
  request = fixture.Authenticated(grant, "GET", "/?page=2147483647");
  constexpr unsigned kNotFound = 404;
  CHECK_TRUE("production page host resolves a valid cookie before page discovery",
             host.Handle(request).status == kNotFound);
  Header(request, "X-Agiru-CSRF", "");
  constexpr unsigned kForbidden = 403;
  CHECK_TRUE("production page host rejects missing browser CSRF before page discovery",
             host.Handle(request).status == kForbidden);
  const auto rows = fixture.connection.Execute("SELECT count(*) FROM agiru_client.page_contexts");
  CHECK_TRUE("cookie denial and unknown pages create no SQL page contexts",
             rows.Value(0, 0) == "0");
}

agiru::ServerHttpResponse Probe(Fixture &fixture, const agiru::ServerHttpRequest &request) {
  try {
    agiru::Connection connection(fixture.database.Dsn());
    if (auto response =
            agiru::detail::BrowserSessionEndpoint(connection, request, fixture.options)) {
      return std::move(*response);
    }
    const auto client = agiru::detail::AuthenticatePageClient(connection, request, fixture.options);
    agiru::Session session(client.identity.user);
    agiru::SessionCommand command(session, connection);
    connection.Run("INSERT INTO cookie_probe VALUES('00000000-0000-0000-0000-000000000001')");
    command.Keep();
    return {.contentType = "application/json; charset=utf-8",
            .body = "{\"accepted\":true}",
            .headers = {}};
  } catch (const agiru::SessionError &) {
    constexpr unsigned kUnauthorized = 401;
    return {.status = kUnauthorized,
            .contentType = "text/plain; charset=utf-8",
            .body = "SessionIdentity",
            .headers = {}};
  } catch (const agiru::Error &error) {
    constexpr unsigned kUnauthorized = 401;
    constexpr unsigned kForbidden = 403;
    constexpr unsigned kBadRequest = 400;
    return {.status = error.Code() == "PageHostAuthentication" ? kUnauthorized
                      : error.Code() == "PageHostPermission"   ? kForbidden
                                                               : kBadRequest,
            .contentType = "text/plain; charset=utf-8",
            .body = std::string(error.Code()),
            .headers = {}};
  }
}

void Serve(const std::string &path, const std::string &origin) {
  Fixture fixture(origin);
  gate::PrivateAuthFile(path, fixture.source);
  const agiru::HttpServer server(
      [&fixture](const auto &request) { return Probe(fixture, request); }, {.workers = 2});
  std::puts("READY");
  std::fflush(stdout);
  while (true) {
    const int input = std::getchar();
    if (input == EOF || input == 'Q') { break; }
  }
}

}

int main(int argc, char **argv) {
  return gate::Run("BrowserHttp", [&] {
    if (argc == 4 && std::string_view(argv[1]) == "--serve") {
      Serve(argv[2], argv[3]);
      return;
    }
    CookieContracts();
    BrowserRefusals();
    Lifecycle();
    AccountAndIssuance();
    ProductionHost();
  });
}

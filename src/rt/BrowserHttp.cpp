#include "BrowserHttp.h"

#include "runtime/BrowserSession.h"
#include "runtime/ClientCredentials.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/HttpServer.h"
#include "runtime/PageHostOptions.h"
#include "runtime/SecureToken.h"
#include "runtime/Session.h"
#include "runtime/SessionCommand.h"

#include <optional>
#include <string>
#include <string_view>

namespace agiru::detail {
namespace {

constexpr std::string_view kCookie = "__Host-agiru";
constexpr std::string_view kFlags = "; Secure; HttpOnly; SameSite=Strict; Path=/";
constexpr std::string_view kHttpsScheme = "https://";

[[noreturn]] void Refuse(std::string_view code) {
  throw Error("browser request refused", std::string(code));
}

std::string_view Trim(std::string_view value) {
  const auto first = value.find_first_not_of(" \t");
  if (first == std::string_view::npos) { return {}; }
  value.remove_prefix(first);
  return value.substr(0, value.find_last_not_of(" \t") + 1);
}

std::optional<std::string_view> Cookie(const ServerHttpRequest &request) {
  auto remaining = request.Header("Cookie");
  std::optional<std::string_view> cookie;
  while (!remaining.empty()) {
    const auto separator = remaining.find(';');
    const auto part = Trim(remaining.substr(0, separator));
    const auto equals = part.find('=');
    if (equals == std::string_view::npos || equals == 0) { Refuse("PageHostInput"); }
    if (part.substr(0, equals) == kCookie) {
      if (cookie) { Refuse("PageHostAuthentication"); }
      cookie = part.substr(equals + 1);
    }
    if (separator == std::string_view::npos) { break; }
    remaining.remove_prefix(separator + 1);
    if (Trim(remaining).empty()) { Refuse("PageHostInput"); }
  }
  return cookie;
}

void BrowserContext(const ServerHttpRequest &request, const PageHostOptions &options) {
  const auto mode = request.Header("Sec-Fetch-Mode");
  if (!options.browserCookies || request.Header("X-Forwarded-Proto") != "https" ||
      request.Header("X-Forwarded-Host") !=
          std::string_view(options.origin).substr(kHttpsScheme.size()) ||
      request.Header("Sec-Fetch-Site") != "same-origin" ||
      (mode != "cors" && mode != "same-origin") || request.Header("Sec-Fetch-Dest") != "empty" ||
      request.Header("X-Agiru-Client") != "browser" || !request.Header("Sec-Fetch-User").empty() ||
      !request.Header("Purpose").empty() || !request.Header("Sec-Purpose").empty() ||
      (!request.Header("Origin").empty() && request.Header("Origin") != options.origin) ||
      (request.method != "GET" && request.Header("Origin") != options.origin)) {
    Refuse("PageHostPermission");
  }
}

BrowserSessionIdentity Browser(const Connection &connection,
                               const ServerHttpRequest &request,
                               const PageHostOptions &options,
                               bool requireCsrf) {
  BrowserContext(request, options);
  if (!request.Header("Authorization").empty()) { Refuse("PageHostAuthentication"); }
  const auto cookie = Cookie(request);
  const auto identity = cookie ? LookupBrowserSession(connection, *cookie) : std::nullopt;
  if (!identity) { Refuse("PageHostAuthentication"); }
  if (requireCsrf && !SecureTokenEqual(identity->csrf, request.Header("X-Agiru-CSRF"))) {
    Refuse("PageHostPermission");
  }
  return *identity;
}

ServerHttpResponse Response(std::string_view csrf) {
  return {.contentType = "application/json; charset=utf-8",
          .body = R"({"csrf":")" + std::string(csrf) + R"("})",
          .headers = {{.name = "Vary", .value = "Origin, Sec-Fetch-Site"}}};
}

ServerHttpResponse Grant(const BrowserSessionGrant &grant) {
  auto response = Response(grant.csrf);
  response.headers.push_back(
      {.name = "Set-Cookie",
       .value = std::string(kCookie) + "=" + grant.secret + std::string(kFlags)});
  return response;
}

ServerHttpResponse
Issue(Connection &connection, const ServerHttpRequest &request, const PageHostOptions &options) {
  BrowserContext(request, options);
  const auto source = LookupClientCredentialIdentity(connection, request.Header("Authorization"));
  if (!source) { Refuse("PageHostAuthentication"); }
  const auto cookie = Cookie(request);
  if (cookie && LookupBrowserSession(connection, *cookie)) { Refuse("PageHostAuthentication"); }
  Session session(source->user, options.session);
  SessionCommand command(session, connection);
  const auto grant = IssueBrowserSession(connection, *source, options.browser);
  auto response = Grant(grant);
  command.Keep();
  return response;
}

}

void ValidateBrowserHttpOptions(const PageHostOptions &options) {
  try {
    ValidateBrowserSessionOptions(options.browser);
  } catch (const Error &) { Refuse("PageHostConfiguration"); }
  if (options.browserCookies &&
      (!options.origin.starts_with(kHttpsScheme) || options.origin.size() <= kHttpsScheme.size() ||
       std::string_view(options.origin)
               .substr(kHttpsScheme.size())
               .find_first_of("/\\?#@, \t\r\n") != std::string_view::npos)) {
    Refuse("PageHostConfiguration");
  }
}

PageClient AuthenticatePageClient(const Connection &connection,
                                  const ServerHttpRequest &request,
                                  const PageHostOptions &options) {
  if (Cookie(request)) {
    const auto browser = Browser(connection, request, options, true);
    return {.identity = browser.client, .csrf = browser.csrf};
  }
  const auto client = LookupClientCredentialIdentity(connection, request.Header("Authorization"));
  if (!client) { Refuse("PageHostAuthentication"); }
  return {.identity = *client, .csrf = {}};
}

std::optional<ServerHttpResponse> BrowserSessionEndpoint(Connection &connection,
                                                         const ServerHttpRequest &request,
                                                         const PageHostOptions &options) {
  const bool endpoint = request.target == "/session" || request.target == "/session/logout" ||
                        request.target == "/session/rotate";
  if (!endpoint) { return std::nullopt; }
  if (request.Header("Accept") != "application/json" || !request.body.empty() ||
      (request.method == "POST" && request.Header("Content-Type") != "application/json")) {
    Refuse("PageHostInput");
  }
  if (request.target == "/session" && request.method == "POST") {
    return Issue(connection, request, options);
  }
  if (request.method != "POST" && !(request.target == "/session" && request.method == "GET")) {
    Refuse("PageHostInput");
  }
  const auto identity = Browser(connection, request, options, request.method != "GET");
  Session session(identity.client.user, options.session);
  SessionCommand command(session, connection);
  auto response = Response(identity.csrf);
  if (request.target == "/session/rotate") {
    response = Grant(RotateBrowserSession(connection, identity));
  } else if (request.target == "/session/logout") {
    static_cast<void>(RevokeBrowserSession(connection, identity.client.verifier));
    response.body = "{}";
    response.headers.push_back(
        {.name = "Set-Cookie",
         .value = std::string(kCookie) + "=" + std::string(kFlags) + "; Max-Age=0"});
  }
  command.Keep();
  return response;
}

void RenewPageClient(const Connection &connection,
                     const ClientCredentialIdentity &client,
                     std::string_view csrf) {
  if (!csrf.empty()) {
    static_cast<void>(
        RenewBrowserSession(connection, {.client = client, .csrf = std::string(csrf)}));
  }
}

ServerHttpRequest RetainedPageRequest(const ServerHttpRequest &request, std::string_view csrf) {
  ServerHttpRequest retained{
      .method = request.method, .target = request.target, .headers = {}, .body = request.body};
  for (const std::string_view name : {"Content-Type", "Origin"}) {
    retained.headers.push_back(
        {.name = std::string(name), .value = std::string(request.Header(name))});
  }
  retained.headers.push_back({.name = "X-Agiru-CSRF", .value = std::string(csrf)});
  return retained;
}

}

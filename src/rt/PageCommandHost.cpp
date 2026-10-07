#include "runtime/PageCommandHost.h"

#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/Catalogue.h"
#include "runtime/ClientCredentials.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/HttpServer.h"
#include "runtime/PageDispatcher.h"
#include "runtime/PageHtml.h"
#include "runtime/PageInstance.h"
#include "runtime/SecureToken.h"
#include "runtime/Session.h"
#include "runtime/SessionCommand.h"
#include "runtime/TablePermissions.h"
#include "type/Guid.h"
#include "type/RecordId.h"
#include "type/Utf8.h"

#include "HtmlText.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace agiru {
namespace {

constexpr unsigned kBadRequest = 400;
constexpr unsigned kUnauthorized = 401;
constexpr unsigned kForbidden = 403;
constexpr unsigned kNotFound = 404;
constexpr unsigned kMethodNotAllowed = 405;
constexpr unsigned kConflict = 409;
constexpr unsigned kGone = 410;
constexpr unsigned kUnavailable = 503;
constexpr std::size_t kMaximumParameters = 8;
constexpr auto kMaximumContextLifetime = std::chrono::hours(24);
constexpr std::string_view kActionPrefix = "$agiru.";
using Parameters = std::map<std::string, std::string, std::less<>>;

[[noreturn]] void Refuse(std::string_view code) {
  throw Error("Page host refused this operation", std::string(code));
}

bool Token(std::string_view value) {
  return !value.empty() && value.size() <= PageHtmlContext::kHandleBytes &&
         std::ranges::all_of(value, [](char unit) {
           return (unit >= '0' && unit <= '9') || (unit >= 'a' && unit <= 'z') ||
                  (unit >= 'A' && unit <= 'Z') || unit == '_' || unit == '-';
         });
}

unsigned HexDigit(char unit) {
  if (unit >= '0' && unit <= '9') { return static_cast<unsigned>(unit - '0'); }
  if (unit >= 'a' && unit <= 'f') { return static_cast<unsigned>(unit - 'a') + 10; }
  if (unit >= 'A' && unit <= 'F') { return static_cast<unsigned>(unit - 'A') + 10; }
  Refuse("PageHostInput");
}

std::string Decode(std::string_view input, bool form) {
  std::string output;
  output.reserve(input.size());
  for (std::size_t at = 0; at < input.size(); ++at) {
    char unit = input[at];
    if (unit == '%') {
      if (input.size() - at < 3) { Refuse("PageHostInput"); }
      unit = static_cast<char>((HexDigit(input[at + 1]) << 4) | HexDigit(input[at + 2]));
      at += 2;
    } else if (form && unit == '+') {
      unit = ' ';
    }
    if (unit == '\0') { Refuse("PageHostInput"); }
    output += unit;
  }
  if (!IsValidUtf8(output)) { Refuse("PageHostInput"); }
  return output;
}

Parameters Parse(std::string_view input, bool form) {
  Parameters values;
  while (!input.empty()) {
    const auto separator = input.find('&');
    const auto pair = input.substr(0, separator);
    const auto equal = pair.find('=');
    if (equal == std::string_view::npos || equal == 0 || values.size() >= kMaximumParameters) {
      Refuse("PageHostInput");
    }
    if (!values.emplace(Decode(pair.substr(0, equal), form), Decode(pair.substr(equal + 1), form))
             .second) {
      Refuse("PageHostInput");
    }
    if (separator == std::string_view::npos) { break; }
    input.remove_prefix(separator + 1);
    if (input.empty()) { Refuse("PageHostInput"); }
  }
  return values;
}

std::string_view Get(const Parameters &values, std::string_view name) {
  const auto found = values.find(name);
  return found == values.end() ? std::string_view{} : found->second;
}

void Known(const Parameters &values, std::span<const std::string_view> names) {
  for (const auto &[name, value] : values) {
    static_cast<void>(value);
    if (std::ranges::find(names, name) == names.end()) { Refuse("PageHostUnsupported"); }
  }
}

std::int64_t Number(std::string_view text) {
  std::int64_t result = 0;
  if (text.empty() || text.front() == '+' || text.front() == '-') { Refuse("PageHostInput"); }
  const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
  if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || result < 0) {
    Refuse("PageHostInput");
  }
  return result;
}

class Authorization final : public PageAuthorization {
public:
  Authorization(const PageDef &page, PageHostAuthorization &authority)
      : page_(page), authority_(authority) {}

  void Require(PageId page, const PageControlCommand &command) override {
    if (page != page_.id) { Refuse("PageHostPermission"); }
    authority_(page_, PageHostOperation::Control, command);
  }

private:
  const PageDef &page_;
  PageHostAuthorization &authority_;
};

struct Context {
  Context(const Guid &principal, const PageHostOptions &options)
      : session(principal, options.session),
        deadline(std::chrono::steady_clock::now() + options.lifetime) {
    session.CompanyName(options.company);
  }

  std::mutex mutex;
  Session session;
  std::unique_ptr<PageInstance> page;
  std::vector<std::unique_ptr<PageInstance>> parents;
  std::string handle = GenerateSecureToken();
  std::string csrf = GenerateSecureToken();
  std::string prefix = GenerateSecureToken();
  std::int64_t revision = 0;
  std::chrono::steady_clock::time_point deadline;
  bool invalid = false;
};

PageOpenMode Mode(const PageDef &page, std::string_view input) {
  if (input.empty() || input == "View") { return PageOpenMode::View; }
  if (input == "Edit") {
    if (page.type == PageType::List || page.type == PageType::RoleCenter ||
        page.type == PageType::CardPart) {
      return PageOpenMode::View;
    }
    return PageOpenMode::Edit;
  }
  if (input == "Create") {
    if (page.type == PageType::CardPart || page.type == PageType::List ||
        page.type == PageType::ListPart || page.type == PageType::RoleCenter) {
      return PageOpenMode::View;
    }
    if (page.type == PageType::Worksheet) { Refuse("PageHostInput"); }
    return PageOpenMode::New;
  }
  Refuse("PageHostInput");
}

PageHostOperation Opening(PageOpenMode mode) {
  switch (mode) {
    case PageOpenMode::View: return PageHostOperation::OpenView;
    case PageOpenMode::Edit: return PageHostOperation::OpenEdit;
    case PageOpenMode::New: return PageHostOperation::OpenNew;
    case PageOpenMode::Unknown: break;
  }
  Refuse("PageHostInput");
}

unsigned Status(std::string_view code) {
  if (code == "PageHostAuthentication" || code == "SessionIdentity") { return kUnauthorized; }
  if (code == "PageHostPermission" || code == "Permission") { return kForbidden; }
  if (code == "PageHostMissing") { return kNotFound; }
  if (code == "PageHostGone") { return kGone; }
  if (code == "PageHostBusy" || code == "PageHostRevision" || code == "PageHostReceipt") {
    return kConflict;
  }
  if (code == "PageHostCapacity") { return kUnavailable; }
  return kBadRequest;
}

std::string DiagnosticHtml(std::string_view message) {
  std::string html = "<p>";
  constexpr std::size_t kClosingBytes = 4;
  detail::AppendHtmlText(html, message, PageHtmlLimits::kDefaultBytes - kClosingBytes);
  html += "</p>";
  return html;
}

}

void ValidatePageHostOptions(const PageHostOptions &options) {
  if (options.database.empty() || options.company.empty() || options.origin.empty() ||
      options.contexts == 0 || options.navigationDepth == 0 || options.commands == 0 ||
      options.receiptBytes == 0 || options.lifetime.count() <= 0 ||
      options.lifetime > kMaximumContextLifetime) {
    Refuse("PageHostConfiguration");
  }
}

void InstallPageCommandHost(const Connection &connection) {
  connection.Run(R"(CREATE TABLE IF NOT EXISTS agiru_client.page_contexts (
    handle text PRIMARY KEY,
    user_security_id uuid NOT NULL REFERENCES "User"("User Security ID") ON DELETE CASCADE,
    host_id text NOT NULL,
    company text NOT NULL,
    revision bigint NOT NULL DEFAULT 0 CHECK (revision >= 0),
    expires_at timestamptz NOT NULL,
    invalidated boolean NOT NULL DEFAULT false
  ))");
  connection.Run(R"(CREATE TABLE IF NOT EXISTS agiru_client.page_commands (
    handle text NOT NULL REFERENCES agiru_client.page_contexts(handle) ON DELETE CASCADE,
    command_id text NOT NULL,
    payload_digest text NOT NULL,
    outcome text NOT NULL CHECK (outcome IN ('started', 'complete', 'failed')),
    html text,
    page_id integer,
    PRIMARY KEY(handle, command_id),
    CHECK ((outcome = 'complete') = (html IS NOT NULL))
  ))");
}

struct PageCommandHost::Impl {
  Impl(PageHostOptions configuration,
       PageHostAuthorization authority,
       std::shared_ptr<const TablePermissionAuthority> tableAuthorization)
      : options(std::move(configuration)),
        authorize(std::move(authority)),
        tableAuthority(std::move(tableAuthorization)) {
    ValidatePageHostOptions(options);
    if (!authorize || !tableAuthority) { Refuse("PageHostConfiguration"); }
  }

  std::shared_ptr<Context> Retain(const Guid &user) {
    const std::lock_guard lock(mutex);
    std::erase_if(contexts, [](const auto &item) {
      const std::unique_lock idle(item.second->mutex, std::try_to_lock);
      return idle.owns_lock() &&
             (item.second->invalid || item.second->deadline <= std::chrono::steady_clock::now());
    });
    if (contexts.size() >= options.contexts) { Refuse("PageHostCapacity"); }
    auto context = std::make_shared<Context>(user, options);
    context->session.TablePermissions(tableAuthority);
    contexts.emplace(context->handle, context);
    return context;
  }

  std::shared_ptr<Context> Find(std::string_view handle) {
    if (!Token(handle)) { Refuse("PageHostInput"); }
    const std::lock_guard lock(mutex);
    const auto found = contexts.find(handle);
    if (found == contexts.end()) { Refuse("PageHostGone"); }
    return found->second;
  }

  void Ownership(const Connection &connection, const Context &context, const Guid &user) const {
    const std::array<std::optional<std::string>, 4> binds{
        context.handle, user.ToStorageText(), host, options.company};
    std::string sql = "SELECT revision FROM agiru_client.page_contexts WHERE handle = $1 "
                      "AND user_security_id = $2::uuid AND host_id = $3 AND company = $4 "
                      "AND NOT invalidated AND expires_at > clock_timestamp()";
    if (connection.InTransaction()) { sql += " FOR UPDATE"; }
    const auto rows = connection.Execute(sql, binds);
    if (context.invalid || rows.Rows() != 1) { Refuse("PageHostGone"); }
    const auto revision = rows.Value(0, 0);
    if (!revision || Number(*revision) != context.revision) { Refuse("PageHostRevision"); }
  }

  std::string Render(Context &context) {
    auto &page = *context.page;
    authorize(page.Declaration(), PageHostOperation::Read, {});
    std::vector<PageHtmlAction> actions;
    if (page.Declaration().source.Value() != 0) {
      actions = {{.identity = "$agiru.first", .caption = "First"},
                 {.identity = "$agiru.previous", .caption = "Previous"},
                 {.identity = "$agiru.next", .caption = "Next"},
                 {.identity = "$agiru.last", .caption = "Last"},
                 {.identity = "$agiru.save", .caption = "Save"}};
    }
    if (page.Declaration().cardPageId.Value() != 0) {
      actions.push_back({"$agiru.card", "Edit card"});
    }
    if (!context.parents.empty()) { actions.push_back({"$agiru.back", "Back"}); }
    Authorization authority(page.Declaration(), authorize);
    return RenderPageHtml(page.Declaration(),
                          page.Controls(),
                          authority,
                          {.pageHandle = context.handle,
                           .revision = std::to_string(context.revision),
                           .commandPrefix = context.prefix,
                           .csrf = context.csrf,
                           .actions = actions})
        .html;
  }

  ServerHttpResponse Response(Context &context, std::string html) const {
    return {.body = std::move(html),
            .headers = {{.name = "Content-Location", .value = "/?handle=" + context.handle}}};
  }

  ServerHttpResponse Open(Connection &connection, const Guid &user, const Parameters &values) {
    constexpr std::array<std::string_view, 3> names{"page", "mode", "company"};
    Known(values, names);
    const auto company = Get(values, "company");
    if (!company.empty() && company != options.company) { Refuse("PageHostPermission"); }
    const auto number = Number(Get(values, "page"));
    if (number == 0 || number > std::numeric_limits<std::int32_t>::max()) {
      Refuse("PageHostInput");
    }
    auto context = Retain(user);
    const std::lock_guard lock(context->mutex);
    try {
      SessionCommand command(context->session, connection);
      context->page = MakeInstalledPage(PageId{static_cast<std::int32_t>(number)});
      const auto mode = Mode(context->page->Declaration(), Get(values, "mode"));
      authorize(context->page->Declaration(), Opening(mode), {});
      context->session.OpenCompany();
      context->page->Open(mode);
      const std::array<std::optional<std::string>, 5> binds{
          context->handle,
          user.ToStorageText(),
          host,
          options.company,
          std::to_string(options.lifetime.count())};
      connection.Run(
          "INSERT INTO "
          "agiru_client.page_contexts(handle,user_security_id,host_id,company,expires_at) "
          "VALUES ($1,$2::uuid,$3,$4,clock_timestamp() + $5::integer * interval '1 second')",
          binds);
      auto result = Response(*context, Render(*context));
      command.Keep();
      return result;
    } catch (...) {
      context->invalid = true;
      throw;
    }
  }

  ServerHttpResponse Read(Connection &connection, const Guid &user, const Parameters &values) {
    constexpr std::array<std::string_view, 1> names{"handle"};
    Known(values, names);
    auto context = Find(Get(values, "handle"));
    const std::unique_lock lock(context->mutex, std::try_to_lock);
    if (!lock.owns_lock()) { Refuse("PageHostBusy"); }
    Ownership(connection, *context, user);
    SessionCommand command(context->session, connection);
    auto result = Response(*context, Render(*context));
    command.Keep();
    return result;
  }

  void Lifecycle(Context &context, std::string_view control) const {
    auto &page = *context.page;
    if (control == "$agiru.card") {
      if (context.parents.size() >= options.navigationDepth) { Refuse("PageHostCapacity"); }
      const auto &source = page.Declaration();
      if (source.cardPageId.Value() == 0) { Refuse("PageHostUnsupported"); }
      authorize(source, PageHostOperation::Read, {});
      auto card = MakeInstalledPage(source.cardPageId);
      if (card->Declaration().source != source.source) { Refuse("PageHostUnsupported"); }
      authorize(card->Declaration(), PageHostOperation::OpenEdit, {});
      const RecordId record = page.CurrentRecord();
      if (record.IsEmpty()) { Refuse("PageHostMissing"); }
      card->Open(PageOpenMode::Edit);
      if (!card->SelectRecord(record)) { Refuse("PageHostMissing"); }
      context.parents.push_back(std::move(context.page));
      context.page = std::move(card);
      return;
    }
    if (control == "$agiru.back") {
      if (context.parents.empty()) { Refuse("PageHostUnsupported"); }
      authorize(page.Declaration(), PageHostOperation::Close, {});
      authorize(context.parents.back()->Declaration(), PageHostOperation::Read, {});
      page.Close();
      context.page = std::move(context.parents.back());
      context.parents.pop_back();
      const RecordId selected = context.page->CurrentRecord();
      if (!selected.IsEmpty() && !context.page->SelectRecord(selected)) {
        Refuse("PageHostMissing");
      }
      return;
    }
    if (control == "$agiru.save") {
      authorize(page.Declaration(), PageHostOperation::Save, {});
      page.Save();
      return;
    }
    PagePosition position = PagePosition::Unknown;
    if (control == "$agiru.first") { position = PagePosition::First; }
    if (control == "$agiru.previous") { position = PagePosition::Previous; }
    if (control == "$agiru.next") { position = PagePosition::Next; }
    if (control == "$agiru.last") { position = PagePosition::Last; }
    if (position == PagePosition::Unknown) { Refuse("PageHostUnsupported"); }
    authorize(page.Declaration(), PageHostOperation::Move, {});
    static_cast<void>(page.Move(position));
  }

  void Execute(Context &context, const Parameters &values) {
    const auto operation = Get(values, "operation");
    const auto control = Get(values, "control");
    if (operation == "action" && control.starts_with(kActionPrefix)) {
      const auto &declaration = context.page->Declaration();
      if (Control(declaration.layout, control) != nullptr ||
          Control(declaration.actions, control) != nullptr) {
        Refuse("PageHostUnsupported");
      }
      Lifecycle(context, control);
      return;
    }
    Authorization authority(context.page->Declaration(), authorize);
    PageDispatcher dispatcher(context.page->Declaration(), context.page->Controls(), authority);
    static_cast<void>(dispatcher.Execute(
        {.operation = operation == "set" ? PageControlOperation::Set : PageControlOperation::Action,
         .control = control,
         .text = Get(values, "text")}));
  }

  void AuthorizeSnapshot(const PageDef &page, std::span<const ControlDef> controls) {
    for (const auto &control : controls) {
      authorize(page,
                PageHostOperation::Control,
                {.operation = control.kind == ControlKind::Field ? PageControlOperation::ReadValue
                                                                 : PageControlOperation::Inspect,
                 .control = control.name});
      AuthorizeSnapshot(page, control.children);
    }
  }

  std::optional<ServerHttpResponse> Replay(Connection &connection,
                                           Context &context,
                                           const Parameters &values,
                                           const std::string &digest) {
    const std::array<std::optional<std::string>, 2> binds{context.handle,
                                                          std::string(Get(values, "command"))};
    const auto rows = connection.Execute(
        "SELECT payload_digest,outcome,html,page_id FROM agiru_client.page_commands "
        "WHERE handle = $1 AND command_id = $2",
        binds);
    if (rows.Rows() == 0) { return std::nullopt; }
    if (rows.Rows() != 1 || rows.Value(0, 0) != digest || rows.Value(0, 1) != "complete") {
      Refuse("PageHostReceipt");
    }
    const auto html = rows.Value(0, 2);
    const auto pageId = rows.Value(0, 3);
    if (!html || !pageId) { Refuse("PageHostReceipt"); }
    const auto number = Number(*pageId);
    if (number > std::numeric_limits<std::int32_t>::max()) { Refuse("PageHostReceipt"); }
    const auto *entry = FindPage(PageId{static_cast<std::int32_t>(number)});
    if (entry == nullptr) { Refuse("PageHostReceipt"); }
    SessionCommand command(context.session, connection);
    Ownership(connection, context, context.session.UserSecurityId());
    authorize(*entry->page, PageHostOperation::Read, {});
    AuthorizeSnapshot(*entry->page, entry->page->layout);
    AuthorizeSnapshot(*entry->page, entry->page->actions);
    auto result = Response(context, std::string(*html));
    command.Keep();
    return result;
  }

  void Budget(const Connection &connection,
              std::string_view handle,
              std::size_t bytes,
              bool starting) const {
    const std::array<std::optional<std::string>, 1> binds{std::string(handle)};
    const auto rows = connection.Execute("SELECT count(*),COALESCE(sum(octet_length(html)),0) "
                                         "FROM agiru_client.page_commands WHERE handle = $1",
                                         binds);
    const auto count = rows.Value(0, 0);
    const auto stored = rows.Value(0, 1);
    if (!count || !stored ||
        (starting && std::cmp_greater_equal(Number(*count), options.commands)) ||
        bytes > options.receiptBytes ||
        std::cmp_greater(Number(*stored), options.receiptBytes - bytes)) {
      Refuse("PageHostCapacity");
    }
  }

  void Invalidate(Context &context, std::string_view commandId) const {
    context.invalid = true;
    const Connection cleanup(options.database);
    const std::array<std::optional<std::string>, 1> handle{context.handle};
    cleanup.Run("UPDATE agiru_client.page_contexts SET invalidated = true WHERE handle = $1",
                handle);
    const std::array<std::optional<std::string>, 2> binds{context.handle, std::string(commandId)};
    cleanup.Run("UPDATE agiru_client.page_commands SET outcome = 'failed',html = NULL "
                "WHERE handle = $1 AND command_id = $2 AND outcome = 'started'",
                binds);
  }

  ServerHttpResponse
  Write(Connection &connection, const Guid &user, const ServerHttpRequest &request) {
    if (request.Header("Content-Type") != "application/x-www-form-urlencoded" ||
        request.Header("Origin") != options.origin) {
      Refuse("PageHostInput");
    }
    const auto values = Parse(request.body, true);
    constexpr std::array<std::string_view, 7> names{
        "page", "revision", "command", "csrf", "operation", "control", "text"};
    Known(values, names);
    const auto operation = Get(values, "operation");
    const auto commandId = Get(values, "command");
    if (!Token(commandId) || Get(values, "control").empty() ||
        (operation != "set" && operation != "action") ||
        (operation == "set") != values.contains("text")) {
      Refuse("PageHostInput");
    }
    auto context = Find(Get(values, "page"));
    const std::unique_lock lock(context->mutex, std::try_to_lock);
    if (!lock.owns_lock()) { Refuse("PageHostBusy"); }
    Ownership(connection, *context, user);
    if (Get(values, "csrf") != context->csrf) { Refuse("PageHostPermission"); }
    const auto digest = SecureTokenDigest(request.body);
    if (auto replay = Replay(connection, *context, values, digest)) { return std::move(*replay); }
    if (Number(Get(values, "revision")) != context->revision ||
        !commandId.starts_with(context->prefix + "_")) {
      Refuse("PageHostRevision");
    }
    const auto suffix = commandId.substr(context->prefix.size() + 1);
    static_cast<void>(Number(suffix));
    Budget(connection, context->handle, 0, true);
    const std::array<std::optional<std::string>, 3> started{
        context->handle, std::string(commandId), digest};
    connection.Run(
        "INSERT INTO agiru_client.page_commands(handle,command_id,payload_digest,outcome) "
        "VALUES ($1,$2,$3,'started')",
        started);
    try {
      SessionCommand command(context->session, connection);
      Ownership(connection, *context, user);
      Execute(*context, values);
      if (context->revision == std::numeric_limits<std::int64_t>::max()) {
        Refuse("PageHostRevision");
      }
      ++context->revision;
      context->prefix = GenerateSecureToken();
      auto result = Response(*context, Render(*context));
      Budget(connection, context->handle, result.body.size(), false);
      const std::array<std::optional<std::string>, 2> revision{context->handle,
                                                               std::to_string(context->revision)};
      if (connection
              .Execute("UPDATE agiru_client.page_contexts SET revision = $2::bigint "
                       "WHERE handle = $1 AND revision = $2::bigint - 1",
                       revision)
              .Affected() != 1) {
        Refuse("PageHostRevision");
      }
      const std::array<std::optional<std::string>, 4> completed{
          context->handle,
          std::string(commandId),
          result.body,
          std::to_string(context->page->Declaration().id.Value())};
      connection.Run("UPDATE agiru_client.page_commands SET outcome = 'complete',html = $3,page_id "
                     "= $4::integer "
                     "WHERE handle = $1 AND command_id = $2",
                     completed);
      command.Keep();
      return result;
    } catch (const std::exception &execution) {
      try {
        Invalidate(*context, commandId);
      } catch (const std::exception &cleanup) {
        throw Error(std::string(execution.what()) + "; page cleanup failed: " + cleanup.what(),
                    "PageHostCleanup");
      }
      throw;
    } catch (...) {
      Invalidate(*context, commandId);
      throw;
    }
  }

  ServerHttpResponse Handle(const ServerHttpRequest &request) {
    Connection connection(options.database);
    const auto user = LookupClientCredential(connection, request.Header("Authorization"));
    if (!user) { Refuse("PageHostAuthentication"); }
    if (request.method == "GET" && request.target.starts_with("/?")) {
      const auto values = Parse(std::string_view(request.target).substr(2), false);
      return values.contains("handle") ? Read(connection, *user, values)
                                       : Open(connection, *user, values);
    }
    if (request.method == "POST" && request.target == "/commands") {
      return Write(connection, *user, request);
    }
    return {.status = kMethodNotAllowed,
            .body = "<p>Unsupported page endpoint or method</p>",
            .headers = {{.name = "Allow", .value = "GET, POST"}}};
  }

  PageHostOptions options;
  PageHostAuthorization authorize;
  std::shared_ptr<const TablePermissionAuthority> tableAuthority;
  std::string host = GenerateSecureToken();
  std::mutex mutex;
  std::map<std::string, std::shared_ptr<Context>, std::less<>> contexts;
};

PageCommandHost::PageCommandHost(PageHostOptions options,
                                 PageHostAuthorization authorization,
                                 std::shared_ptr<const TablePermissionAuthority> tableAuthorization)
    : impl_(std::make_unique<Impl>(
          std::move(options), std::move(authorization), std::move(tableAuthorization))) {}

PageCommandHost::~PageCommandHost() = default;

ServerHttpResponse PageCommandHost::Handle(const ServerHttpRequest &request) {
  try {
    return impl_->Handle(request);
  } catch (const SessionError &) {
    return {.status = kUnauthorized,
            .body = "<p>Authentication required</p>",
            .headers = {{.name = "WWW-Authenticate", .value = "Bearer realm=\"agiru\""}}};
  } catch (const Error &error) {
    if (error.Code() == "PageHostCleanup" ||
        (!error.Code().starts_with("PageHost") && error.Code() != "SessionIdentity" &&
         error.Code() != "PageValidation" && error.Code() != "PageCommand" &&
         error.Code() != "Permission")) {
      throw;
    }
    ServerHttpResponse response{.status = error.Code() == "Permission" ? kForbidden
                                                                       : Status(error.Code()),
                                .body = DiagnosticHtml(error.what()),
                                .headers = {}};
    if (response.status == kUnauthorized) {
      response.headers.push_back({.name = "WWW-Authenticate", .value = "Bearer realm=\"agiru\""});
    }
    return response;
  }
}

}

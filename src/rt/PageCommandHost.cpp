#include "runtime/PageCommandHost.h"

#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/Catalogue.h"
#include "runtime/ClientCredentials.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/HttpServer.h"
#include "runtime/HttpServerOptions.h"
#include "runtime/PageDispatcher.h"
#include "runtime/PageHostOptions.h"
#include "runtime/PageHtml.h"
#include "runtime/PageInstance.h"
#include "runtime/PageWindow.h"
#include "runtime/RecordWindow.h"
#include "runtime/SecureToken.h"
#include "runtime/Session.h"
#include "runtime/SessionCommand.h"
#include "runtime/TablePermissions.h"
#include "runtime/UiHost.h"
#include "type/Guid.h"
#include "type/RecordId.h"
#include "type/Utf8.h"

#include "BrowserHttp.h"
#include "HtmlText.h"
#include "PageInteraction.h"
#include "PageListHtml.h"
#include "PageModal.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
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
#include <thread>
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
constexpr unsigned kInternalError = 500;
constexpr std::size_t kMaximumParameters = 8;
constexpr auto kMaximumContextLifetime = std::chrono::hours(24);
constexpr std::string_view kActionPrefix = "$agiru.";
constexpr std::string_view kCallPrefix = "/calls/";
using Parameters = std::map<std::string, std::string, std::less<>>;
using Client = ClientCredentialIdentity;

struct Diagnostic {
  std::string_view message;
  std::string_view code;
  std::string_view command;
  std::string_view outcome;
};

struct CommandOutcome {
  std::string_view command;
  std::string_view outcome;
};

class CommandFailure final : public Error {
public:
  explicit CommandFailure(const Diagnostic &diagnostic)
      : Error(diagnostic.message, diagnostic.code),
        command_(diagnostic.command),
        outcome_(diagnostic.outcome) {}

  std::string_view Command() const { return command_; }

  std::string_view Outcome() const { return outcome_; }

private:
  std::string command_;
  std::string outcome_;
};

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
  Authorization(const PageDef &page, const PageHostAuthorization &authority)
      : page_(page), authority_(authority) {}

  void Require(PageId page, const PageControlCommand &command) override {
    if (page != page_.id) { Refuse("PageHostPermission"); }
    authority_(page_, PageHostOperation::Control, command);
  }

private:
  const PageDef &page_;
  const PageHostAuthorization &authority_;
};

struct PageFrame {
  std::unique_ptr<PageInstance> page;
  std::optional<PageListView> list;
};

using Call = detail::PageCall;

class Executor {
public:
  explicit Executor(const PageHostOptions &options) : capacity_(options.executionQueue) {
    try {
      for (std::size_t at = 0; at < options.executionWorkers; ++at) {
        workers_.emplace_back([this] { Work(); });
      }
    } catch (...) {
      Stop();
      throw;
    }
  }

  ~Executor() { Stop(); }

  Executor(const Executor &) = delete;
  Executor &operator=(const Executor &) = delete;

  void Submit(std::function<void()> work) {
    const std::lock_guard lock(mutex_);
    if (stopping_ || queue_.size() >= capacity_) { Refuse("PageHostCapacity"); }
    queue_.push_back(std::move(work));
    ready_.notify_one();
  }

private:
  void Stop() {
    {
      const std::lock_guard lock(mutex_);
      stopping_ = true;
    }
    ready_.notify_all();
    for (auto &worker : workers_) {
      if (worker.joinable()) { worker.join(); }
    }
  }

  void Work() {
    for (;;) {
      std::function<void()> work;
      {
        std::unique_lock lock(mutex_);
        ready_.wait(lock, [this] { return stopping_ || !queue_.empty(); });
        if (queue_.empty()) { return; }
        work = std::move(queue_.front());
        queue_.pop_front();
      }
      work();
    }
  }

  std::size_t capacity_;
  std::mutex mutex_;
  std::condition_variable ready_;
  std::deque<std::function<void()>> queue_;
  std::vector<std::thread> workers_;
  bool stopping_ = false;
};

struct Context {
  Context(const Client &principal, const PageHostOptions &options)
      : client(principal),
        session(principal.user, options.session),
        deadline(std::chrono::steady_clock::now() + options.lifetime) {
    session.CompanyName(options.company);
  }

  std::mutex mutex;
  std::mutex callMutex;
  std::shared_ptr<Call> call;
  const Client client;
  Session session;
  std::unique_ptr<PageInstance> page;
  std::optional<PageListView> list;
  std::vector<PageFrame> parents;
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

bool Insertable(const PageDef &page) {
  if (page.insertAllowed.empty()) { return true; }
  const auto equal = [&](std::string_view literal) {
    return std::ranges::equal(page.insertAllowed, literal, [](char left, char right) {
      const char lower = left >= 'A' && left <= 'Z' ? static_cast<char>(left - 'A' + 'a') : left;
      return lower == right;
    });
  };
  if (equal("true")) { return true; }
  if (equal("false")) { return false; }
  Refuse("PageHostUnsupported");
}

unsigned Status(std::string_view code) {
  if (code == "PageHostAuthentication" || code == "SessionIdentity") { return kUnauthorized; }
  if (code == "PageHostPermission" || code == "Permission") { return kForbidden; }
  if (code == "PageHostMissing") { return kNotFound; }
  if (code == "PageHostGone") { return kGone; }
  if (code == "PageHostBusy" || code == "PageHostRevision" || code == "PageHostReceipt" ||
      code == "PageHostDialogStale" || code == "PageHostModalStale" || code == "PageHostReplay") {
    return kConflict;
  }
  if (code == "PageHostCapacity") { return kUnavailable; }
  return kBadRequest;
}

std::string DiagnosticHtml(const Diagnostic &diagnostic) {
  std::string html = R"(<article data-agiru-error="1" data-code=")";
  constexpr std::size_t kClosingBytes = 14;
  constexpr auto kBytes = PageHtmlLimits::kDefaultBytes - kClosingBytes;
  detail::AppendHtmlText(html, diagnostic.code.empty() ? "AlError" : diagnostic.code, kBytes);
  html += "\" data-command=\"";
  detail::AppendHtmlText(html, diagnostic.command, kBytes);
  html += "\" data-outcome=\"";
  detail::AppendHtmlText(html, diagnostic.outcome, kBytes);
  html += "\"><h1>Request failed</h1><p>";
  detail::AppendHtmlText(html, diagnostic.message, kBytes);
  html += "</p></article>";
  return html;
}

ServerHttpResponse
FailureResponse(const Error &error, unsigned status, const CommandOutcome &result) {
  return {.status = status,
          .body = DiagnosticHtml({.message = error.what(),
                                  .code = error.Code(),
                                  .command = result.command,
                                  .outcome = result.outcome}),
          .headers = {}};
}

std::string RequestCommand(const ServerHttpRequest &request) {
  if (request.method != "POST" || (request.target != "/commands" && request.target != "/answers" &&
                                   !request.target.starts_with("/modal-commands/"))) {
    return {};
  }
  try {
    const auto values = Parse(request.body, true);
    const auto command = Get(values, "command");
    return Token(command) ? std::string(command) : std::string{};
  } catch (const Error &) { return {}; }
}

}

void ValidatePageHostOptions(const PageHostOptions &options) {
  detail::ValidateBrowserHttpOptions(options);
  try {
    ValidateRecordWindowLimit(options.listRows);
  } catch (const Error &) { Refuse("PageHostConfiguration"); }
  if (options.database.empty() || options.company.empty() || options.origin.empty() ||
      options.contexts == 0 || options.navigationDepth == 0 || options.commands == 0 ||
      options.receiptBytes == 0 || options.executionWorkers == 0 ||
      options.executionWorkers > HttpServerOptions::kMaxWorkers || options.executionQueue == 0 ||
      options.responseWait.count() <= 0 || options.responseWait > std::chrono::seconds(1) ||
      options.dialogTimeout.count() <= 0 || options.dialogTimeout > kMaximumContextLifetime ||
      options.lifetime.count() <= 0 || options.lifetime > kMaximumContextLifetime) {
    Refuse("PageHostConfiguration");
  }
}

void InstallPageCommandHost(const Connection &connection) {
  detail::InstallPageDialogs(connection);
  detail::InstallPageModals(connection);
  connection.Run(R"(CREATE TABLE IF NOT EXISTS agiru_client.page_contexts (
    handle text PRIMARY KEY,
    user_security_id uuid NOT NULL REFERENCES "User"("User Security ID") ON DELETE CASCADE,
    host_id text NOT NULL,
    company text NOT NULL,
    credential_digest text CHECK (credential_digest ~ '^[0-9a-f]{64}$'),
    revision bigint NOT NULL DEFAULT 0 CHECK (revision >= 0),
    expires_at timestamptz NOT NULL,
    invalidated boolean NOT NULL DEFAULT false
  ))");
  connection.Run("ALTER TABLE agiru_client.page_contexts ADD COLUMN IF NOT EXISTS "
                 "credential_digest text CHECK (credential_digest ~ '^[0-9a-f]{64}$')");
  connection.Run("UPDATE agiru_client.page_contexts SET invalidated = true "
                 "WHERE credential_digest IS NULL AND NOT invalidated");
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
    executor = std::make_unique<Executor>(options);
  }

  ~Impl() {
    {
      const std::lock_guard lock(mutex);
      for (const auto &[handle, weak] : calls) {
        static_cast<void>(handle);
        if (auto call = weak.lock()) {
          const std::lock_guard state(call->mutex);
          call->cancelled = true;
          call->ready.notify_all();
        }
      }
    }
    executor.reset();
  }

  std::string Working(const Call &call) const { return detail::RenderPageInteraction(call); }

  void ModalReadAuthority(Connection &connection, const Call &call, PageId page) const {
    Session authority(call.user, options.session);
    authority.CompanyName(options.company);
    authority.TablePermissions(tableAuthority);
    SessionCommand command(authority, connection);
    const auto *entry = FindPage(page);
    if (entry == nullptr) { Refuse("PageHostGone"); }
    authorize(*entry->page, PageHostOperation::Read, {});
    AuthorizeSnapshot(*entry->page, entry->page->layout);
    AuthorizeSnapshot(*entry->page, entry->page->actions);
    command.Keep();
  }

  ServerHttpResponse Await(Connection &connection, const std::shared_ptr<Call> &call) const {
    std::unique_lock lock(call->mutex);
    call->ready.wait_for(lock, options.responseWait, [&] {
      return call->finished || (call->question && !call->question->answer) ||
             (call->modal && !call->modal->busy);
    });
    if (call->finished) {
      if (call->error) {
        try {
          std::rethrow_exception(call->error);
        } catch (const CommandFailure &) { throw; } catch (const Error &error) {
          if (call->command.empty()) { throw; }
          throw CommandFailure(
              {.message = error.what(),
               .code = error.Code(),
               .command = call->command,
               .outcome = error.Code().starts_with("PageHost") ? "refused" : "unknown"});
        }
      }
      auto result = call->result;
      if (!result) { Refuse("PageHostReceipt"); }
      return std::move(*result);
    }
    if (call->modal) { ModalReadAuthority(connection, *call, call->modal->page); }
    return {.body = Working(*call),
            .headers = {{.name = "Content-Location", .value = "/?handle=" + call->pageHandle}}};
  }

  void PublishCall(const std::shared_ptr<Call> &call,
                   std::function<ServerHttpResponse()> execution) {
    {
      const std::lock_guard lock(mutex);
      std::erase_if(calls, [](const auto &entry) { return entry.second.expired(); });
      calls.emplace(call->handle, call);
    }
    try {
      executor->Submit([call, execution = std::move(execution)] {
        std::optional<ServerHttpResponse> result;
        std::exception_ptr error;
        try {
          result = execution();
        } catch (...) { error = std::current_exception(); }
        {
          const std::lock_guard lock(call->mutex);
          call->result = std::move(result);
          call->error = error;
          call->finished = true;
        }
        call->ready.notify_all();
      });
    } catch (...) {
      const std::lock_guard lock(mutex);
      calls.erase(call->handle);
      throw;
    }
  }

  void CallOwnership(Connection &connection,
                     const Call &call,
                     const Client &client,
                     bool answering = false) const {
    const std::array<std::optional<std::string>, 5> binds{
        call.pageHandle, client.user.ToStorageText(), host, options.company, client.verifier};
    const auto rows =
        connection.Execute("SELECT handle FROM agiru_client.page_contexts WHERE handle = $1 "
                           "AND user_security_id = $2::uuid AND host_id = $3 AND company = $4 "
                           "AND credential_digest = $5 "
                           "AND expires_at > clock_timestamp()",
                           binds);
    if (client.user != call.user || rows.Rows() != 1) { Refuse("PageHostGone"); }
    Session authority(client.user, options.session);
    authority.CompanyName(options.company);
    authority.TablePermissions(tableAuthority);
    SessionCommand command(authority, connection);
    const auto *entry = FindPage(call.page);
    if (entry == nullptr) { Refuse("PageHostGone"); }
    authorize(*entry->page, PageHostOperation::Read, {});
    if (answering && !call.command.empty()) {
      authorize(*entry->page,
                PageHostOperation::Control,
                {.operation = call.operation == "set" ? PageControlOperation::Set
                                                      : PageControlOperation::Action,
                 .control = call.control});
    }
    if (!call.command.empty()) {
      const std::array<std::optional<std::string>, 2> receipt{call.pageHandle, call.command};
      const auto completed = connection.Execute(
          "SELECT page_id FROM agiru_client.page_commands WHERE handle = $1 AND command_id = $2 "
          "AND outcome = 'complete'",
          receipt);
      if (completed.Rows() == 1) {
        const auto identity = completed.Value(0, 0);
        if (!identity) { Refuse("PageHostReceipt"); }
        const auto number = Number(*identity);
        if (number > std::numeric_limits<std::int32_t>::max()) { Refuse("PageHostReceipt"); }
        entry = FindPage(PageId{static_cast<std::int32_t>(number)});
        if (entry == nullptr) { Refuse("PageHostReceipt"); }
        authorize(*entry->page, PageHostOperation::Read, {});
        AuthorizeSnapshot(*entry->page, entry->page->layout);
        AuthorizeSnapshot(*entry->page, entry->page->actions);
      }
    }
    command.Keep();
  }

  std::shared_ptr<Call> FindCall(std::string_view handle) {
    if (!Token(handle)) { Refuse("PageHostInput"); }
    std::shared_ptr<Call> call;
    {
      const std::lock_guard lock(mutex);
      const auto found = calls.find(handle);
      if (found == calls.end()) { Refuse("PageHostGone"); }
      call = found->second.lock();
      if (!call) { Refuse("PageHostGone"); }
    }
    return call;
  }

  ServerHttpResponse Poll(Connection &connection, const Client &client, std::string_view handle) {
    const auto call = FindCall(handle);
    CallOwnership(connection, *call, client);
    return Await(connection, call);
  }

  std::shared_ptr<Context> Retain(const Client &client) {
    const std::lock_guard lock(mutex);
    std::erase_if(contexts, [](const auto &item) {
      const std::unique_lock idle(item.second->mutex, std::try_to_lock);
      return idle.owns_lock() && item.second->deadline <= std::chrono::steady_clock::now();
    });
    if (contexts.size() >= options.contexts) { Refuse("PageHostCapacity"); }
    auto context = std::make_shared<Context>(client, options);
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

  void Ownership(const Connection &connection, const Context &context, const Client &client) const {
    const std::array<std::optional<std::string>, 5> binds{
        context.handle, client.user.ToStorageText(), host, options.company, client.verifier};
    std::string sql = "SELECT revision FROM agiru_client.page_contexts WHERE handle = $1 "
                      "AND user_security_id = $2::uuid AND host_id = $3 AND company = $4 "
                      "AND credential_digest = $5 "
                      "AND NOT invalidated AND expires_at > clock_timestamp()";
    if (connection.InTransaction()) { sql += " FOR UPDATE"; }
    const auto rows = connection.Execute(sql, binds);
    if (context.invalid || rows.Rows() != 1) { Refuse("PageHostGone"); }
    const auto revision = rows.Value(0, 0);
    if (!revision || Number(*revision) != context.revision) { Refuse("PageHostRevision"); }
  }

  std::string Render(Context &context) const {
    auto &page = *context.page;
    authorize(page.Declaration(), PageHostOperation::Read, {});
    std::vector<PageHtmlAction> actions;
    if (page.Declaration().source.Value() != 0) {
      actions = {{.identity = "$agiru.first", .caption = "First"},
                 {.identity = "$agiru.previous", .caption = "Previous"},
                 {.identity = "$agiru.next", .caption = "Next"},
                 {.identity = "$agiru.last", .caption = "Last"},
                 {.identity = "$agiru.save", .caption = "Save"}};
      if (context.list.has_value()) {
        actions[1].enabled = context.list->previous;
        actions[2].enabled = context.list->next;
      }
    }
    if (page.Declaration().cardPageId.Value() != 0) {
      actions.push_back({"$agiru.card", "Edit card"});
      const auto *card = FindPage(page.Declaration().cardPageId);
      if (card != nullptr && card->page->source == page.Declaration().source &&
          Insertable(*card->page)) {
        actions.push_back({"$agiru.new", "New"});
      }
    }
    if (!context.parents.empty()) { actions.push_back({"$agiru.back", "Back"}); }
    Authorization authority(page.Declaration(), authorize);
    const auto revision = std::to_string(context.revision);
    const PageHtmlContext envelope{.pageHandle = context.handle,
                                   .revision = revision,
                                   .commandPrefix = context.prefix,
                                   .csrf = context.csrf,
                                   .actions = actions};
    if (context.list.has_value()) {
      AuthorizeSnapshot(page.Declaration(), page.Declaration().layout);
      return RenderPageListHtml(
                 page.Declaration(), page.Controls(), authority, envelope, *context.list)
          .html;
    }
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

  std::string RenderMessages(Context &context, const std::shared_ptr<Call> &call) const {
    auto html = Render(context);
    const std::lock_guard lock(call->mutex);
    return detail::AppendPageMessages(*call, std::move(html));
  }

  ServerHttpResponse RunOpen(Connection &connection,
                             const std::shared_ptr<Context> &context,
                             const Parameters &values,
                             const std::shared_ptr<Call> &call) {
    const auto number = Number(Get(values, "page"));
    const std::lock_guard lock(context->mutex);
    try {
      InstallUiHost(context->session, detail::MakePageUiHost(call, options, authorize));
      SessionCommand command(context->session, connection);
      context->page = MakeInstalledPage(PageId{static_cast<std::int32_t>(number)});
      const auto mode = Mode(context->page->Declaration(), Get(values, "mode"));
      authorize(context->page->Declaration(), Opening(mode), {});
      context->session.OpenCompany();
      if (context->page->Declaration().type == PageType::List) {
        auto &list = context->list.emplace(options.listRows);
        Authorization authority(context->page->Declaration(), authorize);
        PageListLoader loader(context->page->Declaration(), authority, list);
        list.state = context->page->OpenWindow(mode, options.listRows, loader);
        list.next = list.state.more;
      } else {
        context->page->Open(mode);
      }
      auto result = Response(*context, RenderMessages(*context, call));
      command.Keep();
      return result;
    } catch (...) {
      context->invalid = true;
      const Connection cleanup(options.database);
      const std::array<std::optional<std::string>, 1> binds{context->handle};
      cleanup.Run("UPDATE agiru_client.page_contexts SET invalidated = true WHERE handle = $1",
                  binds);
      throw;
    }
  }

  ServerHttpResponse Open(Connection &connection,
                          const Client &client,
                          const Parameters &values,
                          std::string_view browserCsrf) {
    constexpr std::array<std::string_view, 3> names{"page", "mode", "company"};
    Known(values, names);
    const auto company = Get(values, "company");
    if (!company.empty() && company != options.company) { Refuse("PageHostPermission"); }
    const auto number = Number(Get(values, "page"));
    if (number == 0 || number > std::numeric_limits<std::int32_t>::max()) {
      Refuse("PageHostInput");
    }
    const auto page = PageId{static_cast<std::int32_t>(number)};
    const auto *entry = FindPage(page);
    if (entry == nullptr) { Refuse("PageHostMissing"); }
    try {
      Session admission(client.user, options.session);
      admission.CompanyName(options.company);
      admission.TablePermissions(tableAuthority);
      SessionCommand command(admission, connection);
      const auto mode = Mode(*entry->page, Get(values, "mode"));
      authorize(*entry->page, Opening(mode), {});
      if (mode == PageOpenMode::New && !Insertable(*entry->page)) { Refuse("PageHostUnsupported"); }
      detail::RenewPageClient(connection, client, browserCsrf);
      command.Keep();
    } catch (const Error &error) {
      throw CommandFailure(
          {.message = error.what(), .code = error.Code(), .command = {}, .outcome = "refused"});
    }
    auto context = Retain(client);
    auto call = std::make_shared<Call>();
    call->pageHandle = context->handle;
    call->host = host;
    call->csrf = context->csrf;
    call->deadline = context->deadline;
    call->user = client.user;
    call->credential = client.verifier;
    call->page = page;
    call->revision = "0";
    {
      const std::lock_guard lock(context->callMutex);
      context->call = call;
    }
    const std::array<std::optional<std::string>, 6> binds{context->handle,
                                                          client.user.ToStorageText(),
                                                          host,
                                                          options.company,
                                                          std::to_string(options.lifetime.count()),
                                                          client.verifier};
    connection.Run(
        "INSERT INTO "
        "agiru_client.page_contexts(handle,user_security_id,host_id,company,expires_at,credential_"
        "digest) "
        "VALUES ($1,$2::uuid,$3,$4,clock_timestamp() + $5::integer * interval '1 second',$6)",
        binds);
    try {
      PublishCall(call, [this, context, call, values = Parameters(values)] {
        Connection lease(options.database);
        return RunOpen(lease, context, values, call);
      });
    } catch (...) {
      const std::lock_guard lock(context->mutex);
      context->invalid = true;
      const std::array<std::optional<std::string>, 1> handle{context->handle};
      connection.Run("UPDATE agiru_client.page_contexts SET invalidated = true WHERE handle = $1",
                     handle);
      const std::lock_guard retained(mutex);
      contexts.erase(context->handle);
      throw;
    }
    return Await(connection, call);
  }

  ServerHttpResponse Read(Connection &connection, const Client &client, const Parameters &values) {
    constexpr std::array<std::string_view, 1> names{"handle"};
    Known(values, names);
    auto context = Find(Get(values, "handle"));
    {
      const std::lock_guard callLock(context->callMutex);
      if (context->call) {
        const std::lock_guard state(context->call->mutex);
        if (!context->call->finished) {
          CallOwnership(connection, *context->call, client);
          if (context->call->modal) {
            ModalReadAuthority(connection, *context->call, context->call->modal->page);
          }
          return {
              .body = Working(*context->call),
              .headers = {{.name = "Content-Location", .value = "/?handle=" + context->handle}}};
        }
      }
    }
    const std::unique_lock lock(context->mutex, std::try_to_lock);
    if (!lock.owns_lock()) { Refuse("PageHostBusy"); }
    Ownership(connection, *context, client);
    SessionCommand command(context->session, connection);
    std::shared_ptr<Call> call;
    {
      const std::lock_guard callLock(context->callMutex);
      call = context->call;
    }
    auto result = Response(*context, call ? RenderMessages(*context, call) : Render(*context));
    command.Keep();
    return result;
  }

  void ListMovement(Context &context, PageWindowPosition position) const {
    if (!context.list.has_value()) { Refuse("PageHostUnsupported"); }
    if ((position == PageWindowPosition::Next && !context.list->next) ||
        (position == PageWindowPosition::Previous && !context.list->previous)) {
      Refuse("PageHostUnsupported");
    }
    auto &page = *context.page;
    authorize(page.Declaration(), PageHostOperation::Move, {});
    PageListView next{.limit = options.listRows};
    Authorization authority(page.Declaration(), authorize);
    PageListLoader loader(page.Declaration(), authority, next);
    next.state = page.ReadWindow(position, options.listRows, loader);
    next.backwards =
        position == PageWindowPosition::Previous || position == PageWindowPosition::Last;
    next.previous = next.backwards ? next.state.more : position != PageWindowPosition::First;
    next.next = next.backwards ? position != PageWindowPosition::Last : next.state.more;
    if (next.rows.empty() &&
        (position == PageWindowPosition::Next || position == PageWindowPosition::Previous)) {
      if (position == PageWindowPosition::Next) {
        context.list->next = false;
      } else {
        context.list->previous = false;
      }
      return;
    }
    context.list = std::move(next);
  }

  void SelectRow(Context &context, std::string_view control) const {
    if (!context.list.has_value()) { Refuse("PageHostUnsupported"); }
    const auto handle = control.substr(std::string_view("$agiru.row_").size());
    const auto row = std::ranges::find(context.list->rows, handle, &PageListRow::handle);
    if (row == context.list->rows.end()) { Refuse("PageHostMissing"); }
    authorize(context.page->Declaration(), PageHostOperation::Move, {});
    if (!context.page->SelectWindowRecord(row->identity)) { Refuse("PageHostMissing"); }
    Authorization authority(context.page->Declaration(), authorize);
    PageListLoader loader(context.page->Declaration(), authority, *context.list);
    loader.Current(context.page->CurrentRecord(), context.page->Controls());
  }

  void OpenCard(Context &context, PageOpenMode mode) const {
    auto &page = *context.page;
    if (context.parents.size() >= options.navigationDepth) { Refuse("PageHostCapacity"); }
    const auto &source = page.Declaration();
    if (source.cardPageId.Value() == 0) { Refuse("PageHostUnsupported"); }
    authorize(source, PageHostOperation::Read, {});
    auto card = MakeInstalledPage(source.cardPageId);
    if (card->Declaration().source != source.source) { Refuse("PageHostUnsupported"); }
    authorize(card->Declaration(), Opening(mode), {});
    if (mode == PageOpenMode::New && !Insertable(card->Declaration())) {
      Refuse("PageHostUnsupported");
    }
    const RecordId record = mode == PageOpenMode::Edit ? page.CurrentRecord() : RecordId{};
    if (mode == PageOpenMode::Edit && record.IsEmpty()) { Refuse("PageHostMissing"); }
    card->Open(mode);
    if (mode == PageOpenMode::Edit && !card->SelectRecord(record)) { Refuse("PageHostMissing"); }
    context.parents.push_back({.page = std::move(context.page), .list = std::move(context.list)});
    context.page = std::move(card);
    context.list.reset();
  }

  void Back(Context &context) const {
    auto &page = *context.page;
    if (context.parents.empty()) { Refuse("PageHostUnsupported"); }
    authorize(page.Declaration(), PageHostOperation::Close, {});
    authorize(context.parents.back().page->Declaration(), PageHostOperation::Read, {});
    page.Close();
    context.page = std::move(context.parents.back().page);
    context.list = std::move(context.parents.back().list);
    context.parents.pop_back();
    const RecordId selected = context.page->CurrentRecord();
    if (!selected.IsEmpty() &&
        !(context.list.has_value() ? context.page->SelectWindowRecord(selected)
                                   : context.page->SelectRecord(selected))) {
      Refuse("PageHostMissing");
    }
    if (context.list.has_value() && !selected.IsEmpty()) {
      Authorization authority(context.page->Declaration(), authorize);
      PageListLoader loader(context.page->Declaration(), authority, *context.list);
      loader.Current(selected, context.page->Controls());
    }
  }

  void Lifecycle(Context &context, std::string_view control) const {
    auto &page = *context.page;
    if (control.starts_with("$agiru.row_")) {
      SelectRow(context, control);
      return;
    }
    if (control == "$agiru.card") {
      OpenCard(context, PageOpenMode::Edit);
      return;
    }
    if (control == "$agiru.new") {
      OpenCard(context, PageOpenMode::New);
      return;
    }
    if (control == "$agiru.back") {
      Back(context);
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
    if (context.list.has_value()) {
      PageWindowPosition window = PageWindowPosition::Unknown;
      if (position == PagePosition::First) { window = PageWindowPosition::First; }
      if (position == PagePosition::Previous) { window = PageWindowPosition::Previous; }
      if (position == PagePosition::Next) { window = PageWindowPosition::Next; }
      if (position == PagePosition::Last) { window = PageWindowPosition::Last; }
      ListMovement(context, window);
      return;
    }
    authorize(page.Declaration(), PageHostOperation::Move, {});
    static_cast<void>(page.Move(position));
  }

  void Execute(Context &context, const Parameters &values) const {
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

  void AuthorizeSnapshot(const PageDef &page, std::span<const ControlDef> controls) const {
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
                                           const std::string &digest) const {
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
    Ownership(connection, context, context.client);
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
    if (cleanup
            .Execute("UPDATE agiru_client.page_contexts SET invalidated = true WHERE handle = $1",
                     handle)
            .Affected() != 1) {
      Refuse("PageHostCleanup");
    }
    const std::array<std::optional<std::string>, 2> binds{context.handle, std::string(commandId)};
    if (cleanup
            .Execute("UPDATE agiru_client.page_commands SET outcome = 'failed',html = NULL "
                     "WHERE handle = $1 AND command_id = $2 AND outcome = 'started'",
                     binds)
            .Affected() != 1) {
      Refuse("PageHostCleanup");
    }
  }

  ServerHttpResponse Write(Connection &connection,
                           const Client &client,
                           const ServerHttpRequest &request,
                           const std::shared_ptr<Call> &call) {
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
    const std::unique_lock lock(context->mutex);
    Ownership(connection, *context, client);
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
      InstallUiHost(context->session, detail::MakePageUiHost(call, options, authorize));
      SessionCommand command(context->session, connection);
      Ownership(connection, *context, client);
      Execute(*context, values);
      if (context->revision == std::numeric_limits<std::int64_t>::max()) {
        Refuse("PageHostRevision");
      }
      ++context->revision;
      context->prefix = GenerateSecureToken();
      auto result = Response(*context, RenderMessages(*context, call));
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
      detail::RenewPageClient(connection, client, request.Header("X-Agiru-CSRF"));
      command.Keep();
      return result;
    } catch (const std::exception &execution) {
      try {
        Invalidate(*context, commandId);
      } catch (...) {
        throw CommandFailure({.message = "Page cleanup failed; command outcome is unknown",
                              .code = "PageHostCleanup",
                              .command = commandId,
                              .outcome = "unknown"});
      }
      const auto *al = dynamic_cast<const Error *>(&execution);
      throw CommandFailure({.message = al == nullptr ? "Server execution failed" : al->what(),
                            .code = al == nullptr ? "ServerFailure" : al->Code(),
                            .command = commandId,
                            .outcome = "failed"});
    } catch (...) {
      try {
        Invalidate(*context, commandId);
      } catch (...) {
        throw CommandFailure({.message = "Page cleanup failed; command outcome is unknown",
                              .code = "PageHostCleanup",
                              .command = commandId,
                              .outcome = "unknown"});
      }
      throw CommandFailure({.message = "Server execution failed",
                            .code = "ServerFailure",
                            .command = commandId,
                            .outcome = "failed"});
    }
  }

  ServerHttpResponse
  SubmitWrite(Connection &connection, const Client &client, const ServerHttpRequest &request) {
    if (request.Header("Content-Type") != "application/x-www-form-urlencoded" ||
        request.Header("Origin") != options.origin) {
      Refuse("PageHostInput");
    }
    const auto values = Parse(request.body, true);
    auto context = Find(Get(values, "page"));
    std::shared_ptr<Call> call;
    {
      const std::lock_guard admission(context->callMutex);
      if (context->call) {
        const std::lock_guard state(context->call->mutex);
        if (!context->call->finished) { Refuse("PageHostBusy"); }
      }
      const std::unique_lock idle(context->mutex, std::try_to_lock);
      if (!idle.owns_lock()) { Refuse("PageHostBusy"); }
      Ownership(connection, *context, client);
      call = std::make_shared<Call>();
      call->pageHandle = context->handle;
      call->host = host;
      call->csrf = context->csrf;
      call->deadline = context->deadline;
      call->user = client.user;
      call->credential = client.verifier;
      call->page = context->page->Declaration().id;
      call->revision = std::to_string(context->revision);
      call->command = Get(values, "command");
      call->operation = Get(values, "operation");
      call->control = Get(values, "control");
      if (!Token(call->command)) { Refuse("PageHostInput"); }
      const auto previous = std::exchange(context->call, call);
      try {
        PublishCall(call,
                    [this, client = Client(client), call, request = ServerHttpRequest(request)] {
                      Connection lease(options.database);
                      return Write(lease, client, request, call);
                    });
      } catch (...) {
        context->call = previous;
        throw;
      }
    }
    return Await(connection, call);
  }

  ServerHttpResponse
  Answer(Connection &connection, const Client &client, const ServerHttpRequest &request) {
    if (request.Header("Content-Type") != "application/x-www-form-urlencoded" ||
        request.Header("Origin") != options.origin) {
      Refuse("PageHostInput");
    }
    const auto values = Parse(request.body, true);
    constexpr std::array<std::string_view, 6> names{
        "page", "revision", "command", "csrf", "operation", "control"};
    Known(values, names);
    if (values.size() != names.size() || Get(values, "operation") != "action" ||
        !Token(Get(values, "command"))) {
      Refuse("PageHostInput");
    }
    auto context = Find(Get(values, "page"));
    std::shared_ptr<Call> call;
    {
      const std::lock_guard lock(context->callMutex);
      call = context->call;
    }
    if (!call) { Refuse("PageHostGone"); }
    CallOwnership(connection, *call, client, true);
    if (Get(values, "revision") != call->revision) { Refuse("PageHostRevision"); }
    if (Get(values, "csrf") != call->csrf) { Refuse("PageHostPermission"); }
    {
      const std::lock_guard lock(call->mutex);
      if (call->modal) { ModalReadAuthority(connection, *call, call->modal->page); }
      if (detail::AcceptPageAnswer(
              connection, *call, Get(values, "command"), Get(values, "control"))) {
        detail::RenewPageClient(connection, client, request.Header("X-Agiru-CSRF"));
      }
    }
    return Await(connection, call);
  }

  void ModalInputAuthority(Connection &connection,
                           const Call &call,
                           const detail::PageModal &modal,
                           const detail::PageModalInput &input) const {
    Session authority(call.user, options.session);
    authority.CompanyName(options.company);
    authority.TablePermissions(tableAuthority);
    SessionCommand command(authority, connection);
    const auto *entry = FindPage(modal.page);
    if (entry == nullptr) { Refuse("PageHostGone"); }
    auto operation = PageHostOperation::Control;
    if (input.operation == "action" && input.control.starts_with("$agiru.")) {
      operation = input.control == "$agiru.modal_ok" || input.control == "$agiru.modal_cancel"
                      ? PageHostOperation::Close
                  : input.control == "$agiru.save" ? PageHostOperation::Save
                                                   : PageHostOperation::Move;
    }
    authorize(*entry->page, PageHostOperation::Read, {});
    authorize(*entry->page,
              operation,
              {.operation = input.operation == "set" ? PageControlOperation::Set
                                                     : PageControlOperation::Action,
               .control = input.control,
               .text = input.text});
    command.Keep();
  }

  ServerHttpResponse ModalResult(Connection &connection,
                                 const std::shared_ptr<Call> &call,
                                 std::shared_ptr<detail::PageModalInput> input) const {
    if (!input) { return Await(connection, call); }
    {
      std::unique_lock lock(call->mutex);
      call->ready.wait_for(
          lock, options.responseWait, [&] { return input->finished || call->finished; });
      if (call->finished && call->error) {
        lock.unlock();
        return Await(connection, call);
      }
      if (!input->finished) {
        input = detail::ReadPageModalInput(connection, *call, input->modal, input->command);
      }
      if (input->finished) {
        if (input->error) {
          try {
            std::rethrow_exception(input->error);
          } catch (const Error &error) {
            throw CommandFailure({.message = error.what(),
                                  .code = error.Code(),
                                  .command = input->command,
                                  .outcome = "failed"});
          }
        }
        const auto response = input->response;
        if (response) {
          ModalReadAuthority(connection, *call, input->page);
          return *response;
        }
      }
    }
    return Await(connection, call);
  }

  ServerHttpResponse
  ModalPoll(Connection &connection, const Client &client, std::string_view target) {
    const auto split = target.find('/');
    if (split == std::string_view::npos || !Token(target.substr(0, split)) ||
        !Token(target.substr(split + 1))) {
      Refuse("PageHostInput");
    }
    const auto modal = target.substr(0, split);
    const auto command = target.substr(split + 1);
    const std::array<std::optional<std::string>, 3> binds{
        std::string(modal), client.user.ToStorageText(), host};
    const auto rows = connection.Execute(
        "SELECT call_handle FROM agiru_client.page_modals WHERE handle=$1 "
        "AND user_security_id=$2::uuid AND host_id=$3 AND expires_at>clock_timestamp()",
        binds);
    const auto identity = rows.Rows() == 1 ? rows.Value(0, 0) : std::nullopt;
    if (!identity) { Refuse("PageHostGone"); }
    const auto call = FindCall(*identity);
    CallOwnership(connection, *call, client);
    const auto input = detail::ReadPageModalInput(connection, *call, modal, command);
    ModalReadAuthority(connection, *call, input->page);
    return ModalResult(connection, call, input);
  }

  ServerHttpResponse
  ModalCommand(Connection &connection, const Client &client, const ServerHttpRequest &request) {
    if (request.Header("Content-Type") != "application/x-www-form-urlencoded" ||
        request.Header("Origin") != options.origin) {
      Refuse("PageHostInput");
    }
    const auto handle =
        std::string_view(request.target).substr(std::string_view("/modal-commands/").size());
    if (!Token(handle)) { Refuse("PageHostInput"); }
    const auto values = Parse(request.body, true);
    constexpr std::array<std::string_view, 7> names{
        "page", "revision", "command", "csrf", "operation", "control", "text"};
    Known(values, names);
    const auto operation = Get(values, "operation");
    if (!Token(Get(values, "command")) || Get(values, "control").empty() ||
        (operation != "set" && operation != "action") ||
        (operation == "set") != values.contains("text") ||
        values.size() != names.size() - (operation == "set" ? 0 : 1)) {
      Refuse("PageHostInput");
    }
    auto context = Find(Get(values, "page"));
    std::shared_ptr<Call> call;
    {
      const std::lock_guard lock(context->callMutex);
      call = context->call;
    }
    if (!call) { Refuse("PageHostGone"); }
    CallOwnership(connection, *call, client, true);
    if (Get(values, "csrf") != call->csrf) { Refuse("PageHostPermission"); }
    std::shared_ptr<detail::PageModalInput> input;
    {
      const std::lock_guard lock(call->mutex);
      detail::PageModalInput candidate{.command = std::string(Get(values, "command")),
                                       .operation = std::string(operation),
                                       .control = std::string(Get(values, "control")),
                                       .text = std::string(Get(values, "text")),
                                       .digest = SecureTokenDigest(request.body)};
      const std::array<std::optional<std::string>, 4> identity{
          std::string(handle), call->handle, client.user.ToStorageText(), host};
      const auto rows = connection.Execute(
          "SELECT page_id FROM agiru_client.page_modals WHERE handle=$1 AND call_handle=$2 "
          "AND user_security_id=$3::uuid AND host_id=$4 AND expires_at>clock_timestamp()",
          identity);
      const auto page = rows.Rows() == 1 ? rows.Value(0, 0) : std::nullopt;
      if (!page || Number(*page) > std::numeric_limits<std::int32_t>::max()) {
        Refuse("PageHostModalStale");
      }
      const detail::PageModal modal{.page = PageId{static_cast<std::int32_t>(Number(*page))}};
      try {
        ModalInputAuthority(connection, *call, modal, candidate);
      } catch (const Error &error) {
        if (error.Code() != "Permission" && error.Code() != "PageHostPermission") { throw; }
        throw CommandFailure({.message = error.what(),
                              .code = error.Code(),
                              .command = candidate.command,
                              .outcome = "refused"});
      }
      bool fresh = false;
      input = detail::AcceptPageModal(
          connection, *call, handle, Get(values, "revision"), std::move(candidate), options, fresh);
      if (fresh) { detail::RenewPageClient(connection, client, request.Header("X-Agiru-CSRF")); }
    }
    return ModalResult(connection, call, input);
  }

  ServerHttpResponse Handle(const ServerHttpRequest &request) {
    Connection connection(options.database);
    if (auto endpoint = detail::BrowserSessionEndpoint(connection, request, options)) {
      return std::move(*endpoint);
    }
    const auto client = detail::AuthenticatePageClient(connection, request, options);
    const auto retained = detail::RetainedPageRequest(request, client.csrf);
    return Dispatch(connection, client.identity, retained);
  }

  ServerHttpResponse
  Dispatch(Connection &connection, const Client &client, const ServerHttpRequest &request) {
    if (request.method == "GET" && request.target.starts_with("/modal-commands/")) {
      return ModalPoll(
          connection,
          client,
          std::string_view(request.target).substr(std::string_view("/modal-commands/").size()));
    }
    if (request.method == "GET" && request.target.starts_with(kCallPrefix)) {
      return Poll(connection, client, std::string_view(request.target).substr(kCallPrefix.size()));
    }
    if (request.method == "GET" && request.target.starts_with("/?")) {
      const auto values = Parse(std::string_view(request.target).substr(2), false);
      return values.contains("handle")
                 ? Read(connection, client, values)
                 : Open(connection, client, values, request.Header("X-Agiru-CSRF"));
    }
    if (request.method == "POST" && request.target == "/commands") {
      return SubmitWrite(connection, client, request);
    }
    if (request.method == "POST" && request.target == "/answers") {
      return Answer(connection, client, request);
    }
    if (request.method == "POST" && request.target.starts_with("/modal-commands/")) {
      return ModalCommand(connection, client, request);
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
  std::map<std::string, std::weak_ptr<Call>, std::less<>> calls;
  std::unique_ptr<Executor> executor;
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
  } catch (const CommandFailure &error) {
    const auto status =
        (error.Code().starts_with("PageHost") && error.Code() != "PageHostCleanup") ||
                error.Code() == "Permission" || error.Code() == "PageValidation" ||
                error.Code() == "PageCommand"
            ? Status(error.Code())
            : kInternalError;
    return FailureResponse(error, status, {.command = error.Command(), .outcome = error.Outcome()});
  } catch (const SessionError &) {
    return {.status = kUnauthorized,
            .body = DiagnosticHtml({.message = "Authentication required",
                                    .code = "SessionIdentity",
                                    .command = {},
                                    .outcome = "unknown"}),
            .headers = {{.name = "WWW-Authenticate", .value = "Bearer realm=\"agiru\""}}};
  } catch (const Error &error) {
    const bool refusal = error.Code().starts_with("PageHost") && error.Code() != "PageHostCleanup";
    const auto status = refusal || error.Code() == "Permission" ||
                                error.Code() == "SessionIdentity" ||
                                error.Code() == "PageValidation" || error.Code() == "PageCommand"
                            ? Status(error.Code())
                            : kInternalError;
    const auto command = RequestCommand(request);
    auto response = FailureResponse(
        error, status, {.command = command, .outcome = refusal ? "refused" : "unknown"});
    if (response.status == kUnauthorized) {
      response.headers.push_back({.name = "WWW-Authenticate", .value = "Bearer realm=\"agiru\""});
    }
    return response;
  }
}

}

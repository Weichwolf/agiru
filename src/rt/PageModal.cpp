#include "PageModal.h"

#include "meta/Ids.h"
#include "meta/PageDef.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/HttpServer.h"
#include "runtime/PageCommandHost.h"
#include "runtime/PageDispatcher.h"
#include "runtime/PageHostOptions.h"
#include "runtime/PageHtml.h"
#include "runtime/PageInstance.h"
#include "runtime/PageWindow.h"
#include "runtime/SecureToken.h"
#include "runtime/Transaction.h"
#include "type/Action.h"
#include "type/Guid.h"

#include "PageInteraction.h"
#include "PageListHtml.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace agiru::detail {
namespace {

[[noreturn]] void Refuse(std::string_view code) {
  throw Error("Modal page operation refused", code);
}

std::uint64_t Unsigned(std::string_view text) {
  std::uint64_t value = 0;
  const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
  if (text.empty() || parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) {
    Refuse("PageHostReceipt");
  }
  return value;
}

class ModalAuthorization final : public PageAuthorization {
public:
  ModalAuthorization(const PageDef &page, const PageHostAuthorization &authority)
      : page_(page), authority_(authority) {}

  void Require(PageId page, const PageControlCommand &command) override {
    if (page != page_.id) { Refuse("PageHostPermission"); }
    authority_(page_, PageHostOperation::Control, command);
  }

private:
  const PageDef &page_;
  const PageHostAuthorization &authority_;
};

void PublishModal(const PageHostOptions &options, const PageCall &call, const PageModal &modal) {
  const Connection connection(options.database);
  const std::array<std::optional<std::string>, 8> binds{modal.handle,
                                                        call.pageHandle,
                                                        call.handle,
                                                        call.user.ToStorageText(),
                                                        call.host,
                                                        std::to_string(modal.page.Value()),
                                                        modal.prefix,
                                                        options.company};
  if (connection
          .Execute(
              "INSERT INTO agiru_client.page_modals "
              "(handle,page_handle,call_handle,user_security_id,host_id,page_id,prefix,expires_at) "
              "SELECT $1,$2,$3,$4::uuid,$5,$6::integer,$7,expires_at "
              "FROM agiru_client.page_contexts WHERE handle=$2 AND user_security_id=$4::uuid "
              "AND host_id=$5 AND company=$8 AND NOT invalidated AND expires_at>clock_timestamp()",
              binds)
          .Affected() != 1) {
    Refuse("PageHostGone");
  }
}

void CloseModalReceipt(const PageHostOptions &options, const PageModal &modal) {
  const Connection connection(options.database);
  const std::array<std::optional<std::string>, 1> binds{modal.handle};
  if (connection
          .Execute(
              "WITH closed AS (UPDATE agiru_client.page_modals SET active=false WHERE handle=$1 "
              "RETURNING handle), abandoned AS (UPDATE agiru_client.page_modal_commands "
              "SET outcome='failed',html='',error_code='UiDialogCancelled',"
              "error_text='Modal call unwound; reconcile the root command' "
              "WHERE modal_handle IN (SELECT handle FROM closed) AND outcome='queued' "
              "RETURNING command_id) SELECT handle FROM closed",
              binds)
          .Rows() != 1) {
    Refuse("PageHostCleanup");
  }
}

void CompleteInput(const PageHostOptions &options,
                   const PageModal &modal,
                   const PageModalInput &input,
                   std::string_view prefix,
                   std::string_view html,
                   const Error *failure,
                   bool closed) {
  const Connection connection(options.database);
  const std::array<std::optional<std::string>, 11> binds{
      modal.handle,
      input.command,
      std::to_string(modal.revision),
      std::string(prefix),
      std::string(html),
      failure != nullptr ? "failed" : "processed",
      closed ? "false" : "true",
      input.digest,
      std::to_string(options.receiptBytes),
      failure != nullptr ? std::optional<std::string>(failure->Code()) : std::nullopt,
      failure != nullptr ? std::optional<std::string>(failure->what()) : std::nullopt};
  if (connection
          .Execute(
              "WITH fence AS (UPDATE agiru_client.page_modals SET revision=revision+1,prefix=$4,"
              "active=$7::boolean WHERE handle=$1 AND revision=$3::bigint AND active "
              "AND COALESCE((SELECT sum(octet_length(c.html)) FROM "
              "agiru_client.page_modal_commands c "
              "JOIN agiru_client.page_modals m ON m.handle=c.modal_handle WHERE m.call_handle="
              "agiru_client.page_modals.call_handle),0)+octet_length($5)<=$9::bigint "
              "RETURNING handle) UPDATE agiru_client.page_modal_commands SET outcome=$6,html=$5,"
              "error_code=$10,error_text=$11 "
              "WHERE modal_handle IN (SELECT handle FROM fence) AND command_id=$2 "
              "AND payload_digest=$8 AND outcome='queued'",
              binds)
          .Affected() != 1) {
    Refuse("PageHostCleanup");
  }
}

std::shared_ptr<PageModalInput>
ReplayInput(const Result &replay, PageModalInput input, std::string_view pageHandle) {
  if (replay.Rows() != 1 || replay.Value(0, 0) != input.digest) { Refuse("PageHostReplay"); }
  auto result = std::make_shared<PageModalInput>(std::move(input));
  const auto page = replay.Value(0, 3);
  if (!page || Unsigned(*page) > std::numeric_limits<std::int32_t>::max()) {
    Refuse("PageHostReceipt");
  }
  result->page = PageId{static_cast<std::int32_t>(Unsigned(*page))};
  if (replay.Value(0, 1) == "queued") { return result; }
  result->finished = true;
  if (replay.Value(0, 1) == "failed") {
    const auto code = replay.Value(0, 4);
    const auto text = replay.Value(0, 5);
    if (!code || !text) { Refuse("PageHostReceipt"); }
    result->error = std::make_exception_ptr(Error(std::string(*text), std::string(*code)));
  }
  const auto html = replay.Value(0, 2);
  if (!html) { Refuse("PageHostReceipt"); }
  if (!html->empty()) {
    result->response = ServerHttpResponse{
        .body = std::string(*html),
        .headers = {{.name = "Content-Location", .value = "/?handle=" + std::string(pageHandle)}}};
  }
  return result;
}

class ModalExecution {
public:
  ModalExecution(std::shared_ptr<PageCall> call,
                 const PageHostOptions &options,
                 const PageHostAuthorization &authorization,
                 PageInstance &page)
      : call_(std::move(call)),
        options_(options),
        authorization_(authorization),
        page_(page),
        modal_(std::make_shared<PageModal>()),
        controls_(page.Declaration(), authorization) {
    modal_->page = page.Declaration().id;
  }

  Action Run() {
    const auto deadline =
        std::min(call_->deadline, std::chrono::steady_clock::now() + options_.dialogTimeout);
    {
      const std::lock_guard lock(call_->mutex);
      if (call_->cancelled || call_->finished || call_->question) { Refuse("UiHostUnavailable"); }
      std::size_t depth = 0;
      for (auto parent = call_->modal; parent; parent = parent->parent) { ++depth; }
      if (depth >= options_.navigationDepth || call_->questions >= options_.commands) {
        Refuse("UiTransportLimit");
      }
      ++call_->questions;
      modal_->parent = call_->modal;
      call_->modal = modal_;
    }
    try {
      PublishModal(options_, *call_, *modal_);
      published_ = true;
      Open();
      PublishSnapshot();
      for (;;) {
        const auto input = Wait(deadline);
        if (const auto closed = ExecuteInput(input)) {
          CloseModalReceipt(options_, *modal_);
          Restore();
          return *closed;
        }
      }
    } catch (...) {
      Restore();
      if (published_) { CloseModalReceipt(options_, *modal_); }
      throw;
    }
  }

private:
  void Restore() {
    const std::lock_guard lock(call_->mutex);
    call_->modal = modal_->parent;
    call_->ready.notify_all();
  }

  void Open() {
    const auto &declaration = page_.Declaration();
    const bool readOnly = declaration.editable == "false" || declaration.editable == "False";
    const auto mode = readOnly ? PageOpenMode::View : PageOpenMode::Edit;
    authorization_(
        declaration, readOnly ? PageHostOperation::OpenView : PageHostOperation::OpenEdit, {});
    if (declaration.type == PageType::List) {
      auto &list = list_.emplace(options_.listRows);
      PageListLoader loader(declaration, controls_, list);
      list.state = page_.OpenWindow(mode, options_.listRows, loader);
      list.next = list.state.more;
    } else {
      page_.Open(mode);
    }
  }

  std::string Render(std::int64_t revision, std::string_view prefix) {
    authorization_(page_.Declaration(), PageHostOperation::Read, {});
    std::vector<PageHtmlAction> actions{{.identity = "$agiru.modal_ok", .caption = "OK"},
                                        {.identity = "$agiru.modal_cancel", .caption = "Cancel"}};
    if (page_.Declaration().source.Value() != 0) {
      actions.insert(
          actions.end(),
          {{.identity = "$agiru.first", .caption = "First"},
           {.identity = "$agiru.previous",
            .caption = "Previous",
            .enabled = !list_ || list_->previous},
           {.identity = "$agiru.next", .caption = "Next", .enabled = !list_ || list_->next},
           {.identity = "$agiru.last", .caption = "Last"},
           {.identity = "$agiru.save", .caption = "Save"}});
    }
    const PageHtmlModal modal{
        .call = call_->handle, .originCommand = call_->command, .handle = modal_->handle};
    const std::string path = "/modal-commands/" + modal_->handle;
    const auto revisionText = std::to_string(revision);
    const PageHtmlContext envelope{.pageHandle = call_->pageHandle,
                                   .revision = revisionText,
                                   .commandPrefix = prefix,
                                   .csrf = call_->csrf,
                                   .commandPath = path,
                                   .actions = actions,
                                   .modal = &modal};
    return list_ ? RenderPageListHtml(
                       page_.Declaration(), page_.Controls(), controls_, envelope, *list_)
                       .html
                 : RenderPageHtml(page_.Declaration(), page_.Controls(), controls_, envelope).html;
  }

  void PublishSnapshot() {
    auto html = Render(modal_->revision, modal_->prefix);
    const std::lock_guard lock(call_->mutex);
    static_cast<void>(AppendPageMessages(*call_, html));
    modal_->html = std::move(html);
    modal_->busy = false;
    call_->ready.notify_all();
  }

  std::shared_ptr<PageModalInput> Wait(std::chrono::steady_clock::time_point deadline) {
    std::unique_lock lock(call_->mutex);
    if (!call_->ready.wait_until(
            lock, deadline, [&] { return call_->cancelled || modal_->input; }) ||
        call_->cancelled) {
      Refuse("UiDialogCancelled");
    }
    modal_->executing = std::exchange(modal_->input, nullptr);
    return modal_->executing;
  }

  void Move(PageWindowPosition position, PagePosition single) {
    authorization_(page_.Declaration(), PageHostOperation::Move, {});
    if (!list_) {
      static_cast<void>(page_.Move(single));
      return;
    }
    if ((position == PageWindowPosition::Next && !list_->next) ||
        (position == PageWindowPosition::Previous && !list_->previous)) {
      Refuse("PageHostMissing");
    }
    PageListView next{.limit = options_.listRows};
    PageListLoader loader(page_.Declaration(), controls_, next);
    next.state = page_.ReadWindow(position, options_.listRows, loader);
    next.backwards =
        position == PageWindowPosition::Previous || position == PageWindowPosition::Last;
    next.previous = next.backwards ? next.state.more : position != PageWindowPosition::First;
    next.next = next.backwards ? position != PageWindowPosition::Last : next.state.more;
    if (next.rows.empty() &&
        (position == PageWindowPosition::Next || position == PageWindowPosition::Previous)) {
      if (position == PageWindowPosition::Next) {
        list_->next = false;
      } else {
        list_->previous = false;
      }
      return;
    }
    list_ = std::move(next);
  }

  std::optional<Action> Execute(const PageModalInput &input) {
    const auto &control = input.control;
    if (input.operation == "set" || !control.starts_with("$agiru.")) {
      PageDispatcher dispatcher(page_.Declaration(), page_.Controls(), controls_);
      static_cast<void>(
          dispatcher.Execute({.operation = input.operation == "set" ? PageControlOperation::Set
                                                                    : PageControlOperation::Action,
                              .control = control,
                              .text = input.text}));
      if (!page_.IsOpen()) { Refuse("UiModalClosedUnsupported"); }
      return {};
    }
    if (Control(page_.Declaration().layout, control) != nullptr ||
        Control(page_.Declaration().actions, control) != nullptr) {
      Refuse("PageHostUnsupported");
    }
    if (control == "$agiru.modal_ok" || control == "$agiru.modal_cancel") {
      authorization_(page_.Declaration(), PageHostOperation::Close, {});
      return page_.CloseModal(control == "$agiru.modal_ok" ? Action::OK : Action::Cancel);
    }
    if (control.starts_with("$agiru.row_")) {
      if (!list_) { Refuse("PageHostUnsupported"); }
      const auto row = std::ranges::find(
          list_->rows, std::string_view(control).substr(11), &PageListRow::handle);
      if (row == list_->rows.end()) { Refuse("PageHostMissing"); }
      authorization_(page_.Declaration(), PageHostOperation::Move, {});
      if (!page_.SelectWindowRecord(row->identity)) { Refuse("PageHostMissing"); }
      PageListLoader loader(page_.Declaration(), controls_, *list_);
      loader.Current(page_.CurrentRecord(), page_.Controls());
      return {};
    }
    if (control == "$agiru.save") {
      authorization_(page_.Declaration(), PageHostOperation::Save, {});
      page_.Save();
      return {};
    }
    if (control == "$agiru.first") {
      Move(PageWindowPosition::First, PagePosition::First);
    } else if (control == "$agiru.previous") {
      Move(PageWindowPosition::Previous, PagePosition::Previous);
    } else if (control == "$agiru.next") {
      Move(PageWindowPosition::Next, PagePosition::Next);
    } else if (control == "$agiru.last") {
      Move(PageWindowPosition::Last, PagePosition::Last);
    } else {
      Refuse("PageHostUnsupported");
    }
    return {};
  }

  std::optional<Action> ExecuteInput(const std::shared_ptr<PageModalInput> &input) {
    if (modal_->revision == std::numeric_limits<std::int64_t>::max()) {
      Refuse("PageHostRevision");
    }
    std::optional<Action> result;
    std::exception_ptr error;
    std::optional<Error> failure;
    {
      Scope boundary;
      try {
        authorization_(page_.Declaration(), PageHostOperation::Read, {});
        result = Execute(*input);
        boundary.Keep();
      } catch (const Error &executionError) {
        const bool validation =
            input->operation == "set" && executionError.Code() == "TestValidation";
        if ((!validation && input->control != "$agiru.modal_ok" &&
             input->control != "$agiru.modal_cancel") ||
            !page_.IsOpen()) {
          throw;
        }
        boundary.Discard(executionError);
        error = std::current_exception();
        failure.emplace(executionError.what(), executionError.Code());
      }
    }
    const auto prefix = GenerateSecureToken();
    auto html = result ? std::string{} : Render(modal_->revision + 1, prefix);
    const std::lock_guard lock(call_->mutex);
    auto receiptHtml = result ? std::string{} : AppendPageMessages(*call_, html);
    CompleteInput(options_,
                  *modal_,
                  *input,
                  prefix,
                  receiptHtml,
                  failure ? &*failure : nullptr,
                  result.has_value());
    ++modal_->revision;
    modal_->prefix = prefix;
    modal_->html = std::move(html);
    modal_->busy = result.has_value();
    input->error = error;
    if (!result && !error) {
      input->response = ServerHttpResponse{
          .body = std::move(receiptHtml),
          .headers = {{.name = "Content-Location", .value = "/?handle=" + call_->pageHandle}}};
    }
    input->finished = true;
    call_->ready.notify_all();
    return result;
  }

  std::shared_ptr<PageCall> call_;
  const PageHostOptions &options_;
  const PageHostAuthorization &authorization_;
  PageInstance &page_;
  std::shared_ptr<PageModal> modal_;
  ModalAuthorization controls_;
  std::optional<PageListView> list_;
  bool published_ = false;
};

}

void InstallPageModals(const Connection &connection) {
  connection.Run(R"(CREATE TABLE IF NOT EXISTS agiru_client.page_modals (
    handle text PRIMARY KEY,page_handle text NOT NULL,call_handle text NOT NULL,
    user_security_id uuid NOT NULL,host_id text NOT NULL,page_id integer NOT NULL,
    revision bigint NOT NULL DEFAULT 0 CHECK(revision>=0),prefix text NOT NULL,
    active boolean NOT NULL DEFAULT true,expires_at timestamptz NOT NULL
  ))");
  connection.Run(R"(CREATE TABLE IF NOT EXISTS agiru_client.page_modal_commands (
    modal_handle text NOT NULL,command_id text NOT NULL,payload_digest text NOT NULL,
    outcome text NOT NULL CHECK(outcome IN ('queued','processed','failed')),html text,
    error_code text,error_text text,
    PRIMARY KEY(modal_handle,command_id),CHECK((outcome='queued')=(html IS NULL)),
    CHECK((outcome='failed')=(error_text IS NOT NULL AND error_code IS NOT NULL))
  ))");
}

Action RunPageModal(const std::shared_ptr<PageCall> &call,
                    const PageHostOptions &options,
                    const PageHostAuthorization &authorization,
                    PageInstance &page) {
  return ModalExecution(call, options, authorization, page).Run();
}

std::shared_ptr<PageModalInput> AcceptPageModal(const Connection &connection,
                                                PageCall &call,
                                                std::string_view handle,
                                                std::string_view revision,
                                                PageModalInput input,
                                                const PageHostOptions &options) {
  input.modal = handle;
  const std::array<std::optional<std::string>, 7> binds{std::string(handle),
                                                        input.command,
                                                        input.digest,
                                                        call.handle,
                                                        call.user.ToStorageText(),
                                                        call.host,
                                                        std::string(revision)};
  const std::array<std::optional<std::string>, 5> replayBinds{
      binds[0], binds[1], binds[3], binds[4], binds[5]};
  const auto replay = connection.Execute(
      "SELECT c.payload_digest,c.outcome,c.html,m.page_id,c.error_code,c.error_text "
      "FROM agiru_client.page_modal_commands c "
      "JOIN agiru_client.page_modals m ON m.handle=c.modal_handle WHERE m.handle=$1 "
      "AND c.command_id=$2 AND m.call_handle=$3 AND m.user_security_id=$4::uuid "
      "AND m.host_id=$5 AND m.expires_at>clock_timestamp()",
      replayBinds);
  if (replay.Rows() != 0) { return ReplayInput(replay, std::move(input), call.pageHandle); }
  const auto modal = call.modal;
  if (call.finished || call.cancelled || call.question || !modal || modal->handle != handle ||
      modal->busy) {
    Refuse("PageHostModalStale");
  }
  if (revision != std::to_string(modal->revision) ||
      !input.command.starts_with(modal->prefix + "_")) {
    Refuse("PageHostRevision");
  }
  const auto suffix = std::string_view(input.command).substr(modal->prefix.size() + 1);
  std::size_t ordinal = 0;
  const auto parsed = std::from_chars(suffix.data(), suffix.data() + suffix.size(), ordinal);
  if (suffix.empty() || parsed.ec != std::errc{} || parsed.ptr != suffix.data() + suffix.size()) {
    Refuse("PageHostInput");
  }
  const auto budget = connection.Execute(
      "SELECT count(*),COALESCE(sum(octet_length(c.html)),0) FROM agiru_client.page_modal_commands "
      "c "
      "JOIN agiru_client.page_modals m ON m.handle=c.modal_handle WHERE m.call_handle=$1",
      std::span(binds).subspan(3, 1));
  const auto count = budget.Value(0, 0);
  const auto bytes = budget.Value(0, 1);
  if (!count || !bytes || call.questions >= options.commands ||
      Unsigned(*count) >= options.commands || Unsigned(*bytes) >= options.receiptBytes) {
    Refuse("PageHostCapacity");
  }
  const std::array<std::optional<std::string>, 8> admitted{
      binds[0], binds[1], binds[2], binds[3], binds[4], binds[5], binds[6], modal->prefix};
  if (connection
          .Execute(
              "INSERT INTO "
              "agiru_client.page_modal_commands(modal_handle,command_id,payload_digest,outcome) "
              "SELECT $1,$2,$3,'queued' FROM agiru_client.page_modals WHERE handle=$1 AND "
              "call_handle=$4 "
              "AND user_security_id=$5::uuid AND host_id=$6 AND revision=$7::bigint "
              "AND prefix=$8 AND active AND expires_at>clock_timestamp() "
              "AND EXISTS(SELECT 1 FROM agiru_client.page_contexts p WHERE "
              "p.handle=page_handle AND p.user_security_id=$5::uuid AND p.host_id=$6 "
              "AND NOT p.invalidated AND p.expires_at>clock_timestamp())",
              admitted)
          .Affected() != 1) {
    Refuse("PageHostModalStale");
  }
  ++call.questions;
  auto accepted = std::make_shared<PageModalInput>(std::move(input));
  accepted->page = modal->page;
  modal->input = accepted;
  modal->busy = true;
  call.ready.notify_all();
  return accepted;
}

std::shared_ptr<PageModalInput> ReadPageModalInput(const Connection &connection,
                                                   const PageCall &call,
                                                   std::string_view modal,
                                                   std::string_view command) {
  const std::array<std::optional<std::string>, 5> binds{
      std::string(modal), std::string(command), call.handle, call.user.ToStorageText(), call.host};
  const auto rows = connection.Execute(
      "SELECT c.payload_digest,c.outcome,c.html,m.page_id,c.error_code,c.error_text "
      "FROM agiru_client.page_modal_commands c JOIN agiru_client.page_modals m "
      "ON m.handle=c.modal_handle WHERE m.handle=$1 AND c.command_id=$2 "
      "AND m.call_handle=$3 AND m.user_security_id=$4::uuid AND m.host_id=$5 "
      "AND m.expires_at>clock_timestamp()",
      binds);
  const auto digest = rows.Rows() == 1 ? rows.Value(0, 0) : std::nullopt;
  if (!digest) { Refuse("PageHostGone"); }
  return ReplayInput(rows,
                     {.modal = std::string(modal),
                      .command = std::string(command),
                      .digest = std::string(*digest)},
                     call.pageHandle);
}

}

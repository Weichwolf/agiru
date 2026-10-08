#include "PageInteraction.h"

#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/PageCommandHost.h"
#include "runtime/PageHostOptions.h"
#include "runtime/PageHtml.h"
#include "runtime/UiHost.h"
#include "type/Action.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include "HtmlText.h"
#include "PageModal.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstddef>
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

constexpr std::size_t kMaximumChoices = 128;
constexpr auto kBytes = PageHtmlLimits::kDefaultBytes;

[[noreturn]] void Refuse(std::string_view code) {
  throw Error("Page interaction refused", code);
}

void Text(std::string &html, std::string_view text) {
  AppendHtmlText(html, text, kBytes);
}

void Messages(std::string &html, const PageCall &call) {
  for (const auto &message : call.messages) {
    html += "<p data-message=\"";
    Text(html, message.handle);
    html += "\">";
    Text(html, message.text);
    html += "</p>";
  }
}

void Hidden(std::string &html, std::string_view name, std::string_view value) {
  html += R"(<input type="hidden" name=")";
  Text(html, name);
  html += "\" value=\"";
  Text(html, value);
  html += "\">";
}

void Choice(std::string &html,
            const PageCall &call,
            const PageQuestion &question,
            std::size_t index) {
  const auto identity = "$agiru.answer_" + question.handle + "_" + std::to_string(index);
  html += "<section data-control=\"" + identity + R"(" data-kind="action">)";
  html += "<form method=\"post\" action=\"/answers\" hx-post=\"/answers\" "
          "hx-target=\"closest article\" hx-swap=\"outerHTML\">";
  Hidden(html, "page", call.pageHandle);
  Hidden(html, "revision", call.revision);
  Hidden(html, "command", question.handle + "_" + std::to_string(index));
  Hidden(html, "csrf", call.csrf);
  Hidden(html, "operation", "action");
  Hidden(html, "control", identity);
  html += "<button type=\"submit\">";
  Text(html, question.choices[index]);
  html += "</button></form></section>";
}

void PublishQuestion(const PageHostOptions &options,
                     const PageCall &call,
                     const PageQuestion &question) {
  const Connection connection(options.database);
  const std::array<std::optional<std::string>, 9> binds{question.handle,
                                                        call.pageHandle,
                                                        call.handle,
                                                        call.user.ToStorageText(),
                                                        call.host,
                                                        call.revision,
                                                        call.command,
                                                        question.kind,
                                                        std::to_string(question.choices.size())};
  if (connection
          .Execute("INSERT INTO agiru_client.page_dialogs "
                   "(handle,page_handle,call_handle,user_security_id,host_id,revision,command_id,"
                   "kind,choices,expires_at) "
                   "SELECT $1,$2,$3,$4::uuid,$5,$6::bigint,$7,$8,$9::integer,expires_at "
                   "FROM agiru_client.page_contexts WHERE handle=$2 AND user_security_id=$4::uuid "
                   "AND host_id=$5 AND NOT invalidated AND expires_at>clock_timestamp()",
                   binds)
          .Affected() != 1) {
    Refuse("PageHostGone");
  }
}

void CloseQuestion(const PageHostOptions &options, const PageQuestion &question) {
  const Connection connection(options.database);
  const std::array<std::optional<std::string>, 1> binds{question.handle};
  if (connection.Execute("UPDATE agiru_client.page_dialogs SET closed=true WHERE handle=$1", binds)
          .Affected() != 1) {
    Refuse("PageHostCleanup");
  }
}

class PageUiHost final : public UiHost {
public:
  PageUiHost(const std::shared_ptr<PageCall> &call,
             PageHostOptions options,
             PageHostAuthorization authorization)
      : call_(call), options_(std::move(options)), authorization_(std::move(authorization)) {}

  Action RunModal(PageInstance &page) override {
    return RunPageModal(Active(), options_, authorization_, page);
  }

  void QueueMessage(std::string_view text) override {
    auto call = Active();
    const std::lock_guard lock(call->mutex);
    if (call->finished || call->cancelled) { Refuse("UiHostUnavailable"); }
    if (text.size() > kBytes - call->messageBytes ||
        call->messages.size() >= PageHtmlLimits::kDefaultControls) {
      Refuse("UiTransportLimit");
    }
    call->messages.push_back({.text = std::string(text)});
    call->messageBytes += text.size();
  }

  Boolean Confirm(std::string_view text, Boolean defaultButton) override {
    return Ask({.kind = "confirm",
                .prompt = std::string(text),
                .choices = {"No", "Yes"},
                .defaultChoice = defaultButton ? 1 : 0}) == 1;
  }

  Integer
  StrMenu(std::string_view options, Integer defaultChoice, std::string_view instruction) override {
    PageQuestion question{.kind = "menu",
                          .prompt = std::string(instruction),
                          .choices = {"Cancel"},
                          .defaultChoice = defaultChoice};
    for (std::size_t at = 0;;) {
      const auto end = options.find(',', at);
      question.choices.emplace_back(
          options.substr(at, end == std::string_view::npos ? end : end - at));
      if (question.choices.size() > kMaximumChoices) { Refuse("UiTransportLimit"); }
      if (end == std::string_view::npos) { break; }
      at = end + 1;
    }
    return Ask(std::move(question));
  }

  void OpenProgress(const void *owner,
                    std::string_view text,
                    std::span<const UiValueBinding> values) override {
    static_cast<void>(owner);
    static_cast<void>(text);
    static_cast<void>(values);
    Refuse("UiProgressUnsupported");
  }

  void UpdateProgress(const void *owner, Integer number, const Variant &value) override {
    static_cast<void>(owner);
    static_cast<void>(number);
    static_cast<void>(value);
    Refuse("UiProgressUnsupported");
  }

  void CloseProgress(const void *owner) override {
    static_cast<void>(owner);
    Refuse("UiProgressUnsupported");
  }

private:
  std::shared_ptr<PageCall> Active() const {
    auto call = call_.lock();
    if (!call) { Refuse("UiHostUnavailable"); }
    return call;
  }

  Integer Ask(PageQuestion question) {
    auto call = Active();
    std::size_t bytes = question.prompt.size();
    for (const auto &choice : question.choices) {
      if (choice.size() > kBytes - std::min(bytes, kBytes)) { Refuse("UiTransportLimit"); }
      bytes += choice.size();
    }
    if (bytes > kBytes || question.choices.size() > kMaximumChoices || question.defaultChoice < 0 ||
        std::cmp_greater_equal(question.defaultChoice, question.choices.size())) {
      Refuse("UiTransportLimit");
    }
    auto held = std::make_shared<PageQuestion>(std::move(question));
    std::unique_lock lock(call->mutex);
    if (call->finished || call->cancelled || call->question) { Refuse("UiHostUnavailable"); }
    if (call->questions >= options_.commands) { Refuse("UiTransportLimit"); }
    call->question = held;
    try {
      static_cast<void>(RenderPageInteraction(*call));
      PublishQuestion(options_, *call, *held);
    } catch (...) {
      call->question.reset();
      throw;
    }
    ++call->questions;
    call->ready.notify_all();
    const auto deadline =
        std::min(call->deadline, std::chrono::steady_clock::now() + options_.dialogTimeout);
    const bool answered = call->ready.wait_until(
        lock, deadline, [&] { return call->cancelled || held->answer.has_value(); });
    const auto answer = held->answer;
    const bool cancelled = call->cancelled;
    call->question.reset();
    lock.unlock();
    CloseQuestion(options_, *held);
    if (!answered || cancelled || !answer) { Refuse("UiDialogCancelled"); }
    return *answer;
  }

  std::weak_ptr<PageCall> call_;
  PageHostOptions options_;
  PageHostAuthorization authorization_;
};

std::string ModalPollAttributes(const PageCall &call) {
  for (auto modal = call.modal; modal; modal = modal->parent) {
    const auto input = modal->input ? modal->input : modal->executing;
    if (input && (!input->finished || input->error)) {
      return " data-poll=\"/modal-commands/" + input->modal + "/" + input->command +
             "\" data-poll-state=\"" + (input->finished ? "failed" : "pending") + "\"";
    }
  }
  return {};
}

}

void InstallPageDialogs(const Connection &connection) {
  connection.Run(R"(CREATE TABLE IF NOT EXISTS agiru_client.page_dialogs (
    handle text PRIMARY KEY, page_handle text NOT NULL, call_handle text NOT NULL,
    user_security_id uuid NOT NULL, host_id text NOT NULL, revision bigint NOT NULL,
    command_id text NOT NULL, kind text NOT NULL CHECK (kind IN ('confirm','menu')),
    choices integer NOT NULL CHECK (choices > 0 AND choices <= 128), answer integer,
    closed boolean NOT NULL DEFAULT false, expires_at timestamptz NOT NULL,
    CHECK (answer IS NULL OR (answer >= 0 AND answer < choices))
  ))");
}

std::unique_ptr<UiHost> MakePageUiHost(const std::shared_ptr<PageCall> &call,
                                       const PageHostOptions &options,
                                       const PageHostAuthorization &authorization) {
  return std::make_unique<PageUiHost>(call, options, authorization);
}

std::string RenderPageInteraction(const PageCall &call) {
  const auto poll = ModalPollAttributes(call);
  const auto question = call.question && !call.question->answer ? call.question : nullptr;
  if (!question && call.modal && !call.modal->busy) {
    auto html = call.modal->html;
    const auto end = html.find('>');
    if (end == std::string::npos || poll.size() > kBytes - std::min(html.size(), kBytes)) {
      Refuse("UiTransportLimit");
    }
    html.insert(end, poll);
    return AppendPageMessages(call, std::move(html));
  }
  std::string html = R"(<article data-agiru-profile="3" data-view="interaction" data-page=")";
  html += std::to_string(call.page.Value());
  html += "\" data-handle=\"" + call.pageHandle + "\" data-revision=\"" + call.revision;
  html += "\" data-state=\"" + (question ? question->kind : "working");
  html += "\" data-call=\"" + call.handle + "\" data-origin-command=\"" + call.command + "\"";
  html += poll;
  if (question) {
    html += " data-dialog=\"" + question->handle + "\" data-default=\"" +
            std::to_string(question->defaultChoice) + "\"";
  }
  html += question ? "><h1>Question</h1>" : "><h1>Working</h1>";
  if (question) {
    Messages(html, call);
    html += "<p data-prompt=\"true\">";
    Text(html, question->prompt);
    html += "</p>";
    for (std::size_t index = 0; index < question->choices.size(); ++index) {
      Choice(html, call, *question, index);
    }
  }
  html += "<output data-unsupported-count=\"0\"></output></article>";
  if (html.size() > kBytes) { Refuse("UiTransportLimit"); }
  return html;
}

std::string AppendPageMessages(const PageCall &call, std::string html) {
  std::string messages;
  Messages(messages, call);
  const auto heading = html.find("</h1>");
  if (heading == std::string::npos) { Refuse("UiTransportLimit"); }
  if (messages.size() > kBytes - std::min(html.size(), kBytes)) { Refuse("UiTransportLimit"); }
  html.insert(heading + std::string_view("</h1>").size(), messages);
  return html;
}

void AcceptPageAnswer(const Connection &connection,
                      PageCall &call,
                      std::string_view command,
                      std::string_view control) {
  const auto separator = command.rfind('_');
  if (separator == std::string_view::npos || control != "$agiru.answer_" + std::string(command)) {
    Refuse("PageHostInput");
  }
  const auto handle = command.substr(0, separator);
  const auto value = command.substr(separator + 1);
  Integer choice = 0;
  const auto parsed = std::from_chars(value.data(), value.data() + value.size(), choice);
  if (value.empty() || parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() ||
      choice < 0 || value != std::to_string(choice)) {
    Refuse("PageHostInput");
  }
  const std::array<std::optional<std::string>, 5> binds{std::string(handle),
                                                        call.handle,
                                                        call.user.ToStorageText(),
                                                        call.host,
                                                        std::to_string(choice)};
  const bool active =
      !call.finished && !call.cancelled && call.question && call.question->handle == handle;
  if (active && std::cmp_greater_equal(choice, call.question->choices.size())) {
    Refuse("PageHostInput");
  }
  std::optional<Result> result;
  if (active) {
    result.emplace(connection.Execute(
        "UPDATE agiru_client.page_dialogs SET answer=$5::integer WHERE handle=$1 AND "
        "call_handle=$2 "
        "AND user_security_id=$3::uuid AND host_id=$4 AND NOT closed AND answer IS NULL "
        "AND expires_at>clock_timestamp() AND $5::integer<choices RETURNING answer",
        binds));
  }
  if (!result || result->Rows() == 0) {
    const auto replay =
        connection.Execute("SELECT answer FROM agiru_client.page_dialogs WHERE handle=$1 "
                           "AND call_handle=$2 AND user_security_id=$3::uuid AND host_id=$4 AND "
                           "expires_at>clock_timestamp()",
                           std::span(binds).first(4));
    if (replay.Rows() != 1 || replay.Value(0, 0) != std::to_string(choice)) {
      Refuse("PageHostDialogStale");
    }
    return;
  }
  call.question->answer = choice;
  call.ready.notify_all();
}

}

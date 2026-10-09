#pragma once

#include "meta/Ids.h"
#include "runtime/HttpServer.h"
#include "runtime/PageCommandHost.h"
#include "runtime/PageHostOptions.h"
#include "runtime/SecureToken.h"
#include "runtime/UiHost.h"
#include "type/Guid.h"

#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace agiru {
class Connection;
}

namespace agiru::detail {

struct PageModal;

struct PageQuestion {
  std::string handle = GenerateSecureToken();
  std::string kind;
  std::string prompt;
  std::vector<std::string> choices;
  Integer defaultChoice = 0;
  std::optional<Integer> answer{};
};

struct PageMessage {
  std::string handle = GenerateSecureToken();
  std::string text;
};

struct PageCall {
  std::mutex mutex;
  std::condition_variable ready;
  std::string handle = GenerateSecureToken();
  std::string pageHandle;
  std::string host;
  std::string csrf;
  std::string browserCsrf;
  std::string credential;
  Guid user;
  PageId page;
  std::string revision;
  std::string command;
  std::string operation;
  std::string control;
  std::chrono::steady_clock::time_point deadline;
  std::shared_ptr<PageQuestion> question;
  std::shared_ptr<PageModal> modal;
  std::vector<PageMessage> messages;
  std::size_t messageBytes = 0;
  std::size_t questions = 0;
  std::optional<ServerHttpResponse> result;
  std::exception_ptr error;
  bool finished = false;
  bool cancelled = false;
};

void InstallPageDialogs(const Connection &connection);
bool WaitForPageInteraction(PageCall &call,
                            std::unique_lock<std::mutex> &lock,
                            std::chrono::steady_clock::time_point deadline,
                            const std::function<bool()> &ready);
std::unique_ptr<UiHost> MakePageUiHost(const std::shared_ptr<PageCall> &call,
                                       const PageHostOptions &options,
                                       const PageHostAuthorization &authorization);
std::string RenderPageInteraction(const PageCall &call);
std::string AppendPageMessages(const PageCall &call, std::string html);
bool AcceptPageAnswer(const Connection &connection,
                      PageCall &call,
                      std::string_view command,
                      std::string_view control);

}

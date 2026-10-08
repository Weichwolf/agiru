#pragma once

#include "meta/Ids.h"
#include "runtime/HttpServer.h"
#include "runtime/PageHostOptions.h"
#include "runtime/SecureToken.h"
#include "runtime/UiHost.h"
#include "type/Guid.h"

#include <chrono>
#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace agiru {
class Connection;
}

namespace agiru::detail {

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
  Guid user;
  PageId page;
  std::string revision;
  std::string command;
  std::string operation;
  std::string control;
  std::chrono::steady_clock::time_point deadline;
  std::shared_ptr<PageQuestion> question;
  std::vector<PageMessage> messages;
  std::size_t messageBytes = 0;
  std::size_t questions = 0;
  std::optional<ServerHttpResponse> result;
  std::exception_ptr error;
  bool finished = false;
  bool cancelled = false;
};

void InstallPageDialogs(const Connection &connection);
std::unique_ptr<UiHost> MakePageUiHost(const std::shared_ptr<PageCall> &call,
                                       const PageHostOptions &options);
std::string RenderPageInteraction(const PageCall &call);
std::string AppendPageMessages(const PageCall &call, std::string html);
void AcceptPageAnswer(const Connection &connection,
                      PageCall &call,
                      std::string_view command,
                      std::string_view control);

}

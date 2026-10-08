#pragma once

#include "meta/Ids.h"
#include "runtime/HttpServer.h"
#include "runtime/PageCommandHost.h"
#include "runtime/PageHostOptions.h"
#include "runtime/SecureToken.h"
#include "type/Action.h"

#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace agiru {
class Connection;
class PageInstance;
}

namespace agiru::detail {
struct PageCall;

struct PageModalInput {
  PageId page{};
  std::string modal{};
  std::string command{};
  std::string operation{};
  std::string control{};
  std::string text{};
  std::string digest{};
  bool finished = false;
  std::optional<ServerHttpResponse> response{};
  std::exception_ptr error{};
};

struct PageModal {
  std::string handle = GenerateSecureToken();
  std::string prefix = GenerateSecureToken();
  PageId page{};
  std::int64_t revision = 0;
  std::string html{};
  std::shared_ptr<PageModalInput> input{};
  std::shared_ptr<PageModalInput> executing{};
  std::shared_ptr<PageModal> parent{};
  bool busy = true;
};

void InstallPageModals(const Connection &connection);
std::shared_ptr<PageModalInput> ReadPageModalInput(const Connection &connection,
                                                   const PageCall &call,
                                                   std::string_view modal,
                                                   std::string_view command);
Action RunPageModal(const std::shared_ptr<PageCall> &call,
                    const PageHostOptions &options,
                    const PageHostAuthorization &authorization,
                    PageInstance &page);
std::shared_ptr<PageModalInput> AcceptPageModal(const Connection &connection,
                                                PageCall &call,
                                                std::string_view handle,
                                                std::string_view revision,
                                                PageModalInput input,
                                                const PageHostOptions &options);
}

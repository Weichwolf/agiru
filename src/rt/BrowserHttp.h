#pragma once

#include "runtime/ClientCredentials.h"
#include "runtime/HttpServer.h"

#include <optional>
#include <string>
#include <string_view>

namespace agiru {
class Connection;
struct PageHostOptions;
}

namespace agiru::detail {

struct PageClient {
  ClientCredentialIdentity identity;
  std::string csrf;
};

void ValidateBrowserHttpOptions(const PageHostOptions &options);
PageClient AuthenticatePageClient(const Connection &connection,
                                  const ServerHttpRequest &request,
                                  const PageHostOptions &options);
std::optional<ServerHttpResponse> BrowserSessionEndpoint(Connection &connection,
                                                         const ServerHttpRequest &request,
                                                         const PageHostOptions &options);
void RenewPageClient(const Connection &connection,
                     const ClientCredentialIdentity &client,
                     std::string_view csrf);
ServerHttpRequest RetainedPageRequest(const ServerHttpRequest &request, std::string_view csrf);

}

#include "runtime/NativeService.h"

#include "meta/PageDef.h"
#include "runtime/BrowserSession.h"
#include "runtime/ClientCredentials.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/HttpServer.h"
#include "runtime/HttpServerOptions.h"
#include "runtime/NativePermissions.h"
#include "runtime/PageCommandHost.h"
#include "runtime/PageHostOptions.h"
#include "runtime/PermissionSetRegistry.h"
#include "runtime/Session.h"
#include "type/Guid.h"

#include <array>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <memory>
#include <print>
#include <string>
#include <string_view>
#include <utility>

#include <unistd.h>

namespace agiru {
namespace {

class Shutdown {
  using SignalMask = decltype(std::declval<struct sigaction>().sa_mask);

public:
  Shutdown() {
    sigemptyset(&signals_);
    sigaddset(&signals_, SIGINT);
    sigaddset(&signals_, SIGTERM);
    if (pthread_sigmask(SIG_BLOCK, &signals_, &previous_) != 0) {
      throw Error("cannot establish native shutdown handling", "ServiceSignal");
    }
  }

  ~Shutdown() { static_cast<void>(pthread_sigmask(SIG_SETMASK, &previous_, nullptr)); }

  Shutdown(const Shutdown &) = delete;
  Shutdown &operator=(const Shutdown &) = delete;

  void Wait() const {
    int signal = 0;
    if (sigwait(&signals_, &signal) != 0) {
      throw Error("native shutdown wait failed", "ServiceSignal");
    }
  }

private:
  SignalMask signals_{};
  SignalMask previous_{};
};

void VerifyStorage(const std::string &database, const std::string &company, bool browserCookies) {
  const Connection connection(database);
  const auto rows = connection.Execute(R"(SELECT "Name" FROM "Company" LIMIT 2)");
  if (rows.Rows() != 1 || rows.Value(0, 0) != company) {
    throw Error("configured company must be the only company in the flat storage profile",
                "ServiceCompany");
  }
  constexpr std::array<std::string_view, 8> probes{
      R"(SELECT 1 FROM "User" LIMIT 0)",
      R"(SELECT 1 FROM "Access Control" LIMIT 0)",
      R"(SELECT 1 FROM "Tenant Permission Set" LIMIT 0)",
      R"(SELECT 1 FROM "Tenant Permission" LIMIT 0)",
      R"(SELECT 1 FROM "Tenant Permission Set Rel." LIMIT 0)",
      "SELECT 1 FROM agiru_client.credentials LIMIT 0",
      "SELECT 1 FROM agiru_client.page_contexts LIMIT 0",
      "SELECT 1 FROM agiru_client.page_commands LIMIT 0"};
  for (const auto probe : probes) { static_cast<void>(connection.Execute(probe)); }
  if (browserCookies) {
    static_cast<void>(connection.Execute("SELECT 1 FROM agiru_client.browser_sessions LIMIT 0"));
  }
}

}

void RunNativeService(const NativeServiceOptions &options) {
  ValidatePageHostOptions(options.pages);
  ValidateHttpServerOptions(options.http);
  if (!options.http.loopback || options.http.port == 0) {
    throw Error("native listener must use a nonzero private loopback port", "ServerConfiguration");
  }
  VerifyStorage(options.pages.database, options.pages.company, options.pages.browserCookies);
  const InstalledPermissionSets system;
  const auto permissions = std::make_shared<NativePermissions>(system);
  PageCommandHost host(
      options.pages,
      [permissions](const PageDef &page, auto, const auto &) { permissions->RequirePage(page); },
      permissions);
  const Shutdown shutdown;
  const HttpServer server([&host](const auto &request) { return host.Handle(request); },
                          options.http);
  std::println("READY {} {}", getpid(), server.Port());
  std::fflush(stdout);
  shutdown.Wait();
}

void InitializeNativeClient(const std::string &database) {
  const Session session(database);
  InstallClientCredentials(session.Database());
  InstallBrowserSessions(session.Database());
  InstallPageCommandHost(session.Database());
  Commit();
}

std::string IssueNativeClientCredential(const std::string &database,
                                        std::string_view user,
                                        std::chrono::seconds lifetime) {
  const Session session(database);
  const auto secret = IssueClientCredential(session.Database(), Guid(user), lifetime);
  Commit();
  return secret;
}

}

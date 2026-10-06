#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/HttpServer.h"

#include "Check.h"

#include <array>
#include <condition_variable>
#include <cstdio>
#include <fstream>
#include <ios>
#include <iterator>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {

void Contracts() {
  const agiru::ServerHttpRequest request{.method = "GET",
                                         .target = "/?company=CRONUS%20CH",
                                         .headers = {{.name = "Authorization", .value = "fixture"}},
                                         .body = {}};
  CHECK_TEXT(
      "header identity is ASCII case-insensitive", request.Header("aUtHoRiZaTiOn"), "fixture");
  CHECK_TRUE("absent headers do not invent authentication", request.Header("Cookie").empty());
  const auto handler = [](const auto &) { return agiru::ServerHttpResponse{}; };
  for (const auto options : {agiru::HttpServerOptions{.workers = 0},
                             agiru::HttpServerOptions{.queue = 0},
                             agiru::HttpServerOptions{.connections = 0},
                             agiru::HttpServerOptions{.timeoutSeconds = 0},
                             agiru::HttpServerOptions{.totalBodyBytes = 1},
                             agiru::HttpServerOptions{.responseBytes = 1}}) {
    bool refused = false;
    try {
      const agiru::HttpServer server(handler, options);
    } catch (const agiru::Error &error) { refused = error.Code() == "HttpServerOptions"; }
    CHECK_TRUE("invalid resource ceilings refuse before native work", refused);
  }
  bool refused = false;
  try {
    const agiru::HttpServer server({}, {.port = 0});
  } catch (const agiru::Error &error) { refused = error.Code() == "HttpServerOptions"; }
  CHECK_TRUE("missing handler is not a successful server", refused);
  const agiru::HttpServer server(handler, {.port = 0});
  CHECK_TRUE("native listener exposes its actual OS-assigned port", server.Port() != 0);
  refused = false;
  try {
    const agiru::HttpServer collision(handler, {.port = server.Port()});
  } catch (const agiru::Error &error) { refused = error.Code() == "HttpServerListen"; }
  CHECK_TRUE("occupied listening port refuses without stealing authority", refused);
}

class TransportFixture {
public:
  explicit TransportFixture(std::string html) : html_(std::move(html)) {
    const agiru::Connection database(AGIRU_TEST_DSN);
    database.Run("CREATE TABLE agiru_http_transport_fixture(method text, target text, body text)");
  }

  ~TransportFixture() {
    try {
      const agiru::Connection database(AGIRU_TEST_DSN);
      database.Run("DROP TABLE agiru_http_transport_fixture");
    } catch (...) { std::fputs("HTTP transport fixture cleanup failed\n", stderr); }
  }

  agiru::ServerHttpResponse Handle(const agiru::ServerHttpRequest &request) {
    if (request.target.starts_with("/echo-target")) {
      return {.status = agiru::ServerHttpResponse::kOk,
              .contentType = "text/plain",
              .body = request.target,
              .headers = {}};
    }
    if (request.target == "/proxy-headers") {
      std::string body;
      for (const std::string_view name : {"Host",
                                          "X-Forwarded-Proto",
                                          "X-Forwarded-Host",
                                          "X-Forwarded-For",
                                          "X-Real-IP",
                                          "Forwarded",
                                          "Authorization",
                                          "Cookie"}) {
        body += name;
        body += '=';
        body += request.Header(name);
        body += '\n';
      }
      return {.status = agiru::ServerHttpResponse::kOk,
              .contentType = "text/plain",
              .body = std::move(body),
              .headers = {}};
    }
    if (request.target == "/binary") {
      return {.status = agiru::ServerHttpResponse::kOk,
              .contentType = "application/octet-stream",
              .body = request.body,
              .headers = {}};
    }
    if (request.target == "/hold") {
      std::fputs("HOLD\n", stdout);
      std::fflush(stdout);
      std::unique_lock lock(mutex_);
      released_.wait(lock, [this] { return release_; });
    }
    if (request.target == "/throw") { throw std::runtime_error("fixture handler failure"); }
    if (request.target == "/oversized-response") {
      return {.status = agiru::ServerHttpResponse::kOk,
              .contentType = "text/plain",
              .body = std::string(agiru::HttpServerOptions::kDefaultBodyBytes + 1, 'x'),
              .headers = {}};
    }
    if (request.target == "/injected-header") {
      return {.status = agiru::ServerHttpResponse::kOk,
              .contentType = "text/plain",
              .body = "unsafe",
              .headers = {{.name = "X-Test", .value = "bad\r\nInjected: yes"}}};
    }
    const agiru::Connection database(AGIRU_TEST_DSN);
    const std::array<std::optional<std::string>, 3> binds{
        request.method, request.target, request.body};
    database.Run("INSERT INTO agiru_http_transport_fixture VALUES ($1,$2,$3)", binds);
    return {.status = agiru::ServerHttpResponse::kOk,
            .contentType = "text/html; charset=utf-8",
            .body = html_,
            .headers = {{.name = "X-Fixture-Method", .value = request.method}}};
  }

  void Release() {
    {
      const std::lock_guard lock(mutex_);
      release_ = true;
    }
    released_.notify_all();
  }

private:
  std::string html_;
  std::mutex mutex_;
  std::condition_variable released_;
  bool release_ = false;
};

void Serve(std::string_view htmlPath) {
  std::ifstream input(std::string(htmlPath), std::ios::binary);
  if (!input) { throw std::runtime_error("missing C++ HTML fixture"); }
  std::string html{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
  if (html.empty() || html.size() > agiru::HttpServerOptions::kDefaultBodyBytes) {
    throw std::runtime_error("invalid C++ HTML fixture size");
  }
  TransportFixture fixture(std::move(html));
  const agiru::HttpServer server(
      [&fixture](const auto &request) { return fixture.Handle(request); },
      {.port = 18080,
       .loopback = true,
       .workers = 2,
       .queue = 1,
       .connections = 16,
       .timeoutSeconds = 5,
       .bodyBytes = 4096,
       .totalBodyBytes = 8192});
  std::fputs("READY\n", stdout);
  std::fflush(stdout);
  for (;;) {
    const int command = std::getchar();
    if (command == 'B') {
      const auto bytes = std::to_string(server.BufferedRequestBytes());
      std::fputs("BYTES ", stdout);
      std::fputs(bytes.c_str(), stdout);
      std::fputs("\n", stdout);
      std::fflush(stdout);
    }
    if (command == 'R') { fixture.Release(); }
    if (command == EOF || command == 'Q') {
      fixture.Release();
      break;
    }
  }
}

}

int main(int argc, char **argv) {
  return gate::Run("HttpServer", [argc, argv] {
    if (argc == 1) {
      Contracts();
    } else if (argc == 3 && std::string_view(argv[1]) == "--serve") {
      Serve(argv[2]);
    } else {
      throw std::runtime_error("HttpServerGate expects --serve <C++ HTML fixture>");
    }
  });
}

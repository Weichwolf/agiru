#include "runtime/HttpServer.h"

#include "runtime/ErrorValue.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <microhttpd.h>
#include <netinet/in.h>
#include <sched.h>
#include <sys/socket.h>
#include <unistd.h>

namespace agiru {
namespace {

constexpr std::size_t kTargetBytes = 8192;
constexpr std::size_t kHeaderBytes = 32768;
constexpr std::size_t kHeaderCount = 64;
constexpr std::size_t kConnectionMemory = 65536;
constexpr std::size_t kMaxQueue = 4096;
constexpr unsigned kMaxTimeoutSeconds = 120;
constexpr std::size_t kMaxBodyBytes = 16777216;
constexpr std::size_t kMinResponseBytes = 128;
constexpr std::size_t kContentTypeBytes = 128;
constexpr unsigned kLastResponseStatus = 599;

char Lower(char value) {
  return value >= 'A' && value <= 'Z' ? static_cast<char>(value + ('a' - 'A')) : value;
}

bool SameHeader(std::string_view first, std::string_view second) {
  return std::ranges::equal(first, second, [](char a, char b) { return Lower(a) == Lower(b); });
}

bool HeaderText(std::string_view value) {
  return !value.empty() && value.size() <= kHeaderBytes &&
         std::ranges::all_of(
             value, [](unsigned char c) { return c == '\t' || (c >= ' ' && c != '\x7f'); });
}

ServerHttpResponse Refusal(unsigned status, std::string_view code) {
  return {.status = status,
          .contentType = "text/plain; charset=utf-8",
          .body = std::string(code),
          .headers = {}};
}

bool ValidOptions(const HttpServerOptions &options) {
  return options.workers > 0 && options.workers <= HttpServerOptions::kMaxWorkers &&
         options.queue > 0 && options.queue <= kMaxQueue && options.connections > 0 &&
         options.timeoutSeconds > 0 && options.timeoutSeconds <= kMaxTimeoutSeconds &&
         options.bodyBytes > 0 && options.bodyBytes <= kMaxBodyBytes &&
         options.totalBodyBytes >= options.bodyBytes &&
         options.responseBytes >= kMinResponseBytes && options.responseBytes <= kMaxBodyBytes;
}

bool HeaderName(std::string_view value) {
  return !value.empty() && std::ranges::all_of(value, [](unsigned char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
           std::string_view("!#$%&'*+-.^_`|~").find(static_cast<char>(c)) != std::string_view::npos;
  });
}

}

std::size_t DefaultHttpWorkers() {
  cpu_set_t available{};
  std::size_t count = std::thread::hardware_concurrency();
  if (sched_getaffinity(0, sizeof(available), &available) == 0) {
    count = static_cast<std::size_t>(CPU_COUNT(&available));
  }
  return std::clamp(count, std::size_t{1}, HttpServerOptions::kMaxWorkers);
}

std::string_view ServerHttpRequest::Header(std::string_view name) const {
  const auto found = std::ranges::find_if(
      headers, [name](const auto &header) { return SameHeader(header.name, name); });
  return found == headers.end() ? std::string_view{} : found->value;
}

struct HttpServer::Impl {
  enum class Phase { Receiving, Queued, Ready, Sent };

  struct Request {
    Impl &owner;
    ServerHttpRequest input;
    ServerHttpResponse output;
    MHD_Connection *connection;
    std::size_t headerBytes = 0;
    unsigned refusal = 0;
    std::atomic<Phase> phase = Phase::Receiving;

    Request(Impl &server, MHD_Connection *client) : owner(server), connection(client) {}

    ~Request() { owner.bodyBytes.fetch_sub(input.body.size()); }
  };

  std::function<ServerHttpResponse(const ServerHttpRequest &)> handler;
  HttpServerOptions options;
  MHD_Daemon *daemon = nullptr;
  std::atomic<std::size_t> bodyBytes = 0;
  std::mutex mutex;
  std::condition_variable ready;
  std::deque<Request *> jobs;
  std::vector<std::thread> workers;
  bool accepting = true;

  Impl(std::function<ServerHttpResponse(const ServerHttpRequest &)> callback,
       HttpServerOptions limits)
      : handler(std::move(callback)), options(limits) {
    if (!handler || !ValidOptions(options)) {
      throw Error("Invalid HTTP server limits or handler", "HttpServerOptions");
    }
    try {
      for (std::size_t at = 0; at < options.workers; ++at) {
        workers.emplace_back([this] { Work(); });
      }
      sockaddr_in address{};
      address.sin_family = AF_INET;
      address.sin_port = htons(options.port);
      address.sin_addr.s_addr = htonl(options.loopback ? INADDR_LOOPBACK : INADDR_ANY);
      daemon =
          MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_AUTO |
                               MHD_ALLOW_SUSPEND_RESUME | MHD_USE_ITC | MHD_USE_PEDANTIC_CHECKS,
                           options.port,
                           nullptr,
                           nullptr,
                           Access,
                           this,
                           MHD_OPTION_SOCK_ADDR,
                           &address,
                           MHD_OPTION_CONNECTION_LIMIT,
                           options.connections,
                           MHD_OPTION_CONNECTION_TIMEOUT,
                           options.timeoutSeconds,
                           MHD_OPTION_CONNECTION_MEMORY_LIMIT,
                           kConnectionMemory,
                           MHD_OPTION_URI_LOG_CALLBACK,
                           Begin,
                           this,
                           MHD_OPTION_NOTIFY_COMPLETED,
                           Complete,
                           this,
                           MHD_OPTION_END);
      if (daemon == nullptr) {
        throw Error("Cannot bind native HTTP listener", "HttpServerListen");
      }
    } catch (...) {
      Drain();
      throw;
    }
  }

  ~Impl() {
    const MHD_socket socket = MHD_quiesce_daemon(daemon);
    Drain();
    MHD_stop_daemon(daemon);
    if (socket != MHD_INVALID_SOCKET) { static_cast<void>(close(socket)); }
  }

  void Drain() {
    {
      const std::lock_guard lock(mutex);
      accepting = false;
    }
    ready.notify_all();
    for (auto &worker : workers) {
      if (worker.joinable()) { worker.join(); }
    }
  }

  void Work() {
    for (;;) {
      Request *request = nullptr;
      {
        std::unique_lock lock(mutex);
        ready.wait(lock, [this] { return !jobs.empty() || !accepting; });
        if (jobs.empty()) { return; }
        request = jobs.front();
        jobs.pop_front();
      }
      try {
        request->output = handler(request->input);
      } catch (...) {
        std::fputs("HTTP handler raised an unhandled exception\n", stderr);
        request->output = Refusal(MHD_HTTP_INTERNAL_SERVER_ERROR, "HttpHandlerFailure");
      }
      request->phase.store(Phase::Ready, std::memory_order_release);
      MHD_resume_connection(request->connection);
    }
  }

  static void *Begin(void *context, const char *uri, MHD_Connection *connection) noexcept {
    try {
      auto request = std::make_unique<Request>(*static_cast<Impl *>(context), connection);
      const std::string_view target(uri);
      if (target.size() > kTargetBytes) {
        request->refusal = MHD_HTTP_URI_TOO_LONG;
      } else {
        request->input.target = target;
      }
      return request.release();
    } catch (...) {
      std::fputs("HTTP request allocation failed\n", stderr);
      return nullptr;
    }
  }

  static void Complete([[maybe_unused]] void *context,
                       [[maybe_unused]] MHD_Connection *connection,
                       void **state,
                       [[maybe_unused]] MHD_RequestTerminationCode code) noexcept {
    delete static_cast<Request *>(std::exchange(*state, nullptr));
  }

  static MHD_Result Headers(void *context,
                            [[maybe_unused]] MHD_ValueKind kind,
                            const char *name,
                            const char *value) noexcept {
    auto &request = *static_cast<Request *>(context);
    try {
      const std::string_view key(name == nullptr ? "" : name);
      const std::string_view text(value == nullptr ? "" : value);
      if (request.input.headers.size() >= kHeaderCount ||
          key.size() + text.size() > kHeaderBytes - request.headerBytes || !HeaderName(key) ||
          (!text.empty() && !HeaderText(text))) {
        request.refusal = MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE;
        return MHD_NO;
      }
      if (std::ranges::any_of(request.input.headers,
                              [key](const auto &header) { return SameHeader(header.name, key); })) {
        request.refusal = MHD_HTTP_BAD_REQUEST;
        return MHD_NO;
      }
      request.headerBytes += key.size() + text.size();
      request.input.headers.push_back({std::string(key), std::string(text)});
      return MHD_YES;
    } catch (...) {
      request.refusal = MHD_HTTP_INTERNAL_SERVER_ERROR;
      return MHD_NO;
    }
  }

  bool Append(Request &request, std::string_view bytes) {
    if (bytes.size() > options.bodyBytes - request.input.body.size()) { return false; }
    auto current = bodyBytes.load();
    do {
      if (bytes.size() > options.totalBodyBytes - current) { return false; }
    } while (!bodyBytes.compare_exchange_weak(current, current + bytes.size()));
    try {
      request.input.body += bytes;
    } catch (...) {
      bodyBytes.fetch_sub(bytes.size());
      throw;
    }
    return true;
  }

  static MHD_Result Send(Request &request) {
    auto &output = request.output;
    if (output.status < MHD_HTTP_OK || output.status > kLastResponseStatus ||
        output.body.size() > request.owner.options.responseBytes ||
        !HeaderText(output.contentType) || output.contentType.size() > kContentTypeBytes ||
        output.headers.size() > kHeaderCount) {
      output = Refusal(MHD_HTTP_INTERNAL_SERVER_ERROR, "HttpResponseRefused");
    }
    std::size_t headerBytes = 0;
    for (const auto &header : output.headers) {
      if (!HeaderName(header.name) || (!header.value.empty() && !HeaderText(header.value)) ||
          header.name.size() + header.value.size() > kHeaderBytes - headerBytes ||
          SameHeader(header.name, "Content-Length") ||
          SameHeader(header.name, "Transfer-Encoding") || SameHeader(header.name, "Content-Type")) {
        output = Refusal(MHD_HTTP_INTERNAL_SERVER_ERROR, "HttpResponseRefused");
        break;
      }
      headerBytes += header.name.size() + header.value.size();
    }
    const std::unique_ptr<MHD_Response, decltype(&MHD_destroy_response)> response(
        MHD_create_response_from_buffer(
            output.body.size(), output.body.data(), MHD_RESPMEM_MUST_COPY),
        MHD_destroy_response);
    if (!response) { return MHD_NO; }
    if (MHD_add_response_header(response.get(), "Content-Type", output.contentType.c_str()) !=
            MHD_YES ||
        MHD_add_response_header(response.get(), "X-Content-Type-Options", "nosniff") != MHD_YES ||
        MHD_add_response_header(response.get(), "Cache-Control", "no-store") != MHD_YES) {
      return MHD_NO;
    }
    for (const auto &header : output.headers) {
      if (MHD_add_response_header(response.get(), header.name.c_str(), header.value.c_str()) !=
          MHD_YES) {
        return MHD_NO;
      }
    }
    const auto result = MHD_queue_response(request.connection, output.status, response.get());
    request.phase.store(Phase::Sent, std::memory_order_release);
    return result;
  }

  static MHD_Result Access(void *context,
                           [[maybe_unused]] MHD_Connection *connection,
                           [[maybe_unused]] const char *const url,
                           const char *method,
                           [[maybe_unused]] const char *const version,
                           const char *upload,
                           std::size_t *uploadSize,
                           void **state) noexcept {
    try {
      if (*state == nullptr) { return MHD_NO; }
      auto &owner = *static_cast<Impl *>(context);
      auto &request = *static_cast<Request *>(*state);
      if (request.input.method.empty()) {
        request.input.method = method;
        static_cast<void>(
            MHD_get_connection_values(request.connection, MHD_HEADER_KIND, Headers, &request));
        return MHD_YES;
      }
      if (request.refusal != 0) {
        *uploadSize = 0;
        request.output = Refusal(request.refusal, "HttpRequestRefused");
        return Send(request);
      }
      if (*uploadSize != 0) {
        if (!owner.Append(request, std::string_view(upload, *uploadSize))) {
          request.refusal = MHD_HTTP_CONTENT_TOO_LARGE;
        }
        *uploadSize = 0;
        return MHD_YES;
      }
      const auto phase = request.phase.load(std::memory_order_acquire);
      if (phase == Phase::Ready) { return Send(request); }
      if (phase != Phase::Receiving) { return MHD_YES; }
      {
        const std::lock_guard lock(owner.mutex);
        if (!owner.accepting || owner.jobs.size() >= owner.options.queue) {
          request.output = Refusal(MHD_HTTP_SERVICE_UNAVAILABLE, "HttpExecutorBusy");
          return Send(request);
        }
        owner.jobs.push_back(&request);
        MHD_suspend_connection(request.connection);
        request.phase.store(Phase::Queued, std::memory_order_release);
      }
      owner.ready.notify_one();
      return MHD_YES;
    } catch (...) {
      std::fputs("HTTP request processing failed\n", stderr);
      return MHD_NO;
    }
  }
};

HttpServer::HttpServer(std::function<ServerHttpResponse(const ServerHttpRequest &)> handler,
                       HttpServerOptions options)
    : impl_(std::make_unique<Impl>(std::move(handler), options)) {}

HttpServer::~HttpServer() = default;

std::uint16_t HttpServer::Port() const {
  const auto *info = MHD_get_daemon_info(impl_->daemon, MHD_DAEMON_INFO_BIND_PORT);
  if (info == nullptr) { throw Error("Cannot read bound HTTP port", "HttpServerPort"); }
  return info->port;
}

std::size_t HttpServer::BufferedRequestBytes() const {
  return impl_->bodyBytes.load();
}

}

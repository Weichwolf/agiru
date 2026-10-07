#include "platform/User.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/HttpServer.h"
#include "runtime/HttpServerOptions.h"
#include "runtime/Scopes.h"
#include "runtime/Session.h"
#include "runtime/SessionCommand.h"
#include "runtime/SessionOptions.h"
#include "runtime/Storage.h"
#include "runtime/Transaction.h"
#include "type/CommitBehavior.h"
#include "type/ErrorBehavior.h"
#include "type/ErrorInfo.h"
#include "type/Guid.h"

#include "Check.h"
#include "OwnedDatabase.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <print>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <netinet/in.h>
#include <sched.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {

constexpr int kTimeoutSeconds = 15;
constexpr std::size_t kResponseBytes = 4096;

void Ready(int descriptor, short events, std::chrono::steady_clock::time_point deadline) {
  pollfd pending{.fd = descriptor, .events = events, .revents = 0};
  const auto left = std::chrono::duration_cast<std::chrono::milliseconds>(
                        deadline - std::chrono::steady_clock::now())
                        .count();
  if (left <= 0 || poll(&pending, 1, static_cast<int>(left)) <= 0) {
    throw std::runtime_error("parallel fixture socket deadline failed");
  }
}

agiru::Guid Identity(std::size_t number) {
  std::array<std::uint8_t, agiru::Guid::kSize> bytes{};
  bytes.back() = static_cast<std::uint8_t>(number + 1);
  return agiru::Guid(bytes);
}

class Socket {
public:
  Socket() : descriptor_(socket(AF_INET, SOCK_STREAM, 0)) {
    if (descriptor_ < 0) { throw std::runtime_error("parallel fixture socket failed"); }
  }

  ~Socket() { close(descriptor_); }

  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;

  int Descriptor() const { return descriptor_; }

private:
  int descriptor_;
};

std::string Get(std::uint16_t port, std::size_t identity) {
  const Socket socket;
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(kTimeoutSeconds);
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (connect(socket.Descriptor(), reinterpret_cast<const sockaddr *>(&address), sizeof(address)) !=
      0) {
    throw std::runtime_error("parallel fixture connect failed");
  }
  const std::string request = "GET /" + std::to_string(identity) +
                              " HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
  std::size_t sent = 0;
  while (sent < request.size()) {
    Ready(socket.Descriptor(), POLLOUT, deadline);
    const auto count =
        send(socket.Descriptor(), request.data() + sent, request.size() - sent, MSG_NOSIGNAL);
    if (count <= 0) { throw std::runtime_error("parallel fixture send failed"); }
    sent += static_cast<std::size_t>(count);
  }
  std::string response;
  std::array<char, kResponseBytes> bytes{};
  for (;;) {
    Ready(socket.Descriptor(), POLLIN, deadline);
    const auto count = recv(socket.Descriptor(), bytes.data(), bytes.size(), 0);
    if (count == 0) { break; }
    if (count < 0 || response.size() + static_cast<std::size_t>(count) > bytes.size()) {
      throw std::runtime_error("parallel fixture response failed or exceeded its bound");
    }
    response.append(bytes.data(), static_cast<std::size_t>(count));
  }
  return response;
}

class Parallel {
public:
  Parallel(std::string dsn, std::size_t count) : dsn_(std::move(dsn)), count_(count) {
    for (std::size_t at = 0; at < count_; ++at) {
      sessions_.push_back(std::make_unique<agiru::Session>(
          Identity(at), agiru::SessionOptions{.disableWriteInsideTryFunctions = at % 2 == 0}));
    }
  }

  agiru::ServerHttpResponse Handle(const agiru::ServerHttpRequest &request) {
    const auto at = static_cast<std::size_t>(std::stoul(request.target.substr(1)));
    if (at >= count_) { throw std::runtime_error("parallel fixture identity out of bounds"); }
    agiru::Connection connection(dsn_);
    agiru::SessionCommand command(*sessions_[at], connection);
    const bool wasClean = !agiru::CommitScope::Standing() && !agiru::ErrorScope::Collecting() &&
                          agiru::ErrorScope::Collected().empty() &&
                          !sessions_[at]->Transaction().IsTrying();
    const std::string own = std::to_string(at);
    {
      const agiru::CommitScope restricted(agiru::CommitBehavior::Ignore);
      const agiru::ErrorScope errors(agiru::ErrorBehavior::Collect);
      agiru::RaiseOrCollect(agiru::ErrorInfo::Create(own, true));
      const std::array<std::optional<std::string>, 1> binds{own};
      connection.Run("INSERT INTO parallel_values VALUES ($1::integer)", binds);
      if (!agiru::Tried([&] { throw agiru::Error(own); })) {
        std::unique_lock lock(mutex_);
        threads_.insert(std::this_thread::get_id());
        cpus_.insert(sched_getcpu());
        ++arrived_;
        ready_.notify_all();
        if (!ready_.wait_for(lock, std::chrono::seconds(10), [&] { return arrived_ == count_; })) {
          throw std::runtime_error("HTTP executor serialized independent sessions");
        }
      }
      const bool privateState =
          wasClean && agiru::GetLastErrorText() == own &&
          agiru::ErrorScope::Collected() == std::vector<std::string>{own} &&
          sessions_[at]->Options().disableWriteInsideTryFunctions == (at % 2 == 0) &&
          agiru::Session::Current().UserSecurityId() == Identity(at);
      agiru::ErrorScope::Clear();
      if (!privateState) { throw std::runtime_error("parallel session authority leaked"); }
    }
    if (at % 2 == 0) { command.Keep(); }
    return {.contentType = "text/plain", .body = "private:" + own, .headers = {}};
  }

  std::size_t Threads() const { return threads_.size(); }

  std::size_t Cpus() const { return cpus_.size(); }

private:
  std::string dsn_;
  std::size_t count_;
  std::vector<std::unique_ptr<agiru::Session>> sessions_;
  std::mutex mutex_;
  std::condition_variable ready_;
  std::size_t arrived_ = 0;
  std::set<std::thread::id> threads_;
  std::set<int> cpus_;
};

void NativeParallelism() {
  const std::size_t workers = agiru::DefaultHttpWorkers();
  cpu_set_t affinity{};
  if (sched_getaffinity(0, sizeof(affinity), &affinity) != 0) {
    throw std::runtime_error("native affinity inventory failed");
  }
  CHECK_TRUE("default executor uses the available affinity CPUs within its explicit ceiling",
             workers == std::min(static_cast<std::size_t>(CPU_COUNT(&affinity)),
                                 agiru::HttpServerOptions::kMaxWorkers));
  const std::size_t count = std::min(workers, std::size_t{32});
  const gate::OwnedDatabase database("parallel");
  {
    const agiru::Session seed(database.Dsn());
    agiru::CreateTable(seed.Database(), agiru::TableTraits<agiru::platform::User>::kTable);
    for (std::size_t at = 0; at < count; ++at) {
      agiru::platform::User user;
      user.UserSecurityID = Identity(at);
      user.UserName = "Parallel " + std::to_string(at);
      user.Insert();
    }
    seed.Database().Run("CREATE TABLE parallel_values (id integer PRIMARY KEY)");
    agiru::Commit();
  }
  Parallel fixture(database.Dsn(), count);
  const agiru::HttpServer server([&](const auto &request) { return fixture.Handle(request); },
                                 {.port = 0, .workers = count});
  std::vector<std::string> replies(count);
  std::vector<std::thread> clients;
  clients.reserve(count);
  for (std::size_t at = 0; at < count; ++at) {
    clients.emplace_back([&, at] {
      try {
        replies[at] = Get(server.Port(), at);
      } catch (const std::exception &error) { replies[at] = error.what(); }
    });
  }
  for (auto &client : clients) { client.join(); }
  CHECK_TRUE("independent HTTP sessions occupy every configured worker concurrently",
             fixture.Threads() == count);
  const agiru::Connection observer(database.Dsn());
  for (std::size_t at = 0; at < count; ++at) {
    CHECK_TRUE("HTTP responses retain exact session-private state",
               replies[at].starts_with("HTTP/1.1 200") &&
                   replies[at].ends_with("private:" + std::to_string(at)));
    const std::array<std::optional<std::string>, 1> binds{std::to_string(at)};
    const auto result =
        observer.Execute("SELECT 1 FROM parallel_values WHERE id = $1::integer", binds);
    CHECK_TRUE("each parallel command commits or rolls back only its own writes",
               result.Rows() == (at % 2 == 0 ? 1 : 0));
  }
  std::println("Native parallelism: {} affinity CPUs, {} concurrent workers, {} observed CPUs",
               workers,
               count,
               fixture.Cpus());
}
}

int main() {
  return gate::Run("SessionParallel", NativeParallelism);
}

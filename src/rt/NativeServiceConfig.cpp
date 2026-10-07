#include "runtime/ConnectionInfo.h"
#include "runtime/ErrorValue.h"
#include "runtime/HttpServerOptions.h"
#include "runtime/NativeService.h"
#include "runtime/PageHostOptions.h"

#include "JsonEngine.h"

#include <array>
#include <cerrno>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace agiru {
namespace {

using Node = detail::JsonNode;
constexpr std::size_t kReadBlockBytes = 4096;

[[noreturn]] void Invalid() {
  throw Error("invalid server configuration", "ServerConfiguration");
}

void Object(const Node &node, std::span<const std::string_view> fields) {
  if (!node.is_object() || node.size() != fields.size()) { Invalid(); }
  for (const auto field : fields) {
    if (!node.contains(field)) { Invalid(); }
  }
}

std::string Text(const Node &node, std::string_view field) {
  const auto &value = node[field];
  if (!value.is_string() || value.Text().empty() || value.Text().contains('\0')) { Invalid(); }
  return value.Text();
}

bool Boolean(const Node &node, std::string_view field) {
  if (!node[field].is_boolean()) { Invalid(); }
  return node[field].Boolean();
}

template <class T> T Number(const Node &node, std::string_view field) {
  const auto &value = node[field];
  if (!value.is_number_integer()) { Invalid(); }
  std::uint64_t count = 0;
  const auto &text = value.Text();
  const auto read = std::from_chars(text.data(), text.data() + text.size(), count);
  if (read.ec != std::errc{} || read.ptr != text.data() + text.size() || count == 0 ||
      count > static_cast<std::uint64_t>(std::numeric_limits<T>::max())) {
    Invalid();
  }
  return static_cast<T>(count);
}

HttpServerOptions Transport(const Node &node) {
  constexpr std::array<std::string_view, 9> fields{"port",
                                                   "loopback",
                                                   "workers",
                                                   "queue",
                                                   "connections",
                                                   "timeout_seconds",
                                                   "body_bytes",
                                                   "total_body_bytes",
                                                   "response_bytes"};
  Object(node, fields);
  HttpServerOptions options;
  options.port = Number<std::uint16_t>(node, "port");
  options.loopback = Boolean(node, "loopback");
  options.workers = node["workers"].is_string() && node["workers"].Text() == "auto"
                        ? DefaultHttpWorkers()
                        : Number<std::size_t>(node, "workers");
  options.queue = Number<std::size_t>(node, "queue");
  options.connections = Number<unsigned>(node, "connections");
  options.timeoutSeconds = Number<unsigned>(node, "timeout_seconds");
  options.bodyBytes = Number<std::size_t>(node, "body_bytes");
  options.totalBodyBytes = Number<std::size_t>(node, "total_body_bytes");
  options.responseBytes = Number<std::size_t>(node, "response_bytes");
  if (!options.loopback) { Invalid(); }
  ValidateHttpServerOptions(options);
  return options;
}

void Pages(const Node &node, PageHostOptions &options) {
  constexpr std::array<std::string_view, 5> fields{
      "contexts", "navigation_depth", "commands", "receipt_bytes", "lifetime_seconds"};
  Object(node, fields);
  options.contexts = Number<std::size_t>(node, "contexts");
  options.navigationDepth = Number<std::size_t>(node, "navigation_depth");
  options.commands = Number<std::size_t>(node, "commands");
  options.receiptBytes = Number<std::size_t>(node, "receipt_bytes");
  options.lifetime = std::chrono::seconds(Number<std::int64_t>(node, "lifetime_seconds"));
  ValidatePageHostOptions(options);
}

class File {
public:
  explicit File(const std::string &path)
      : descriptor_(open(path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK)) {
    if (descriptor_ < 0) { Invalid(); }
  }

  ~File() { close(descriptor_); }

  File(const File &) = delete;
  File &operator=(const File &) = delete;

  int Descriptor() const { return descriptor_; }

private:
  int descriptor_;
};

}

NativeServiceOptions ParseNativeServiceOptions(std::string_view text) {
  if (text.size() > NativeServiceOptions::kConfigBytes) { Invalid(); }
  try {
    const auto root = Node::parse(text, true);
    constexpr std::array<std::string_view, 7> fields{
        "schema", "database", "company", "origin", "http", "pages", "transactions"};
    Object(root, fields);
    if (Number<unsigned>(root, "schema") != 1) { Invalid(); }
    NativeServiceOptions options;
    options.pages.database = Text(root, "database");
    static_cast<void>(ConnectionInfo(options.pages.database));
    options.pages.company = Text(root, "company");
    options.pages.origin = Text(root, "origin");
    options.http = Transport(root["http"]);
    Pages(root["pages"], options.pages);
    constexpr std::array<std::string_view, 1> transactions{"disable_write_inside_try_functions"};
    Object(root["transactions"], transactions);
    options.pages.session.disableWriteInsideTryFunctions =
        Boolean(root["transactions"], "disable_write_inside_try_functions");
    return options;
  } catch (const Error &) { Invalid(); }
}

NativeServiceOptions LoadNativeServiceOptions(std::string_view path) {
  if (path.empty() || path.contains('\0')) { Invalid(); }
  const File file{std::string(path)};
  struct stat properties{};
  if (fstat(file.Descriptor(), &properties) != 0 || !S_ISREG(properties.st_mode) ||
      properties.st_size < 0 ||
      static_cast<std::uint64_t>(properties.st_size) > NativeServiceOptions::kConfigBytes) {
    Invalid();
  }
  std::string text;
  std::array<char, kReadBlockBytes> bytes{};
  for (;;) {
    const auto count = read(file.Descriptor(), bytes.data(), bytes.size());
    if (count == 0) { break; }
    if (count < 0) {
      if (errno == EINTR) { continue; }
      Invalid();
    }
    if (static_cast<std::size_t>(count) > NativeServiceOptions::kConfigBytes - text.size()) {
      Invalid();
    }
    text.append(bytes.data(), static_cast<std::size_t>(count));
  }
  return ParseNativeServiceOptions(text);
}

}

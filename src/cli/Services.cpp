#include "Services.h"

#include "runtime/ErrorValue.h"
#include "runtime/NativeService.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

namespace agiru::cli {
namespace {

using Options = std::map<std::string_view, std::string>;

Options Read(std::span<const std::string_view> arguments, std::span<const std::string_view> names) {
  Options options;
  for (std::size_t at = 0; at < arguments.size(); at += 2) {
    const auto name = arguments[at];
    if (std::ranges::find(names, name) == names.end() || at + 1 >= arguments.size() ||
        arguments[at + 1].empty() || !options.emplace(name, arguments[at + 1]).second) {
      throw Error("invalid, duplicate or missing service option " + std::string(name),
                  "ServiceInput");
    }
  }
  return options;
}

const std::string &Required(const Options &options, std::string_view name) {
  const auto found = options.find(name);
  if (found == options.end()) {
    throw Error("required option " + std::string(name), "ServiceInput");
  }
  return found->second;
}

std::uint32_t Number(const Options &options,
                     std::string_view name,
                     std::uint32_t fallback,
                     std::uint32_t maximum) {
  const auto found = options.find(name);
  if (found == options.end()) { return fallback; }
  const auto &text = found->second;
  std::uint32_t value = 0;
  const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
  if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || value == 0 ||
      value > maximum) {
    throw Error("invalid positive service bound " + std::string(name), "ServiceInput");
  }
  return value;
}

int Serve(std::span<const std::string_view> arguments) {
  constexpr std::array<std::string_view, 1> names{"--config"};
  const auto options = Read(arguments, names);
  RunNativeService(LoadNativeServiceOptions(Required(options, "--config")));
  return 0;
}

int Initialize(std::span<const std::string_view> arguments) {
  constexpr std::array<std::string_view, 1> names{"--database"};
  InitializeNativeClient(Required(Read(arguments, names), "--database"));
  return 0;
}

int Credential(std::span<const std::string_view> arguments) {
  constexpr std::array<std::string_view, 3> names{"--database", "--user", "--seconds"};
  const auto options = Read(arguments, names);
  constexpr std::uint32_t kOneHourSeconds = 3600;
  constexpr std::uint32_t kOneDaySeconds = 86400;
  const auto lifetime =
      std::chrono::seconds(Number(options, "--seconds", kOneHourSeconds, kOneDaySeconds));
  const auto secret = IssueNativeClientCredential(
      Required(options, "--database"), Required(options, "--user"), lifetime);
  std::println(R"({{"authorization":"Bearer {}"}})", secret);
  return 0;
}

}

std::optional<int> Service(std::span<const std::string_view> arguments) {
  if (arguments.empty()) { return std::nullopt; }
  const auto command = arguments.front();
  if (command == "serve") { return Serve(arguments.subspan(1)); }
  if (command == "client-init") { return Initialize(arguments.subspan(1)); }
  if (command == "client-token") { return Credential(arguments.subspan(1)); }
  return std::nullopt;
}

}

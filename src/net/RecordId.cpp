#include "type/RecordId.h"

#include "runtime/Error.h"
#include "type/Integer.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru {

Integer RecordId::TableNo() const {
  if (IsEmpty()) { return 0; }
  return static_cast<Integer>(table_.Value());
}

namespace {

constexpr char kSeparator = '\x1f';
constexpr std::string_view kNotAnId = "a stored RecordId is a table number, a caption and its key";

}

std::string RecordId::ToStorageText() const {
  if (IsEmpty()) { return {}; }
  std::string out = std::to_string(table_.Value());
  out += kSeparator;
  out += caption_;
  for (const std::string &value : key_) {
    out += kSeparator;
    out += value;
  }
  return out;
}

namespace {

bool IsSqlServersBlank(std::string_view text) {
  if (!text.starts_with("\\x")) { return false; }
  const std::string_view digits = text.substr(2);
  return !digits.empty() && digits.find_first_not_of('0') == std::string_view::npos;
}

}

std::expected<RecordId, Refusal> RecordId::FromStorageText(std::string_view text) {
  if (text.empty() || IsSqlServersBlank(text)) { return RecordId{}; }
  std::vector<std::string> parts;
  std::size_t at = 0;
  while (at <= text.size()) {
    const std::size_t next = text.find(kSeparator, at);
    parts.emplace_back(text.substr(at, next == std::string_view::npos ? next : next - at));
    if (next == std::string_view::npos) { break; }
    at = next + 1;
  }
  if (parts.size() < 3) { return std::unexpected(Refusal{.what = kNotAnId}); }
  const std::string &number = parts.front();
  if (number.empty() || number.find_first_not_of("0123456789") != std::string::npos) {
    return std::unexpected(Refusal{.what = kNotAnId});
  }
  std::vector<std::string> key(parts.begin() + 2, parts.end());
  return RecordId{TableId{static_cast<std::int32_t>(std::stol(number))}, parts[1], std::move(key)};
}

std::string RecordId::ToText() const {
  if (IsEmpty()) { return {}; }
  std::string out = caption_ + ": ";
  for (std::size_t i = 0; i < key_.size(); ++i) {
    if (i != 0) { out += ","; }
    out += key_[i];
  }
  return out;
}

}

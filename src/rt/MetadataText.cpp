#include "MetadataText.h"

#include "runtime/ErrorValue.h"
#include "type/StringValue.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

namespace agiru::detail {

std::size_t WhitespacePrefix(std::string_view value) {
  constexpr std::string_view kAsciiWhitespace = "\t\n\v\f\r ";
  constexpr std::array<std::string_view, 19> kUnicodeWhitespace{"\u0085",
                                                                "\u00A0",
                                                                "\u1680",
                                                                "\u2000",
                                                                "\u2001",
                                                                "\u2002",
                                                                "\u2003",
                                                                "\u2004",
                                                                "\u2005",
                                                                "\u2006",
                                                                "\u2007",
                                                                "\u2008",
                                                                "\u2009",
                                                                "\u200A",
                                                                "\u2028",
                                                                "\u2029",
                                                                "\u202F",
                                                                "\u205F",
                                                                "\u3000"};
  if (value.empty()) { return 0; }
  if (kAsciiWhitespace.find(value.front()) != std::string_view::npos) { return 1; }
  const auto *const found = std::ranges::find_if(
      kUnicodeWhitespace, [&](std::string_view space) { return value.starts_with(space); });
  return found == kUnicodeWhitespace.end() ? 0 : found->size();
}

bool MetadataBlank(std::string_view value) {
  while (!value.empty()) {
    const std::size_t size = WhitespacePrefix(value);
    if (size == 0) { return false; }
    value.remove_prefix(size);
  }
  return true;
}

std::string_view TrimWhitespace(std::string_view value) {
  while (const std::size_t size = WhitespacePrefix(value)) { value.remove_prefix(size); }
  constexpr std::size_t kWhitespaceUtf8Limit = 3;
  while (!value.empty()) {
    std::size_t size = 1;
    for (; size <= std::min(value.size(), kWhitespaceUtf8Limit); ++size) {
      if (WhitespacePrefix(value.substr(value.size() - size)) == size) { break; }
    }
    if (size > std::min(value.size(), kWhitespaceUtf8Limit)) { break; }
    value.remove_suffix(size);
  }
  return value;
}

std::string_view MetadataText(std::string_view value, std::size_t length) {
  if (Utf16Length(value) <= length) { return value; }
  const auto result = value.substr(0, ByteOfUnit(value, length + 1));
  if (Utf16Length(result) != length) {
    throw Error("metadata text truncation crosses an unsupported UTF-16 surrogate boundary");
  }
  return result;
}

}

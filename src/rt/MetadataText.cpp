#include "MetadataText.h"

#include "runtime/ErrorValue.h"
#include "type/StringValue.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

namespace agiru::detail {

bool MetadataBlank(std::string_view value) {
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
  while (!value.empty()) {
    if (kAsciiWhitespace.find(value.front()) != std::string_view::npos) {
      value.remove_prefix(1);
      continue;
    }
    const auto *const found = std::ranges::find_if(
        kUnicodeWhitespace, [&](std::string_view space) { return value.starts_with(space); });
    if (found == kUnicodeWhitespace.end()) { return false; }
    value.remove_prefix(found->size());
  }
  return true;
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

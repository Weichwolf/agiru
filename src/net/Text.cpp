#include "agiru/Text.h"

#include "agiru/StringValue.h"

#include <algorithm>
#include <compare>
#include <cstddef>
#include <string>
#include <string_view>

namespace agiru::detail {
namespace {
constexpr unsigned char kContinuationMask = 0xC0;
constexpr unsigned char kContinuationMark = 0x80;
constexpr unsigned char kFourByteMark = 0xF0;

bool IsAsciiDigits(std::string_view s) {
  return !s.empty() && std::ranges::all_of(s, [](char c) { return c >= '0' && c <= '9'; });
}

} // namespace

std::size_t Utf16Length(std::string_view s) {
  std::size_t units = 0;
  for (const char c : s) {
    const auto b = static_cast<unsigned char>(c);
    if ((b & kContinuationMask) == kContinuationMark) { continue; }
    units += (b >= kFourByteMark) ? 2 : 1;
  }
  return units;
}

void RaiseTooLong(std::string_view value, std::size_t actual, std::size_t max) {
  throw StringError("The length of the string is " + std::to_string(actual) +
                    ", but it must be less than or equal to " + std::to_string(max) +
                    " characters. Value: '" + std::string(value) + "'.");
}

void CheckLength(std::string_view s, std::size_t max) {
  const std::size_t actual = Utf16Length(s);
  if (actual > max) { RaiseTooLong(s, actual, max); }
}

std::string NormaliseCode(std::string_view s) {
  const std::size_t first = s.find_first_not_of(' ');
  if (first == std::string_view::npos) { return {}; }
  const std::size_t last = s.find_last_not_of(' ');
  std::string out(s.substr(first, last - first + 1));
  for (char &c : out) {
    if (c >= 'a' && c <= 'z') { c = static_cast<char>(c - ('a' - 'A')); }
  }
  return out;
}

std::strong_ordering CompareCode(std::string_view a, std::string_view b) {
  if (IsAsciiDigits(a) && IsAsciiDigits(b)) {
    const std::string_view lhs = a.substr(std::min(a.find_first_not_of('0'), a.size()));
    const std::string_view rhs = b.substr(std::min(b.find_first_not_of('0'), b.size()));
    if (lhs.size() != rhs.size()) {
      return lhs.size() < rhs.size() ? std::strong_ordering::less : std::strong_ordering::greater;
    }
    return lhs.compare(rhs) <=> 0;
  }
  return a.compare(b) <=> 0;
}

} // namespace agiru::detail

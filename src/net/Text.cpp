#include "type/Text.h"

#include "type/StringValue.h"

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
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

// THE UTF-8 DECODER'S OWN CONSTANTS, from the encoding rather than from taste: a lead byte says
// how many follow, and the low bits of each carry the code point six at a time.
constexpr std::int32_t kFourByteBits = 0x07;
constexpr unsigned char kThreeByteMark = 0xE0;
constexpr std::int32_t kThreeByteBits = 0x0F;
constexpr unsigned char kTwoByteMark = 0xC0;
constexpr std::int32_t kTwoByteBits = 0x1F;
constexpr std::int32_t kContinuationBits = 0x3F;
constexpr int kBitsPerContinuation = 6;

std::int32_t CodePointAt(std::string_view s, std::size_t unit) {
  // THE INDEX IS IN UTF-16 UNITS, because that is what AL counts and what `Utf16Length` returns.
  // A code point outside the BMP is two units, so it is refused rather than half-read: AL would
  // give a surrogate and nothing here can carry one.
  std::size_t at = 0;
  for (std::size_t i = 0; i < s.size();) {
    const auto lead = static_cast<unsigned char>(s[i]);
    std::size_t width = 1;
    std::int32_t point = lead;
    if (lead >= kFourByteMark) {
      width = 4;
      point = lead & kFourByteBits;
    } else if (lead >= kThreeByteMark) {
      width = 3;
      point = lead & kThreeByteBits;
    } else if (lead >= kTwoByteMark) {
      width = 2;
      point = lead & kTwoByteBits;
    }
    for (std::size_t k = 1; k < width && i + k < s.size(); ++k) {
      point = (point << kBitsPerContinuation) |
              (static_cast<unsigned char>(s[i + k]) & kContinuationBits);
    }
    const std::size_t units = width == 4 ? 2 : 1;
    if (unit < at + units) {
      if (units == 2) {
        throw StringError("the character at position " + std::to_string(unit + 1) +
                          " is outside the basic plane and AL would give half of it");
      }
      return point;
    }
    at += units;
    i += width;
  }
  throw StringError("the string index " + std::to_string(unit + 1) + " is outside 1.." +
                    std::to_string(at));
}

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

// A DECLARED LENGTH OF ZERO IS NO LENGTH AT ALL, which is what AL means by a bare `Text`. The page
// gives `Text[50]` a maximum and `Text` none -- a variable declared without brackets holds up to
// the platform's own limit, not up to nothing. Treating 0 as "refuse everything" would have made
// every unbounded Text variable in the BaseApp raise on its first assignment.
void CheckLength(std::string_view s, std::size_t max) {
  if (max == 0) { return; }
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

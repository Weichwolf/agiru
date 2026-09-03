#include "type/Text.h"

#include "type/Char.h"
#include "type/Integer.h"
#include "type/List.h"
#include "type/StringValue.h"

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::detail {
namespace {
constexpr unsigned char kContinuationMask = 0xC0;
constexpr unsigned char kContinuationMark = 0x80;
constexpr unsigned char kFourByteMark = 0xF0;

bool IsAsciiDigits(std::string_view s) {
  return !s.empty() && std::ranges::all_of(s, [](char c) { return c >= '0' && c <= '9'; });
}

}

constexpr std::int32_t kFourByteBits = 0x07;
constexpr unsigned char kThreeByteMark = 0xE0;
constexpr std::int32_t kThreeByteBits = 0x0F;
constexpr unsigned char kTwoByteMark = 0xC0;
constexpr std::int32_t kTwoByteBits = 0x1F;
constexpr std::int32_t kContinuationBits = 0x3F;
constexpr int kBitsPerContinuation = 6;

std::int32_t CodePointAt(std::string_view s, std::size_t unit) {
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

namespace {

constexpr std::string_view kWhitespace = " \t\n\v\f\r";

constexpr std::int32_t kOneByteLimit = 0x80;
constexpr std::int32_t kTwoByteLimit = 0x800;
constexpr std::int32_t kBasicPlaneLimit = 0x10000;

std::size_t StartByte(std::string_view s, Integer startIndex) {
  return ByteOfUnit(s, startIndex < 1 ? 1 : static_cast<std::size_t>(startIndex));
}

void AppendCodePoint(std::string &out, std::int32_t point) {
  if (point < kOneByteLimit) {
    out.push_back(static_cast<char>(point));
    return;
  }
  if (point < kTwoByteLimit) {
    out.push_back(static_cast<char>(kTwoByteMark | (point >> kBitsPerContinuation)));
    out.push_back(static_cast<char>(kContinuationMark | (point & kContinuationBits)));
    return;
  }
  if (point < kBasicPlaneLimit) {
    out.push_back(static_cast<char>(kThreeByteMark | (point >> (2 * kBitsPerContinuation))));
    out.push_back(static_cast<char>(kContinuationMark |
                                    ((point >> kBitsPerContinuation) & kContinuationBits)));
    out.push_back(static_cast<char>(kContinuationMark | (point & kContinuationBits)));
    return;
  }
  out.push_back(static_cast<char>(kFourByteMark | (point >> (3 * kBitsPerContinuation))));
  out.push_back(static_cast<char>(kContinuationMark |
                                  ((point >> (2 * kBitsPerContinuation)) & kContinuationBits)));
  out.push_back(
      static_cast<char>(kContinuationMark | ((point >> kBitsPerContinuation) & kContinuationBits)));
  out.push_back(static_cast<char>(kContinuationMark | (point & kContinuationBits)));
}

}

std::size_t ByteOfUnit(std::string_view s, std::size_t unit) {
  std::size_t at = 1;
  for (std::size_t i = 0; i < s.size(); ++i) {
    if ((static_cast<unsigned char>(s[i]) & kContinuationMask) == kContinuationMark) { continue; }
    if (at == unit) { return i; }
    at += (static_cast<unsigned char>(s[i]) >= kFourByteMark) ? 2 : 1;
  }
  return s.size();
}

std::size_t UnitOfByte(std::string_view s, std::size_t at) {
  return Utf16Length(s.substr(0, std::min(at, s.size()))) + 1;
}

Integer IndexOfText(std::string_view s, std::string_view value, Integer startIndex) {
  const std::size_t from = StartByte(s, startIndex);
  const std::size_t at = s.find(value, from);
  return at == std::string_view::npos ? 0 : static_cast<Integer>(UnitOfByte(s, at));
}

Integer LastIndexOfText(std::string_view s, std::string_view value, Integer startIndex) {
  const std::size_t upto =
      startIndex < 1 ? std::string_view::npos : ByteOfUnit(s, static_cast<std::size_t>(startIndex));
  const std::size_t at = s.rfind(value, upto);
  return at == std::string_view::npos ? 0 : static_cast<Integer>(UnitOfByte(s, at));
}

Integer IndexOfAnyText(std::string_view s, std::string_view values, Integer startIndex) {
  const std::size_t from = StartByte(s, startIndex);
  const std::size_t at = s.find_first_of(values, from);
  return at == std::string_view::npos ? 0 : static_cast<Integer>(UnitOfByte(s, at));
}

std::string PadText(std::string_view s, Integer count, PadSide side, Char pad) {
  const std::size_t have = Utf16Length(s);
  const auto want = count < 0 ? std::size_t{0} : static_cast<std::size_t>(count);
  if (have >= want) { return std::string(s); }
  std::string filling;
  for (std::size_t i = have; i < want; ++i) { AppendCodePoint(filling, pad.AsInteger()); }
  return side == PadSide::Left ? filling + std::string(s) : std::string(s) + filling;
}

std::string RemoveText(std::string_view s, Integer startIndex, std::optional<Integer> count) {
  const std::size_t from = StartByte(s, startIndex);
  if (!count.has_value()) { return std::string(s.substr(0, from)); }
  const std::size_t upto =
      *count <= 0 ? from : ByteOfUnit(s, UnitOfByte(s, from) + static_cast<std::size_t>(*count));
  return std::string(s.substr(0, from)) + std::string(s.substr(upto));
}

std::string SubstringText(std::string_view s, Integer startIndex, std::optional<Integer> count) {
  if (startIndex < 1 || static_cast<std::size_t>(startIndex) > Utf16Length(s) + 1) {
    throw StringError("the string index " + std::to_string(startIndex) + " is outside 1.." +
                      std::to_string(Utf16Length(s)));
  }
  const std::size_t from = StartByte(s, startIndex);
  if (!count.has_value()) { return std::string(s.substr(from)); }
  if (*count < 0) { throw StringError("a substring cannot have a negative length"); }
  const std::size_t upto =
      ByteOfUnit(s, static_cast<std::size_t>(startIndex) + static_cast<std::size_t>(*count));
  return std::string(s.substr(from, upto - from));
}

std::string ReplaceText(std::string_view s, Replacement what) {
  if (what.from.empty()) { return std::string(s); }
  std::string out;
  std::size_t at = 0;
  for (std::size_t hit = s.find(what.from); hit != std::string_view::npos;
       hit = s.find(what.from, at)) {
    out += s.substr(at, hit - at);
    out += what.to;
    at = hit + what.from.size();
  }
  out += s.substr(at);
  return out;
}

List<std::string> SplitText(std::string_view s, std::span<const std::string> separators) {
  List<std::string> pieces;
  std::size_t at = 0;
  while (at <= s.size()) {
    std::size_t hit = std::string_view::npos;
    std::size_t width = 0;
    if (separators.empty()) {
      hit = s.find_first_of(kWhitespace, at);
      width = 1;
    } else {
      for (const std::string &separator : separators) {
        if (separator.empty()) { continue; }
        const std::size_t found = s.find(separator, at);
        if (found < hit) {
          hit = found;
          width = separator.size();
        }
      }
    }
    if (hit == std::string_view::npos) { break; }
    pieces.Add(std::string(s.substr(at, hit - at)));
    at = hit + width;
  }
  pieces.Add(std::string(s.substr(std::min(at, s.size()))));
  return pieces;
}

std::vector<std::string> EachChar(const List<Char> &values) {
  std::vector<std::string> out;
  for (Integer i = 1; i <= values.Count(); ++i) {
    std::string one;
    AppendCodePoint(one, values.Get(i).AsInteger());
    out.push_back(one);
  }
  return out;
}

std::string TextOfChars(const List<Char> &values) {
  std::string out;
  for (Integer i = 1; i <= values.Count(); ++i) { AppendCodePoint(out, values.Get(i).AsInteger()); }
  return out;
}

std::vector<std::string> EachText(const List<std::string> &values) {
  std::vector<std::string> out;
  for (Integer i = 1; i <= values.Count(); ++i) { out.push_back(values.Get(i)); }
  return out;
}

std::string LowerText(std::string_view s) {
  std::string out(s);
  for (char &c : out) {
    if (c >= 'A' && c <= 'Z') { c = static_cast<char>(c + ('a' - 'A')); }
  }
  return out;
}

std::string UpperText(std::string_view s) {
  std::string out(s);
  for (char &c : out) {
    if (c >= 'a' && c <= 'z') { c = static_cast<char>(c - ('a' - 'A')); }
  }
  return out;
}

std::string TrimText(std::string_view s, TrimSides sides, std::string_view chars) {
  const std::string_view strip = chars.empty() ? kWhitespace : chars;
  const bool start = sides != TrimSides::End;
  const bool end = sides != TrimSides::Start;
  std::size_t from = 0;
  std::size_t upto = s.size();
  if (start) {
    const std::size_t first = s.find_first_not_of(strip);
    if (first == std::string_view::npos) { return {}; }
    from = first;
  }
  if (end) {
    const std::size_t last = s.find_last_not_of(strip);
    if (last == std::string_view::npos) { return {}; }
    upto = last + 1;
  }
  return std::string(s.substr(from, upto - from));
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

}

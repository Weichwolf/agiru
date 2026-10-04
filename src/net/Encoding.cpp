#include "dotnet/Encoding.h"

#include "dotnet/Regex.h"
#include "runtime/ErrorValue.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/StringValue.h"
#include "type/Variant.h"

#include "CodePage.h"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace agiru::dotnet {

namespace {

constexpr std::int32_t kByteMask = 0xFF;
constexpr std::int32_t kTwoByteLead = 0xC0;
constexpr std::int32_t kThreeByteLead = 0xE0;
constexpr std::int32_t kFourByteLead = 0xF0;
constexpr std::int32_t kContinuation = 0x80;
constexpr std::int32_t kSixBits = 0x3F;
constexpr std::int32_t kAsciiLimit = 0x80;
constexpr std::int32_t kSurrogateBase = 0x10000;
constexpr std::int32_t kHighSurrogate = 0xD800;
constexpr std::int32_t kLowSurrogate = 0xDC00;
constexpr std::int32_t kSurrogateMask = 0x3FF;
constexpr std::int32_t kTenBits = 10;
constexpr std::int32_t kBits6 = 6;
constexpr std::int32_t kBits12 = 12;
constexpr std::int32_t kBits18 = 18;
constexpr std::int32_t kBits8 = 8;
constexpr char kReplacement = '?';
constexpr std::int32_t kUnicodeReplacement = 0xFFFD;
constexpr std::uint32_t kLastScalar = 0x10FFFF;
constexpr std::int32_t kUtf8TwoByteLimit = 0x800;
constexpr unsigned char kFirstTwoByteLead = 0xC2;
constexpr unsigned char kLastFourByteLead = 0xF4;
constexpr unsigned char kSurrogateThreeByteLead = 0xED;
constexpr unsigned char kThreeByteSecondMinimum = 0xA0;
constexpr unsigned char kFourByteSecondBoundary = 0x90;
constexpr std::int32_t kAsciiMask = 0x7F;

void AppendUtf8(std::string &out, std::int32_t code) {
  if (code < kAsciiLimit) {
    out += static_cast<char>(code);
  } else if (code < kUtf8TwoByteLimit) {
    out += static_cast<char>(kTwoByteLead | (code >> kBits6));
    out += static_cast<char>(kContinuation | (code & kSixBits));
  } else if (code < kSurrogateBase) {
    out += static_cast<char>(kThreeByteLead | (code >> kBits12));
    out += static_cast<char>(kContinuation | ((code >> kBits6) & kSixBits));
    out += static_cast<char>(kContinuation | (code & kSixBits));
  } else {
    out += static_cast<char>(kFourByteLead | (code >> kBits18));
    out += static_cast<char>(kContinuation | ((code >> kBits12) & kSixBits));
    out += static_cast<char>(kContinuation | ((code >> kBits6) & kSixBits));
    out += static_cast<char>(kContinuation | (code & kSixBits));
  }
}

bool IsHighSurrogate(std::int32_t code) {
  return code >= kHighSurrogate && code < kLowSurrogate;
}

bool IsLowSurrogate(std::int32_t code) {
  return code >= kLowSurrogate && code <= (kLowSurrogate | kSurrogateMask);
}

std::int32_t JoinSurrogates(std::int32_t high, std::int32_t low) {
  return kSurrogateBase + ((high - kHighSurrogate) << kTenBits) + low - kLowSurrogate;
}

std::int32_t NextUtf8(std::string_view input, std::size_t &position, bool allowSurrogates) {
  const auto lead = static_cast<unsigned char>(input[position++]);
  if (lead < kAsciiLimit) { return lead; }
  if (lead < kFirstTwoByteLead || lead > kLastFourByteLead) { return kUnicodeReplacement; }
  const std::size_t length = lead < kThreeByteLead ? 2 : (lead < kFourByteLead ? 3 : 4);
  std::int32_t code = lead & (kAsciiMask >> length);
  for (std::size_t index = 1; index < length; ++index) {
    if (position == input.size()) { return kUnicodeReplacement; }
    const auto byte = static_cast<unsigned char>(input[position]);
    if (byte < kContinuation || byte > (kContinuation | kSixBits)) { return kUnicodeReplacement; }
    if (index == 1 &&
        ((lead == kThreeByteLead && byte < kThreeByteSecondMinimum) ||
         (lead == kSurrogateThreeByteLead && byte >= kThreeByteSecondMinimum && !allowSurrogates) ||
         (lead == kFourByteLead && byte < kFourByteSecondBoundary) ||
         (lead == kLastFourByteLead && byte >= kFourByteSecondBoundary))) {
      return kUnicodeReplacement;
    }
    ++position;
    code = (code << kBits6) | (byte & kSixBits);
  }
  return code;
}

std::int32_t NextTextScalar(std::string_view input, std::size_t &position) {
  const std::int32_t code = NextUtf8(input, position, true);
  if (IsHighSurrogate(code) && position < input.size()) {
    std::size_t next = position;
    const std::int32_t low = NextUtf8(input, next, true);
    if (IsLowSurrogate(low)) {
      position = next;
      return JoinSurrogates(code, low);
    }
  }
  return IsHighSurrogate(code) || IsLowSurrogate(code) ? kUnicodeReplacement : code;
}

void AppendUtf16Unit(std::string &out, std::int32_t code) {
  out += static_cast<char>(code & kByteMask);
  out += static_cast<char>((code >> kBits8) & kByteMask);
}

void AppendUtf16(std::string &out, std::int32_t code) {
  if (code >= kSurrogateBase) {
    const std::int32_t offset = code - kSurrogateBase;
    AppendUtf16Unit(out, kHighSurrogate | (offset >> kTenBits));
    AppendUtf16Unit(out, kLowSurrogate | (offset & kSurrogateMask));
    return;
  }
  AppendUtf16Unit(out, code);
}

std::int32_t Utf16Unit(std::string_view bytes, std::size_t position) {
  return static_cast<unsigned char>(bytes[position]) |
         (static_cast<unsigned char>(bytes[position + 1]) << kBits8);
}

std::string DecodeUtf16(std::string_view bytes) {
  std::string out;
  out.reserve(bytes.size());
  std::size_t position = 0;
  while (position + 1 < bytes.size()) {
    std::int32_t code = Utf16Unit(bytes, position);
    position += 2;
    if (IsHighSurrogate(code) && position + 1 < bytes.size() &&
        IsLowSurrogate(Utf16Unit(bytes, position))) {
      code = JoinSurrogates(code, Utf16Unit(bytes, position));
      position += 2;
    } else if (IsHighSurrogate(code) || IsLowSurrogate(code)) {
      code = kUnicodeReplacement;
    }
    AppendUtf8(out, code);
  }
  if (position < bytes.size()) { AppendUtf8(out, kUnicodeReplacement); }
  return out;
}

std::string DecodeUtf32(std::string_view bytes) {
  std::string out;
  out.reserve(bytes.size());
  std::size_t position = 0;
  while (position + 3 < bytes.size()) {
    std::uint32_t code = 0;
    for (std::size_t index = 0; index < 4; ++index) {
      code |= static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[position + index]))
              << (index * kBits8);
    }
    position += 4;
    if (code > kLastScalar ||
        (code >= kHighSurrogate && code <= (kLowSurrogate | kSurrogateMask))) {
      code = kUnicodeReplacement;
    }
    AppendUtf8(out, static_cast<std::int32_t>(code));
  }
  if (position < bytes.size()) { AppendUtf8(out, kUnicodeReplacement); }
  return out;
}

Array BytesToArray(std::string_view bytes) {
  Array out;
  for (const char c : bytes) {
    out.Add(Variant{Integer{static_cast<std::int32_t>(static_cast<unsigned char>(c))}});
  }
  return out;
}

std::string ArrayToBytes(const Array &bytes, Integer index, Integer count) {
  std::string out;
  const Integer length = bytes.Length();
  for (Integer i = index; i < index + count && i < length; ++i) {
    const Integer value = bytes.GetValue(i);
    out += static_cast<char>(static_cast<std::int32_t>(value) & kByteMask);
  }
  return out;
}

}

class Encoding Encoding::Binder::operator()() const {
  return Encoding::UTF8();
}

class Encoding Encoding::UTF8() {
  return Made(kUtf8, true);

}

class Encoding
Encoding::Unicode() {

  return Made(kUtf16, true);
}

class Encoding Encoding::ASCII() {
  return Made(kAscii, false);

}

class Encoding
Encoding::Default() {

  return Made(kUtf8, false);
}

class Encoding Encoding::UTF32() {
  return Made(kUtf32, true);

}

class Encoding
Encoding::GetEncoding(Integer codePage) {

  const std::int32_t page = codePage;
  if (page == kDefault) { return Made(kWindows1252, false); }
  if (page == kUtf8) { return UTF8(); }
  if (page == kUtf16) { return Made(kUtf16, true); }
  if (page == kUtf32) { return Made(kUtf32, true); }
  if (FindSingleByteCodePage(page) != nullptr) { return Made(page, false); }
  throw Error("Encoding.GetEncoding: code page " + std::to_string(page) +
              " has no implementation (board:0035)");
}

class Encoding Encoding::GetEncoding(std::string_view name) {
  std::string lowered;
  for (const char c : name) {
    lowered += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  if (lowered == "utf-8") { return UTF8(); }
  if (lowered == "utf-16" || lowered == "unicode" || lowered == "utf-16le") { return Unicode(); }
  if (lowered == "utf-32" || lowered == "utf-32le") { return UTF32(); }
  if (lowered == "us-ascii" || lowered == "ascii") { return ASCII(); }
  if (lowered == "windows-1252" || lowered == "cp1252" || lowered == "x-ansi") {
    return GetEncoding(Integer{kWindows1252});
  }
  if (lowered == "iso-8859-1" || lowered == "latin1" || lowered == "cp819") {
    return GetEncoding(Integer{kLatin1});
  }
  throw Error("Encoding.GetEncoding: the encoding " + std::string(name) + " is not one known here");
}

Array Encoding::Convert(const class Encoding &from, const class Encoding &to, const Array &bytes) {
  return BytesToArray(to.Encode(from.Decode(ArrayToBytes(bytes, 0, bytes.Length()))));
}

std::string Encoding::Encode(std::string_view text) const {
  const SingleByteCodePage *single = FindSingleByteCodePage(codePage_);
  const bool unicode = codePage_ == kUtf8 || codePage_ == kUnset || codePage_ == kDefault ||
                       codePage_ == kUtf16 || codePage_ == kUtf32;
  if (!unicode && single == nullptr) {
    throw Error("Encoding.Encode: code page " + std::to_string(codePage_) +
                " has no implementation (board:0035)");
  }
  std::string out;
  out.reserve(text.size());
  for (std::size_t position = 0; position < text.size();) {
    const std::int32_t code = NextTextScalar(text, position);
    if (codePage_ == kUtf8 || codePage_ == kUnset || codePage_ == kDefault) {
      AppendUtf8(out, code);
      continue;
    }
    if (codePage_ == kUtf32) {
      for (int shift = 0; shift < kBits8 * 4; shift += kBits8) {
        out += static_cast<char>((code >> shift) & kByteMask);
      }
      continue;
    }
    if (codePage_ == kUtf16) {
      AppendUtf16(out, code);
      continue;
    }
    if (code >= kSurrogateBase) {
      out.append(2, kReplacement);
    } else {
      out += static_cast<char>(EncodeSingleByteUnit(*single, code));
    }
  }
  return out;
}

std::string Encoding::Decode(std::string_view bytes) const {
  if (codePage_ == kUtf8 || codePage_ == kUnset || codePage_ == kDefault) {
    std::string out;
    out.reserve(bytes.size());
    for (std::size_t position = 0; position < bytes.size();) {
      AppendUtf8(out, NextUtf8(bytes, position, false));
    }
    return out;
  }
  std::string out;
  if (codePage_ == kUtf32) { return DecodeUtf32(bytes); }
  if (codePage_ == kUtf16) { return DecodeUtf16(bytes); }
  const SingleByteCodePage *single = FindSingleByteCodePage(codePage_);
  if (single == nullptr) {
    throw Error("Encoding.Decode: code page " + std::to_string(codePage_) +
                " has no implementation (board:0035)");
  }
  out.reserve(bytes.size());
  for (const char c : bytes) {
    const auto byte = static_cast<unsigned char>(c);
    AppendUtf8(out, single->decode[byte]);
  }
  return out;
}

Array Encoding::GetBytes(std::string_view text) const {
  return BytesToArray(Encode(text));
}

Integer Encoding::GetByteCount(std::string_view text) const {
  return static_cast<Integer>(Encode(text).size());
}

::agiru::Text<0> Encoding::GetString(const Array &bytes) const {
  return GetString(bytes, 0, bytes.Length());
}

::agiru::Text<0> Encoding::GetString(const Array &bytes, Integer index, Integer count) const {
  return ::agiru::Text<0>{Decode(ArrayToBytes(bytes, index, count))};
}

Array Encoding::GetChars(const Array &bytes) const {
  return GetChars(bytes, 0, bytes.Length());
}

Array Encoding::GetChars(const Array &bytes, Integer index, Integer count) const {
  Array out;
  const std::string text = Decode(ArrayToBytes(bytes, index, count));
  for (std::size_t position = 0; position < text.size();) {
    const std::int32_t code = NextUtf8(text, position, false);
    if (code >= kSurrogateBase) {
      const std::int32_t offset = code - kSurrogateBase;
      out.Add(Variant{Integer{kHighSurrogate | (offset >> kTenBits)}});
      out.Add(Variant{Integer{kLowSurrogate | (offset & kSurrogateMask)}});
    } else {
      out.Add(Variant{Integer{code}});
    }
  }
  return out;
}

Array Encoding::GetBytes(const Array &chars, Integer index, Integer count) const {
  std::string text;
  const Integer length = chars.Length();
  for (Integer i = index; i < index + count && i < length; ++i) {
    const Integer code = chars.GetValue(i);
    AppendUtf8(text, static_cast<std::int32_t>(code));
  }
  return BytesToArray(Encode(text));
}

Integer Encoding::GetBytes(
    const Array &chars, Integer index, Integer count, Array &bytes, Integer byteIndex) const {
  const Array made = GetBytes(chars, index, count);
  const Integer length = made.Length();
  for (Integer i = 0; i < length; ++i) {
    const Integer at = byteIndex + i;
    if (at >= bytes.Length()) { break; }
    bytes.SetValue(made.GetValue(i), at);
  }
  return length;
}

Array Encoding::GetPreamble() const {
  if (!preamble_) { return {}; }
  if (codePage_ == kUtf8) { return BytesToArray("\xEF\xBB\xBF"); }
  if (codePage_ == kUtf16) { return BytesToArray("\xFF\xFE"); }
  if (codePage_ == kUtf32) { return BytesToArray(std::string_view("\xFF\xFE\0\0", 4)); }
  return {};
}

::agiru::Text<0> Encoding::WebName() const {
  if (codePage_ == kUtf16) { return ::agiru::Text<0>{"utf-16"}; }
  if (codePage_ == kUtf32) { return ::agiru::Text<0>{"utf-32"}; }
  if (codePage_ == kAscii) { return ::agiru::Text<0>{"us-ascii"}; }
  if (codePage_ == kLatin1) { return ::agiru::Text<0>{"iso-8859-1"}; }
  if (codePage_ == kUtf8 || codePage_ == kUnset || codePage_ == kDefault) {
    return ::agiru::Text<0>{"utf-8"};
  }
  return ::agiru::Text<0>{"windows-" + std::to_string(codePage_)};
}

class UTF8Encoding UTF8Encoding::Binder::operator()(Boolean emitBom) const {
  class UTF8Encoding out;
  static_cast<class Encoding &>(out) = Encoding::Made(Encoding::kUtf8, static_cast<bool>(emitBom));
  return out;
}

class UnicodeEncoding UnicodeEncoding::Binder::operator()() const {
  class UnicodeEncoding out;
  static_cast<class Encoding &>(out) = Encoding::Unicode();
  return out;
}

class ASCIIEncoding ASCIIEncoding::Binder::operator()() const {
  class ASCIIEncoding out;
  static_cast<class Encoding &>(out) = Encoding::ASCII();
  return out;
}

}

#include "dotnet/Encoding.h"

#include "dotnet/Regex.h"
#include "runtime/Error.h"
#include "type/Integer.h"
#include "type/Text.h"
#include "type/Variant.h"

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
constexpr std::int32_t kLatinLimit = 0x100;
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

void AppendUtf8(std::string &out, std::int32_t code) {
  if (code < kAsciiLimit) {
    out += static_cast<char>(code);
  } else if (code < (1 << (kBits6 + 5))) {
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

std::vector<std::int32_t> CodePointsOf(std::string_view utf8) {
  std::vector<std::int32_t> points;
  for (std::size_t i = 0; i < utf8.size();) {
    const auto lead = static_cast<unsigned char>(utf8[i]);
    std::int32_t code = lead;
    std::size_t more = 0;
    if (lead >= kFourByteLead) {
      code = lead & 0x07;
      more = 3;
    } else if (lead >= kThreeByteLead) {
      code = lead & 0x0F;
      more = 2;
    } else if (lead >= kTwoByteLead) {
      code = lead & 0x1F;
      more = 1;
    }
    ++i;
    for (std::size_t k = 0; k < more && i < utf8.size(); ++k, ++i) {
      code = (code << kBits6) | (static_cast<unsigned char>(utf8[i]) & kSixBits);
    }
    points.push_back(code);
  }
  return points;
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

class Encoding Encoding::Unicode() {
  return Made(kUtf16, true);
}

class Encoding Encoding::ASCII() {
  return Made(kAscii, false);
}

class Encoding Encoding::Default() {
  return Made(kUtf8, false);
}

class Encoding Encoding::UTF32() {
  return Made(kUtf32, true);
}

class Encoding Encoding::GetEncoding(Integer codePage) {
  const std::int32_t page = codePage;
  if (page == kDefault || page == kUtf8) { return Made(kUtf8, false); }
  if (page == kUtf16) { return Made(kUtf16, true); }
  if (page == kUtf32) { return Made(kUtf32, true); }
  return Made(page, false);
}

class Encoding Encoding::GetEncoding(std::string_view name) {
  std::string lowered;
  for (const char c : name) {
    lowered += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  if (lowered == "utf-8" || lowered == "utf8") { return Made(kUtf8, false); }
  if (lowered == "utf-16" || lowered == "unicode") { return Made(kUtf16, true); }
  if (lowered == "us-ascii" || lowered == "ascii") { return Made(kAscii, false); }
  if (lowered.starts_with("windows-") || lowered.starts_with("iso-8859-")) {
    return Made(kWindows1252, false);
  }
  throw Error("Encoding.GetEncoding: the encoding " + std::string(name) + " is not one known here");
}

Array Encoding::Convert(const class Encoding &from, const class Encoding &to, const Array &bytes) {
  return BytesToArray(to.Encode(from.Decode(ArrayToBytes(bytes, 0, bytes.Length()))));
}

std::string Encoding::Encode(std::string_view text) const {
  if (codePage_ == kUtf8 || codePage_ == kUnset || codePage_ == kDefault) {
    return std::string(text);
  }
  std::string out;
  for (const std::int32_t code : CodePointsOf(text)) {
    if (codePage_ == kUtf32) {
      for (int shift = 0; shift < kBits8 * 4; shift += kBits8) {
        out += static_cast<char>((code >> shift) & kByteMask);
      }
      continue;
    }
    if (codePage_ == kUtf16) {
      if (code >= kSurrogateBase) {
        const std::int32_t offset = code - kSurrogateBase;
        const std::int32_t high = kHighSurrogate | (offset >> kTenBits);
        const std::int32_t low = kLowSurrogate | (offset & kSurrogateMask);
        for (const std::int32_t unit : {high, low}) {
          out += static_cast<char>(unit & kByteMask);
          out += static_cast<char>((unit >> kBits8) & kByteMask);
        }
      } else {
        out += static_cast<char>(code & kByteMask);
        out += static_cast<char>((code >> kBits8) & kByteMask);
      }
      continue;
    }
    const std::int32_t limit = codePage_ == kAscii ? kAsciiLimit : kLatinLimit;
    out += code < limit ? static_cast<char>(code) : kReplacement;
  }
  return out;
}

std::string Encoding::Decode(std::string_view bytes) const {
  if (codePage_ == kUtf8 || codePage_ == kUnset || codePage_ == kDefault) {
    return std::string(bytes);
  }
  std::string out;
  if (codePage_ == kUtf32) {
    for (std::size_t i = 0; i + 3 < bytes.size(); i += 4) {
      std::int32_t code = 0;
      for (int k = 3; k >= 0; --k) {
        code = (code << kBits8) | static_cast<unsigned char>(bytes[i + static_cast<std::size_t>(k)]);
      }
      AppendUtf8(out, code);
    }
    return out;
  }
  if (codePage_ == kUtf16) {
    for (std::size_t i = 0; i + 1 < bytes.size(); i += 2) {
      std::int32_t unit = static_cast<unsigned char>(bytes[i]) |
                          (static_cast<unsigned char>(bytes[i + 1]) << kBits8);
      if (unit >= kHighSurrogate && unit < kLowSurrogate && i + 3 < bytes.size()) {
        const std::int32_t low = static_cast<unsigned char>(bytes[i + 2]) |
                                 (static_cast<unsigned char>(bytes[i + 3]) << kBits8);
        unit = kSurrogateBase + (((unit & kSurrogateMask) << kTenBits) | (low & kSurrogateMask));
        i += 2;
      }
      AppendUtf8(out, unit);
    }
    return out;
  }
  for (const char c : bytes) { AppendUtf8(out, static_cast<unsigned char>(c)); }
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
  Array out;
  for (const std::int32_t code : CodePointsOf(Decode(ArrayToBytes(bytes, 0, bytes.Length())))) {
    out.Add(Variant{Integer{code}});
  }
  return out;
}

Array Encoding::GetChars(const Array &bytes, Integer index, Integer count) const {
  Array out;
  for (const std::int32_t code : CodePointsOf(Decode(ArrayToBytes(bytes, index, count)))) {
    out.Add(Variant{Integer{code}});
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

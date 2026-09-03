#include "type/Guid.h"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <string_view>

namespace agiru {

namespace {

constexpr std::array<std::size_t, 5> kGroups{4, 2, 2, 2, 6};
constexpr int kHexPerByte = 2;
constexpr unsigned kBitsPerByte = 8;
constexpr unsigned kBitsPerNibble = 4;
constexpr std::uint8_t kByteMask = 0xFF;
constexpr std::uint8_t kNibble = 0x0F;
constexpr std::uint8_t kVariantMask = 0x3F;
constexpr std::uint8_t kVariantBits = 0x80;
constexpr std::uint8_t kVersion4 = 4;
constexpr std::uint8_t kVersion7 = 7;
constexpr std::size_t kVersionByte = 6;
constexpr std::size_t kVariantByte = 8;
constexpr std::size_t kTimestampBytes = 6;
constexpr int kDecimalDigits = 10;

std::uint64_t Random() {
  thread_local std::mt19937_64 engine{std::random_device{}()};
  return engine();
}

void FillRandom(std::array<std::uint8_t, Guid::kSize> &bytes, std::size_t from) {
  std::uint64_t word = 0;
  for (std::size_t i = from; i < bytes.size(); ++i) {
    if ((i - from) % sizeof(word) == 0) { word = Random(); }
    bytes[i] = static_cast<std::uint8_t>(word & kByteMask);
    word >>= kBitsPerByte;
  }
}

void StampVersion(std::array<std::uint8_t, Guid::kSize> &bytes, std::uint8_t version) {
  bytes[kVersionByte] =
      static_cast<std::uint8_t>((bytes[kVersionByte] & kNibble) | (version << kBitsPerNibble));
  bytes[kVariantByte] =
      static_cast<std::uint8_t>((bytes[kVariantByte] & kVariantMask) | kVariantBits);
}

int HexValue(char c) {
  if (c >= '0' && c <= '9') { return c - '0'; }
  if (c >= 'a' && c <= 'f') { return (c - 'a') + kDecimalDigits; }
  if (c >= 'A' && c <= 'F') { return (c - 'A') + kDecimalDigits; }
  return -1;
}

std::string Hyphenated(const std::array<std::uint8_t, Guid::kSize> &bytes) {
  static constexpr std::string_view kDigits = "0123456789abcdef";
  std::string out;
  out.reserve((Guid::kSize * kHexPerByte) + 4);
  std::size_t at = 0;
  for (const std::size_t group : kGroups) {
    if (at != 0) { out += '-'; }
    for (std::size_t i = 0; i < group; ++i, ++at) {
      out += kDigits[(bytes[at] >> kBitsPerNibble) & kNibble];
      out += kDigits[bytes[at] & kNibble];
    }
  }
  return out;
}

}

Guid Guid::Create() {
  std::array<std::uint8_t, kSize> bytes{};
  FillRandom(bytes, 0);
  StampVersion(bytes, kVersion4);
  return Guid{bytes};
}

Guid Guid::CreateSequential() {
  std::array<std::uint8_t, kSize> bytes{};
  const auto now =
      static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                     std::chrono::system_clock::now().time_since_epoch())
                                     .count());
  for (std::size_t i = 0; i < kTimestampBytes; ++i) {
    bytes[i] =
        static_cast<std::uint8_t>((now >> ((kTimestampBytes - 1 - i) * kBitsPerByte)) & kByteMask);
  }
  FillRandom(bytes, kTimestampBytes);
  StampVersion(bytes, kVersion7);
  return Guid{bytes};
}

Guid Guid::FromText(std::string_view text) {
  if (!text.empty() && text.front() == '{' && text.back() == '}') {
    text.remove_prefix(1);
    text.remove_suffix(1);
  }
  std::array<std::uint8_t, kSize> bytes{};
  std::size_t at = 0;
  std::size_t cursor = 0;
  while (at < kSize && cursor + 1 < text.size()) {
    if (text[cursor] == '-') {
      ++cursor;
      continue;
    }
    const int high = HexValue(text[cursor]);
    const int low = HexValue(text[cursor + 1]);
    if (high < 0 || low < 0) { return Guid{}; }
    bytes[at] = static_cast<std::uint8_t>((high << kBitsPerNibble) | low);
    ++at;
    cursor += kHexPerByte;
  }
  return at == kSize ? Guid{bytes} : Guid{};
}

std::string Guid::ToText() const {
  return "{" + Hyphenated(bytes_) + "}";
}

std::string Guid::ToStorageText() const {
  return Hyphenated(bytes_);
}

}

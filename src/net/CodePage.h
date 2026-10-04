#pragma once

#include <cstdint>
#include <span>

namespace agiru::dotnet {

struct CodePageMapping {
  std::uint16_t unit;
  std::uint8_t byte;
};

struct SingleByteCodePage {
  std::int32_t number;
  std::span<const CodePageMapping> encode;
  std::span<const std::uint16_t> decode;
  bool asciiIdentity;
};

const SingleByteCodePage *FindSingleByteCodePage(std::int32_t number);
std::uint8_t EncodeSingleByteUnit(const SingleByteCodePage &page, std::int32_t unit);

}

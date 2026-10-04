#pragma once

#include "runtime/ErrorValue.h"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace gate {

/// \brief Split a tab-separated reference row with an exact field count.
/// \param row The unchanged row. \return Views borrowing the row.
/// \throws agiru::Error on a missing or extra field.
template <std::size_t Count>
std::array<std::string_view, Count> ReferenceFields(std::string_view row) {
  static_assert(Count != 0);
  std::array<std::string_view, Count> fields{};
  std::size_t start = 0;
  for (std::size_t index = 0; index < Count; ++index) {
    const std::size_t end = row.find('\t', start);
    if ((index != Count - 1 && end == std::string_view::npos) ||
        (index == Count - 1 && end != std::string_view::npos)) {
      throw agiru::Error("wrong reference field count");
    }
    fields[index] = row.substr(start, end == std::string_view::npos ? end : end - start);
    if (end != std::string_view::npos) { start = end + 1; }
  }
  return fields;
}

/// \brief Decode one uppercase hexadecimal digit. \param value The digit.
/// \return Its numeric value. \throws agiru::Error on a nonhexadecimal digit.
inline unsigned HexDigit(char value) {
  if (value >= '0' && value <= '9') { return static_cast<unsigned>(value - '0'); }
  if (value >= 'A' && value <= 'F') { return static_cast<unsigned>(value - 'A') + 10U; }
  throw agiru::Error("invalid hexadecimal reference fixture");
}

/// \brief Decode an exact byte sequence, including NULs. \param hex Uppercase hexadecimal.
/// \return The bytes. \throws agiru::Error on a malformed or incomplete byte.
inline std::string Unhex(std::string_view hex) {
  if (hex.size() % 2 != 0) { throw agiru::Error("incomplete hexadecimal reference fixture"); }
  std::string result;
  result.reserve(hex.size() / 2);
  for (std::size_t index = 0; index < hex.size(); index += 2) {
    result.push_back(static_cast<char>((HexDigit(hex[index]) << 4U) | HexDigit(hex[index + 1])));
  }
  return result;
}

}

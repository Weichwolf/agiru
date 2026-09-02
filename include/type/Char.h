#pragma once

#include "type/Integer.h"

#include <cstdint>
#include <string>

/// \file
/// \brief AL `Char` -- one character.

namespace agiru {

/// \brief AL `Char`.
///
/// From `char-data-type.md`: a single character. AL converts it to and from Integer freely --
/// `ConvertStr` and `XMLDOMManagement.IsValidXMLNameStartCharacter` both do -- so the conversion is
/// explicit here rather than a separate method, and the value is the CODE POINT.
///
/// \note IT IS A CODE POINT AND NOT A BYTE. AL text is UTF-16 to the platform and a `Char` holds
/// one
///       unit of it; a `char` would lose every character above 127, which is most of a European
///       BaseApp's captions.
class Char {
public:
  /// \brief The character with code point zero.
  constexpr Char() = default;

  /// \brief The character with a given code point.
  /// \param code The code point.
  constexpr explicit Char(std::int32_t code) : code_(code) {}

  /// \brief The code point.
  /// \return It.
  [[nodiscard]] constexpr Integer AsInteger() const { return code_; }

  /// \brief Compares two characters.
  /// \param o The other.
  /// \return How they order.
  [[nodiscard]] constexpr auto operator<=>(const Char &o) const = default;

private:
  std::int32_t code_ = 0;
};

} // namespace agiru

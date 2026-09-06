#pragma once

#include "runtime/Error.h"
#include "type/Integer.h"

#include <compare>
#include <cstdint>
#include <string>
#include <string_view>

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

  /// \brief AL passes `'+'` where a `Char` is declared -- a one-character text IS a character.
  /// \param text The text, whose single character is taken.
  /// \throws Error when the text is not exactly one character.
  constexpr explicit(false) Char(std::string_view text)
      : code_(text.size() == 1 ? static_cast<std::int32_t>(static_cast<unsigned char>(text.front()))
                               : throw Error("A text of length " + std::to_string(text.size()) +
                                             " is not one character")) {}

  /// \brief AL passes a one-character literal where a `Char` is declared.
  /// \tparam N The literal's length, one character and its terminator.
  /// \param text The literal.
  template <std::size_t N>
  constexpr explicit(false) Char(const char (&text)[N]) : Char(std::string_view(text, N - 1)) {}

  /// \brief Assigns a code point.
  ///
  /// \param code The code point.
  /// \return This character.
  ///
  /// \note AL ASSIGNS AN INTEGER TO A CHAR, which is the other half of the conversion its own page
  ///       describes: `CRLF[1] := 13` is how a body builds a line break.
  /// \brief AL `Char := Text[Index]` -- the character standing at a position.
  /// \tparam P The position's type.
  /// \param position The position, which reads as a character.
  /// \return This character.
  ///
  /// \note IT TAKES THE POSITION BY ITS MARKER, so the assignment is one overload rather than an
  ///       ambiguity between the position's conversion to a `Char` and to a code point.
  template <typename P>
    requires requires(const P &at) { typename P::IsATextPosition; }
  constexpr Char &operator=(const P &position) {
    *this = static_cast<Char>(position);
    return *this;
  }

  /// \brief AL `Char := 'x'` -- a one-character text.
  /// \param text The text, whose single character is taken.
  /// \return This character.
  /// \throws Error when the text is not exactly one character, which is what AL raises.
  constexpr Char &operator=(std::string_view text) {
    if (text.size() != 1) {
      throw Error("A text of length " + std::to_string(text.size()) + " is not one character");
    }
    code_ = static_cast<std::int32_t>(static_cast<unsigned char>(text.front()));
    return *this;
  }

  /// \brief AL `Char := "x"` where the literal is a character array.
  /// \tparam N The array's length, one character and its terminator.
  /// \param text The literal.
  /// \return This character.
  template <std::size_t N> constexpr Char &operator=(const char (&text)[N]) {
    return *this = std::string_view(text, N - 1);
  }

  constexpr Char &operator=(std::int32_t code) {
    code_ = code;
    return *this;
  }

  /// \brief The code point.
  /// \return It.
  [[nodiscard]] constexpr Integer AsInteger() const { return code_; }

  /// \brief AL `Integer := Char` -- a Char is its code where an Integer is asked for.
  /// \return The code.
  [[nodiscard]] constexpr operator std::int32_t() const { return code_; }

  /// \brief Compares two characters.
  /// \param o The other.
  /// \return How they order.
  [[nodiscard]] constexpr auto operator<=>(const Char &o) const = default;

  /// \brief AL `Char >= '0'`: a Char against a one-character text literal, which AL reads as a
  ///        Char. A longer text refuses, the way AL's conversion does.
  /// \param text The literal.
  /// \return The ordering of this against the literal's one character.
  [[nodiscard]] constexpr std::strong_ordering operator<=>(std::string_view text) const {
    return code_ <=> OneOf(text);
  }

  /// \brief AL `Char = 'a'`.
  /// \param text The literal.
  /// \return Whether it is that one character.
  [[nodiscard]] constexpr bool operator==(std::string_view text) const {
    return code_ == OneOf(text);
  }

private:
  static constexpr std::int32_t OneOf(std::string_view text) {
    if (text.size() != 1) {
      throw Error("a Char is compared with a text that is not one character: '" +
                  std::string(text) + "'");
    }
    return static_cast<unsigned char>(text[0]);
  }

  std::int32_t code_ = 0;
};

/// \brief AL `Text + Char` -- the character is appended.
/// \param text The text.
/// \param character The character.
/// \return The text with the character on the end.
///
/// \note WITHOUT IT THE `+` WAS POINTER ARITHMETIC. A `Char` converts to its code point, so
///       `std::string + Char` read as "advance a pointer" and the diagnostic named the operands
///       rather than the AL line that concatenated them.
[[nodiscard]] inline std::string operator+(std::string_view text, Char character) {
  std::string out(text);
  out += static_cast<char>(static_cast<std::int32_t>(character));
  return out;
}

/// \brief AL `Char + Text` -- the character comes first.
/// \param character The character.
/// \param text The text.
/// \return The character with the text after it.
[[nodiscard]] inline std::string operator+(Char character, std::string_view text) {
  std::string out(1, static_cast<char>(static_cast<std::int32_t>(character)));
  out += text;
  return out;
}

/// \brief AL `Text + Char` where the left side is already a `std::string`.
/// \param text The text.
/// \param character The character.
/// \return The text with the character on the end.
[[nodiscard]] inline std::string operator+(const std::string &text, Char character) {
  return std::string_view(text) + character;
}
}

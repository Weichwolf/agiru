#pragma once

#include "dotnet/Refused.h"
#include "dotnet/Regex.h"
#include "type/Boolean.h"
#include "type/Char.h"
#include "type/Integer.h"
#include "type/Text.h"

#include <concepts>
#include <string>
#include <string_view>

namespace agiru::dotnet {

/// \brief .NET `System.String`, rebuilt as the BaseApp uses it: a text a `DotNet String` variable
///        is assigned from, whose members split, trim, pad, search and slice it, and which reads
///        back as AL `Text` wherever a text is wanted.
///
/// \note THE BASEAPP DECLARES 30 OF THEM and calls 20 members (measured 2026-09-12): `Split` and
///       `ToCharArray` between them (`Office Line Generation`, `Office Add-in`, `File Management`,
///       `DotNet_String`), `Replace`, `Trim`, the pads, the searches, `Substring`, `Normalize`.
///       What it never calls is not here: `Format` with a culture stays a refusal by name
///       (`Type Helper` formats a date and a decimal through it, board:0035), and so does
///       `GetType`.
///
/// \note INDEXES ARE ZERO-BASED AND COUNT UTF-8 BYTES, not UTF-16 code units: the text the
///       BaseApp hands here is ASCII (file names, separators, document numbers), and a `Substring`
///       over a multi-byte character would be the first place that difference shows.
///
/// \note `Normalize` IS THE IDENTITY. Unicode normalisation is a table this runtime does not
///       carry (ICU would be the dependency); the BaseApp normalises file names before comparing
///       them, and an identity keeps every comparison that held before.
class String {
public:
  /// \brief The binder behind `DotNetString := DotNetString.String(...)`, the constructor as AL
  ///        spells it.
  struct Binder {
    /// \brief `new String(chars)`: the characters, in order. \param chars An array of `Char`s.
    /// \return The string.
    [[nodiscard]] class String operator()(const Array &chars) const;
    /// \brief `new String(text)`. \param text The text. \return The string.
    [[nodiscard]] class String operator()(std::string_view text) const;
    /// \brief `new String(x)` over a value this runtime does not carry. \param refused The value.
    /// \return Never.
    /// \throws Error naming the refused member.
    [[nodiscard]] class String operator()(const Refused &refused) const;
  };

  /// \brief `String.String(...)`, the constructor as AL calls it.
  Binder String;

  /// \brief AL `DotNetString := Text`. \param text The text. \return This string.
  class String &operator=(std::string_view text) {
    value_ = text;
    return *this;
  }

  /// \brief AL `DotNetString := Text`, from a `Text` or `Code` of any length.
  /// \tparam N The declared length. \param text The text. \return This string.
  template <std::size_t N> class String &operator=(const ::agiru::Text<N> &text) {
    value_ = std::string_view(text);
    return *this;
  }

  /// \brief AL `DotNetString := Text`, from a literal. \param text The literal.
  /// \return This string.
  class String &operator=(const char *text) {
    value_ = text == nullptr ? std::string_view{} : std::string_view(text);
    return *this;
  }

  /// \brief AL `DotNetString := Absent.Member`, from a value this runtime does not carry.
  /// \param refused The value. \return Never.
  /// \throws Error naming the refused member.
  class String &operator=(const Refused &refused) {
    static_cast<void>(refused());
    return *this;
  }

  /// \brief What AL reads wherever it puts the string into a `Text`: the text.
  /// \return The characters.
  operator std::string_view() const { return std::string_view(value_); } // NOLINT(*-explicit-constructor)

  /// \brief AL `Proc(var Param: Text)` given a `DotNet String`, which AL allows because a .NET
  ///        string IS a text there: the callee writes into this string's own text.
  /// \return The text, by reference.
  operator ::agiru::Text<0> &() { return value_; } // NOLINT(*-explicit-constructor)

  /// \brief The same, read-only. \return The text.
  operator const ::agiru::Text<0> &() const { return value_; } // NOLINT(*-explicit-constructor)

  /// \brief `String.ToString()`. \return The text.
  [[nodiscard]] ::agiru::Text<0> ToString() const { return value_; }

  /// \brief `String.Length`. \return How many characters.
  [[nodiscard]] ::agiru::Integer Length() const {
    return static_cast<::agiru::Integer>(std::string_view(value_).size());
  }

  /// \brief `String.Chars(index)`, zero-based. \param index The position. \return The character.
  /// \throws Error when the index is outside the string.
  [[nodiscard]] ::agiru::Char Chars(::agiru::Integer index) const;

  /// \brief `String.Split(separators)`: the pieces between any of the separator characters, empty
  ///        pieces kept, the way `Split(char[])` keeps them.
  /// \param separators An array of `Char`s, each one text.
  /// \return An array of texts.
  [[nodiscard]] Array Split(const Array &separators) const;

  /// \brief `String.Split(separators)` with the separator characters as ONE text, which is what
  ///        `Path.GetInvalidFileNameChars()` answers here. \param separators The characters.
  /// \return An array of texts.
  [[nodiscard]] Array Split(std::string_view separators) const;

  /// \brief `String.Split(separator)`. \param separator One character. \return An array of texts.
  /// \note A TEMPLATE CONSTRAINED TO `Char`, so that a text literal -- which `Char` also
  ///       accepts -- goes to the text overload rather than being ambiguous between the two.
  template <typename C>
    requires std::same_as<C, ::agiru::Char>
  [[nodiscard]] Array Split(C separator) const {
    return Split(std::string_view(::agiru::Encoded(separator)));
  }

  /// \brief `String.ToCharArray()`. \return An array with one text per character.
  [[nodiscard]] Array ToCharArray() const;

  /// \brief `String.ToCharArray(startIndex, length)`. \param startIndex Zero-based.
  /// \param length How many. \return An array with one text per character.
  /// \throws Error when the range is outside the string.
  [[nodiscard]] Array ToCharArray(::agiru::Integer startIndex, ::agiru::Integer length) const;

  /// \brief `String.Replace(oldValue, newValue)`: every occurrence. \param oldValue What to find.
  /// \param newValue What to put. \return The new string.
  /// \throws Error when `oldValue` is empty, as .NET does.
  [[nodiscard]] class String Replace(std::string_view oldValue, std::string_view newValue) const;

  /// \brief `String.Trim()`: white space off both ends. \return The new string.
  [[nodiscard]] class String Trim() const;

  /// \brief `String.Trim(chars)`. \param chars The characters to take off. \return The new string.
  [[nodiscard]] class String Trim(const Array &chars) const;

  /// \brief `String.TrimStart()`. \return The new string.
  [[nodiscard]] class String TrimStart() const;

  /// \brief `String.TrimStart(chars)`. \param chars The characters to take off.
  /// \return The new string.
  [[nodiscard]] class String TrimStart(const Array &chars) const;

  /// \brief `String.TrimEnd()`. \return The new string.
  [[nodiscard]] class String TrimEnd() const;

  /// \brief `String.TrimEnd(chars)`. \param chars The characters to take off.
  /// \return The new string.
  [[nodiscard]] class String TrimEnd(const Array &chars) const;

  /// \brief `String.PadLeft(totalWidth [, paddingChar])`. \param totalWidth The width.
  /// \param paddingChar The filler, a space by default. \return The new string.
  [[nodiscard]] class String PadLeft(::agiru::Integer totalWidth, ::agiru::Char paddingChar = ::agiru::Char{' '}) const;

  /// \brief `String.PadRight(totalWidth [, paddingChar])`. \param totalWidth The width.
  /// \param paddingChar The filler, a space by default. \return The new string.
  [[nodiscard]] class String PadRight(::agiru::Integer totalWidth, ::agiru::Char paddingChar = ::agiru::Char{' '}) const;

  /// \brief `String.IndexOf(value [, startIndex])`. \param value The text to find.
  /// \param startIndex Where to start, zero-based. \return The position, or -1.
  [[nodiscard]] ::agiru::Integer IndexOf(std::string_view value, ::agiru::Integer startIndex = 0) const;

  /// \brief `String.IndexOf(char [, startIndex])`. \param value The character.
  /// \param startIndex Where to start, zero-based. \return The position, or -1.
  /// \note Constrained to `Char` for the reason `Split(Char)` gives.
  template <typename C>
    requires std::same_as<C, ::agiru::Char>
  [[nodiscard]] ::agiru::Integer IndexOf(C value, ::agiru::Integer startIndex = 0) const {
    return IndexOf(std::string_view(::agiru::Encoded(value)), startIndex);
  }

  /// \brief `String.IndexOfAny(chars)`. \param chars The characters. \return The first position
  ///        of any of them, or -1.
  [[nodiscard]] ::agiru::Integer IndexOfAny(const Array &chars) const;

  /// \brief `String.IndexOfAny(chars)` with the characters as ONE text, which is what
  ///        `Path.GetInvalidFileNameChars()` answers here. \param chars The characters.
  /// \return The first position of any of them, or -1.
  [[nodiscard]] ::agiru::Integer IndexOfAny(std::string_view chars) const;

  /// \brief `String.LastIndexOf(value)`. \param value The text to find. \return The position, or
  ///        -1.
  [[nodiscard]] ::agiru::Integer LastIndexOf(std::string_view value) const;

  /// \brief `String.Substring(startIndex)`. \param startIndex Zero-based. \return The rest.
  /// \throws Error when the index is outside the string.
  [[nodiscard]] class String Substring(::agiru::Integer startIndex) const;

  /// \brief `String.Substring(startIndex, length)`. \param startIndex Zero-based.
  /// \param length How many. \return The slice.
  /// \throws Error when the range is outside the string.
  [[nodiscard]] class String Substring(::agiru::Integer startIndex, ::agiru::Integer length) const;

  /// \brief `String.StartsWith(value)`. \param value The prefix. \return Whether it starts so.
  [[nodiscard]] ::agiru::Boolean StartsWith(std::string_view value) const {
    return std::string_view(value_).starts_with(value);
  }

  /// \brief `String.EndsWith(value)`. \param value The suffix. \return Whether it ends so.
  [[nodiscard]] ::agiru::Boolean EndsWith(std::string_view value) const {
    return std::string_view(value_).ends_with(value);
  }

  /// \brief `String.Contains(value)`. \param value The text. \return Whether it is inside.
  [[nodiscard]] ::agiru::Boolean Contains(std::string_view value) const {
    return std::string_view(value_).find(value) != std::string_view::npos;
  }

  /// \brief `String.Normalize([form])`: the identity, see the class note.
  /// \tparam Arguments The normalisation form, when given.
  /// \param arguments Read only to be discarded. \return The same string.
  template <typename... Arguments>
  [[nodiscard]] class String Normalize(const Arguments &...arguments) const {
    (static_cast<void>(arguments), ...);
    return *this;
  }

  /// \brief `String.IsNormalized([form])`: true, see the class note.
  /// \tparam Arguments The normalisation form, when given.
  /// \param arguments Read only to be discarded. \return True.
  template <typename... Arguments>
  [[nodiscard]] ::agiru::Boolean IsNormalized(const Arguments &...arguments) const {
    (static_cast<void>(arguments), ...);
    return true;
  }

  /// \brief `String.Equals(other)`. \param other The other. \return Whether the same text.
  [[nodiscard]] ::agiru::Boolean Equals(std::string_view other) const {
    return std::string_view(value_) == other;
  }

  /// \brief `String.Format(...)` with a culture, which `Type Helper` calls to format a date and
  ///        a decimal; not rebuilt (board:0035).
  Refused Format{{.type = "String", .member = "Format"}};

  /// \brief `String.GetType()`; not rebuilt (board:0035).
  Refused GetType{{.type = "String", .member = "GetType"}};

  /// \brief Two strings compare by text. \param other The other. \return Whether equal.
  [[nodiscard]] bool operator==(const class String &other) const {
    return std::string_view(value_) == std::string_view(other.value_);
  }

private:
  ::agiru::Text<0> value_;
};

}

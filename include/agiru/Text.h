#pragma once

#include "agiru/Error.h"

#include <compare>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

/// \file
/// \brief AL's string types: Text[N] and Code[N].

namespace agiru {

/// \brief An error raised by a string assignment, such as an over-length value.
class StringError : public Error {
public:
  using Error::Error;
};

/// \brief Internals of the string types. Not part of the door.
namespace detail {

/// \brief Lets the runtime write a value it read from a column.
///
/// The storage layer reaches a field as an offset and a type tag, so it cannot use the typed
/// assignment. This is the one way in, named so that every caller can be found, and it applies
/// the same rules assignment does.
class ValueAccess;

/// \brief Counts a UTF-8 string the way .NET counts a string: in UTF-16 code units.
///
/// \param s The UTF-8 text.
/// \return The number of UTF-16 code units, so a character outside the Basic Multilingual Plane
///         counts two while occupying four bytes.
///
/// BC's strings are .NET strings, and `text-strlen-method.md` gives the length of one. Storage here
/// is UTF-8; the counting rule is .NET's.
std::size_t Utf16Length(std::string_view s);

/// \brief Raises the platform's own over-length message.
///
/// \param value  The offending text, quoted in the message.
/// \param actual Its length.
/// \param max    The declared maximum.
/// \throws StringError always.
///
/// \warning The wording is load-bearing: BC test code matches substrings of it through
///          `Assert.ExpectedError`, so a paraphrase would turn a green case red.
[[noreturn]] void RaiseTooLong(std::string_view value, std::size_t actual, std::size_t max);

/// \brief Raises when a value is longer than a declared length.
/// \param s   The text to measure.
/// \param max The declared maximum, in UTF-16 code units.
void CheckLength(std::string_view s, std::size_t max);

/// \brief Applies AL's Code normalisation.
///
/// \param s The raw text.
/// \return The text uppercased, with leading and trailing spaces removed.
///
/// Verbatim from `code-data-type.md`: a Code is "a special type of string that is converted to
/// uppercase and removes any trailing or leading spaces", and "the length of a Code variable equals
/// the number of characters in the text without leading or trailing spaces" -- so the trim happens
/// BEFORE the length is checked. The document's own example turns `' 2 '` into `'2'`.
///
/// \note Only ASCII letters are uppercased. .NET's ToUpper is culture-aware and would fold
///       non-ASCII letters too; no BC document states which culture applies, and guessing one would
///       be a silent semantic (board:0010).
std::string NormaliseCode(std::string_view s);

/// \brief Compares two Code values the way AL orders them.
///
/// \param a Left operand.
/// \param b Right operand.
/// \return Numeric ordering when both sides consist entirely of digits, so `"109003"` is less than
///         `"1010999"`; plain string ordering otherwise.
///
/// \warning NOT IN THE PLATFORM DOCUMENTATION -- searched, absent. It rests on the predecessor
///          (openerp `runtime/fields.py:_Code`), which measured it against the BC test suite, and
///          on the mechanism that needs it:
///          `Business Foundation/App/NoSeries/src/Single/NoSeriesStatelessImpl.Codeunit.al:109`,
///          which compares number-series codes with less-than and greater-than. Marked a conjecture
///          on purpose until a document or a real BC confirms it (board:0011).
/// \note Only the ordering is numeric. Equality stays exact string comparison, so `"01"` and `"1"`
///       remain different primary keys.
std::strong_ordering CompareCode(std::string_view a, std::string_view b);

} // namespace detail

/// \brief The part of a string field that does not depend on its declared length.
///
/// It exists so the runtime can read any Text[N] or Code[N] through one pointer: the field table
/// addresses a field by offset and type, and without a common base it would need one branch per
/// declared length. Keeping the data only here also leaves a generated record standard-layout,
/// which is what `offsetof` over the field table requires.
class StringValue {
public:
  /// \return The stored text.
  [[nodiscard]] std::string_view Value() const { return value_; }

  /// \return True when the field holds the empty string.
  [[nodiscard]] bool IsEmpty() const { return value_.empty(); }

  /// \return The length in UTF-16 code units, as AL's `StrLen` counts it.
  [[nodiscard]] std::size_t Length() const { return detail::Utf16Length(value_); }

protected:
  friend class detail::ValueAccess;

  /// \brief Stores an already validated value.
  /// \param value The text, checked and normalised by the derived type.
  void Set(std::string value) { value_ = std::move(value); }

  /// \return The stored text, for a derived type's own comparisons.
  [[nodiscard]] const std::string &Stored() const { return value_; }

private:
  std::string value_;
};

/// \brief AL `Text[N]`.
///
/// \tparam N The declared length, in UTF-16 code units.
///
/// The length is a compile-time property because AL declares it that way, which makes `MaxStrLen`
/// a constant and an over-length assignment the only runtime check. The BaseApp uses 64 distinct
/// lengths (measured 2026-09-01), so the instantiation count is bounded; the work itself lives in
/// non-template functions so that 64 instantiations do not become 64 copies of the logic.
///
/// \see `text-data-type.md`
template <std::size_t N> class Text : public StringValue {
public:
  /// \brief The declared length.
  static constexpr std::size_t kMaxLength = N;

  /// \brief An empty text.
  Text() = default;

  /// \brief Constructs from text.
  /// \param value The text.
  /// \throws StringError when it is longer than N.
  explicit Text(std::string_view value) { Assign(value); }

  /// \brief Assigns text.
  /// \param value The text.
  /// \return This object.
  /// \throws StringError when it is longer than N.
  Text &operator=(std::string_view value) {
    Assign(value);
    return *this;
  }

  /// \brief Assigns text, checking the declared length.
  /// \param value The text.
  /// \throws StringError when it is longer than N.
  void Assign(std::string_view value) {
    detail::CheckLength(value, N);
    Set(std::string(value));
  }

  /// \brief Orders two texts lexicographically.
  /// \param o The other text.
  /// \return The ordering.
  [[nodiscard]] std::strong_ordering operator<=>(const Text &o) const {
    return Stored().compare(o.Stored()) <=> 0;
  }

  /// \brief Compares two texts for equality.
  /// \param o The other text.
  /// \return True when the stored text is identical.
  [[nodiscard]] bool operator==(const Text &o) const { return Stored() == o.Stored(); }

  /// \brief Compares against a literal, the way AL writes `Code <> ''`.
  /// \param value The text.
  /// \return True when the stored text is identical.
  [[nodiscard]] bool operator==(std::string_view value) const { return Stored() == value; }
};

/// \brief AL `Code[N]`.
///
/// \tparam N The declared length, in UTF-16 code units, measured after normalisation.
///
/// A Code is uppercased and trimmed on every assignment, and ordered numerically when both sides
/// are entirely digits.
///
/// \see `code-data-type.md`, detail::NormaliseCode, detail::CompareCode
template <std::size_t N> class Code : public StringValue {
public:
  /// \brief The declared length.
  static constexpr std::size_t kMaxLength = N;

  /// \brief An empty code.
  Code() = default;

  /// \brief Constructs from text, normalising it first.
  /// \param value The text.
  /// \throws StringError when the normalised value is longer than N.
  explicit Code(std::string_view value) { Assign(value); }

  /// \brief Assigns text, normalising it first.
  /// \param value The text.
  /// \return This object.
  /// \throws StringError when the normalised value is longer than N.
  Code &operator=(std::string_view value) {
    Assign(value);
    return *this;
  }

  /// \brief Assigns text: trim, uppercase, then check the declared length.
  /// \param value The text.
  /// \throws StringError when the normalised value is longer than N.
  void Assign(std::string_view value) {
    std::string normalised = detail::NormaliseCode(value);
    detail::CheckLength(normalised, N);
    Set(std::move(normalised));
  }

  /// \brief Orders two codes, numerically where both are all digits.
  /// \param o The other code.
  /// \return The ordering.
  [[nodiscard]] std::strong_ordering operator<=>(const Code &o) const {
    return detail::CompareCode(Stored(), o.Stored());
  }

  /// \brief Compares two codes for exact equality.
  /// \param o The other code.
  /// \return True when the stored text is identical, so `"01"` differs from `"1"`.
  [[nodiscard]] bool operator==(const Code &o) const { return Stored() == o.Stored(); }

  /// \brief Compares against a literal, the way AL writes `Code <> ''`.
  /// \param value The text.
  /// \return True when the stored text is identical.
  [[nodiscard]] bool operator==(std::string_view value) const { return Stored() == value; }
};

/// \brief AL `MaxStrLen(String)`.
/// \tparam T A Text or Code.
/// \return The declared length.
/// \see `text-maxstrlen-string-method.md`
template <typename T> constexpr std::size_t MaxStrLen(const T & /*value*/) {
  return T::kMaxLength;
}

/// \brief AL `StrLen(String)`.
/// \tparam T A Text or Code.
/// \param value The string.
/// \return Its length in UTF-16 code units.
/// \see `text-strlen-method.md`
template <typename T> std::size_t StrLen(const T &value) {
  return value.Length();
}

} // namespace agiru

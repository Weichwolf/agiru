#pragma once

#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Char.h"
#include "type/Integer.h"
#include "type/List.h"

#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/// \file
/// \brief What AL's two string types share.
///
/// `StringValue` and the helpers below have no AL counterpart, so they are free to be named what
/// they are. The AL types themselves live one per header, as the documentation lists them:
/// agiru/Text.h and agiru/Code.h.

namespace agiru {

class Variant;

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

/// \brief The code point at a ZERO-BASED UTF-16 unit position.
/// \param s    The text.
/// \param unit The position.
/// \return The code point.
/// \throws StringError when the position is outside the text or inside a surrogate pair.
std::int32_t CodePointAt(std::string_view s, std::size_t unit);

/// \brief The byte offset of a ONE-BASED UTF-16 unit position, clamped to the end.
/// \param s    The text.
/// \param unit The position, counting from one.
/// \return The byte offset.
std::size_t ByteOfUnit(std::string_view s, std::size_t unit);

/// \brief The one-based UTF-16 unit position of a byte offset.
/// \param s  The text.
/// \param at The byte offset.
/// \return The position, counting from one.
std::size_t UnitOfByte(std::string_view s, std::size_t at);

/// \brief AL `Text.IndexOf`.
/// \param s          The text.
/// \param value      The string to seek.
/// \param startIndex The one-based position to start at.
/// \return The one-based position, or 0.
Integer IndexOfText(std::string_view s, std::string_view value, Integer startIndex);

/// \brief AL `Text.LastIndexOf`.
/// \param s          The text.
/// \param value      The string to seek.
/// \param startIndex The one-based position the backward search starts at; 0 means the end.
/// \return The one-based position, or 0.
Integer LastIndexOfText(std::string_view s, std::string_view value, Integer startIndex);

/// \brief AL `Text.IndexOfAny`.
/// \param s          The text.
/// \param values     The characters to seek.
/// \param startIndex The one-based position to start at.
/// \return The one-based position, or 0.
Integer IndexOfAnyText(std::string_view s, std::string_view values, Integer startIndex);

/// \brief Which end a text is padded at.
enum class PadSide : std::uint8_t {
  Left,  ///< AL `Text.PadLeft` -- the text is right-aligned.
  Right, ///< AL `Text.PadRight` -- the text is left-aligned.
};

/// \brief Which ends a text is trimmed at.
enum class TrimSides : std::uint8_t {
  Start, ///< AL `Text.TrimStart`.
  End,   ///< AL `Text.TrimEnd`.
  Both,  ///< AL `Text.Trim`.
};

/// \brief What `Text.Replace` replaces, and with what.
struct Replacement {
  std::string_view from; ///< What is replaced.
  std::string_view to;   ///< What replaces it.
};

/// \brief AL `Text.PadLeft` and `Text.PadRight`.
/// \param s     The text.
/// \param count The length to pad to.
/// \param side  Which end to pad at.
/// \param pad   The padding character.
/// \return The padded text.
std::string PadText(std::string_view s, Integer count, PadSide side, Char pad);

/// \brief AL `Text.Remove`.
/// \param s          The text.
/// \param startIndex The one-based position to delete from.
/// \param count      How many characters to delete, or nothing for everything after the position.
/// \return What is left.
std::string RemoveText(std::string_view s, Integer startIndex, std::optional<Integer> count);

/// \brief AL `Text.Substring`.
/// \param s          The text.
/// \param startIndex The one-based position the substring starts at.
/// \param count      How many characters it holds, or nothing for the rest of the text.
/// \return The substring.
std::string SubstringText(std::string_view s, Integer startIndex, std::optional<Integer> count);

/// \brief AL `Text.Replace` -- every occurrence.
/// \param s    The text.
/// \param what What is replaced, and with what.
/// \return The result.
std::string ReplaceText(std::string_view s, Replacement what);

/// \brief AL `Text.Split`.
/// \param s          The text.
/// \param separators The separators; empty means white space.
/// \return The pieces, in order.
List<std::string> SplitText(std::string_view s, std::span<const std::string> separators);

/// \brief Each character of a `List of [Char]` as its own one-character string.
/// \param values The characters.
/// \return The strings.
std::vector<std::string> EachChar(const List<Char> &values);

/// \brief A `List of [Char]` as one string.
/// \param values The characters.
/// \return The string.
std::string TextOfChars(const List<Char> &values);

/// \brief The elements of a `List of [Text]`, contiguously.
/// \param values The list.
/// \return The elements.
std::vector<std::string> EachText(const List<std::string> &values);

/// \brief AL `Text.ToLower`.
/// \param s The text.
/// \return It in lower case.
std::string LowerText(std::string_view s);

/// \brief AL `Text.ToUpper`.
/// \param s The text.
/// \return It in upper case.
std::string UpperText(std::string_view s);

/// \brief AL `Text.Trim`, `Text.TrimStart` and `Text.TrimEnd`.
/// \param s     The text.
/// \param sides Which ends to strip.
/// \param chars The characters to strip; empty means white space.
/// \return The trimmed text.
std::string TrimText(std::string_view s, TrimSides sides, std::string_view chars);

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
/// \param max The declared maximum, in UTF-16 code units; 0 means UNBOUNDED.
///
/// \note Zero is no limit rather than a limit of nothing, because that is what AL means by a bare
///       `Text` with no brackets after it.
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
  ///
  /// \note AN `Integer` AND NOT A `std::size_t`, because AL's `StrLen` returns one and a body
  ///       hands the result straight to something that takes an Integer -- or to an `Any`, where a
  ///       `std::size_t` is not an alternative at all.
  [[nodiscard]] Integer Length() const { return static_cast<Integer>(detail::Utf16Length(value_)); }

  /// \brief AL `X[i]` on text -- the character at a ONE-BASED position.
  ///
  /// \param index The position, counting from one as AL counts.
  /// \return The character.
  /// \throws StringError when the index is outside the text.
  [[nodiscard]] Char operator[](Integer index) const;

  /// \brief AL `Text.Contains(Text)`.
  /// \param Value The string to seek.
  /// \return True when this text holds it.
  /// \see `text-contains-method.md`
  [[nodiscard]] Boolean Contains(std::string_view Value) const {
    return value_.find(Value) != std::string::npos;
  }

  /// \brief AL `Text.EndsWith(Text)`.
  /// \param Value The string to match.
  /// \return True when this text ends with it.
  /// \see `text-endswith-method.md`
  [[nodiscard]] Boolean EndsWith(std::string_view Value) const { return value_.ends_with(Value); }

  /// \brief AL `Text.StartsWith(Text)`.
  /// \param Value The string to match.
  /// \return True when this text begins with it.
  /// \see `text-startswith-method.md`
  [[nodiscard]] Boolean StartsWith(std::string_view Value) const {
    return value_.starts_with(Value);
  }

  /// \brief AL `Text.IndexOf(Text)`.
  /// \param Value The string to seek.
  /// \return Its one-based position, or 0 when the text does not hold it.
  /// \see `text-indexof-method.md`
  [[nodiscard]] Integer IndexOf(std::string_view Value) const {
    return detail::IndexOfText(value_, Value, 1);
  }

  /// \brief AL `Text.IndexOf(Text, Integer)`.
  /// \param Value      The string to seek.
  /// \param StartIndex The one-based position to start at.
  /// \return Its one-based position, or 0 when the text does not hold it from there on.
  /// \see `text-indexof-method.md`
  [[nodiscard]] Integer IndexOf(std::string_view Value, Integer StartIndex) const {
    return detail::IndexOfText(value_, Value, StartIndex);
  }

  /// \brief AL `Text.LastIndexOf(Text)`.
  /// \param Value The string to seek.
  /// \return The one-based position of its LAST occurrence, or 0.
  /// \see `text-lastindexof-method.md`
  [[nodiscard]] Integer LastIndexOf(std::string_view Value) const {
    return detail::LastIndexOfText(value_, Value, 0);
  }

  /// \brief AL `Text.LastIndexOf(Text, Integer)`.
  /// \param Value      The string to seek.
  /// \param StartIndex The one-based position the backward search starts from.
  /// \return The one-based position of the last occurrence at or before it, or 0.
  /// \see `text-lastindexof-method.md`
  [[nodiscard]] Integer LastIndexOf(std::string_view Value, Integer StartIndex) const {
    return detail::LastIndexOfText(value_, Value, StartIndex);
  }

  /// \brief AL `Text.IndexOfAny(Text)`.
  /// \param Values The characters to seek, as a string.
  /// \return The one-based position of the first of them, or 0.
  /// \see `text-indexofany-text-integer-method.md`
  [[nodiscard]] Integer IndexOfAny(std::string_view Values) const {
    return detail::IndexOfAnyText(value_, Values, 1);
  }

  /// \brief AL `Text.IndexOfAny(Text, Integer)`.
  /// \param Values     The characters to seek, as a string.
  /// \param StartIndex The one-based position to start at.
  /// \return The one-based position of the first of them, or 0.
  /// \see `text-indexofany-text-integer-method.md`
  [[nodiscard]] Integer IndexOfAny(std::string_view Values, Integer StartIndex) const {
    return detail::IndexOfAnyText(value_, Values, StartIndex);
  }

  /// \brief AL `Text.IndexOfAny(List of [Char])`.
  /// \param Values The characters to seek.
  /// \return The one-based position of the first of them, or 0.
  /// \see `text-indexofany-list[char]-integer-method.md`
  [[nodiscard]] Integer IndexOfAny(const List<Char> &Values) const {
    return detail::IndexOfAnyText(value_, detail::TextOfChars(Values), 1);
  }

  /// \brief AL `Text.IndexOfAny(List of [Char], Integer)`.
  /// \param Values     The characters to seek.
  /// \param StartIndex The one-based position to start at.
  /// \return The one-based position of the first of them, or 0.
  /// \see `text-indexofany-list[char]-integer-method.md`
  [[nodiscard]] Integer IndexOfAny(const List<Char> &Values, Integer StartIndex) const {
    return detail::IndexOfAnyText(value_, detail::TextOfChars(Values), StartIndex);
  }

  /// \brief AL `Text.PadLeft(Integer)` -- right-aligns by padding with spaces.
  /// \param Count The length the result is padded to.
  /// \return The padded text, or this text when it is already that long.
  /// \see `text-padleft-method.md`
  [[nodiscard]] std::string PadLeft(Integer Count) const {
    return detail::PadText(value_, Count, detail::PadSide::Left, Char{' '});
  }

  /// \brief AL `Text.PadLeft(Integer, Char)`.
  /// \param Count   The length the result is padded to.
  /// \param Padding The padding character.
  /// \return The padded text, or this text when it is already that long.
  /// \see `text-padleft-method.md`
  [[nodiscard]] std::string PadLeft(Integer Count, Char Padding) const {
    return detail::PadText(value_, Count, detail::PadSide::Left, Padding);
  }

  /// \brief AL `Text.PadRight(Integer)` -- left-aligns by padding with spaces.
  /// \param Count The length the result is padded to.
  /// \return The padded text, or this text when it is already that long.
  /// \see `text-padright-method.md`
  [[nodiscard]] std::string PadRight(Integer Count) const {
    return detail::PadText(value_, Count, detail::PadSide::Right, Char{' '});
  }

  /// \brief AL `Text.PadRight(Integer, Char)`.
  /// \param Count   The length the result is padded to.
  /// \param Padding The padding character.
  /// \return The padded text, or this text when it is already that long.
  /// \see `text-padright-method.md`
  [[nodiscard]] std::string PadRight(Integer Count, Char Padding) const {
    return detail::PadText(value_, Count, detail::PadSide::Right, Padding);
  }

  /// \brief AL `Text.Remove(Integer)` -- everything from a position onwards.
  /// \param StartIndex The one-based position to begin deleting at.
  /// \return What is left.
  /// \see `text-remove-method.md`
  [[nodiscard]] std::string Remove(Integer StartIndex) const {
    return detail::RemoveText(value_, StartIndex, std::nullopt);
  }

  /// \brief AL `Text.Remove(Integer, Integer)`.
  /// \param StartIndex The one-based position to begin deleting at.
  /// \param Count      How many characters to delete.
  /// \return What is left.
  /// \see `text-remove-method.md`
  [[nodiscard]] std::string Remove(Integer StartIndex, Integer Count) const {
    return detail::RemoveText(value_, StartIndex, Count);
  }

  /// \brief AL `Text.Replace(Text, Text)` -- every occurrence.
  /// \param OldValue The string to replace.
  /// \param NewValue What replaces it.
  /// \return The result.
  /// \see `text-replace-method.md`
  [[nodiscard]] std::string Replace(std::string_view OldValue, std::string_view NewValue) const {
    return detail::ReplaceText(value_, {.from = OldValue, .to = NewValue});
  }

  /// \brief AL `Text.Substring(Integer)` -- everything from a position onwards.
  /// \param StartIndex The one-based position the substring starts at.
  /// \return The substring.
  /// \see `text-substring-method.md`
  [[nodiscard]] std::string Substring(Integer StartIndex) const {
    return detail::SubstringText(value_, StartIndex, std::nullopt);
  }

  /// \brief AL `Text.Substring(Integer, Integer)`.
  /// \param StartIndex The one-based position the substring starts at.
  /// \param Count      How many characters it holds.
  /// \return The substring.
  /// \see `text-substring-method.md`
  /// \note A COUNT PAST THE END IS NOT AN ERROR since application version 27.1, which the page
  ///       states outright; it yields the rest of the text.
  [[nodiscard]] std::string Substring(Integer StartIndex, Integer Count) const {
    return detail::SubstringText(value_, StartIndex, Count);
  }

  /// \brief AL `Text.Split()` -- at white space.
  /// \return The pieces, in order.
  /// \see `text-split-text-method.md`
  [[nodiscard]] List<std::string> Split() const { return detail::SplitText(value_, {}); }

  /// \brief AL `Text.Split(Text)`.
  /// \param Separators The separator.
  /// \return The pieces, in order.
  /// \see `text-split-text-method.md`
  [[nodiscard]] List<std::string> Split(std::string_view Separators) const {
    const std::string one{Separators};
    return detail::SplitText(value_, std::span<const std::string>{&one, 1});
  }

  /// \brief AL `Text.Split(List of [Text])`.
  /// \param Separators The separators.
  /// \return The pieces, in order.
  /// \see `text-split-list[text]-method.md`
  [[nodiscard]] List<std::string> Split(const List<std::string> &Separators) const {
    return detail::SplitText(value_, detail::EachText(Separators));
  }

  /// \brief AL `Text.Split(List of [Char])`.
  /// \param Separators The separators, one character each.
  /// \return The pieces, in order.
  /// \see `text-split-list[char]-method.md`
  [[nodiscard]] List<std::string> Split(const List<Char> &Separators) const {
    return detail::SplitText(value_, detail::EachChar(Separators));
  }

  /// \brief AL `Text.ToLower()`.
  /// \return This text in lower case.
  /// \see `text-tolower-method.md`
  /// \warning IT LOWERS ASCII ONLY. The platform lowers by the invariant culture, which covers
  /// every
  ///          cased script; anything above 127 is left alone here and is board:0041.
  [[nodiscard]] std::string ToLower() const { return detail::LowerText(value_); }

  /// \brief AL `Text.ToUpper()`.
  /// \return This text in upper case.
  /// \see `text-toupper-method.md`
  /// \warning IT RAISES ASCII ONLY, for the reason ToLower gives.
  [[nodiscard]] std::string ToUpper() const { return detail::UpperText(value_); }

  /// \brief AL `Text.Trim()` -- white space off both ends.
  /// \return The trimmed text.
  /// \see `text-trim-method.md`
  [[nodiscard]] std::string Trim() const {
    return detail::TrimText(value_, detail::TrimSides::Both, {});
  }

  /// \brief AL `Text.TrimStart()` -- white space off the front.
  /// \return The trimmed text.
  /// \see `text-trimstart-method.md`
  [[nodiscard]] std::string TrimStart() const {
    return detail::TrimText(value_, detail::TrimSides::Start, {});
  }

  /// \brief AL `Text.TrimStart(Text)`.
  /// \param Chars The characters to strip.
  /// \return The trimmed text.
  /// \see `text-trimstart-method.md`
  [[nodiscard]] std::string TrimStart(std::string_view Chars) const {
    return detail::TrimText(value_, detail::TrimSides::Start, Chars);
  }

  /// \brief AL `Text.TrimEnd()` -- white space off the back.
  /// \return The trimmed text.
  /// \see `text-trimend-method.md`
  [[nodiscard]] std::string TrimEnd() const {
    return detail::TrimText(value_, detail::TrimSides::End, {});
  }

  /// \brief AL `Text.TrimEnd(Text)`.
  /// \param Chars The characters to strip.
  /// \return The trimmed text.
  /// \see `text-trimend-method.md`
  [[nodiscard]] std::string TrimEnd(std::string_view Chars) const {
    return detail::TrimText(value_, detail::TrimSides::End, Chars);
  }

  /// \brief AL `+=` on text -- appends.
  ///
  /// \tparam T The other side, which must read as a `std::string_view`.
  /// \param value The text to append.
  /// \return This value.
  ///
  /// \note THE LENGTH IS CHECKED BY WHOEVER OWNS IT. `StringValue` carries no declared length --
  ///       `Text<N>` and `Code<N>` do -- so the append goes through the derived type's `Assign`,
  ///       which is what raises when the result no longer fits.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  StringValue &operator+=(const T &value) {
    value_ += std::string_view(value);
    return *this;
  }

  /// \brief Reads as text wherever text is wanted.
  ///
  /// \return The stored text.
  ///
  /// \note AL HANDS A `Code` TO A `Text` PARAMETER WITHOUT CEREMONY, and every builtin that takes
  ///       text takes it. Without this the door refuses what AL writes constantly -- and the LENGTH
  ///       check that matters is on the way IN, not on the way out.
  operator std::string_view() const { return value_; }

  /// \brief The DECLARED length, which is not the current one.
  ///
  /// \return The maximum this string accepts, or 0 when it was declared without a length.
  ///
  /// \note IT IS A FIELD AND NOT A TEMPLATE ARGUMENT, and the predecessor is why. AL passes a
  ///       `Text[30]` field to a `var Text` parameter -- `PostCode.LookupPostCode(City, ...)` in
  ///       Customer.Table.al -- so the callee holds a reference whose STATIC type has no length and
  ///       whose value has one. `~/Git/openerp/` had the same question and no value to ask, so its
  ///       `MaxStrLen` PARSED THE CALLING SOURCE LINE to recover the declaration; the comment there
  ///       lists what that cost when it guessed wrong. Eight bytes per string buys the right answer
  ///       at the point of use.
  [[nodiscard]] std::size_t Max() const { return max_; }

protected:
  friend class detail::ValueAccess;

  /// \brief A string with a declared length.
  /// \param max The declared length, or 0 for none.
  explicit StringValue(std::size_t max) : max_(max) {}

  /// \brief A string with no declared length.
  StringValue() = default;

  /// \brief Copies the value AND the declared length, which is what constructing one is.
  StringValue(const StringValue &) = default;

  /// \brief Moves the value and the declared length.
  StringValue(StringValue &&) = default;

  ~StringValue() = default;

  /// \brief Copies the VALUE and keeps this string's own declared length.
  ///
  /// \param o The other string.
  /// \return This string.
  ///
  /// \note ASSIGNMENT DOES NOT MOVE THE DECLARATION. `Customer.City := SomeUnboundedText` leaves
  ///       City a `Text[30]`; copying the source's length would quietly widen the target and the
  ///       next over-long value would go in unchecked.
  StringValue &operator=(const StringValue &o) {
    if (this != &o) { value_ = o.value_; }
    return *this;
  }

  /// \brief The same, moving the value.
  /// \param o The other string.
  /// \return This string.
  StringValue &operator=(StringValue &&o) noexcept {
    if (this != &o) { value_ = std::move(o.value_); }
    return *this;
  }

  /// \brief Stores an already validated value.
  /// \param value The text, checked and normalised by the derived type.
  void Set(std::string value) { value_ = std::move(value); }

  /// \return The stored text, for a derived type's own comparisons.
  [[nodiscard]] const std::string &Stored() const { return value_; }

private:
  std::string value_;
  std::size_t max_ = 0;
};

/// \brief AL `+` on text -- concatenation.
///
/// \param left  The left side.
/// \param right The right side.
/// \return The two joined, as a plain string: AL decides the LENGTH at the assignment, not here.
///
/// \note AL WRITES `Code + Code` AND MEANS TEXT. `NoSeries.Code + GenerateRandomCode(...)` is the
///       shape, and what it produces is checked against the declared length of whatever it is
///       assigned to -- which is where `Text` and `Code` already check it.
[[nodiscard]] inline std::string operator+(const StringValue &left, const StringValue &right) {
  return std::string(left.Value()) + std::string(right.Value());
}

/// \brief AL `+` on text and a literal.
///
/// \tparam T The other side, which must read as a `std::string_view`.
/// \param left  The text.
/// \param right The literal, or anything else that reads as text.
/// \return The two joined.
///
/// \note AL WRITES `X + '@' + Y` and means text. Without this the literal reaches neither side --
///       `std::string_view` and `const StringValue &` are both one user-defined conversion away and
///       neither is chosen.
template <typename T>
  requires std::convertible_to<const T &, std::string_view> && (!std::derived_from<T, StringValue>)
[[nodiscard]] std::string operator+(const StringValue &left, const T &right) {
  return std::string(left.Value()) + std::string(std::string_view(right));
}

/// \brief AL `+` on a literal and text.
/// \tparam T The other side, which must read as a `std::string_view`.
/// \param left  The literal.
/// \param right The text.
/// \return The two joined.
template <typename T>
  requires std::convertible_to<const T &, std::string_view> && (!std::derived_from<T, StringValue>)
[[nodiscard]] std::string operator+(const T &left, const StringValue &right) {
  return std::string(std::string_view(left)) + std::string(right.Value());
}

} // namespace agiru

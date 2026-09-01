#pragma once

#include "agiru/Error.h"

#include <compare>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace agiru {

class StringError : public Error {
public:
  using Error::Error;
};

namespace detail {

/// AL string length is counted the way .NET counts it -- in UTF-16 code units, because BC's strings
/// ARE .NET strings. Storage here is UTF-8, so a character outside the Basic Multilingual Plane
/// counts two while occupying four bytes. `text-strlen-method.md` and
/// `text-maxstrlen-string-method.md` give the length of a string; `code-data-type.md` states "The
/// Code data type supports Unicode".
std::size_t Utf16Length(std::string_view s);

/// Raises the platform's own message, whose exact wording is load-bearing: BC test code matches
/// substrings of it through `Assert.ExpectedError`, so a paraphrase would turn a green case red.
[[noreturn]] void RaiseTooLong(std::string_view value, std::size_t actual, std::size_t max);

void CheckLength(std::string_view s, std::size_t max);

/// AL `Code` normalisation, verbatim from `code-data-type.md`: "a special type of string that is
/// converted to uppercase and removes any trailing or leading spaces", and "the length of a Code
/// variable equals the number of characters in the text without leading or trailing spaces" -- so
/// the trim happens BEFORE the length is checked. The documentation's own example: `' 2 '` becomes
/// `'2'`, one character long.
///
/// Only ASCII letters are uppercased. .NET's `ToUpper` is culture-aware and would fold non-ASCII
/// letters too; no BC document states which culture applies to a Code field, and guessing one would
/// be a silent semantic (board:0010).
std::string NormaliseCode(std::string_view s);

/// AL `Code` ordering. A Code whose content is entirely digits is ordered NUMERICALLY, so
/// "109003" < "1010999" rather than the other way round.
///
/// NOT IN THE PLATFORM DOCUMENTATION -- searched, absent. It rests on the predecessor
/// (openerp `runtime/fields.py:_Code`), which measured it against the BC test suite, and on the
/// mechanism that depends on it: `Business Foundation/App/NoSeries/src/Single/
/// NoSeriesStatelessImpl.Codeunit.al:109`, `NoIsWithinValidRange(CurrentNo: Code[20];
/// StartingNo: Code[20]; EndingNo: Code[20])`, which compares number-series codes with `<` and `>`.
/// Marked as a conjecture on purpose until a document or a real BC confirms it (board:0011).
///
/// Only the ORDERING is numeric. Equality stays exact string comparison, so that "01" and "1"
/// remain different primary keys.
std::strong_ordering CompareCode(std::string_view a, std::string_view b);

} // namespace detail

/// The part of a string field that does not depend on its declared length.
///
/// It exists so that the runtime can read ANY `Text[N]` or `Code[N]` through one pointer: the field
/// table addresses a field by offset and type, and without a common base it would need one branch
/// per declared length. Having the data live only here also keeps a record standard-layout, which
/// is what `offsetof` over the field table requires.
class StringValue {
public:
  [[nodiscard]] std::string_view Value() const { return value_; }

  [[nodiscard]] bool IsEmpty() const { return value_.empty(); }

  [[nodiscard]] std::size_t Length() const { return detail::Utf16Length(value_); }

protected:
  void Set(std::string value) { value_ = std::move(value); }

  [[nodiscard]] const std::string &Stored() const { return value_; }

private:
  std::string value_;
};

/// AL `Text[N]`. `text-data-type.md`.
///
/// The length is a compile-time property because AL declares it that way, which makes `MaxStrLen`
/// a constant and an over-length assignment the only runtime check. The BaseApp uses 64 distinct
/// lengths (measured 2026-09-01), so the instantiation count is bounded and small; the work itself
/// lives in non-template functions so that 64 instantiations do not become 64 copies of the logic.
template <std::size_t N> class Text : public StringValue {
public:
  static constexpr std::size_t kMaxLength = N;

  Text() = default;

  explicit Text(std::string_view value) { Assign(value); }

  Text &operator=(std::string_view value) {
    Assign(value);
    return *this;
  }

  void Assign(std::string_view value) {
    detail::CheckLength(value, N);
    Set(std::string(value));
  }

  [[nodiscard]] std::strong_ordering operator<=>(const Text &o) const {
    return Stored().compare(o.Stored()) <=> 0;
  }

  [[nodiscard]] bool operator==(const Text &o) const { return Stored() == o.Stored(); }
};

/// AL `Code[N]`. `code-data-type.md`.
template <std::size_t N> class Code : public StringValue {
public:
  static constexpr std::size_t kMaxLength = N;

  Code() = default;

  explicit Code(std::string_view value) { Assign(value); }

  Code &operator=(std::string_view value) {
    Assign(value);
    return *this;
  }

  void Assign(std::string_view value) {
    std::string normalised = detail::NormaliseCode(value);
    detail::CheckLength(normalised, N);
    Set(std::move(normalised));
  }

  [[nodiscard]] std::strong_ordering operator<=>(const Code &o) const {
    return detail::CompareCode(Stored(), o.Stored());
  }

  [[nodiscard]] bool operator==(const Code &o) const { return Stored() == o.Stored(); }
};

/// AL `MaxStrLen(String)` -- `text-maxstrlen-string-method.md`.
template <typename T> constexpr std::size_t MaxStrLen(const T & /*value*/) {
  return T::kMaxLength;
}

/// AL `StrLen(String)` -- `text-strlen-method.md`.
template <typename T> std::size_t StrLen(const T &value) {
  return value.Length();
}

} // namespace agiru

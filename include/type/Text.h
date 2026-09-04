#pragma once

#include "type/StringValue.h"

#include <compare>
#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>

/// \file
/// \brief AL `Text[N]`.

namespace agiru {

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
///
/// \note `Text<0>` IS DECLARED IN `StringValue.h` AND NOT HERE. Its methods return a text, and a
///       member defined inside a class body needs its return type COMPLETE -- so the unbounded
///       text has to be complete where `StringValue` is, and this header keeps the sized ones.

/// \brief AL `Text[N]`.
///
/// \tparam N The declared length, in UTF-16 code units.
///
/// The length is a compile-time property because AL declares it that way, which makes `MaxStrLen` a
/// constant; it is ALSO carried in the value, because a `var Text` parameter binds to the base and
/// has to check the same limit. The BaseApp uses 64 distinct lengths (measured 2026-09-01), so the
/// instantiation count is bounded; the work itself lives in non-template functions.
///
/// \see `text-data-type.md`
template <std::size_t N> class Text : public Text<0> {
public:
  /// \brief The declared length.
  static constexpr std::size_t kMaxLength = N;

  /// \brief An empty text of the declared length.
  Text() : Text<0>(N) {}

  /// \brief Constructs from anything that reads as text.
  /// \tparam T The source, which must read as a `std::string_view`.
  /// \param value The text.
  /// \throws StringError when it is longer than N.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  Text(const T &value) : Text<0>(N) {
    Assign(std::string_view(value));
  }

  /// \brief Assigns anything that reads as text.
  /// \tparam T The source, which must read as a `std::string_view`.
  /// \param value The text.
  /// \return This object.
  /// \throws StringError when it is longer than N.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  Text &operator=(const T &value) {
    Assign(std::string_view(value));
    return *this;
  }
};

/// \brief AL `MaxStrLen(String)`.
/// \tparam T A Text or Code.
/// \return The declared length.
/// \see `text-maxstrlen-string-method.md`
/// \note AN UNBOUNDED TEXT DOES NOT HAVE A MAXIMUM OF ZERO. `text-data-type.md` gives the
///       platform's own limit for a Text declared without a length, and the predecessor measured
///       what returning the current length instead cost: every filter-building loop written as
///       `StrLen(Filter) + ... <= MaxStrLen(Filter)` stopped after its first iteration.
constexpr Integer kUnboundedLength = 2147483647;

/// \brief AL `MaxStrLen(String)`.
/// \tparam T A Text or Code.
/// \param value The string.
/// \return Its DECLARED length, which is the platform's own limit when it was declared without one.
template <typename T> Integer MaxStrLen(const T &value) {
  const std::size_t declared = value.Max();
  return declared == 0 ? kUnboundedLength : static_cast<Integer>(declared);
}

/// \brief AL `StrLen(String)`.
///
/// \param value The string.
/// \return Its length in UTF-16 code units.
/// \see `text-strlen-method.md`
/// \note IT TAKES A `string_view` RATHER THAN A `Text`, because AL's argument is a text EXPRESSION:
///       `StrLen(DelChr(S, '=', ','))` hands it the result of another builtin, which carries no
///       declared length and is therefore not a `Text<N>`. Text and Code convert to it.
[[nodiscard]] inline Integer StrLen(std::string_view value) {
  return static_cast<Integer>(detail::Utf16Length(value));
}

}

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
template <std::size_t N> class Text : public StringValue {
public:
  /// \brief The declared length.
  static constexpr std::size_t kMaxLength = N;

  /// \brief An empty text.
  Text() = default;

  /// \brief Constructs from anything that reads as text.
  ///
  /// \tparam T The source, which must read as a `std::string_view`.
  /// \param value The text.
  ///
  /// \throws StringError when it does not fit N.
  ///
  /// \note ONE CONSTRAINED TEMPLATE AND NOT TWO OVERLOADS, because two were ambiguous: a string
  ///       literal reaches `std::string_view` and `const std::string &` equally well. And NOT
  ///       EXPLICIT, because AL assigns text without ceremony -- a body writes
  ///       `AssignCompany(X, CompanyName())` where the parameter is a `Text` and the builtin
  ///       returns a string. What is still checked is the LENGTH, which is what AL checks too.
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions): see the note above.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  Text(const T &value) {
    Assign(std::string_view(value));
  }

  /// \brief Assigns anything that reads as text.
  ///
  /// \tparam T The source, which must read as a `std::string_view`.
  /// \param value The text.
  /// \return This object.
  /// \throws StringError when it does not fit the declared length.
  ///
  /// \note CONSTRAINED THE SAME WAY THE CONSTRUCTOR IS. With a converting constructor in place, a
  ///       plain assignment from a string view is ambiguous with the implicit copy assignment for
  ///       anything that reaches both, and a string literal reaches both.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  Text &operator=(const T &value) {
    Assign(std::string_view(value));
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

  /// \brief Compares against a literal, which is how AL writes an emptiness test.
  /// \param value The text.
  /// \return True when the stored text is identical.
  /// \note CONSTRAINED, for the reason the constructor is: with a converting constructor in
  ///       place, a plain equality against a string view is ambiguous with the one against another
  ///       Text for anything that reaches both, and a string literal reaches both.
  /// \tparam T The source, which must read as a `std::string_view`.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  [[nodiscard]] bool operator==(const T &value) const {
    return Stored() == std::string_view(value);
  }
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

#pragma once

#include "type/StringValue.h"

#include <compare>
#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

/// \file
/// \brief AL `Code[N]`.

namespace agiru {

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

  /// \brief Constructs from anything that reads as text.
  ///
  /// \tparam T The source, which must read as a `std::string_view`.
  /// \param value The text.
  ///
  /// \throws StringError when the normalised value does not fit N.
  ///
  /// \note ONE CONSTRAINED TEMPLATE AND NOT TWO OVERLOADS, because two were ambiguous: a string
  ///       literal reaches `std::string_view` and `const std::string &` equally well. And NOT
  ///       EXPLICIT, because AL assigns text to a Code without ceremony. The NORMALISATION is what
  ///       `Assign` still does.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  Code(const T &value) {
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
  Code &operator=(const T &value) {
    Assign(std::string_view(value));
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

  /// \brief Compares against a literal, which is how AL writes an emptiness test.
  /// \param value The text.
  /// \return True when the stored text is identical.
  /// \note CONSTRAINED, for the reason the constructor is: with a converting constructor in
  ///       place, a plain equality against a string view is ambiguous with the one against another
  ///       Code for anything that reaches both, and a string literal reaches both.
  /// \tparam T The source, which must read as a `std::string_view`.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  [[nodiscard]] bool operator==(const T &value) const {
    return Stored() == std::string_view(value);
  }
};

} // namespace agiru

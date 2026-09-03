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
template <std::size_t N> class Code;

/// \brief AL `Code` -- a code with no declared length, and the base every sized one derives from.
///
/// A Code is uppercased and trimmed on every assignment, and ordered numerically when both sides
/// are entirely digits.
///
/// \note THE SIZED ONES DERIVE FROM IT for the reason `Text` gives: AL hands a `Code[20]` to a
///       `var Code` parameter, so a reference to the unbounded type must bind to the sized one.
///
/// \see `code-data-type.md`, detail::NormaliseCode, detail::CompareCode
template <> class Code<0> : public StringValue {
public:
  /// \brief The declared length, which is none.
  static constexpr std::size_t kMaxLength = 0;

  /// \brief An empty code.
  Code() = default;

  /// \brief Constructs from anything that reads as text.
  ///
  /// \tparam T The source, which must read as a `std::string_view`.
  /// \param value The text.
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

  /// \brief Copies the code and its declared length.
  /// \param o The other code.
  Code(const Code &o) = default;

  /// \brief Moves the code.
  /// \param o The other code.
  Code(Code &&o) = default;

  ~Code() = default;

  /// \brief Assigns anything that reads as text.
  /// \tparam T The source, which must read as a `std::string_view`.
  /// \param value The text.
  /// \return This object.
  /// \throws StringError when the normalised value does not fit the declared length.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  Code &operator=(const T &value) {
    Assign(std::string_view(value));
    return *this;
  }

  /// \brief Assigns another code, checking THIS one's declared length.
  /// \param o The other code.
  /// \return This object.
  /// \throws StringError when it does not fit.
  Code &operator=(const Code &o) {
    if (this != &o) { Assign(o.Value()); }
    return *this;
  }

  /// \brief The same, and the length still has to be checked.
  /// \param o The other code.
  /// \return This object.
  Code &operator=(Code &&o) noexcept(false) {
    if (this != &o) { Assign(o.Value()); }
    return *this;
  }

  /// \brief Assigns text: trim, uppercase, then check the declared length.
  /// \param value The text.
  /// \throws StringError when the normalised value is too long.
  void Assign(std::string_view value) {
    std::string normalised = detail::NormaliseCode(value);
    detail::CheckLength(normalised, Max());
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
  /// \tparam T The source, which must read as a `std::string_view` and must not be a string type
  ///           itself: a sized `Code` reaches both this and the one above, and C++20's reversed
  ///           candidate then makes the two ambiguous.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view> &&
             (!std::derived_from<T, StringValue>)
  [[nodiscard]] bool operator==(const T &value) const {
    return Stored() == std::string_view(value);
  }

protected:
  /// \brief A code with a declared length, for the sized specialisation.
  /// \param max The declared length.
  explicit Code(std::size_t max) : StringValue(max) {}
};

/// \brief AL `Code[N]`.
///
/// \tparam N The declared length, in UTF-16 code units, measured after normalisation.
///
/// \see `code-data-type.md`
template <std::size_t N> class Code : public Code<0> {
public:
  /// \brief The declared length.
  static constexpr std::size_t kMaxLength = N;

  /// \brief An empty code of the declared length.
  Code() : Code<0>(N) {}

  /// \brief Constructs from anything that reads as text.
  /// \tparam T The source, which must read as a `std::string_view`.
  /// \param value The text.
  /// \throws StringError when the normalised value is longer than N.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  Code(const T &value) : Code<0>(N) {
    Assign(std::string_view(value));
  }

  /// \brief Assigns anything that reads as text.
  /// \tparam T The source, which must read as a `std::string_view`.
  /// \param value The text.
  /// \return This object.
  /// \throws StringError when the normalised value is longer than N.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  Code &operator=(const T &value) {
    Assign(std::string_view(value));
    return *this;
  }
};

} // namespace agiru

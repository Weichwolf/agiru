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
template <std::size_t N> class Text;

/// \brief AL `Text` -- a text with no declared length, and the base every sized one derives from.
///
/// \note THE SIZED ONES DERIVE FROM IT BECAUSE AL PASSES ONE FOR THE OTHER. `var CityTxt: Text`
///       takes `Customer.City`, a `Text[30]`, in the BaseApp's own Customer table -- so a reference
///       to the unbounded type must bind to the sized one, and only a base can do that. What keeps
///       the assignment honest through such a reference is `Max()`, which is the VALUE's and not
///       the reference's.
template <> class Text<0> : public StringValue {
public:
  /// \brief The declared length, which is none.
  static constexpr std::size_t kMaxLength = 0;

  /// \brief An empty text.
  Text() = default;

  /// \brief Constructs from anything that reads as text.
  ///
  /// \tparam T The source, which must read as a `std::string_view`.
  /// \param value The text.
  ///
  /// \note ONE CONSTRAINED TEMPLATE AND NOT TWO OVERLOADS, because two were ambiguous: a string
  ///       literal reaches `std::string_view` and `const std::string &` equally well. And NOT
  ///       EXPLICIT, because AL assigns text without ceremony -- a body writes
  ///       `AssignCompany(X, CompanyName())` where the parameter is a `Text` and the builtin
  ///       returns a string.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  Text(const T &value) {
    Assign(std::string_view(value));
  }

  /// \brief Copies the text, keeping this one's declared length.
  /// \param o The other text.
  Text(const Text &o) = default;

  /// \brief Moves the text.
  /// \param o The other text.
  Text(Text &&o) = default;

  ~Text() = default;

  /// \brief Assigns anything that reads as text.
  ///
  /// \tparam T The source, which must read as a `std::string_view`.
  /// \param value The text.
  /// \return This object.
  /// \throws StringError when it does not fit the declared length.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  Text &operator=(const T &value) {
    Assign(std::string_view(value));
    return *this;
  }

  /// \brief Assigns another text, checking THIS one's declared length.
  ///
  /// \param o The other text.
  /// \return This object.
  /// \throws StringError when it does not fit.
  ///
  /// \note IT IS DECLARED RATHER THAN DEFAULTED, and that is the whole point of the hierarchy: the
  ///       compiler's own copy assignment would take the source's bytes without asking whether they
  ///       fit the target, which is exactly the check AL makes.
  Text &operator=(const Text &o) {
    if (this != &o) { Assign(o.Value()); }
    return *this;
  }

  /// \brief The same, moving nothing: the length still has to be checked.
  /// \param o The other text.
  /// \return This object.
  Text &operator=(Text &&o) noexcept(false) {
    if (this != &o) { Assign(o.Value()); }
    return *this;
  }

  /// \brief Assigns text, checking the declared length.
  /// \param value The text.
  /// \throws StringError when it is longer than the declared length.
  void Assign(std::string_view value) {
    detail::CheckLength(value, Max());
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
  /// \tparam T The source, which must read as a `std::string_view` and must not be a string type
  ///           itself: a sized `Text` reaches both this and the one above, and C++20's reversed
  ///           candidate then makes the two ambiguous.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view> &&
             (!std::derived_from<T, StringValue>)
  [[nodiscard]] bool operator==(const T &value) const {
    return Stored() == std::string_view(value);
  }

protected:
  /// \brief A text with a declared length, for the sized specialisation.
  /// \param max The declared length.
  explicit Text(std::size_t max) : StringValue(max) {}
};

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

} // namespace agiru

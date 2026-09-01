#pragma once

#include "agiru/StringValue.h"

#include <compare>
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

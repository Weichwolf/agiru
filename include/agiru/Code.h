#pragma once

#include "agiru/StringValue.h"

#include <compare>
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

} // namespace agiru

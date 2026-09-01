#pragma once

#include <cstdint>
#include <string>

/// \file
/// \brief AL `BigInteger`.

namespace agiru {

/// \brief AL `BigInteger`. \see `biginteger-data-type.md`
using BigInteger = std::int64_t;

/// \brief The range AL gives `BigInteger`, also symmetric. \see `biginteger-data-type.md`
struct BigIntegerRange {
  static constexpr BigInteger kMinimum = -9223372036854775807LL; ///< The smallest AL BigInteger.
  static constexpr BigInteger kMaximum = 9223372036854775807LL;  ///< The largest AL BigInteger.
};

/// \brief AL `BigInteger.ToText()`.
/// \param value The number.
/// \return Its digits, invariant.
/// \see `biginteger-totext-method.md`
[[nodiscard]] std::string ToText(BigInteger value);

} // namespace agiru

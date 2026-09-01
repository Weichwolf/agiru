#pragma once

#include <cstdint>
#include <string>

/// \file
/// \brief AL `Integer`.

namespace agiru {

/// \brief AL `Integer`.
///
/// An alias rather than a class, and deliberately: generated AL code does arithmetic on integers
/// constantly -- loop counters, entry numbers, quantities -- and a wrapper would either forward
/// every operator or change how that code reads. AL's Integer IS a 32-bit whole number; what a
/// wrapper would add is the RANGE, and the range belongs to the field rather than to the value,
/// which is where AL enforces it too.
///
/// \see `integer-data-type.md`
using Integer = std::int32_t;

/// \brief The range AL gives its whole-number types, which is SYMMETRIC.
///
/// `integer-data-type.md`: "Stores whole numbers with values that range from -2,147,483,647 to
/// 2,147,483,647" -- so the most negative 32-bit value is NOT an AL Integer, and the same holds one
/// size up. A field assignment that would land there is out of range rather than merely unusual.
struct IntegerRange {
  static constexpr Integer kMinimum = -2147483647; ///< The smallest AL Integer.
  static constexpr Integer kMaximum = 2147483647;  ///< The largest AL Integer.
};

/// \brief AL `Integer.ToText()`.
/// \param value The number.
/// \return Its digits, invariant.
/// \see `integer-totext-method.md`
[[nodiscard]] std::string ToText(Integer value);

} // namespace agiru

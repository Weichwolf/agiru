#pragma once

#include "type/Decimal.h"

#include <compare>

namespace agiru {

/// \brief Numeric policy qualified against BC 29.0.54011.55407 Decimal18 (WI 0066).
///
/// The underlying Decimal remains CLR-compatible. This policy rounds mantissas to
/// eighteen significant digits, ties away from zero, retaining scale up to 28.
/// It is not field-range validation, SQL rounding or display formatting. Generated
/// AL activation and target-version boundary qualification remain open in WI 0073.
class AlDecimalArithmetic {
public:
  /// \brief Applies the native significance policy without modifying the input.
  /// \param value A CLR Decimal, including its representation scale.
  /// \return The normalized value; unchanged representation below 10^18 mantissa
  ///         units. Rounded fractional results lose trailing zeros; integer rounding
  ///         saturates at the greatest eighteen-digit multiple within the CLR range.
  [[nodiscard]] static Decimal Normalize(const Decimal &value);

  /// \brief Normalizes owned operands, adds through the CLR core, then normalizes.
  /// \param left First operand. \param right Second operand.
  /// \return The AL-policy sum. \throws DecimalError on CLR overflow.
  [[nodiscard]] static Decimal Add(Decimal left, Decimal right);

  /// \brief Normalizes owned operands, subtracts through CLR, then normalizes.
  /// \param left Minuend. \param right Subtrahend.
  /// \return The AL-policy difference. \throws DecimalError on CLR overflow.
  [[nodiscard]] static Decimal Subtract(Decimal left, Decimal right);

  /// \brief Normalizes owned operands, multiplies through CLR, then normalizes.
  /// \param left First factor. \param right Second factor.
  /// \return The AL-policy product. \throws DecimalError on CLR overflow.
  [[nodiscard]] static Decimal Multiply(Decimal left, Decimal right);

  /// \brief Normalizes owned operands, divides through CLR, then normalizes.
  /// \param left Dividend. \param right Divisor.
  /// \return The AL-policy quotient. \throws DecimalError on zero divisor/overflow.
  [[nodiscard]] static Decimal Divide(Decimal left, Decimal right);

  /// \brief Normalizes owned operands, takes the CLR remainder, then normalizes.
  /// \param left Dividend. \param right Divisor.
  /// \return The AL-policy remainder. \throws DecimalError on a zero divisor.
  [[nodiscard]] static Decimal Remainder(Decimal left, Decimal right);

  /// \brief Compares the normalized values, not the unconverted CLR operands.
  /// \param left First operand. \param right Second operand.
  /// \return The numeric ordering, independently of representation scale.
  [[nodiscard]] static std::strong_ordering Compare(Decimal left, Decimal right);
};

}

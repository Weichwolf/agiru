#pragma once

#include "runtime/Error.h"

#include <compare>
#include <cstdint>
#include <string>
#include <string_view>

/// \file
/// \brief AL's Decimal -- the .NET CLR decimal, digit for digit.

namespace agiru {

/// \brief An error raised by decimal arithmetic, such as an overflow or a division by zero.
class DecimalError : public Error {
public:
  using Error::Error;
};

/// \brief AL `Decimal`.
///
/// The platform documentation fixes the representation; it is not a choice.
/// `decimal-data-type.md`: "The Decimal data type is mapped to the Microsoft .NET Framework common
/// language runtime (CLR) Decimal data type, which controls the precision and limits", with a
/// maximum calculating value of 79'228'162'514'264'337'593'543'950'335 (two to the ninety-sixth
/// less one) and a scaling factor up to 28.
///
/// The value is the mantissa divided by ten to the power of the scale, with a sign.
///
/// \note THE SCALE IS PART OF THE VALUE. `0.10` and `0.1` compare equal but are not the same
///       decimal: CLR arithmetic carries the scale through addition, subtraction and
///       multiplication, and only division normalises. Ten thousand additions of `0.01` therefore
///       yield `100.00` rather than `100`, and a gate case states exactly that, because it is the
///       first thing anyone assumes wrongly.
class Decimal {
public:
  /// \return The largest representable magnitude, two to the ninety-sixth less one.
  static const Decimal &MaxValue();

  /// \return The negative of MaxValue().
  static const Decimal &MinValue();

  /// \brief Zero.
  constexpr Decimal() = default;

  /// \brief Constructs from a whole number.
  /// \param value The integer value.
  explicit Decimal(std::int64_t value);

  /// \brief Renders the value for round-tripping: sign, digits, a full stop, scale preserved.
  ///
  /// \return The invariant notation, with no grouping.
  ///
  /// \warning This is NOT AL `Decimal.ToText()`, and it does not carry that name for exactly that
  ///          reason. `decimal-totext--method.md` defines `ToText()` as `Format(value, 0, 0)`, and
  ///          `devenv-format-property.md` shows Standard Format 0 to be locale dependent WITH
  ///          thousands separators. AL's `ToText` needs a locale and a field's `DecimalPlaces`;
  ///          neither exists at this layer (board:0007).
  [[nodiscard]] std::string ToInvariantString() const;

  /// \brief Reads the invariant notation.
  ///
  /// \param text The text, optionally signed, with at most one full stop.
  /// \return The value, preserving the written scale as CLR parsing does, so `1.2300` keeps four
  ///         decimal places.
  /// \throws DecimalError when the text is not a number, or carries more than 28 decimal places.
  static Decimal FromInvariantString(std::string_view text);

  /// \return True when the value is zero, whatever its scale.
  [[nodiscard]] bool IsZero() const { return units_ == 0; }

  /// \return True when the value is negative and not zero.
  [[nodiscard]] bool IsNegative() const { return negative_ && units_ != 0; }

  /// \return The number of decimal places the value currently carries.
  [[nodiscard]] std::uint8_t Scale() const { return scale_; }

  /// \return The magnitude, with the same scale.
  [[nodiscard]] Decimal Abs() const;

  /// \return The value with its sign flipped; zero stays unsigned.
  [[nodiscard]] Decimal operator-() const;

  /// \brief Adds, aligning the scales and keeping the wider one.
  /// \param o The addend.
  /// \return This object.
  /// \throws DecimalError on overflow.
  Decimal &operator+=(const Decimal &o);

  /// \brief Subtracts, aligning the scales and keeping the wider one.
  /// \param o The subtrahend.
  /// \return This object.
  /// \throws DecimalError on overflow.
  Decimal &operator-=(const Decimal &o);

  /// \brief Multiplies, carrying the sum of the two scales as CLR does.
  /// \param o The multiplier.
  /// \return This object.
  /// \throws DecimalError on overflow.
  Decimal &operator*=(const Decimal &o);

  /// \brief Divides, filling up to 28 decimal places and normalising the result.
  /// \param o The divisor.
  /// \return This object.
  /// \throws DecimalError when the divisor is zero, or on overflow.
  Decimal &operator/=(const Decimal &o);

  /// \brief Adds two values.
  /// \param a Left operand.
  /// \param b Right operand.
  /// \return The sum.
  friend Decimal operator+(Decimal a, const Decimal &b) { return a += b; }

  /// \brief Subtracts two values.
  /// \param a Left operand.
  /// \param b Right operand.
  /// \return The difference.
  friend Decimal operator-(Decimal a, const Decimal &b) { return a -= b; }

  /// \brief Multiplies two values.
  /// \param a Left operand.
  /// \param b Right operand.
  /// \return The product.
  friend Decimal operator*(Decimal a, const Decimal &b) { return a *= b; }

  /// \brief Divides two values.
  /// \param a Left operand.
  /// \param b Right operand.
  /// \return The quotient.
  friend Decimal operator/(Decimal a, const Decimal &b) { return a /= b; }

  /// \brief Orders by value rather than by representation, so `1.50` equals `1.5`.
  /// \param o The other value.
  /// \return The ordering.
  [[nodiscard]] std::strong_ordering operator<=>(const Decimal &o) const;

  /// \brief Compares by value rather than by representation.
  /// \param o The other value.
  /// \return True when the values are numerically equal.
  [[nodiscard]] bool operator==(const Decimal &o) const {
    return (*this <=> o) == std::strong_ordering::equal;
  }

private:
  friend class DecimalAccess;
  __extension__ using U128 = unsigned __int128;

  struct Repr {
    U128 units;
    std::uint8_t scale;
    bool negative;
  };

  explicit Decimal(Repr r);

  U128 units_{0};
  std::uint8_t scale_{0};
  bool negative_{false};
};

/// \brief AL's three rounding directions.
///
/// From `system-round-method.md`: "'=' rounds up or down to the nearest value (default). Values of
/// 5 or greater are rounded up. ... '>' rounds up ... '<' rounds down".
///
/// \warning THEY ROUND BY MAGNITUDE, not mathematically. The trap is recorded in the predecessor
///          (openerp `builtins/_math.py:_al_round`), measured and paid for: for -1234.56789 at
///          0.001, Down yields -1234.567 (toward zero) and Up yields -1234.568 (away from zero).
///          Ceiling and floor invert both for negative numbers, and negative amounts are the rule
///          in an ERP rather than the exception: credit memos, reversals, negative deltas.
enum class RoundDirection : std::uint8_t {
  Nearest, ///< '=' -- nearest multiple; exactly five rounds away from zero.
  Up,      ///< '>' -- away from zero.
  Down,    ///< '<' -- toward zero.
};

/// \brief AL `System.Round(Number, Precision, Direction)`.
///
/// \param number    The value to round.
/// \param precision The MULTIPLE to round to -- 0.01 for hundredths, 0.05 for five-cent steps.
///                  Both occur in BC.
/// \param direction Which way to go when the value falls between two multiples.
/// \return The rounded value.
/// \throws DecimalError when the precision is zero.
///
/// \note The documentation names the default precision as `Amount Rounding Precision` from GLSetup
///       via Codeunit 45, falling back to two decimal places. That default is known to the runtime,
///       not to the value type, so it is not defaulted here.
/// \see `system-round-method.md`
Decimal Round(const Decimal &number,
              const Decimal &precision,
              RoundDirection direction = RoundDirection::Nearest);

} // namespace agiru

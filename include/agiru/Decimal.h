#pragma once

#include "agiru/Error.h"

#include <compare>
#include <cstdint>
#include <string>
#include <string_view>

namespace agiru {

/// AL `Decimal` -- the .NET CLR `System.Decimal`, digit for digit.
///
/// The platform documentation fixes the representation; it is not a choice.
/// `methods-auto/decimal/decimal-data-type.md`: "The Decimal data type is mapped to the Microsoft
/// .NET Framework common language runtime (CLR) Decimal data type, which controls the precision and
/// limits", maximum calculating value +/- 79'228'162'514'264'337'593'543'950'335 (= 2^96 - 1),
/// scaling factor up to 28.
///
/// Value = (-1)^negative * units / 10^scale.
///
/// THE SCALE IS PART OF THE VALUE. `0.10` and `0.1` compare equal but are not the same decimal:
/// CLR arithmetic carries the scale through addition, subtraction and multiplication, and only
/// division normalises. Ten thousand additions of `0.01` therefore yield `100.00`, not `100` --
/// a gate case states exactly that, because it is the first thing anyone assumes wrongly.
class Decimal {
public:
  /// Largest magnitude the mantissa can hold: 2^96 - 1, from the documentation.
  static const Decimal &MaxValue();
  static const Decimal &MinValue();

  constexpr Decimal() = default;
  explicit Decimal(std::int64_t value);

  /// Round-trip text: sign, digits, `.` as separator, no grouping, scale preserved.
  ///
  /// THIS IS NOT AL `Decimal.ToText()`, and it does not carry that name for exactly that reason.
  /// `decimal-totext--method.md` defines `ToText()` as `Format(value, 0, 0)`, and
  /// `devenv-format-property.md` shows Standard Format 0 to be locale dependent WITH thousands
  /// separators -- `-76,543.21` under US, `-76.543,21` under a European region. AL's `ToText` needs
  /// a locale and a field's `DecimalPlaces`; neither exists at this layer (board:0007).
  [[nodiscard]] std::string ToInvariantString() const;

  /// Reads the invariant notation and preserves the written scale, as CLR parsing does:
  /// `1.2300` keeps four decimal places. Throws `DecimalError` when the text is not a number.
  static Decimal FromInvariantString(std::string_view text);

  [[nodiscard]] bool IsZero() const { return units_ == 0; }

  [[nodiscard]] bool IsNegative() const { return negative_ && units_ != 0; }

  [[nodiscard]] std::uint8_t Scale() const { return scale_; }

  [[nodiscard]] Decimal Abs() const;

  [[nodiscard]] Decimal operator-() const;
  Decimal &operator+=(const Decimal &o);
  Decimal &operator-=(const Decimal &o);
  Decimal &operator*=(const Decimal &o);
  Decimal &operator/=(const Decimal &o);

  friend Decimal operator+(Decimal a, const Decimal &b) { return a += b; }

  friend Decimal operator-(Decimal a, const Decimal &b) { return a -= b; }

  friend Decimal operator*(Decimal a, const Decimal &b) { return a *= b; }

  friend Decimal operator/(Decimal a, const Decimal &b) { return a /= b; }

  /// Compares by value, not by representation: `1.50` equals `1.5`.
  std::strong_ordering operator<=>(const Decimal &o) const;

  bool operator==(const Decimal &o) const { return (*this <=> o) == std::strong_ordering::equal; }

private:
  friend class DecimalAccess;
  __extension__ using U128 = unsigned __int128;

  /// The internal shape, named so that a call site cannot swap the mantissa for the scale.
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

/// AL knows three directions, and they round by MAGNITUDE rather than mathematically.
/// `methods-auto/system/system-round-method.md`: "'=' rounds up or down to the nearest value
/// (default). Values of 5 or greater are rounded up. ... '>' rounds up ... '<' rounds down".
///
/// THE TRAP IS RECORDED IN THE PREDECESSOR (openerp `builtins/_math.py:_al_round`), measured and
/// paid for: '>' and '<' work on the MAGNITUDE, not on `ceil`/`floor`. For -1234.56789 at 0.001,
/// '<' yields -1234.567 (toward zero) and '>' yields -1234.568 (away from zero). `ceil`/`floor`
/// invert both for negative numbers -- and negative amounts are the rule in an ERP rather than the
/// exception: credit memos, reversals, negative deltas.
enum class RoundDirection : std::uint8_t {
  Nearest, ///< '=' -- nearest multiple; exactly five rounds away from zero
  Up,      ///< '>' -- away from zero
  Down,    ///< '<' -- toward zero
};

/// AL `System.Round(Number, Precision, Direction)`.
///
/// `precision` is a MULTIPLE, not a digit count: 0.01 rounds to hundredths, 0.05 to five-cent
/// steps -- both occur in BC. A precision of zero is an error. The documentation names the default
/// as `Amount Rounding Precision` from GLSetup via Codeunit 45, falling back to two decimal places;
/// that default is known to the runtime, not to the value type.
Decimal Round(const Decimal &number,
              const Decimal &precision,
              RoundDirection direction = RoundDirection::Nearest);

class DecimalError : public Error {
public:
  using Error::Error;
};

} // namespace agiru

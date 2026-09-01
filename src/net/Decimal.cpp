#include "agiru/Decimal.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace agiru {

/// The private constructor, for the arithmetic in this file. It is not in the door because
/// (mantissa, scale, sign) is not an AL notion.
class DecimalAccess {
public:
  __extension__ using U128 = unsigned __int128;

  static Decimal Make(U128 units, std::uint8_t scale, bool negative) {
    return Decimal(Decimal::Repr{.units = units, .scale = scale, .negative = negative});
  }

  static U128 Units(const Decimal &d) { return d.units_; }
};

namespace {

__extension__ using U128 = unsigned __int128;

constexpr std::uint8_t kMaxScale = 28;
constexpr unsigned kMantissaBits = 96;
constexpr U128 kMaxUnits = (static_cast<U128>(1) << kMantissaBits) - 1;

/// Every number below carries its origin: the limb width of the wide integer, the index of its top
/// bit, how many limbs make 192 bits, and the digit at which AL's '=' rounds away from zero
/// ("Values of 5 or greater are rounded up", system-round-method.md).
constexpr unsigned kLimbBits = 64;
constexpr unsigned kTopBit = kLimbBits - 1;
constexpr std::size_t kLimbs = 3;
constexpr unsigned kWideBits = kLimbBits * kLimbs;
constexpr std::uint64_t kRoundUpAtDigit = 5;

/// 192 bits as three limbs, least significant first. The product of two 96-bit mantissas fits.
/// Deliberately the simplest correct shape: speed is not yet a question here, correctness is the
/// whole question (board:0008).
class U192 {
public:
  U192() = default;

  static U192 From(U128 v) {
    U192 r;
    r.w_[0] = static_cast<std::uint64_t>(v);
    r.w_[1] = static_cast<std::uint64_t>(v >> kLimbBits);
    return r;
  }

  [[nodiscard]] bool FitsU128() const { return w_[2] == 0; }

  [[nodiscard]] U128 ToU128() const { return (static_cast<U128>(w_[1]) << kLimbBits) | w_[0]; }

  [[nodiscard]] bool Bit(std::size_t i) const {
    return ((w_[i / kLimbBits] >> (i % kLimbBits)) & 1U) != 0;
  }

  void ShiftLeft1() {
    w_[2] = (w_[2] << 1U) | (w_[1] >> kTopBit);
    w_[1] = (w_[1] << 1U) | (w_[0] >> kTopBit);
    w_[0] <<= 1U;
  }

  void SetBit(std::size_t i) { w_[i / kLimbBits] |= (std::uint64_t{1} << (i % kLimbBits)); }

  void Increment() {
    for (std::uint64_t &limb : w_) {
      limb += 1;
      if (limb != 0) { return; }
    }
  }

  /// Times ten. `Multiply` takes a 128-bit operand, so this walks the limbs instead.
  ///
  /// IT REPORTS ITS OWN OVERFLOW. The first version dropped the final carry silently and the
  /// callers guarded with a hand-picked bound on the top limb -- a guess where an exact answer was
  /// available. `false` means the result did not fit 192 bits and `out` is not usable.
  [[nodiscard]] bool TimesTen(U192 &out) const {
    U128 carry = 0;
    for (std::size_t i = 0; i < kLimbs; ++i) {
      const U128 t = static_cast<U128>(w_[i]) * 10 + carry;
      out.w_[i] = static_cast<std::uint64_t>(t);
      carry = t >> kLimbBits;
    }
    return carry == 0;
  }

  /// Divides by a small number and returns the remainder. Carries the scale reduction.
  std::uint64_t DivModSmall(std::uint64_t d) {
    U128 rem = 0;
    for (std::size_t i = kLimbs; i-- > 0;) {
      const U128 cur = (rem << kLimbBits) | w_[i];
      w_[i] = static_cast<std::uint64_t>(cur / d);
      rem = cur % d;
    }
    return static_cast<std::uint64_t>(rem);
  }

  /// Bitwise long division by `divisor`: 192 rounds, obviously correct, slow -- wanted in that
  /// order (board:0008). `*this` is the dividend; the remainder is written to `rem`.
  [[nodiscard]] U192 DividedBy(const U192 &divisor, U192 &rem) const {
    U192 q;
    rem = U192{};
    for (std::size_t i = kWideBits; i-- > 0;) {
      rem.ShiftLeft1();
      if (Bit(i)) { rem.w_[0] |= 1U; }
      if (rem >= divisor) {
        rem -= divisor;
        q.SetBit(i);
      }
    }
    return q;
  }

  std::strong_ordering operator<=>(const U192 &o) const {
    for (std::size_t i = kLimbs; i-- > 0;) {
      if (w_[i] != o.w_[i]) {
        return w_[i] < o.w_[i] ? std::strong_ordering::less : std::strong_ordering::greater;
      }
    }
    return std::strong_ordering::equal;
  }

  bool operator==(const U192 &o) const { return w_ == o.w_; }

  U192 &operator+=(const U192 &o) {
    U128 carry = 0;
    for (std::size_t i = 0; i < kLimbs; ++i) {
      const U128 t = static_cast<U128>(w_[i]) + o.w_[i] + carry;
      w_[i] = static_cast<std::uint64_t>(t);
      carry = t >> kLimbBits;
    }
    return *this;
  }

  U192 &operator-=(const U192 &o) {
    std::uint64_t borrow = 0;
    for (std::size_t i = 0; i < kLimbs; ++i) {
      const std::uint64_t lhs = w_[i];
      const std::uint64_t rhs = o.w_[i];
      w_[i] = lhs - rhs - borrow;
      borrow =
          (lhs < rhs || (lhs == rhs && borrow == 1) || (lhs > rhs && lhs - rhs < borrow)) ? 1 : 0;
    }
    return *this;
  }

  /// 96 x 96 bits -> 192 bits, schoolbook over four 64-bit halves.
  [[nodiscard]] U192 MultipliedBy(U128 b) const {
    const U128 a = ToU128();
    const auto a0 = static_cast<std::uint64_t>(a);
    const auto a1 = static_cast<std::uint64_t>(a >> kLimbBits);
    const auto b0 = static_cast<std::uint64_t>(b);
    const auto b1 = static_cast<std::uint64_t>(b >> kLimbBits);

    const U128 p00 = static_cast<U128>(a0) * b0;
    const U128 p01 = static_cast<U128>(a0) * b1;
    const U128 p10 = static_cast<U128>(a1) * b0;
    const U128 p11 = static_cast<U128>(a1) * b1;

    U192 r;
    r.w_[0] = static_cast<std::uint64_t>(p00);
    const U128 mid =
        (p00 >> kLimbBits) + static_cast<std::uint64_t>(p01) + static_cast<std::uint64_t>(p10);
    r.w_[1] = static_cast<std::uint64_t>(mid);
    r.w_[2] = static_cast<std::uint64_t>((mid >> kLimbBits) + (p01 >> kLimbBits) +
                                         (p10 >> kLimbBits) + p11);
    return r;
  }

private:
  std::array<std::uint64_t, kLimbs> w_{{0, 0, 0}};
};

/// Lowers the scale by one, rounding away from zero -- the rule the documentation states for '=':
/// "Values of 5 or greater are rounded up."
void ReduceScale(U192 &v, std::uint8_t &scale) {
  if (v.DivModSmall(10) >= kRoundUpAtDigit) { v.Increment(); }
  --scale;
}

/// Brings a magnitude down to 96 bits by lowering the scale. If it does not fit even at scale 0
/// it is an overflow -- and an overflow is LOUD.
U128 FitToUnits(U192 v, std::uint8_t &scale) {
  while (scale > kMaxScale) { ReduceScale(v, scale); }
  while (!v.FitsU128() || v.ToU128() > kMaxUnits) {
    if (scale == 0) { throw DecimalError("Decimal: overflow beyond 2^96 - 1"); }
    ReduceScale(v, scale);
  }
  return v.ToU128();
}

/// Strips trailing zeros. Division is the one operation CLR normalises: 1/2 is 0.5, not 0.5000...
void StripTrailingZeros(U128 &units, std::uint8_t &scale) {
  while (scale > 0 && units % 10 == 0) {
    units /= 10;
    --scale;
  }
  if (units == 0) { scale = 0; }
}

/// Brings two values to a common scale. Where the mantissa will not stretch, the common scale
/// drops.
void Align(U192 &a, std::uint8_t &sa, U192 &b, std::uint8_t &sb) {
  while (sa < sb) {
    U192 t;
    if (!a.TimesTen(t) || !t.FitsU128()) { break; }
    a = t;
    ++sa;
  }
  while (sb < sa) {
    U192 t;
    if (!b.TimesTen(t) || !t.FitsU128()) { break; }
    b = t;
    ++sb;
  }
  while (sa > sb) { ReduceScale(a, sa); }
  while (sb > sa) { ReduceScale(b, sb); }
}

std::string U128ToString(U128 v) {
  if (v == 0) { return "0"; }
  std::string s;
  while (v != 0) {
    s.push_back(static_cast<char>('0' + static_cast<int>(v % 10)));
    v /= 10;
  }
  std::ranges::reverse(s);
  return s;
}

/// One half: the threshold '=' compares against, built from the documented rule rather than
/// written as a bare literal.
Decimal kHalf() {
  return DecimalAccess::Make(kRoundUpAtDigit, 1, false);
}

/// Truncates the fractional digits without rounding -- the magnitude, never mathematically.
Decimal TruncateMagnitude(const Decimal &d) {
  U128 u = DecimalAccess::Units(d);
  for (std::uint8_t i = 0; i < d.Scale(); ++i) { u /= 10; }
  return DecimalAccess::Make(u, 0, false);
}

} // namespace

Decimal::Decimal(Repr r) : units_(r.units), scale_(r.scale), negative_(r.negative) {}

Decimal::Decimal(std::int64_t value)
    : units_(value < 0 ? static_cast<U128>(-(value + 1)) + 1 : static_cast<U128>(value)),
      negative_(value < 0) {}

const Decimal &Decimal::MaxValue() {
  static const Decimal v = DecimalAccess::Make(kMaxUnits, 0, false);
  return v;
}

const Decimal &Decimal::MinValue() {
  static const Decimal v = DecimalAccess::Make(kMaxUnits, 0, true);
  return v;
}

Decimal Decimal::Abs() const {
  return DecimalAccess::Make(units_, scale_, false);
}

Decimal Decimal::operator-() const {
  return DecimalAccess::Make(units_, scale_, units_ != 0 && !negative_);
}

std::string Decimal::ToInvariantString() const {
  std::string digits = U128ToString(units_);
  if (scale_ != 0) {
    if (digits.size() <= scale_) { digits.insert(0, scale_ + 1 - digits.size(), '0'); }
    digits.insert(digits.size() - scale_, ".");
  }
  return (IsNegative() ? "-" : "") + digits;
}

Decimal Decimal::FromInvariantString(std::string_view text) {
  std::size_t i = 0;
  while (i < text.size() && (std::isspace(static_cast<unsigned char>(text[i])) != 0)) { ++i; }
  bool neg = false;
  if (i < text.size() && (text[i] == '+' || text[i] == '-')) {
    neg = text[i] == '-';
    ++i;
  }
  U128 units = 0;
  std::uint8_t scale = 0;
  bool seenDigit = false;
  bool seenPoint = false;
  for (; i < text.size(); ++i) {
    const char c = text[i];
    if (c == '.') {
      if (seenPoint) { throw DecimalError("Decimal: second decimal point"); }
      seenPoint = true;
      continue;
    }
    if (std::isdigit(static_cast<unsigned char>(c)) == 0) {
      throw DecimalError("Decimal: not a number");
    }
    seenDigit = true;
    if (units > kMaxUnits / 10) { throw DecimalError("Decimal: overflow while parsing"); }
    units = units * 10 + static_cast<unsigned>(c - '0');
    if (seenPoint) {
      if (scale == kMaxScale) { throw DecimalError("Decimal: more than 28 decimal places"); }
      ++scale;
    }
  }
  if (!seenDigit) { throw DecimalError("Decimal: no digit"); }
  // No stripping: CLR parsing preserves the written scale, so `1.2300` keeps four places.
  return DecimalAccess::Make(units, scale, neg && units != 0);
}

Decimal &Decimal::operator+=(const Decimal &o) {
  U192 a = U192::From(units_);
  U192 b = U192::From(o.units_);
  std::uint8_t sa = scale_;
  std::uint8_t sb = o.scale_;
  Align(a, sa, b, sb);

  bool neg = negative_;
  U192 sum;
  if (negative_ == o.negative_) {
    sum = a;
    sum += b;
  } else if (a >= b) {
    sum = a;
    sum -= b;
  } else {
    sum = b;
    sum -= a;
    neg = o.negative_;
  }

  units_ = FitToUnits(sum, sa);
  scale_ = sa;
  negative_ = neg && units_ != 0;
  return *this;
}

Decimal &Decimal::operator-=(const Decimal &o) {
  return *this += -o;
}

Decimal &Decimal::operator*=(const Decimal &o) {
  const U192 p = U192::From(units_).MultipliedBy(o.units_);
  auto scale = static_cast<std::uint8_t>(scale_ + o.scale_);
  units_ = FitToUnits(p, scale);
  scale_ = scale;
  // No stripping: CLR multiplication carries scale s1 + s2, so 2.50 * 2 is 5.00.
  negative_ = (negative_ != o.negative_) && units_ != 0;
  return *this;
}

Decimal &Decimal::operator/=(const Decimal &o) {
  if (o.units_ == 0) { throw DecimalError("Decimal: division by zero"); }
  if (units_ == 0) { return *this; }

  // Scale up as far as 28 decimal places and 192 bits allow, then round.
  U192 num = U192::From(units_);
  int scale = static_cast<int>(scale_) - static_cast<int>(o.scale_);
  const U192 den = U192::From(o.units_);
  while (scale < kMaxScale) {
    U192 next;
    if (!num.TimesTen(next)) { break; }
    num = next;
    ++scale;
  }

  U192 rem;
  U192 q = num.DividedBy(den, rem);

  U192 twice = rem;
  twice.ShiftLeft1();
  if (twice >= den) { q.Increment(); }

  while (scale < 0) {
    U192 next;
    if (!q.TimesTen(next)) { throw DecimalError("Decimal: overflow while scaling the quotient"); }
    q = next;
    ++scale;
  }
  auto s = static_cast<std::uint8_t>(scale);
  units_ = FitToUnits(q, s);
  scale_ = s;
  negative_ = (negative_ != o.negative_) && units_ != 0;
  StripTrailingZeros(units_, scale_);
  return *this;
}

std::strong_ordering Decimal::operator<=>(const Decimal &o) const {
  if (units_ == 0 && o.units_ == 0) { return std::strong_ordering::equal; }
  if (IsNegative() != o.IsNegative()) {
    return IsNegative() ? std::strong_ordering::less : std::strong_ordering::greater;
  }
  U192 a = U192::From(units_);
  U192 b = U192::From(o.units_);
  std::uint8_t sa = scale_;
  std::uint8_t sb = o.scale_;
  Align(a, sa, b, sb);
  const std::strong_ordering c = a <=> b;
  if (!IsNegative()) { return c; }
  return c == std::strong_ordering::less      ? std::strong_ordering::greater
         : c == std::strong_ordering::greater ? std::strong_ordering::less
                                              : std::strong_ordering::equal;
}

Decimal Round(const Decimal &number, const Decimal &precision, RoundDirection direction) {
  if (precision.IsZero()) { throw DecimalError("Round: precision is zero"); }
  const Decimal p = precision.Abs();
  const Decimal quotient = number.Abs() / p;
  Decimal steps = TruncateMagnitude(quotient);
  const Decimal fraction = quotient - steps;

  switch (direction) {
    case RoundDirection::Nearest:
      if (fraction >= kHalf()) { steps += Decimal(1); }
      break;
    case RoundDirection::Up:
      if (!fraction.IsZero()) { steps += Decimal(1); }
      break;
    case RoundDirection::Down: break;
  }

  const Decimal result = steps * p;
  return number.IsNegative() ? -result : result;
}

} // namespace agiru

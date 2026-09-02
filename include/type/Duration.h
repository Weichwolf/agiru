#pragma once

#include <compare>
#include <cstdint>
#include <string>

/// \file
/// \brief AL `Duration` -- how long, in milliseconds, and possibly negative.

namespace agiru {

/// \brief AL `Duration`.
///
/// From `duration-data-type.md`: "Represents the difference between two DateTimes. This value can
/// be negative. It is stored as a 64-bit integer. The integer value is the number of milliseconds
/// during the duration." The page also gives the algebra outright:
///
///     DateTime - DateTime = Duration
///     DateTime - Duration = DateTime
///     DateTime + Duration = DateTime
///
/// \note A CLASS RATHER THAN AN ALIAS, AND IT WAS AN ALIAS UNTIL SOMETHING NEEDED THE DIFFERENCE.
///       `Duration` and `BigInteger` are both 64-bit integers, and while they were the same C++
///       type nothing could tell them apart -- `Variant` could not hold both, because AL asks
///       `IsDuration()` and `IsBigInteger()` as two questions and `std::variant` refuses a
///       duplicate alternative. `FieldRef` needs the same distinction to render a field at all. The
///       alias's argument was that generated AL code does arithmetic on durations constantly and a
///       wrapper would forward every operator; that argument survives, because the operators below
///       are exactly the ones it would have forwarded, and there are seven of them.
///
/// \note The construction from a number is IMPLICIT on purpose. The page says "the value of the
///       Duration data type can also be explicitly defined in milliseconds", and AL writes
///       `Wait := 1000;` -- so the generated line reads the way the AL line reads.
class Duration {
public:
  /// \brief No time at all.
  constexpr Duration() = default;

  /// \brief A number of milliseconds.
  /// \param milliseconds How long, negative for a duration that runs backwards.
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions) -- AL assigns a number
  // to a Duration directly, and the generated line has to read like the AL line.
  constexpr Duration(std::int64_t milliseconds) : milliseconds_(milliseconds) {}

  /// \return The count of milliseconds, which is what the page says a Duration IS.
  [[nodiscard]] constexpr std::int64_t Milliseconds() const { return milliseconds_; }

  /// \return True when no time passes.
  [[nodiscard]] constexpr bool IsZero() const { return milliseconds_ == 0; }

  /// \brief AL `Format(Duration, 0, 9)` -- the invariant text.
  ///
  /// \return The count of milliseconds as digits.
  ///
  /// \note Named for what it is rather than `ToText()`, because `duration-totext-method.md` says
  ///       ToText() is "equivalent to calling Format(value, 0, 0)", and format 0 renders a duration
  ///       in words -- `2 days 3 hours` -- in the session's language. There is no language here.
  [[nodiscard]] std::string ToInvariantString() const;

  /// \brief Adds two durations.
  /// \param o The other.
  /// \return Their sum.
  [[nodiscard]] constexpr Duration operator+(const Duration &o) const {
    return Duration{milliseconds_ + o.milliseconds_};
  }

  /// \brief Subtracts two durations.
  /// \param o The other.
  /// \return Their difference.
  [[nodiscard]] constexpr Duration operator-(const Duration &o) const {
    return Duration{milliseconds_ - o.milliseconds_};
  }

  /// \brief Reverses a duration.
  /// \return The same length, running the other way.
  [[nodiscard]] constexpr Duration operator-() const { return Duration{-milliseconds_}; }

  /// \brief Repeats a duration.
  /// \param factor How many times.
  /// \return The product.
  [[nodiscard]] constexpr Duration operator*(std::int64_t factor) const {
    return Duration{milliseconds_ * factor};
  }

  /// \brief Divides a duration.
  /// \param divisor By how much.
  /// \return The quotient, truncated toward zero as integer division is.
  [[nodiscard]] constexpr Duration operator/(std::int64_t divisor) const {
    return Duration{milliseconds_ / divisor};
  }

  /// \brief Adds to this duration.
  /// \param o The other.
  /// \return This duration.
  constexpr Duration &operator+=(const Duration &o) {
    milliseconds_ += o.milliseconds_;
    return *this;
  }

  /// \brief Subtracts from this duration.
  /// \param o The other.
  /// \return This duration.
  constexpr Duration &operator-=(const Duration &o) {
    milliseconds_ -= o.milliseconds_;
    return *this;
  }

  /// \brief Orders two durations.
  /// \param o The other.
  /// \return The ordering.
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const Duration &o) const = default;

  /// \brief Compares two durations.
  /// \param o The other.
  /// \return True when they are the same length.
  [[nodiscard]] constexpr bool operator==(const Duration &o) const = default;

private:
  std::int64_t milliseconds_{0};
};

/// \brief Repeats a duration, with the count written first.
/// \param factor How many times.
/// \param d      The duration.
/// \return The product.
[[nodiscard]] constexpr Duration operator*(std::int64_t factor, const Duration &d) {
  return d * factor;
}

} // namespace agiru

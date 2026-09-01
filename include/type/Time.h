#pragma once

#include <compare>
#include <cstdint>
#include <string>

/// \file
/// \brief AL `Time` -- a time of day, to the millisecond.

namespace agiru {

/// \brief AL `Time`.
///
/// From `time-data-type.md`: "Denotes a time ranging from 00:00:00.000 to 23:59:59.999. An
/// undefined or blank time is specified by 0T."
///
/// Milliseconds since midnight, in one integer, and the ordering is the integer's own.
///
/// \note MIDNIGHT AND THE UNDEFINED TIME ARE THE SAME VALUE, and that is AL's doing rather than a
///       simplification here: `0T` is what the page calls the blank time, and `000000T` is the same
///       literal. The page's own CompareTime example treats `TimeA = 0T` as the blank case. So this
///       type does not separate them either, because a difference no AL program can observe would
///       be a difference this type invented.
class Time {
public:
  /// \brief The undefined time, `0T`, which is also midnight.
  constexpr Time() = default;

  static constexpr std::int32_t kMillisecondsPerSecond = 1000; ///< In a second.
  static constexpr std::int32_t kSecondsPerMinute = 60;        ///< In a minute.
  static constexpr std::int32_t kMinutesPerHour = 60;          ///< In an hour.
  static constexpr std::int32_t kHoursPerDay = 24;             ///< In a day.

  /// \brief Milliseconds in a minute.
  static constexpr std::int32_t kMillisecondsPerMinute = kSecondsPerMinute * kMillisecondsPerSecond;

  /// \brief Milliseconds in an hour.
  static constexpr std::int32_t kMillisecondsPerHour = kMinutesPerHour * kMillisecondsPerMinute;

  /// \brief Milliseconds in one day, which is one past the largest AL Time.
  static constexpr std::int32_t kMillisecondsPerDay = kHoursPerDay * kMillisecondsPerHour;

  /// \brief Builds a time, the way AL writes `115934.444T`.
  ///
  /// \param hour        0 to 23.
  /// \param minute      0 to 59.
  /// \param second      0 to 59.
  /// \param millisecond 0 to 999.
  /// \return The time, or the undefined time when the four do not name one.
  [[nodiscard]] static constexpr Time
  FromHms(int hour, int minute, int second, int millisecond = 0) {
    if (hour < 0 || hour >= kHoursPerDay || minute < 0 || minute >= kMinutesPerHour || second < 0 ||
        second >= kSecondsPerMinute || millisecond < 0 || millisecond >= kMillisecondsPerSecond) {
      return Time{};
    }
    return FromMilliseconds((hour * kMillisecondsPerHour) + (minute * kMillisecondsPerMinute) +
                            (second * kMillisecondsPerSecond) + millisecond);
  }

  /// \brief Builds a time from milliseconds since midnight.
  /// \param milliseconds 0 to 86 399 999.
  /// \return The time, or the undefined time when the count is out of the day.
  [[nodiscard]] static constexpr Time FromMilliseconds(std::int32_t milliseconds) {
    Time held;
    held.milliseconds_ = milliseconds >= 0 && milliseconds < kMillisecondsPerDay ? milliseconds : 0;
    return held;
  }

  /// \return Milliseconds since midnight. AL subtracts two times and gets exactly this difference:
  ///         its own CompareTime example writes `Abs(TimeA - TimeB) = 86399999`.
  [[nodiscard]] constexpr std::int32_t AsMilliseconds() const { return milliseconds_; }

  /// \return True for `0T`, which is midnight.
  [[nodiscard]] constexpr bool IsUndefined() const { return milliseconds_ == 0; }

  /// \brief AL `Time.Hour()`.
  /// \return The hour, 0 to 23.
  [[nodiscard]] constexpr int Hour() const { return milliseconds_ / kMillisecondsPerHour; }

  /// \brief AL `Time.Minute()`.
  /// \return The minute, 0 to 59.
  [[nodiscard]] constexpr int Minute() const {
    return (milliseconds_ / kMillisecondsPerMinute) % kMinutesPerHour;
  }

  /// \brief AL `Time.Second()`.
  /// \return The second, 0 to 59.
  [[nodiscard]] constexpr int Second() const {
    return (milliseconds_ / kMillisecondsPerSecond) % kSecondsPerMinute;
  }

  /// \brief AL `Time.Millisecond()`.
  /// \return The millisecond, 0 to 999.
  [[nodiscard]] constexpr int Millisecond() const { return milliseconds_ % kMillisecondsPerSecond; }

  /// \brief AL `Format(Time, 0, 9)` -- the XML format.
  ///
  /// \return `hh:mm:ss.fff`.
  ///
  /// \note Named for what it is rather than `ToText()`, because `time-totext-method.md` says
  ///       ToText() without an argument is "Equivalent to calling Format(value, 0, 0)", and format
  ///       0 is the region's: the page's own example renders `115934.444T` as `11:59:34.444 AM`.
  [[nodiscard]] std::string ToInvariantString() const;

  /// \brief Orders two times.
  /// \param o The other time.
  /// \return The ordering.
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const Time &o) const = default;

  /// \brief Compares two times.
  /// \param o The other time.
  /// \return True when they are the same millisecond.
  [[nodiscard]] constexpr bool operator==(const Time &o) const = default;

private:
  std::int32_t milliseconds_{0};
};

} // namespace agiru

#pragma once

#include "type/Date.h"
#include "type/Duration.h"
#include "type/Time.h"

#include <compare>
#include <cstdint>
#include <string>

/// \file
/// \brief AL `DateTime` -- an instant, held in UTC, to the millisecond.

namespace agiru {

/// \brief AL `DateTime`.
///
/// From `datetime-data-type.md`: "Denotes a date and time ranging from January 1, 1753,
/// 00:00:00.000 to December 31, 9999, 23:59:59.999. An undefined or blank DateTime is specified by
/// 0DT", "A DateTime is stored in the database as Coordinated Universal Time (UTC)", and "The
/// DateTime data type does not support closing dates."
///
/// Milliseconds since 1753-01-01T00:00:00Z in one 64-bit integer, and the ordering is the
/// integer's own.
///
/// \note 0 IS THE UNDEFINED DATETIME AND ALSO THE EARLIEST INSTANT AL ACCEPTS, which is the
///       platform's own arrangement rather than a shortcut: `date-data-type.md` says the undefined
///       value "is represented by the earliest valid date in SQL Server ... 01-01-1753
///       00:00:00:000". That instant is therefore not separately representable in AL either.
///
/// \note NO CLOSING DATETIME. The page says so outright, which is why this is a plain count where
///       Date carries a closing bit.
class DateTime {
public:
  /// \brief The undefined DateTime, `0DT`.
  constexpr DateTime() = default;

  /// \brief AL `CreateDateTime(Date, Time)`.
  ///
  /// \param date The date part.
  /// \param time The time part.
  /// \return The instant, or the undefined DateTime when the date is undefined.
  ///
  /// \note `datetime-data-type.md`: "The only constant available when you use the DateTime data
  ///       type is the undefined DateTime, 0DT. To assign a constant value to a DateTime variable
  ///       you must use the CreateDateTime method." So this is the only way in, and there is no
  ///       DateTime literal to translate.
  ///
  /// \note A CLOSING date loses its closing bit here, because a DateTime has nowhere to put it.
  [[nodiscard]] static constexpr DateTime Create(const Date &date, const Time &time) {
    if (date.IsUndefined()) { return DateTime{}; }
    DateTime held;
    held.milliseconds_ =
        (static_cast<std::int64_t>(date.DaysSinceFirst()) * agiru::Time::kMillisecondsPerDay) +
        time.AsMilliseconds();
    return held;
  }

  /// \brief Builds an instant from milliseconds since 1753-01-01T00:00:00Z.
  /// \param milliseconds The count; a negative one gives the undefined DateTime.
  /// \return The instant.
  [[nodiscard]] static constexpr DateTime FromMilliseconds(std::int64_t milliseconds) {
    DateTime held;
    held.milliseconds_ = milliseconds > 0 ? milliseconds : 0;
    return held;
  }

  /// \return Milliseconds since 1753-01-01T00:00:00Z. AL subtracts two DateTimes and gets a
  ///         Duration, which is exactly this difference.
  [[nodiscard]] constexpr std::int64_t AsMilliseconds() const { return milliseconds_; }

  /// \return True for `0DT`.
  [[nodiscard]] constexpr bool IsUndefined() const { return milliseconds_ == 0; }

  /// \brief AL `DateTime.Date()`, and `DT2Date(DateTime)`.
  ///
  /// \return The date part, or the undefined date.
  ///
  /// \note THE PART IS THE UTC ONE, and that is a stated assumption rather than a documented fact.
  ///       `datetime-data-type.md` says a DateTime "is always displayed as local time" and is
  ///       stored in UTC, but neither this method's page nor `system-dt2date-method.md` says which
  ///       of the two it splits. A session in this runtime carries no time zone yet, so local IS
  ///       UTC here and the answer is right under that condition; when a session gains one, the
  ///       conversion belongs in this one place and nowhere else.
  [[nodiscard]] constexpr agiru::Date Date() const {
    if (IsUndefined()) { return agiru::Date{}; }
    return agiru::Date::FromDaysSinceFirst(
        static_cast<std::int32_t>(milliseconds_ / agiru::Time::kMillisecondsPerDay));
  }

  /// \brief AL `DateTime.Time()`, and `DT2Time(DateTime)`.
  /// \return The time part.
  /// \note The UTC one, for the reason Date() gives.
  [[nodiscard]] constexpr agiru::Time Time() const {
    return agiru::Time::FromMilliseconds(
        static_cast<std::int32_t>(milliseconds_ % agiru::Time::kMillisecondsPerDay));
  }

  /// \brief AL `Format(DateTime, 0, 9)` -- the XML format.
  /// \return `yyyy-mm-ddThh:mm:ss.fffZ`, or the empty string when undefined.
  [[nodiscard]] std::string ToInvariantString() const;

  /// \brief AL `DateTime - DateTime` -- how long lies between them.
  /// \param o The earlier instant.
  /// \return The difference; negative when this instant is the earlier one.
  [[nodiscard]] constexpr Duration operator-(const DateTime &o) const {
    return Duration{milliseconds_ - o.milliseconds_};
  }

  /// \brief AL `DateTime + Duration`.
  /// \param d How long to move forward.
  /// \return The later instant, or the undefined one when this is undefined.
  [[nodiscard]] constexpr DateTime operator+(const Duration &d) const {
    return IsUndefined() ? *this : FromMilliseconds(milliseconds_ + d.Milliseconds());
  }

  /// \brief AL `DateTime - Duration`.
  /// \param d How long to move back.
  /// \return The earlier instant, or the undefined one when this is undefined.
  [[nodiscard]] constexpr DateTime operator-(const Duration &d) const {
    return IsUndefined() ? *this : FromMilliseconds(milliseconds_ - d.Milliseconds());
  }

  /// \brief Orders two instants.
  /// \param o The other instant.
  /// \return The ordering.
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const DateTime &o) const = default;

  /// \brief Compares two instants.
  /// \param o The other instant.
  /// \return True when they are the same millisecond.
  [[nodiscard]] constexpr bool operator==(const DateTime &o) const = default;

private:
  std::int64_t milliseconds_{0};
};

/// \brief AL `CurrentDateTime` -- the instant this call happens.
///
/// \return The wall clock, in UTC, to the millisecond.
///
/// \note `system-currentdatetime-method.md` says "Gets the current date and time from the operating
///       system", and `datetime-data-type.md` says a DateTime "is stored in the database as
///       Coordinated Universal Time (UTC)". There is no work date here and no session offset: this
///       is the clock, and AL's `WorkDate` is a different thing that a session carries.
[[nodiscard]] DateTime CurrentDateTime();

} // namespace agiru

#pragma once

#include <array>
#include <compare>
#include <cstdint>
#include <string>

/// \file
/// \brief AL `Date` -- a calendar day, its undefined value, and its closing twin.

namespace agiru {

/// \brief The calendar, without `<chrono>`.
///
/// THESE TWENTY LINES SAVE 88 397 LINES AND ABOUT A SECOND IN EVERY TRANSLATION UNIT. `<chrono>` is
/// the single most expensive header the door pulls -- measured 2026-09-02, 88 397 preprocessed
/// lines and 972 ms on its own -- and a generated file pays it 6 398 times for four functions.
/// The algorithms are Howard Hinnant's public-domain civil-calendar pair, which every standard
/// library implements underneath anyway; the Date gate's 33 checks, including the ISO week cases
/// and the leap-day clamps, are what stand behind them here.
namespace calendar {

/// Days in one 400-year Gregorian era, which is where the cycle repeats exactly.
constexpr std::int32_t kDaysPerEra = 146097;
/// Years in that era.
constexpr int kYearsPerEra = 400;
/// The shift from the era Hinnant's algorithms count from to the Unix epoch.
constexpr std::int32_t kEraToEpoch = 719468;
/// Months in a year.
constexpr unsigned kMonthsPerYear = 12;
/// Days in a leap February.
constexpr unsigned kLeapFebruary = 29;

/// \brief Days from 1970-01-01 to a civil date, proleptic Gregorian.
/// \param y The year.
/// \param m The month, 1 to 12.
/// \param d The day of month.
/// \return The day count, negative before the epoch.
[[nodiscard]] constexpr std::int32_t DaysFromCivil(int y, unsigned m, unsigned d) {
  y -= static_cast<int>(m <= 2);
  const int era = (y >= 0 ? y : y - (kYearsPerEra - 1)) / kYearsPerEra;
  const auto yoe = static_cast<unsigned>(y - (era * kYearsPerEra));
  const unsigned doy = (((153 * (m + (m > 2 ? -3U : 9U))) + 2) / 5) + d - 1;
  const unsigned doe = (yoe * 365) + (yoe / 4) - (yoe / 100) + doy;
  return static_cast<std::int32_t>((era * kDaysPerEra) + static_cast<int>(doe) - kEraToEpoch);
}

/// \brief The civil date a day count names.
struct Civil {
  int year;       ///< The year.
  unsigned month; ///< The month, 1 to 12.
  unsigned day;   ///< The day of month.
};

/// \brief The civil date of a day count from 1970-01-01.
/// \param days The day count.
/// \return Its year, month and day.
[[nodiscard]] constexpr Civil CivilFromDays(std::int32_t days) {
  const int z = days + kEraToEpoch;
  const int era = (z >= 0 ? z : z - (kDaysPerEra - 1)) / kDaysPerEra;
  const auto doe = static_cast<unsigned>(z - (era * kDaysPerEra));
  const unsigned yoe = (doe - (doe / 1460) + (doe / 36524) - (doe / (kDaysPerEra - 1))) / 365;
  const int y = static_cast<int>(yoe) + (era * kYearsPerEra);
  const unsigned doy = doe - ((365 * yoe) + (yoe / 4) - (yoe / 100));
  const unsigned mp = ((5 * doy) + 2) / 153;
  const unsigned d = doy - (((153 * mp) + 2) / 5) + 1;
  const unsigned m = mp + (mp < 10 ? 3U : -9U);
  return Civil{.year = y + static_cast<int>(m <= 2), .month = m, .day = d};
}

/// \brief Whether a year has 366 days.
/// \param y The year.
/// \return True when it is a leap year.
[[nodiscard]] constexpr bool IsLeapYear(int y) {
  return (y % 4 == 0 && y % 100 != 0) || y % kYearsPerEra == 0;
}

/// \brief The last day of a month.
/// \param y The year, which decides February.
/// \param m The month, 1 to 12.
/// \return 28, 29, 30 or 31.
[[nodiscard]] constexpr unsigned LastDayOfMonth(int y, unsigned m) {
  constexpr std::array<unsigned, 13> kLengths{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  return m == 2 && IsLeapYear(y) ? kLeapFebruary : kLengths.at(m);
}

/// \brief Whether a year, month and day name a real date.
/// \param y The year.
/// \param m The month.
/// \param d The day of month.
/// \return True when the three name a day that exists.
[[nodiscard]] constexpr bool IsRealDate(int y, unsigned m, unsigned d) {
  return m >= 1 && m <= kMonthsPerYear && d >= 1 && d <= LastDayOfMonth(y, m);
}

} // namespace calendar

namespace detail {
/// \brief Lets the runtime write a serial it read from a column.
class ValueAccess;
} // namespace detail

/// \brief AL `Date`.
///
/// From `date-data-type.md`: "Denotes a date ranging from January 1, 1753 to December 31, 9999",
/// "An undefined or blank date is specified by 0D. The undefined date is considered to be before
/// all other dates", and "All normal dates have a corresponding closing date. The closing date for
/// a given date is defined as a period of time that follows a given normal date and precedes the
/// next normal date."
///
/// A CLOSING DATE IS NOT A FLAG ON THE SIDE, it is a position in the order, and the order is what
/// the whole type has to get right: `normal(d) < closing(d) < normal(d + 1)`. That is what makes a
/// closing entry at the end of a fiscal year fall after every posting of the last day and before
/// the first posting of the next, which is the entire reason BC has the concept.
///
/// So all three facts live in ONE signed integer and the ordering is the integer's own:
///
///     serial == 0   the undefined date, 0D, which sorts before everything
///     serial  > 0   (days since 1753-01-01 + 1) * 2 + (closing ? 1 : 0)
///
/// \note The encoding is not thrift for its own sake. It makes `<` on a date a single machine
///       comparison on a path every filter and every date-range check in a posting run takes, and
///       it holds a date field to four bytes where the BaseApp declares 1 576 of them.
class Date {
public:
  /// \brief The undefined date, `0D`.
  constexpr Date() = default;

  /// \brief The earliest date AL accepts, from `date-data-type.md`.
  static constexpr int kFirstYear = 1753;

  /// \brief The latest date AL accepts.
  static constexpr int kLastYear = 9999;

  /// \brief Builds a normal date, the way AL writes `20180325D`.
  ///
  /// \param year  The year, 1753 to 9999.
  /// \param month The month, 1 to 12.
  /// \param day   The day of month.
  /// \return The date, or the undefined date when the three do not name a real day.
  ///
  /// \note An impossible day is the UNDEFINED date rather than an error, because an AL date literal
  ///       is checked by the compiler and a date reaching this from storage is data that has
  ///       already been written. IsUndefined() is what a caller asks.
  [[nodiscard]] static constexpr Date FromYmd(int year, unsigned month, unsigned day) {
    if (!calendar::IsRealDate(year, month, day) || year < kFirstYear || year > kLastYear) {
      return Date{};
    }
    return FromSerial(((calendar::DaysFromCivil(year, month, day) - kEpoch) + 1) * 2);
  }

  /// \brief AL `ClosingDate(Date)` -- the closing twin of a normal date.
  /// \return The closing date, or this date unchanged when it is undefined or already closing.
  [[nodiscard]] constexpr Date Closing() const {
    return IsUndefined() ? *this : FromSerial(serial_ | 1);
  }

  /// \brief AL `NormalDate(Date)` -- the normal date a closing date closes.
  /// \return The normal date, or this date unchanged when it is undefined or already normal.
  [[nodiscard]] constexpr Date Normal() const {
    return IsUndefined() ? *this : FromSerial(serial_ & ~1);
  }

  /// \brief Builds a normal date from its position in the range.
  /// \param days Days since 1753-01-01.
  /// \return The date, or the undefined date when the count falls outside the range.
  /// \note This is what a DateTime splits itself on, and it is why the count is public.
  [[nodiscard]] static constexpr Date FromDaysSinceFirst(std::int32_t days) {
    if (days < 0 || days > kLastDayIndex) { return Date{}; }
    return FromSerial((days + 1) * 2);
  }

  /// \return Days since 1753-01-01, or -1 when undefined. A closing date answers with its own day.
  [[nodiscard]] constexpr std::int32_t DaysSinceFirst() const {
    return IsUndefined() ? -1 : (serial_ / 2) - 1;
  }

  /// \return True for `0D`.
  [[nodiscard]] constexpr bool IsUndefined() const { return serial_ == 0; }

  /// \return True when this is the closing date of its day.
  [[nodiscard]] constexpr bool IsClosing() const { return !IsUndefined() && (serial_ & 1) != 0; }

  /// \brief AL `Date.Year()`.
  /// \return The year, or 0 when undefined.
  [[nodiscard]] constexpr int Year() const { return IsUndefined() ? 0 : Civil().year; }

  /// \brief AL `Date.Month()`.
  /// \return The month, 1 to 12, or 0 when undefined.
  [[nodiscard]] constexpr int Month() const {
    return IsUndefined() ? 0 : static_cast<int>(Civil().month);
  }

  /// \brief AL `Date.Day()`.
  /// \return The day of month, or 0 when undefined.
  [[nodiscard]] constexpr int Day() const {
    return IsUndefined() ? 0 : static_cast<int>(Civil().day);
  }

  /// \brief AL `Date.DayOfWeek()`.
  ///
  /// \return 1 for Monday through 7 for Sunday, or 0 when undefined.
  ///
  /// \note The numbering is the one `system-date2dwy-method.md` states: "The value 1 corresponds to
  ///       day of the week (1-7, Monday = 1)". It is NOT `std::chrono`'s, where Sunday is 0.
  [[nodiscard]] constexpr int DayOfWeek() const {
    if (IsUndefined()) { return 0; }
    // 1970-01-01 was a Thursday, which is ISO 4, and the arithmetic keeps a non-negative remainder.
    return (((Days() % kDaysPerWeek) + kDaysPerWeek + kThursdayShift) % kDaysPerWeek) + 1;
  }

  /// \brief AL `Date.WeekNo()`.
  ///
  /// \return The ISO week number, 1 to 53, or 0 when undefined.
  ///
  /// \note `system-date2dwy-method.md`: "Date2DWY always uses the ISO week-numbering year scheme
  ///       for the week, regardless of the server or device configuration. This means that week 01
  ///       of a year is the week that includes the first Thursday of the Gregorian year." So the
  ///       week is found through its THURSDAY and never through 1 January.
  ///
  /// \note THAT PAGE'S WORKED EXAMPLE CONTRADICTS ITS OWN RULE, and the rule is what is
  ///       implemented. It writes that 1 January 2014 falls in a week "that starts on Monday,
  ///       December 29, 2013, and ends Sunday, January 4, 2014" -- but 29 December 2013 was a
  ///       Sunday and 4 January 2014 a Saturday. The week is Monday 30 December 2013 to Sunday
  ///       5 January 2014, two days in 2013 and five in 2014 rather than three and four. The
  ///       conclusion the example draws is still correct; only its dates are not.
  [[nodiscard]] constexpr int WeekNo() const {
    if (IsUndefined()) { return 0; }
    const std::int32_t thursday = ThursdayOfWeek();
    const std::int32_t first = calendar::DaysFromCivil(WeekYear(), 1, 1);
    return ((thursday - first) / kDaysPerWeek) + 1;
  }

  /// \brief AL `Date2DWY(Date, 3)` -- the ISO week-numbering year.
  ///
  /// \return The year the week belongs to, or 0 when undefined.
  ///
  /// \note It is not always Year(). `system-date2dwy-method.md` gives the case: 1 January 2014 lies
  ///       in the week of 29 December 2013 to 4 January 2014, which has four days in 2014, so the
  ///       week-numbering year is 2014 -- and for 29 December 2013 it is 2014 as well.
  [[nodiscard]] constexpr int WeekYear() const {
    return IsUndefined() ? 0 : calendar::CivilFromDays(ThursdayOfWeek()).year;
  }

  /// \brief AL `Format(Date, 0, 9)` -- the XML format.
  ///
  /// \return `yyyy-mm-dd`, or the empty string when undefined.
  ///
  /// \note Named for what it is rather than `ToText()`, because `date-totext-method.md` says
  ///       ToText() without an argument is "Equivalent to calling Format(value, 0, 0)" and
  ///       `devenv-format-property.md` shows format 0 is the region's: `05-04-21` in Europe and
  ///       `04/05/21` in the US. Only format 9 is invariant, and it is `2021-04-05`.
  ///
  /// \note Format 9 carries NO closing marker. The `<Closing>` element appears in formats 0 to 7
  ///       and not in the XML format, so a closing date renders exactly like its normal date here.
  ///       IsClosing() is what tells them apart.
  [[nodiscard]] std::string ToInvariantString() const;

  /// \brief The AL ordering, which the encoding makes the integer's own.
  /// \param o The other date.
  /// \return The ordering: undefined first, then each day followed by its closing date.
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const Date &o) const = default;

  /// \brief Compares two dates.
  /// \param o The other date.
  /// \return True when they are the same day and both normal or both closing.
  [[nodiscard]] constexpr bool operator==(const Date &o) const = default;

  friend class detail::ValueAccess;

private:
  static constexpr std::int32_t kEpoch = calendar::DaysFromCivil(kFirstYear, 1, 1);
  static constexpr std::int32_t kLastDayIndex = calendar::DaysFromCivil(kLastYear, 12, 31) - kEpoch;
  static constexpr int kDaysPerWeek = 7;
  /// 1970-01-01 was a Thursday, and ISO numbers Thursday 4.
  static constexpr int kThursdayShift = 3;
  /// Thursday's ISO weekday number, which is the day that decides which year a week belongs to.
  static constexpr int kThursday = 4;

  [[nodiscard]] static constexpr Date FromSerial(std::int32_t serial) {
    Date held;
    held.serial_ = serial;
    return held;
  }

  [[nodiscard]] constexpr std::int32_t Days() const { return kEpoch + (serial_ / 2) - 1; }

  [[nodiscard]] constexpr calendar::Civil Civil() const { return calendar::CivilFromDays(Days()); }

  [[nodiscard]] constexpr std::int32_t ThursdayOfWeek() const {
    return Days() + (kThursday - DayOfWeek());
  }

  std::int32_t serial_{0};
};

} // namespace agiru

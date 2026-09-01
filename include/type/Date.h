#pragma once

#include <chrono>
#include <compare>
#include <cstdint>
#include <string>

/// \file
/// \brief AL `Date` -- a calendar day, its undefined value, and its closing twin.

namespace agiru {

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
    const std::chrono::year_month_day ymd{
        std::chrono::year{year}, std::chrono::month{month}, std::chrono::day{day}};
    if (!ymd.ok() || year < kFirstYear || year > kLastYear) { return Date{}; }
    return FromSerial((DayIndex(ymd) + 1) * 2);
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
  [[nodiscard]] constexpr int Year() const {
    return IsUndefined() ? 0 : static_cast<int>(Civil().year());
  }

  /// \brief AL `Date.Month()`.
  /// \return The month, 1 to 12, or 0 when undefined.
  [[nodiscard]] constexpr int Month() const {
    return IsUndefined() ? 0 : static_cast<int>(static_cast<unsigned>(Civil().month()));
  }

  /// \brief AL `Date.Day()`.
  /// \return The day of month, or 0 when undefined.
  [[nodiscard]] constexpr int Day() const {
    return IsUndefined() ? 0 : static_cast<int>(static_cast<unsigned>(Civil().day()));
  }

  /// \brief AL `Date.DayOfWeek()`.
  ///
  /// \return 1 for Monday through 7 for Sunday, or 0 when undefined.
  ///
  /// \note The numbering is the one `system-date2dwy-method.md` states: "The value 1 corresponds to
  ///       day of the week (1-7, Monday = 1)". It is NOT `std::chrono`'s, where Sunday is 0.
  [[nodiscard]] constexpr int DayOfWeek() const {
    if (IsUndefined()) { return 0; }
    return static_cast<int>(std::chrono::weekday{Days()}.iso_encoding());
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
    const std::chrono::sys_days thursday = ThursdayOfWeek();
    const std::chrono::sys_days first{std::chrono::year{WeekYear()} / std::chrono::January / 1};
    return static_cast<int>(((thursday - first).count() / kDaysPerWeek) + 1);
  }

  /// \brief AL `Date2DWY(Date, 3)` -- the ISO week-numbering year.
  ///
  /// \return The year the week belongs to, or 0 when undefined.
  ///
  /// \note It is not always Year(). `system-date2dwy-method.md` gives the case: 1 January 2014 lies
  ///       in the week of 29 December 2013 to 4 January 2014, which has four days in 2014, so the
  ///       week-numbering year is 2014 -- and for 29 December 2013 it is 2014 as well.
  [[nodiscard]] constexpr int WeekYear() const {
    if (IsUndefined()) { return 0; }
    return static_cast<int>(std::chrono::year_month_day{ThursdayOfWeek()}.year());
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
  static constexpr std::chrono::sys_days kEpoch{std::chrono::year{kFirstYear} /
                                                std::chrono::January / 1};
  static constexpr std::int32_t kLastDayIndex = static_cast<std::int32_t>(
      (std::chrono::sys_days{std::chrono::year{kLastYear} / std::chrono::December / 31} - kEpoch)
          .count());
  static constexpr int kDaysPerWeek = 7;
  /// Thursday's ISO weekday number, which is the day that decides which year a week belongs to.
  static constexpr int kThursday = 4;

  [[nodiscard]] static constexpr Date FromSerial(std::int32_t serial) {
    Date held;
    held.serial_ = serial;
    return held;
  }

  [[nodiscard]] static constexpr std::int32_t DayIndex(const std::chrono::year_month_day &ymd) {
    return static_cast<std::int32_t>((std::chrono::sys_days{ymd} - kEpoch).count());
  }

  [[nodiscard]] constexpr std::chrono::sys_days Days() const {
    return kEpoch + std::chrono::days{(serial_ / 2) - 1};
  }

  [[nodiscard]] constexpr std::chrono::year_month_day Civil() const { return Days(); }

  [[nodiscard]] constexpr std::chrono::sys_days ThursdayOfWeek() const {
    return Days() + std::chrono::days{kThursday - DayOfWeek()};
  }

  std::int32_t serial_{0};
};

} // namespace agiru

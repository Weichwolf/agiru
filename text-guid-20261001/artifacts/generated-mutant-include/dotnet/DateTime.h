#pragma once

#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Integer.h"

#include <string>
#include <string_view>

/// \file
/// \brief The .NET type `System.DateTime` -- an instant, and the parts AL reads off it.

namespace agiru::dotnet {

/// \brief `System.DateTime`.
///
/// AL reaches it through `Codeunit DotNet_DateTime`, BC's own wrapper, and reads PARTS off it:
/// `TypeHelper.TryEvaluateDate` builds an AL `Date` out of `.Day()`, `.Month()` and `.Year()`
/// after `XmlConvert` has parsed a text into one.
///
/// \note IT IS THE SAME INSTANT `agiru::DateTime` HOLDS. BC stores every DateTime as UTC
///       (`devenv-about-dates.md`), and a session here runs in UTC until a user setting says
///       otherwise -- so `Now` and `UtcNow` are the same answer, and so are the local and the
///       universal forms (board:0618).
///
/// \note IT DECLARES NO CONSTRUCTOR, because AL spells .NET's as `X := X.DateTime(y, m, d)` and a
///       member may carry its class's own name only in a class that declares none.
struct DateTime {
  /// \brief What `X.DateTime(...)` is: .NET's constructor, spelled as AL spells it.
  struct FromParts {
    /// \brief Builds an instant from a year, a month and a day.
    /// \param year The year. \param month The month. \param day The day.
    /// \return The instant at midnight.
    [[nodiscard]] DateTime
    operator()(::agiru::Integer year, ::agiru::Integer month, ::agiru::Integer day) const;

    /// \brief Builds an instant from the whole six.
    /// \param year The year. \param month The month. \param day The day.
    /// \param hour The hour. \param minute The minute. \param second The second.
    /// \return The instant.
    [[nodiscard]] DateTime operator()(::agiru::Integer year,
                                      ::agiru::Integer month,
                                      ::agiru::Integer day,
                                      ::agiru::Integer hour,
                                      ::agiru::Integer minute,
                                      ::agiru::Integer second) const;

    /// \brief Builds one from an AL instant, which is what the wrapper hands across.
    /// \param at The instant.
    /// \return The same instant.
    [[nodiscard]] DateTime operator()(const ::agiru::DateTime &at) const;

    /// \brief Builds one from TICKS and a `DateTimeKind`, which is .NET's other constructor.
    /// \tparam Kind The kind's type, which is not rebuilt here.
    /// \param ticks The 100-nanosecond intervals since year 1.
    /// \param kind  Local or Utc, discarded: a session with no user setting runs in UTC.
    /// \return The instant.
    template <typename Kind>
    [[nodiscard]] DateTime operator()(::agiru::BigInteger ticks, const Kind &kind) const {
      static_cast<void>(kind);
      return FromTicks(ticks);
    }

    /// \brief Builds one from ticks alone.
    /// \param ticks The 100-nanosecond intervals since year 1.
    /// \return The instant.
    [[nodiscard]] static DateTime FromTicks(::agiru::BigInteger ticks);
  };

  /// \brief `new DateTime(...)` -- AL writes it as a member of the variable it assigns to.
  FromParts DateTime{};

  /// \brief `DateTime.Now` -- this instant.
  /// \return The instant now.
  [[nodiscard]] struct DateTime Now() const;

  /// \brief `DateTime.UtcNow` -- this instant in UTC, which is the same instant.
  /// \return The instant now.
  [[nodiscard]] struct DateTime UtcNow() const;

  /// \brief `DateTime.Today` -- midnight of the current day.
  /// \return That instant.
  [[nodiscard]] struct DateTime Today() const;

  /// \brief `DateTime.Year`. \return The year.
  [[nodiscard]] ::agiru::Integer Year() const { return at_.Date().Year(); }

  /// \brief `DateTime.Month`. \return The month, 1 to 12.
  [[nodiscard]] ::agiru::Integer Month() const { return at_.Date().Month(); }

  /// \brief `DateTime.Day`. \return The day of the month.
  [[nodiscard]] ::agiru::Integer Day() const { return at_.Date().Day(); }

  /// \brief `DateTime.Hour`. \return The hour, 0 to 23.
  [[nodiscard]] ::agiru::Integer Hour() const { return at_.Time().Hour(); }

  /// \brief `DateTime.Minute`. \return The minute, 0 to 59.
  [[nodiscard]] ::agiru::Integer Minute() const { return at_.Time().Minute(); }

  /// \brief `DateTime.Second`. \return The second, 0 to 59.
  [[nodiscard]] ::agiru::Integer Second() const { return at_.Time().Second(); }

  /// \brief `DateTime.Millisecond`. \return The millisecond, 0 to 999.
  [[nodiscard]] ::agiru::Integer Millisecond() const { return at_.Time().Millisecond(); }

  /// \brief `DateTime.DayOfWeek`. \return 1 for Monday through 7 for Sunday.
  [[nodiscard]] ::agiru::Integer DayOfWeek() const { return at_.Date().DayOfWeek(); }

  /// \brief `DateTime.Ticks` -- 100-nanosecond intervals since year 1.
  /// \return The count.
  [[nodiscard]] ::agiru::BigInteger Ticks() const;

  /// \brief `DateTime.FromOADate(days)` -- the OLE Automation date Excel writes.
  /// \param days Days since 1899-12-30, the fraction being the time of day.
  /// \return The instant.
  [[nodiscard]] struct DateTime FromOADate(::agiru::Decimal days) const;

  /// \brief `DateTime.ToOADate()` -- this instant as Excel writes it.
  /// \return Days since 1899-12-30, the fraction being the time of day.
  [[nodiscard]] ::agiru::Decimal ToOADate() const;

  /// \brief `DateTime.ToUniversalTime()`.
  /// \return This value, because the session's zone is UTC.
  [[nodiscard]] struct DateTime ToUniversalTime() const { return *this; }

  /// \brief `DateTime.ToLocalTime()`.
  /// \return This value, because the session's zone is UTC.
  [[nodiscard]] struct DateTime ToLocalTime() const { return *this; }

  /// \brief `DateTime.IsLeapYear(year)`.
  /// \param year The year.
  /// \return Whether it has a 29th of February.
  [[nodiscard]] ::agiru::Boolean IsLeapYear(::agiru::Integer year) const;

  /// \brief `DateTime.ToString()` -- the ISO 8601 form.
  /// \return The instant as text.
  [[nodiscard]] std::string ToString() const;

  /// \brief `DateTime.ToString(format [, culture])` -- a .NET format specification.
  /// \tparam Rest Whatever the call carries after the format.
  /// \param format The .NET format string.
  /// \param rest The culture, which this runtime does not have.
  /// \return Never.
  /// \throws Error always -- a .NET specification is not this runtime's `Format` (board:0007).
  template <typename... Rest>
  [[nodiscard]] std::string ToString(std::string_view format, const Rest &...rest) const {
    (static_cast<void>(rest), ...);
    return Refuse(format);
  }

  /// \brief `.NET DateTime + TimeSpan` -- AL hands it an AL `Duration`.
  /// \param by How far to move.
  /// \return The instant that far on.
  [[nodiscard]] struct DateTime operator+(const ::agiru::Duration &by) const {
    return {.DateTime = {}, .at_ = at_ + by};
  }

  /// \brief `.NET DateTime - TimeSpan`.
  /// \param by How far to move back.
  /// \return The instant that far back.
  [[nodiscard]] struct DateTime operator-(const ::agiru::Duration &by) const {
    return {.DateTime = {}, .at_ = at_ - by};
  }

  /// \brief `.NET DateTime - DateTime` -- how long lies between them.
  /// \param o The other instant.
  /// \return The span.
  [[nodiscard]] ::agiru::Duration operator-(const struct DateTime &o) const { return at_ - o.at_; }

  /// \brief Takes an AL instant, which is what a wrapper hands across.
  /// \param at The instant.
  /// \return This value.
  struct DateTime &operator=(const ::agiru::DateTime &at) {
    at_ = at;
    return *this;
  }

  /// \brief Reads as the AL instant, which is what AL does with a `DotNet DateTime`.
  /// \return The instant.
  ///
  /// \note AL ASSIGNS THE .NET VALUE TO AN AL ONE WITHOUT CEREMONY -- `ExcelBuffer` writes
  ///       `DateTimeResult := DotNetDateTime.DateTime(Ticks, Kind)` -- so the conversion is
  ///       implicit here for the same reason it is there.
  [[nodiscard]] operator ::agiru::DateTime() const { return at_; } // NOLINT(*-explicit-constructor)

  /// \brief The instant this value holds.
  ::agiru::DateTime at_{};

private:
  [[noreturn]] static std::string Refuse(std::string_view format);
};

}

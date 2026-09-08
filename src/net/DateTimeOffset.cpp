#include "dotnet/DateTimeOffset.h"

#include "dotnet/DateTime.h"
#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Duration.h"
#include "type/Time.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace agiru::dotnet {

namespace {

constexpr std::int64_t kDaysFrom1753To1970 = 79257;
constexpr std::int64_t kUnixEpoch = kDaysFrom1753To1970 * ::agiru::Time::kMillisecondsPerDay;
constexpr std::int64_t kMillisecondsPerSecond = 1000;

constexpr std::size_t kYearAt = 0;
constexpr std::size_t kMonthAt = 5;
constexpr std::size_t kDayAt = 8;
constexpr std::size_t kHourAt = 11;
constexpr std::size_t kMinuteAt = 14;
constexpr std::size_t kSecondAt = 17;
constexpr std::size_t kFractionAt = 20;
constexpr std::size_t kIsoLength = 20;

::agiru::DateTime Held(std::int64_t milliseconds) {
  return ::agiru::DateTime::FromMilliseconds(milliseconds);
}

int Number(std::string_view text, std::size_t at, std::size_t digits) {
  int value = 0;
  for (std::size_t i = 0; i < digits; ++i) {
    const char c = text[at + i];
    if (c < '0' || c > '9') { return -1; }
    value = (value * 10) + (c - '0');
  }
  return value;
}

}

DateTimeOffset DateTimeOffset::FromInstant::operator()(const ::agiru::DateTime &at) const {
  return ::agiru::dotnet::DateTimeOffset{.at_ = at};
}

struct DateTimeOffset DateTimeOffset::Now() const {
  return ::agiru::dotnet::DateTimeOffset{.at_ = ::agiru::CurrentDateTime()};
}

struct DateTimeOffset DateTimeOffset::UtcNow() const {
  return Now();
}

::agiru::Duration DateTimeOffset::Offset() const {
  return ::agiru::Duration{0};
}

::agiru::BigInteger DateTimeOffset::ToUnixTimeMilliseconds() const {
  return at_.AsMilliseconds() - kUnixEpoch;
}

::agiru::BigInteger DateTimeOffset::ToUnixTimeSeconds() const {
  return ToUnixTimeMilliseconds() / kMillisecondsPerSecond;
}

struct DateTimeOffset
DateTimeOffset::FromUnixTimeMilliseconds(::agiru::BigInteger milliseconds) const {
  return ::agiru::dotnet::DateTimeOffset{.at_ = Held(milliseconds + kUnixEpoch)};
}

struct DateTimeOffset DateTimeOffset::FromUnixTimeSeconds(::agiru::BigInteger seconds) const {
  return FromUnixTimeMilliseconds(seconds * kMillisecondsPerSecond);
}

struct DateTimeOffset DateTimeOffset::Parse(std::string_view text) const {
  if (text.size() < kIsoLength || text[kFractionAt - 1] != ':') {
    return ::agiru::dotnet::DateTimeOffset{};
  }
  const int year = Number(text, kYearAt, 4);
  const int month = Number(text, kMonthAt, 2);
  const int day = Number(text, kDayAt, 2);
  const int hour = Number(text, kHourAt, 2);
  const int minute = Number(text, kMinuteAt, 2);
  const int second = Number(text, kSecondAt, 2);
  if (year < 0 || month < 0 || day < 0 || hour < 0 || minute < 0 || second < 0) {
    return ::agiru::dotnet::DateTimeOffset{};
  }
  int fraction = 0;
  if (text.size() > kFractionAt + 3 && text[kFractionAt] == '.') {
    fraction = Number(text, kFractionAt + 1, 3);
    fraction = std::max(fraction, 0);
  }
  const ::agiru::Date date =
      ::agiru::Date::FromYmd(year, static_cast<unsigned>(month), static_cast<unsigned>(day));
  if (date.IsUndefined()) { return ::agiru::dotnet::DateTimeOffset{}; }
  return ::agiru::dotnet::DateTimeOffset{
      .at_ =
          ::agiru::DateTime::Create(date, ::agiru::Time::FromHms(hour, minute, second, fraction))};
}

::agiru::dotnet::DateTime DateTimeOffset::DateTime() const {
  return ::agiru::dotnet::DateTime{.at_ = at_};
}

std::string DateTimeOffset::ToString() const {
  return at_.ToInvariantString();
}

std::string DateTimeOffset::Refuse(std::string_view format) {
  throw ::agiru::Error("DateTimeOffset.ToString(\"" + std::string(format) +
                       "\") wants a .NET format specification, and this runtime renders AL's own "
                       "(board:0007)");
}

}

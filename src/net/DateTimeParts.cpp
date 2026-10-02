#include "dotnet/DateTime.h"
#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Integer.h"
#include "type/Time.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace agiru::dotnet {

namespace {

constexpr std::int64_t kTicksPerMillisecond = 10000;
constexpr std::int64_t kAlEpochYear = 1753;
constexpr int kOleEpochYear = 1899;
constexpr unsigned kOleEpochMonth = 12;
constexpr unsigned kOleEpochDay = 30;

std::int64_t Whole(const ::agiru::Decimal &value) {
  const std::string rendered = ::agiru::Round(value).ToInvariantString();
  const std::size_t point = rendered.find('.');
  return std::stoll(point == std::string::npos ? rendered : rendered.substr(0, point));
}

std::int64_t OleEpochToAlEpochDays() {
  return ::agiru::calendar::DaysFromCivil(kOleEpochYear, kOleEpochMonth, kOleEpochDay) -
         ::agiru::calendar::DaysFromCivil(kAlEpochYear, 1, 1);
}

std::int64_t TicksBeforeTheAlEpoch() {
  const std::int64_t days = -::agiru::calendar::DaysFromCivil(1, 1, 1) +
                            ::agiru::calendar::DaysFromCivil(kAlEpochYear, 1, 1);
  return days * ::agiru::Time::kMillisecondsPerDay * kTicksPerMillisecond;
}

}

DateTime DateTime::FromParts::operator()(::agiru::Integer year,
                                         ::agiru::Integer month,
                                         ::agiru::Integer day) const {
  return (*this)(year, month, day, 0, 0, 0);
}

DateTime DateTime::FromParts::operator()(::agiru::Integer year,
                                         ::agiru::Integer month,
                                         ::agiru::Integer day,
                                         ::agiru::Integer hour,
                                         ::agiru::Integer minute,
                                         ::agiru::Integer second) const {
  const ::agiru::Date date =
      ::agiru::Date::FromYmd(year, static_cast<unsigned>(month), static_cast<unsigned>(day));
  return ::agiru::dotnet::DateTime{
      .at_ = ::agiru::DateTime::Create(date, ::agiru::Time::FromHms(hour, minute, second))};
}

DateTime DateTime::FromParts::operator()(const ::agiru::DateTime &at) const {
  return ::agiru::dotnet::DateTime{.at_ = at};
}

struct DateTime DateTime::Now() const {
  return ::agiru::dotnet::DateTime{.at_ = ::agiru::CurrentDateTime()};
}

struct DateTime DateTime::UtcNow() const {
  return Now();
}

struct DateTime DateTime::Today() const {
  return ::agiru::dotnet::DateTime{
      .at_ = ::agiru::DateTime::Create(::agiru::CurrentDateTime().Date(), ::agiru::Time{})};
}

DateTime DateTime::FromParts::FromTicks(::agiru::BigInteger ticks) {
  return ::agiru::dotnet::DateTime{.at_ = ::agiru::DateTime::FromMilliseconds(
                                       (ticks - TicksBeforeTheAlEpoch()) / kTicksPerMillisecond)};
}

struct DateTime DateTime::FromOADate(::agiru::Decimal days) const {
  const ::agiru::Decimal milliseconds = (days - ::agiru::Decimal{OleEpochToAlEpochDays()}) *
                                        ::agiru::Decimal{::agiru::Time::kMillisecondsPerDay};
  return ::agiru::dotnet::DateTime{.at_ = ::agiru::DateTime::FromMilliseconds(Whole(milliseconds))};
}

::agiru::Decimal DateTime::ToOADate() const {
  return ::agiru::Decimal{at_.AsMilliseconds()} /
             ::agiru::Decimal{::agiru::Time::kMillisecondsPerDay} +
         ::agiru::Decimal{OleEpochToAlEpochDays()};
}

::agiru::BigInteger DateTime::Ticks() const {
  return (at_.AsMilliseconds() * kTicksPerMillisecond) + TicksBeforeTheAlEpoch();
}

::agiru::Boolean DateTime::IsLeapYear(::agiru::Integer year) const {
  return ::agiru::calendar::IsLeapYear(year);
}

std::string DateTime::ToString() const {
  return at_.ToInvariantString();
}

std::string DateTime::Refuse(std::string_view format) {
  throw ::agiru::Error("DateTime.ToString(\"" + std::string(format) +
                       "\") wants a .NET format specification, and this runtime renders AL's own "
                       "(board:0007)");
}

}

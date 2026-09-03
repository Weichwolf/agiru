#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Duration.h"
#include "type/Time.h"

#include <chrono>
#include <cstdint>
#include <format>
#include <string>

namespace agiru {

namespace {

constexpr std::int64_t kDaysFrom1753To1970 = 79257;
constexpr std::int64_t kAlEpochToUnixDays = -calendar::DaysFromCivil(Date::kFirstYear, 1, 1);

static_assert(kAlEpochToUnixDays == kDaysFrom1753To1970,
              "1753-01-01 to 1970-01-01, which is what a DateTime's zero has to be moved by");

} // namespace

DateTime CurrentDateTime() {
  const auto since = std::chrono::system_clock::now().time_since_epoch();
  const std::int64_t unixMilliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(since).count();
  return DateTime::FromMilliseconds(unixMilliseconds +
                                    (kAlEpochToUnixDays * Time::kMillisecondsPerDay));
}

std::string Time::ToInvariantString() const {
  return std::format("{:02}:{:02}:{:02}.{:03}", Hour(), Minute(), Second(), Millisecond());
}

std::string DateTime::ToInvariantString() const {
  if (IsUndefined()) { return {}; }
  return Date().ToInvariantString() + "T" + Time().ToInvariantString() + "Z";
}

} // namespace agiru

namespace agiru {

std::string Duration::ToInvariantString() const {
  return std::to_string(milliseconds_);
}

} // namespace agiru

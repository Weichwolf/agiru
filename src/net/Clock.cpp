#include "type/DateTime.h"
#include "type/Duration.h"
#include "type/Time.h"

#include <format>
#include <string>

namespace agiru {

std::string Time::ToInvariantString() const {
  return std::format("{:02}:{:02}:{:02}.{:03}", Hour(), Minute(), Second(), Millisecond());
}

std::string DateTime::ToInvariantString() const {
  if (IsUndefined()) { return {}; }
  // UTC, and the Z says so. `datetime-data-type.md`: "A DateTime is stored in the database as
  // Coordinated Universal Time (UTC)."
  return Date().ToInvariantString() + "T" + Time().ToInvariantString() + "Z";
}

} // namespace agiru

namespace agiru {

std::string Duration::ToInvariantString() const {
  return std::to_string(milliseconds_);
}

} // namespace agiru

#include "type/Date.h"

#include <format>
#include <string>

namespace agiru {

std::string Date::ToInvariantString() const {
  if (IsUndefined()) { return {}; }
  return std::format("{:04}-{:02}-{:02}", Year(), Month(), Day());
}

} // namespace agiru

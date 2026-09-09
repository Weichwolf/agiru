#include "type/Duration.h"

#include "type/Decimal.h"

#include <cstdint>
#include <string>

namespace agiru {

namespace {

std::int64_t WholeMilliseconds(const Decimal &value) {
  return std::stoll(Round(value, Decimal{1}).ToInvariantString());
}

}

Duration Duration::operator*(const Decimal &factor) const {
  return Duration{WholeMilliseconds(Decimal{milliseconds_} * factor)};
}

Duration Duration::operator/(const Decimal &divisor) const {
  return Duration{WholeMilliseconds(Decimal{milliseconds_} / divisor)};
}

}

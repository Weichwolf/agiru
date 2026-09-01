#include "agiru/BigInteger.h"
#include "agiru/Boolean.h"
#include "agiru/Integer.h"

#include <string>

namespace agiru {

std::string ToText(Integer value) {
  return std::to_string(value);
}

std::string ToText(BigInteger value) {
  return std::to_string(value);
}

std::string ToText(Boolean value) {
  return value ? "Yes" : "No";
}

} // namespace agiru

#include "MetadataText.h"

#include "runtime/ErrorValue.h"
#include "type/StringValue.h"

#include <cstddef>
#include <string_view>

namespace agiru::detail {

std::string_view MetadataText(std::string_view value, std::size_t length) {
  if (Utf16Length(value) <= length) { return value; }
  const auto result = value.substr(0, ByteOfUnit(value, length + 1));
  if (Utf16Length(result) != length) {
    throw Error("metadata text truncation crosses an unsupported UTF-16 surrogate boundary");
  }
  return result;
}

}

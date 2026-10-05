#include "ByteArray.h"

#include "dotnet/Regex.h"
#include "runtime/ErrorValue.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include <cstddef>
#include <limits>
#include <span>

namespace agiru::dotnet::detail {

void ValidateByteRegion(const Array &bytes, Integer offset, Integer count) {
  if (offset < 0 || count < 0 || offset > bytes.Length() || count > bytes.Length() - offset) {
    throw Error(".NET byte array: invalid region");
  }
}

void ReadByteBlock(const Array &bytes, Integer offset, std::span<unsigned char> output) {
  if (output.size() > static_cast<std::size_t>(std::numeric_limits<Integer>::max())) {
    throw Error(".NET byte array: block is outside the Integer range");
  }
  ValidateByteRegion(bytes, offset, static_cast<Integer>(output.size()));
  for (std::size_t index = 0; index < output.size(); ++index) {
    const Variant &cell = bytes.GetValue(offset + static_cast<Integer>(index));
    if (!cell.IsInteger()) { throw Error(".NET byte array: expected an Integer byte cell"); }
    const Integer value = cell;
    if (value < 0 || value > std::numeric_limits<unsigned char>::max()) {
      throw Error(".NET byte array: byte value is outside [0,255]");
    }
    output[index] = static_cast<unsigned char>(value);
  }
}

}

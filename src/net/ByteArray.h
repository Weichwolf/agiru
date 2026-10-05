#pragma once

#include "type/Integer.h"

#include <span>

namespace agiru::dotnet {

class Array;

namespace detail {

void ValidateByteRegion(const Array &bytes, Integer offset, Integer count);

void ReadByteBlock(const Array &bytes, Integer offset, std::span<unsigned char> output);

}
}

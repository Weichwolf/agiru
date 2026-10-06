#pragma once

#include <cstddef>
#include <string_view>

namespace agiru::detail {

[[nodiscard]] std::string_view MetadataText(std::string_view value, std::size_t length);

}

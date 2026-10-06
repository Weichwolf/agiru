#pragma once

#include <cstddef>
#include <string_view>

namespace agiru::detail {

[[nodiscard]] std::string_view MetadataText(std::string_view value, std::size_t length);
[[nodiscard]] bool MetadataBlank(std::string_view value);
[[nodiscard]] std::size_t WhitespacePrefix(std::string_view value);
[[nodiscard]] std::string_view TrimWhitespace(std::string_view value);

}

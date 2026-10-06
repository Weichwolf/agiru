#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace agiru::detail {

void AppendHtmlText(std::string &output, std::string_view text, std::size_t bytes);

}

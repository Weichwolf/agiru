#pragma once

#include <optional>
#include <span>
#include <string_view>

namespace agiru::cli {

[[nodiscard]] std::optional<int> Service(std::span<const std::string_view> arguments);

}

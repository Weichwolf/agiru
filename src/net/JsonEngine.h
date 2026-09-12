#pragma once

#include "type/JsonHandle.h"

#include <nlohmann/json.hpp>

namespace agiru::detail {

struct JsonTree {
  nlohmann::json root;
  long uses = 1;
};

[[nodiscard]] nlohmann::json &JsonNodeOf(const JsonHandle &handle);

[[nodiscard]] JsonHandle JsonHandleMade(nlohmann::json value);

[[nodiscard]] JsonHandle JsonHandleAt(const JsonHandle &tree, nlohmann::json &node);

}

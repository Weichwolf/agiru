#pragma once

#include "type/JsonHandle.h"

#include <nlohmann/json.hpp>

namespace agiru::detail {

struct JsonTree {
  nlohmann::ordered_json root;
  long uses = 1;
};

[[nodiscard]] nlohmann::ordered_json &JsonNodeOf(const JsonHandle &handle);

[[nodiscard]] JsonHandle JsonHandleMade(nlohmann::ordered_json value);

[[nodiscard]] JsonHandle JsonHandleAt(const JsonHandle &tree, nlohmann::ordered_json &node);

}

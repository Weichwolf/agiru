#pragma once

#include "meta/TableDef.h"

#include "Filter.h"

#include <optional>
#include <string>
#include <vector>

namespace agiru::detail {

struct Clause {
  std::string sql;
  std::vector<std::optional<std::string>> binds;
};

[[nodiscard]] Clause Where(const FieldDef &def, const Expression &expr, std::size_t first);

}

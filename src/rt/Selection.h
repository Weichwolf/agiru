#pragma once

#include "meta/TableDef.h"
#include "runtime/RecordState.h"

#include <optional>
#include <string>
#include <vector>

namespace agiru::detail {

struct Selection {
  std::string where;
  std::string order;
  std::vector<std::optional<std::string>> binds;
};

[[nodiscard]] Selection Select(const RecordState *state, const TableDef &table);

[[nodiscard]] std::string Columns(const TableDef &table);

[[nodiscard]] std::string Name(const TableDef &table);

}

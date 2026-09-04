#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/RecordState.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::detail {

struct Selection {
  std::string where;
  std::string order;
  std::vector<std::optional<std::string>> binds;
  std::vector<::agiru::FieldNo> sorted;
};

[[nodiscard]] Selection Select(const RecordState *state, const TableDef &table);

[[nodiscard]] std::string Columns(const TableDef &table);

[[nodiscard]] std::string Name(const TableDef &table);

[[nodiscard]] std::string Quoted(std::string_view identifier);

}

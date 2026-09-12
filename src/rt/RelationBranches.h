#pragma once

#include "meta/TableDef.h"

#include <string>
#include <string_view>
#include <vector>

namespace agiru::detail {

struct RelationTerm {
  std::string field;
  std::string kind;
  std::string inner;
};

struct RelationBranch {
  std::vector<RelationTerm> conditions;
  std::string table;
  std::string field;
  std::vector<RelationTerm> filters;
};

[[nodiscard]] std::vector<RelationBranch> RelationBranches(const FieldDef &def);

[[nodiscard]] bool SameName(std::string_view a, std::string_view b);

}

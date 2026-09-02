#pragma once

#include "Ast.h"
#include "EnumWriter.h"

#include <string>
#include <vector>

namespace agiru::gen {

/// One translated table, and what it needed that the run did not have.
struct TableHeader {
  std::string text;
  std::vector<std::string> unresolvedEnums;
};

TableHeader
WriteHeader(const al::TableObject &declared, const std::string &sourcePath, const EnumIndex &enums);

} // namespace agiru::gen

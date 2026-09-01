#pragma once

#include "Ast.h"
#include "EnumWriter.h"

#include <map>
#include <string>
#include <vector>

namespace agiru::gen {

/// Where one AL table object landed: the C++ name it took and the header that declares it.
struct TableRef {
  std::string identifier;
  std::string header;
};

/// Every table object the run has seen, keyed by its AL name in lower case, and by its AL NUMBER
/// as text beside it -- AL names an object either way and test code uses both.
using TableIndex = std::map<std::string, TableRef>;

/// One translated codeunit, and what it needed that the run did not have.
struct CodeunitHeader {
  std::string text;
  std::vector<std::string> unresolvedTables;
};

CodeunitHeader WriteCodeunit(const al::CodeunitObject &unit,
                             const std::string &sourcePath,
                             const TableIndex &tables);

std::string WriteCodeunitSource(const al::CodeunitObject &unit,
                                const std::string &sourcePath,
                                const TableIndex &tables);

std::string CodeunitHeaderPath(const al::CodeunitObject &unit);

} // namespace agiru::gen

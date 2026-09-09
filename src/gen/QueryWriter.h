#pragma once

#include "Ast.h"
#include "CodeunitWriter.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace agiru::gen {

struct QueryWritten {
  std::string header;
  std::string source;
  std::vector<std::string> missing;
  std::size_t triggers = 0;
};

QueryWritten
WriteQuery(const al::QueryObject &query, const std::string &sourcePath, const Objects &objects);

std::string QueryHeaderPath(const al::QueryObject &query);

std::map<std::string, std::string> QueryColumns(const al::QueryObject &query);

std::map<std::string, std::pair<std::string, std::string>>
QueryColumnSources(const al::QueryObject &query);

}

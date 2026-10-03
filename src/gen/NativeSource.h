#pragma once

#include "Ast.h"

#include <filesystem>
#include <string>
#include <vector>

namespace agiru::gen {

struct NativeSourceIssue {
  std::string source;
  std::string reason;
};

struct NativeTableSources {
  std::vector<al::TableObject> tables;
  std::vector<std::string> paths;
  std::vector<NativeSourceIssue> issues;
  std::vector<std::string> otherSources;
};

NativeTableSources ReadNativeTables(const std::filesystem::path &package);

}

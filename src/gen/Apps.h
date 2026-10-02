#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace agiru::gen {

struct App {
  std::string name;
  std::string source;
  std::vector<std::string> depends;
};

std::vector<App> ReadApps(const std::filesystem::path &path);

struct TranspileScope {
  std::vector<std::string> include;
  std::vector<std::string> exclude;
  std::vector<std::string> areaExclude;
  std::vector<std::string> areaExcludeSuffix;
};

[[nodiscard]] bool Holds(const TranspileScope &scope, std::string_view nameSpace);

[[nodiscard]] bool HoldsArea(const TranspileScope &scope, std::string_view area);

TranspileScope ReadScope(const std::filesystem::path &path);

}

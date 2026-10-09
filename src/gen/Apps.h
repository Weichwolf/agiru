#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

struct App {
  std::string name;
  std::string source;
  std::vector<std::string> depends;
};

std::vector<App> ReadApps(const std::filesystem::path &path);

enum class SourceDomain { BCApps, SystemSymbols };

struct SourceExclusion {
  std::string reason;
  std::string source;
  SourceDomain domain = SourceDomain::BCApps;
};

struct TranspileScope {
  std::vector<std::string> include;
  std::vector<std::string> exclude;
  std::vector<std::string> areaExclude;
  std::vector<std::string> areaExcludeSuffix;
  std::vector<SourceExclusion> productExclude;
  std::vector<std::string> sourceInclude;
};

[[nodiscard]] bool Holds(const TranspileScope &scope, std::string_view nameSpace);

[[nodiscard]] bool HoldsArea(const TranspileScope &scope, std::string_view area);

TranspileScope ReadScope(const std::filesystem::path &path);

[[nodiscard]] std::optional<std::string_view>
ProductExclusion(const TranspileScope &scope,
                 const std::filesystem::path &relativeSource,
                 SourceDomain domain = SourceDomain::BCApps);

[[nodiscard]] bool SourceIncluded(const TranspileScope &scope,
                                  const std::filesystem::path &relativeSource,
                                  SourceDomain domain = SourceDomain::BCApps);

}

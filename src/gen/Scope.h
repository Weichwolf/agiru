#pragma once

#include "ObjectKind.h"

#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

std::string_view DirectoryOf(ObjectKind kind);

[[nodiscard]] bool NamespaceInScope(std::string_view nameSpace,
                                    std::span<const std::string> include,
                                    std::span<const std::string> exclude);

class Scope {
public:
  static Scope FromFile(const std::filesystem::path &path);

  [[nodiscard]] bool Contains(std::string_view nameSpace) const;

  [[nodiscard]] std::size_t IncludeCount() const { return include_.size(); }

  [[nodiscard]] std::size_t ExcludeCount() const { return exclude_.size(); }

private:
  std::vector<std::string> include_;
  std::vector<std::string> exclude_;
};

std::string OutputDirectory(std::string_view nameSpace, ObjectKind kind);

std::string NamespaceSuffix(std::string_view nameSpace);

std::string NamespaceOf(std::string_view nameSpace);

}

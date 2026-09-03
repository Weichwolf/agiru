#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

enum class ObjectKind : std::uint8_t {
  Table,
  Codeunit,
  Page,
  Report,
  Query,
  XmlPort,
  Enum,
  Interface,
  PermissionSet,
};

std::string_view DirectoryOf(ObjectKind kind);

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

}

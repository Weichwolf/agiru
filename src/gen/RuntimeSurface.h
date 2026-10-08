#pragma once

#include "ObjectKind.h"

#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <string_view>

namespace agiru::gen {

[[nodiscard]] std::string RuntimeIncludes(std::string_view text, ObjectKind kind);

inline constexpr std::string_view kRuntimeIncludeMarker = "// @door\n";

[[nodiscard]] std::string WithRuntimeIncludes(std::string text, ObjectKind kind);

[[nodiscard]] std::string RuntimeSpelling(std::string_view name);

[[nodiscard]] std::string BuiltinSpelling(std::string_view name);

[[nodiscard]] bool RuntimeDeclares(std::string_view name);

[[nodiscard]] const std::set<std::string> &RebuiltDotNet();

[[nodiscard]] const std::map<std::string, std::string> &PlatformMembers(std::string_view table);

[[nodiscard]] bool RuntimeCallable(std::string_view name);

inline constexpr std::size_t kAllRecordFieldArguments = static_cast<std::size_t>(-1);

[[nodiscard]] std::size_t RecordFieldArguments(std::string_view method);

[[nodiscard]] bool DeclaredByBase(std::string_view header, std::string_view name);

struct StaticMember {
  std::string_view type;
  std::string_view member;
};

[[nodiscard]] bool RuntimeStaticCallable(const StaticMember &wanted);

struct PlatformField {
  std::string_view table;
  std::string_view field;
};

[[nodiscard]] bool PlatformFieldNamed(const PlatformField &wanted);

[[nodiscard]] std::string PlatformFieldSpelling(const PlatformField &wanted);

[[nodiscard]] bool HiddenByABaseMember(std::string_view name);

[[nodiscard]] const std::set<std::string> &TableMembers();

}

#pragma once

#include "Scope.h"

#include <map>
#include <set>
#include <string>
#include <string_view>

namespace agiru::gen {

[[nodiscard]] std::string DoorIncludes(std::string_view text, ObjectKind kind);

inline constexpr std::string_view kDoorMarker = "// @door\n";

[[nodiscard]] std::string WithDoor(std::string text, ObjectKind kind);

[[nodiscard]] std::string AsTheDoorSpellsIt(std::string_view name);

[[nodiscard]] std::string BuiltinSpelling(std::string_view name);

[[nodiscard]] bool DoorDeclares(std::string_view name);

[[nodiscard]] const std::set<std::string> &RebuiltDotNet();

[[nodiscard]] const std::map<std::string, std::string> &PlatformMembers(std::string_view table);

[[nodiscard]] const std::map<std::string, std::string> &PlatformMembers(std::string_view table);

[[nodiscard]] bool DoorCalls(std::string_view name);

struct StaticMember {
  std::string_view type;
  std::string_view member;
};

[[nodiscard]] bool DoorStaticCalls(const StaticMember &wanted);

struct PlatformField {
  std::string_view table;
  std::string_view field;
};

[[nodiscard]] bool PlatformFieldNamed(const PlatformField &wanted);

[[nodiscard]] std::string PlatformFieldSpelling(const PlatformField &wanted);

[[nodiscard]] bool HiddenByABaseMember(std::string_view name);

[[nodiscard]] const std::set<std::string> &TableMembers();

}

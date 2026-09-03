#pragma once

#include "Scope.h"

#include <filesystem>
#include <string>
#include <string_view>

namespace agiru::gen {

[[nodiscard]] std::string DoorIncludes(std::string_view text, ObjectKind kind);

inline constexpr std::string_view kDoorMarker = "// @door\n";

[[nodiscard]] std::string WithDoor(std::string text, ObjectKind kind);

[[nodiscard]] std::string AsTheDoorSpellsIt(std::string_view name);

[[nodiscard]] bool DoorDeclares(std::string_view name);

}

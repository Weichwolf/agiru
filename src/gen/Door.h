#pragma once

#include "Scope.h"

#include <string>
#include <string_view>

namespace agiru::gen {

/// The door headers a generated file needs, as `#include` lines in sorted order.
///
/// A GENERATED FILE INCLUDES WHAT IT USES AND NOT A MASTER HEADER. `agiru.h` was one, and it cost
/// every one of the 5 835 generated translation units the whole door -- 988 ms of parse against the
/// 406 ms a file's own types take (measured 2026-09-03). It also has no counterpart in AL, which
/// declares its dependencies per file with `using`; 36 031 of those lines say what a BC source
/// needs, and a generated file that names everything is a shape no AL reader recognises.
///
/// \param text The file as it will be written, which is what NAMES the types.
/// \param kind What is being generated, which decides the base class and the metadata.
/// \return The include lines, one per header, sorted, ending in a blank line.
[[nodiscard]] std::string DoorIncludes(std::string_view text, ObjectKind kind);

/// The line a writer leaves where the door's includes belong.
///
/// The set can only be known once the file is WRITTEN, because what decides it is which names the
/// file spells; so the writer marks the place and the marker is replaced at the end.
inline constexpr std::string_view kDoorMarker = "// @door\n";

/// Puts the door's includes where the marker is.
/// \param text The finished file, carrying exactly one marker.
/// \param kind What was generated.
/// \return The file with its includes.
[[nodiscard]] std::string WithDoor(std::string text, ObjectKind kind);

} // namespace agiru::gen

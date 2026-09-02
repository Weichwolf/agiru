#pragma once

#include "Scope.h"

#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

std::string Identifier(std::string_view alName);

std::string EnumeratorName(std::string_view optionMember);

std::vector<std::string> EnumeratorNames(const std::vector<std::string> &members);

std::string OptionEnumName(std::string_view tableName, std::string_view fieldName);

std::string TypeName(std::string_view alType);

/// \brief Whether a name is one of AL's own type names.
///
/// \param alType The name as AL wrote it.
/// \return True when `TypeName` recognised it.
///
/// A name that is NEITHER an AL type NOR an object this run has read is a type nobody here defines:
/// AL writes the platform's own enums as bare type names -- `Verbosity`, `DataClassification`,
/// `TelemetryScope` -- and they belong with the absent rather than beside the door.
bool IsAlTypeName(std::string_view alType);

std::string Literal(std::string_view text);

/// What an AL source file declares at its top level: the object's name and the namespace above it.
struct ObjectDeclaration {
  bool found = false;    ///< False when the file declares no object of the wanted kind.
  std::string name;      ///< The AL object name, quotes removed: `Library - Lower Permissions`.
  std::string nameSpace; ///< The `namespace` line above it, empty when there is none.
};

/// Reads the declaration line of an AL object LEXICALLY, without parsing the file.
///
/// This exists because a codeunit may hold a variable of a codeunit declared later in the same app,
/// so the names must be indexed before anything is emitted -- and parsing 4 000 objects twice to
/// learn two lines costs minutes.
///
/// THE DECLARATION MAY BE THE FIRST LINE OF THE FILE, and 5 387 of BCApps' 14 282 codeunit files
/// begin with it -- 38 %, measured 2026-09-02. A search for "\ncodeunit " alone found none of them,
/// and an object that is not indexed is not RESOLVED: the generator then emits the bare AL name and
/// the C++ compiler reports a missing type in every file that names it.
/// \note THE KIND IS A KIND AND NOT A KEYWORD. AL declares nine kinds of object with the same
///       shape, and a free string beside the source is two adjacent texts a caller can swap
///       silently -- which is what `ObjectKind` already exists to prevent for the output path.
ObjectDeclaration DeclarationOf(std::string_view source, ObjectKind kind);

} // namespace agiru::gen

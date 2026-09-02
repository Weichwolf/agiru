#pragma once

#include "Ast.h"
#include "EnumWriter.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace agiru::gen {

/// Where one AL object landed: the C++ name it took and the header that declares it.
struct TableRef {
  std::string identifier;
  std::string header;
};

/// Every table object the run has seen, keyed by its AL name in lower case, and by its AL NUMBER
/// as text beside it -- AL names an object either way and test code uses both.
using TableIndex = std::map<std::string, TableRef>;

/// What one .NET type is asked to do, gathered from the AL that uses it.
using DotNetUse = std::map<std::string, std::set<std::string>>;

/// One translated codeunit, and what it needed that the run did not have.
struct CodeunitHeader {
  std::string text;
  std::vector<std::string> unresolvedTables;

  /// Every `.NET type -> member` the codeunit's bodies call. The .NET surface is not declared
  /// anywhere -- a `dotnet` package names a type and no members -- so the only place it exists is
  /// the call sites (board:0035).
  DotNetUse dotnet;
};

/// EVERY OBJECT KIND GETS ITS OWN INDEX, because AL lets a table and a codeunit carry one name and
/// tells them apart by the keyword: `Record "X"` against `Codeunit "X"`. One index would let a
/// codeunit variable resolve to a table class, which compiles and is wrong.
struct Objects {
  TableIndex tables;
  TableIndex codeunits;
  EnumIndex enums;
};

/// The tables the PLATFORM provides, which no `.al` file declares.
///
/// They are not AL objects; they only look like AL objects to AL code, and internally they do what
/// generated code cannot -- `Field` produces a row per field of every table in the catalogue
/// (board:0032). The generator needs them here for one reason: `Record Field` must resolve into
/// `agiru::platform` and not into `agiru::app`, where nothing will ever declare it.
///
/// This is the same kind of list as `TypeName()`'s AL type names: the platform's own vocabulary,
/// written down once. An app that declared a table of the same name would overwrite the entry,
/// which is the right precedence -- its own object wins in its own tree.
[[nodiscard]] TableIndex PlatformTables();

CodeunitHeader WriteCodeunit(const al::CodeunitObject &unit,
                             const std::string &sourcePath,
                             const Objects &objects);

std::string WriteCodeunitSource(const al::CodeunitObject &unit,
                                const std::string &sourcePath,
                                const Objects &objects);

std::string CodeunitHeaderPath(const al::CodeunitObject &unit);

} // namespace agiru::gen

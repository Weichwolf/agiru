#pragma once

#include "Ast.h"
#include "CodeunitWriter.h"
#include "EnumWriter.h"

#include <string>
#include <vector>

namespace agiru::gen {

/// One translated table, and what it needed that the run did not have.
struct TableHeader {
  std::string text;
  std::vector<std::string> unresolvedEnums;
  DotNetUse dotnet;
  DotNetUse absent;
};

/// \brief The C++ member name for one of a table's own variables.
/// \param table The table.
/// \param name  The AL variable name.
/// \return The identifier, disambiguated when a field or an earlier variable already spells it.
std::string VariableIdentifier(const al::TableObject &table, const std::string &name);

/// \brief The C++ member name for one of a table's fields.
/// \param table The table.
/// \param name  The AL field name, however the caller spelled it.
/// \return The identifier, disambiguated when two AL names collapse onto one.
std::string FieldIdentifier(const al::TableObject &table, const std::string &name);

/// \brief The C++ member-function name for one of a table's procedures.
/// \param table The table.
/// \param name  The AL procedure name.
/// \return The identifier, disambiguated when a field or a variable already spells it.
std::string ProcedureIdentifier(const al::TableObject &table, const std::string &name);

TableHeader WriteHeader(const al::TableObject &declared,
                        const std::string &sourcePath,
                        const EnumIndex &enums,
                        const Objects &objects);

} // namespace agiru::gen

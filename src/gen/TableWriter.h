#pragma once

#include "meta/SystemFields.h"

#include "Ast.h"
#include "CodeunitWriter.h"
#include "EnumWriter.h"

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace agiru::gen {

struct TableHeader {
  std::string text;
  std::vector<std::string> unresolvedEnums;
  DotNetUse dotnet;
  AbsentUse absent;
};

[[nodiscard]] TableRef BindTable(const al::TableObject &table,
                                 std::string identifier,
                                 std::string header,
                                 std::optional<SystemFieldProfile> hostProfile = std::nullopt);

[[nodiscard]] std::string NativeTableAssertions(const al::TableObject &table,
                                                const TableRef &binding);

[[nodiscard]] std::string NativeTableDefinition(const al::TableObject &table,
                                                const TableRef &binding,
                                                const Objects &objects);

std::string VariableIdentifier(const al::TableObject &table, const std::string &name);

std::string FieldIdentifier(const al::TableObject &table, const std::string &name);

std::string ProcedureIdentifier(const al::TableObject &table, const std::string &name);

std::string TableDefinitions(const al::TableObject &declared, const Objects &objects);

TableHeader WriteHeader(const al::TableObject &declared,
                        const std::string &sourcePath,
                        const EnumIndex &enums,
                        const Objects &objects);

[[nodiscard]] std::set<std::string> Shadowed(const al::TableObject &table);

}

#pragma once

#include "Ast.h"
#include "CodeunitWriter.h"
#include "EnumWriter.h"

#include <string>
#include <vector>

namespace agiru::gen {

struct TableHeader {
  std::string text;
  std::vector<std::string> unresolvedEnums;
  DotNetUse dotnet;
  DotNetUse absent;
};

std::string VariableIdentifier(const al::TableObject &table, const std::string &name);

std::string FieldIdentifier(const al::TableObject &table, const std::string &name);

std::string ProcedureIdentifier(const al::TableObject &table, const std::string &name);

TableHeader WriteHeader(const al::TableObject &declared,
                        const std::string &sourcePath,
                        const EnumIndex &enums,
                        const Objects &objects);

}

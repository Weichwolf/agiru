#pragma once

#include "Ast.h"
#include "Expr.h"

#include <string>
#include <vector>

namespace agiru::gen {

std::string
WriteStatements(const al::TableObject &table, const std::vector<al::Stmt> &body, int indent);

std::string WriteSource(const al::TableObject &table, const std::string &sourcePath);

} // namespace agiru::gen

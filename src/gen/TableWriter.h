#pragma once

#include "Ast.h"

#include <string>

namespace agiru::gen {

std::string WriteHeader(const al::TableObject &table, const std::string &sourcePath);

} // namespace agiru::gen

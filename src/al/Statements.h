#pragma once

#include "Expr.h"
#include "Token.h"

#include <span>
#include <vector>

namespace agiru::al {

std::vector<Stmt> ParseStatements(std::span<const Token> tokens);

}

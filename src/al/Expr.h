#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace agiru::al {

enum class ExprKind : std::uint8_t {
  StringLiteral,
  NumberLiteral,
  Name,
  Scope,
  Call,
  Binary,
  Unary,
  Set,
  Range,
  Index,
};

struct Expr {
  ExprKind kind = ExprKind::Name;
  std::string text;
  std::vector<Expr> children;
};

enum class StmtKind : std::uint8_t {
  Expression,
  If,
  Block,
  Repeat,
};

struct Stmt {
  StmtKind kind = StmtKind::Expression;
  Expr expression;
  std::vector<Stmt> body;
  std::vector<Stmt> otherwise;
};

} // namespace agiru::al

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace agiru::al {

enum class ExprKind : std::uint8_t {
  StringLiteral,
  NumberLiteral,
  TemporalLiteral,
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
  While,
  For,
  ForEach,
  Case,
  CaseBranch,
  With,
  Exit,
  Break,
  AssertError,
};

struct Stmt {
  StmtKind kind = StmtKind::Expression;
  Expr expression;
  std::vector<Expr> labels;
  std::vector<Stmt> body;
  std::vector<Stmt> otherwise;
  bool descending = false;
};

}

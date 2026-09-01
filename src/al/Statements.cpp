#include "Statements.h"

#include "Expr.h"
#include "Parser.h"
#include "Token.h"

#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace agiru::al {

namespace {

constexpr int kMaxDepth = 128;

struct Level {
  int precedence;
  const char *word;
};

constexpr std::array kLevels{
    Level{.precedence = 1, .word = "or"},
    Level{.precedence = 2, .word = "and"},
    Level{.precedence = 3, .word = "="},
    Level{.precedence = 3, .word = "<>"},
    Level{.precedence = 3, .word = "<"},
    Level{.precedence = 3, .word = "<="},
    Level{.precedence = 3, .word = ">"},
    Level{.precedence = 3, .word = ">="},
    Level{.precedence = 3, .word = "in"},
    Level{.precedence = 4, .word = "+"},
    Level{.precedence = 4, .word = "-"},
    Level{.precedence = 5, .word = "*"},
    Level{.precedence = 5, .word = "/"},
    Level{.precedence = 5, .word = "div"},
    Level{.precedence = 5, .word = "mod"},
};

class Reader {
public:
  explicit Reader(std::span<const Token> tokens) : tokens_(tokens) {}

  std::vector<Stmt> ReadBlock() {
    std::vector<Stmt> statements;
    while (!AtEnd() && !AtKeyword("end") && !AtKeyword("until")) {
      statements.push_back(ReadStatement());
      while (AtPunctuation(";")) { Advance(); }
    }
    return statements;
  }

private:
  [[nodiscard]] const Token &Peek(std::size_t ahead = 0) const {
    static const Token kEnd{};
    const std::size_t index = position_ + ahead;
    return index < tokens_.size() ? tokens_[index] : kEnd;
  }

  [[nodiscard]] bool AtEnd() const { return position_ >= tokens_.size(); }

  [[nodiscard]] bool AtKeyword(std::string_view word) const { return IsKeyword(Peek(), word); }

  [[nodiscard]] bool AtPunctuation(std::string_view text) const {
    return IsPunctuation(Peek(), text);
  }

  void Advance() { ++position_; }

  void Expect(std::string_view what) {
    if (AtKeyword(what) || AtPunctuation(what)) {
      Advance();
      return;
    }
    throw ParseError("expected '" + std::string(what) + "' but found '" + Peek().text +
                     "' on line " + std::to_string(Peek().line));
  }

  Stmt ReadStatement() {
    if (AtKeyword("begin")) {
      Advance();
      Stmt block{.kind = StmtKind::Block, .expression = {}, .body = ReadBlock(), .otherwise = {}};
      Expect("end");
      return block;
    }
    if (AtKeyword("repeat")) {
      Advance();
      Stmt loop{.kind = StmtKind::Repeat, .expression = {}, .body = ReadBlock(), .otherwise = {}};
      Expect("until");
      loop.expression = ReadExpression(0);
      return loop;
    }
    if (AtKeyword("if")) { return ReadIf(); }
    return Stmt{
        .kind = StmtKind::Expression, .expression = ReadExpression(0), .body = {}, .otherwise = {}};
  }

  Stmt ReadIf() {
    Expect("if");
    Stmt statement{
        .kind = StmtKind::If, .expression = ReadExpression(0), .body = {}, .otherwise = {}};
    Expect("then");
    statement.body.push_back(ReadStatement());
    while (AtPunctuation(";")) { Advance(); }
    if (AtKeyword("else")) {
      Advance();
      statement.otherwise.push_back(ReadStatement());
    }
    return statement;
  }

  [[nodiscard]] static int PrecedenceOf(const Token &token) {
    for (const Level &level : kLevels) {
      if (IsKeyword(token, level.word) || IsPunctuation(token, level.word)) {
        return level.precedence;
      }
    }
    return 0;
  }

  Expr ReadExpression(int minimum) {
    if (++depth_ > kMaxDepth) { throw ParseError("an expression nests too deeply"); }
    Expr left = ReadUnary();
    while (!AtEnd()) {
      const int precedence = PrecedenceOf(Peek());
      if (precedence == 0 || precedence < minimum) { break; }
      const std::string op = Peek().text;
      Advance();
      Expr right = ReadExpression(precedence + 1);
      Expr binary{.kind = ExprKind::Binary, .text = op, .children = {}};
      binary.children.push_back(std::move(left));
      binary.children.push_back(std::move(right));
      left = std::move(binary);
    }
    --depth_;
    return left;
  }

  Expr ReadUnary() {
    if (AtKeyword("not") || AtPunctuation("-")) {
      const std::string op = Peek().text;
      Advance();
      Expr unary{.kind = ExprKind::Unary, .text = op, .children = {}};
      unary.children.push_back(ReadUnary());
      return unary;
    }
    return ReadPostfix();
  }

  Expr ReadPostfix() {
    Expr value = ReadPrimary();
    while (!AtEnd()) {
      if (AtPunctuation("::")) {
        Advance();
        Expr scope{.kind = ExprKind::Scope, .text = Peek().text, .children = {}};
        Advance();
        scope.children.push_back(std::move(value));
        value = std::move(scope);
        continue;
      }
      if (AtPunctuation(".")) {
        Advance();
        Expr member{.kind = ExprKind::Name, .text = Peek().text, .children = {}};
        Advance();
        Expr access{.kind = ExprKind::Binary, .text = ".", .children = {}};
        access.children.push_back(std::move(value));
        access.children.push_back(std::move(member));
        value = std::move(access);
        continue;
      }
      if (AtPunctuation("[")) {
        Advance();
        Expr index{.kind = ExprKind::Index, .text = {}, .children = {}};
        index.children.push_back(std::move(value));
        index.children.push_back(ReadExpression(0));
        Expect("]");
        value = std::move(index);
        continue;
      }
      if (AtPunctuation("(")) {
        Advance();
        Expr call{.kind = ExprKind::Call, .text = {}, .children = {}};
        call.children.push_back(std::move(value));
        while (!AtEnd() && !AtPunctuation(")")) {
          call.children.push_back(ReadExpression(0));
          if (AtPunctuation(",")) { Advance(); }
        }
        Expect(")");
        value = std::move(call);
        continue;
      }
      break;
    }
    return value;
  }

  Expr ReadPrimary() {
    if (AtPunctuation("[")) {
      Advance();
      Expr set{.kind = ExprKind::Set, .text = {}, .children = {}};
      while (!AtEnd() && !AtPunctuation("]")) {
        Expr item = ReadExpression(0);
        if (AtPunctuation("..")) {
          Advance();
          Expr range{.kind = ExprKind::Range, .text = {}, .children = {}};
          range.children.push_back(std::move(item));
          range.children.push_back(ReadExpression(0));
          item = std::move(range);
        }
        set.children.push_back(std::move(item));
        if (AtPunctuation(",")) { Advance(); }
      }
      Expect("]");
      return set;
    }
    if (AtPunctuation("(")) {
      Advance();
      Expr inner = ReadExpression(0);
      Expect(")");
      return inner;
    }
    const Token &token = Peek();
    Advance();
    switch (token.kind) {
      case TokenKind::String:
        return Expr{.kind = ExprKind::StringLiteral, .text = token.text, .children = {}};
      case TokenKind::Integer:
      case TokenKind::Decimal:
        return Expr{.kind = ExprKind::NumberLiteral, .text = token.text, .children = {}};
      default: return Expr{.kind = ExprKind::Name, .text = token.text, .children = {}};
    }
  }

  std::span<const Token> tokens_;
  std::size_t position_ = 0;
  int depth_ = 0;
};

} // namespace

std::vector<Stmt> ParseStatements(std::span<const Token> tokens) {
  return Reader(tokens).ReadBlock();
}

} // namespace agiru::al

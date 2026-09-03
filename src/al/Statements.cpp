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

// THE AL HIERARCHY IS PASCAL'S AND NOT C'S, and this table was C's until it was checked.
// `c-al-operators.md` states the order of precedence outright, and AL inherited it from C/AL:
//
//     1. . (fields)  [] (indexing)  () (parentheses)  :: (scope)
//     2. NOT, unary -, unary +
//     3. *  /  DIV  MOD  AND  XOR
//     4. +  -  OR
//     5. >  <  >=  <=  =  <>  IN
//     6. .. (range)
//
// So AND binds like multiplication, OR like addition, and THE COMPARISONS BIND LOOSEST. `A = B and
// C = D` is `A = (B and C) = D` in AL and `(A = B) and (C = D)` in C. BC code parenthesises heavily
// because of this, which is why the wrong table cost nothing visible -- and why it would have cost
// something eventually, silently, in the one place that did not.
//
// The numbers here run the other way from the documentation's list: HIGHER BINDS TIGHTER.
constexpr std::array kLevels{
    Level{.precedence = 1, .word = "="},
    Level{.precedence = 1, .word = "<>"},
    Level{.precedence = 1, .word = "<"},
    Level{.precedence = 1, .word = "<="},
    Level{.precedence = 1, .word = ">"},
    Level{.precedence = 1, .word = ">="},
    Level{.precedence = 1, .word = "in"},
    Level{.precedence = 1, .word = "is"},
    Level{.precedence = 1, .word = "as"},
    Level{.precedence = 2, .word = "+"},
    Level{.precedence = 2, .word = "-"},
    Level{.precedence = 2, .word = "or"},
    Level{.precedence = 3, .word = "*"},
    Level{.precedence = 3, .word = "/"},
    Level{.precedence = 3, .word = "div"},
    Level{.precedence = 3, .word = "mod"},
    Level{.precedence = 3, .word = "and"},
    Level{.precedence = 3, .word = "xor"},
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
      Stmt block{.kind = StmtKind::Block,
                 .expression = {},
                 .labels = {},
                 .body = ReadBlock(),
                 .otherwise = {},
                 .descending = false};
      Expect("end");
      return block;
    }
    if (AtKeyword("repeat")) {
      Advance();
      Stmt loop{.kind = StmtKind::Repeat,
                .expression = {},
                .labels = {},
                .body = ReadBlock(),
                .otherwise = {},
                .descending = false};
      Expect("until");
      loop.expression = ReadTernary();
      return loop;
    }
    if (AtKeyword("while")) {
      Advance();
      Stmt loop{.kind = StmtKind::While,
                .expression = ReadTernary(),
                .labels = {},
                .body = {},
                .otherwise = {},
                .descending = false};
      Expect("do");
      loop.body.push_back(ReadStatement());
      return loop;
    }
    if (AtKeyword("for")) { return ReadFor(); }
    if (AtKeyword("foreach")) { return ReadForEach(); }
    if (AtKeyword("case")) { return ReadCase(); }
    if (AtKeyword("with")) {
      Advance();
      Stmt scope{.kind = StmtKind::With,
                 .expression = ReadTernary(),
                 .labels = {},
                 .body = {},
                 .otherwise = {},
                 .descending = false};
      Expect("do");
      scope.body.push_back(ReadStatement());
      return scope;
    }
    if (AtKeyword("exit")) {
      Advance();
      Stmt leave{.kind = StmtKind::Exit,
                 .expression = {},
                 .labels = {},
                 .body = {},
                 .otherwise = {},
                 .descending = false};
      if (AtPunctuation("(")) {
        Advance();
        leave.expression = ReadTernary();
        Expect(")");
      }
      return leave;
    }
    // `break` LEAVES THE INNERMOST LOOP and nothing else -- `devenv-al-control-statements.md`
    // separates it from the Break METHOD of a report, which also ends its trigger.
    if (AtKeyword("break")) {
      Advance();
      return Stmt{.kind = StmtKind::Break,
                  .expression = {},
                  .labels = {},
                  .body = {},
                  .otherwise = {},
                  .descending = false};
    }
    if (AtKeyword("if")) { return ReadIf(); }
    // `asserterror <statement>` -- AL's own try/expect. The statement is expected to raise, the
    // error text lands where GetLastErrorText reads it, and execution carries on with the next
    // statement. It is one word in front of an ordinary statement, so it parses as one; what it
    // MEANS is board:0021, and a test suite is mostly made of it.
    if (AtKeyword("asserterror")) {
      Advance();
      Stmt expected{.kind = StmtKind::AssertError,
                    .expression = {},
                    .labels = {},
                    .body = {},
                    .otherwise = {},
                    .descending = false};
      expected.body.push_back(ReadStatement());
      return expected;
    }
    Expr value = ReadTernary();
    for (const std::string_view assignment : {":=", "+=", "-=", "*=", "/="}) {
      if (!AtPunctuation(assignment)) { continue; }
      Advance();
      Expr assign{.kind = ExprKind::Binary, .text = std::string(assignment), .children = {}};
      assign.children.push_back(std::move(value));
      assign.children.push_back(ReadTernary());
      value = std::move(assign);
      break;
    }
    return Stmt{.kind = StmtKind::Expression,
                .expression = std::move(value),
                .labels = {},
                .body = {},
                .otherwise = {},
                .descending = false};
  }

  Stmt ReadFor() {
    Expect("for");
    Expr counter = ReadPostfix();
    Expect(":=");
    Expr start{.kind = ExprKind::Binary, .text = ":=", .children = {}};
    start.children.push_back(std::move(counter));
    start.children.push_back(ReadTernary());
    Stmt loop{.kind = StmtKind::For,
              .expression = std::move(start),
              .labels = {},
              .body = {},
              .otherwise = {},
              .descending = false};
    if (AtKeyword("downto")) {
      loop.descending = true;
      Advance();
    } else {
      Expect("to");
    }
    loop.labels.push_back(ReadTernary());
    Expect("do");
    loop.body.push_back(ReadStatement());
    return loop;
  }

  Stmt ReadForEach() {
    Expect("foreach");
    Stmt loop{.kind = StmtKind::ForEach,
              .expression = ReadPostfix(),
              .labels = {},
              .body = {},
              .otherwise = {},
              .descending = false};
    Expect("in");
    loop.labels.push_back(ReadTernary());
    Expect("do");
    loop.body.push_back(ReadStatement());
    return loop;
  }

  Stmt ReadCase() {
    Expect("case");
    Stmt selector{.kind = StmtKind::Case,
                  .expression = ReadTernary(),
                  .labels = {},
                  .body = {},
                  .otherwise = {},
                  .descending = false};
    Expect("of");
    while (!AtEnd() && !AtKeyword("end") && !AtKeyword("else")) {
      Stmt branch{.kind = StmtKind::CaseBranch,
                  .expression = {},
                  .labels = {},
                  .body = {},
                  .otherwise = {},
                  .descending = false};
      while (!AtEnd() && !AtPunctuation(":")) {
        Expr label = ReadTernary();
        if (AtPunctuation("..")) {
          Advance();
          Expr range{.kind = ExprKind::Range, .text = {}, .children = {}};
          range.children.push_back(std::move(label));
          range.children.push_back(ReadTernary());
          label = std::move(range);
        }
        branch.labels.push_back(std::move(label));
        if (AtPunctuation(",")) { Advance(); }
      }
      Expect(":");
      branch.body.push_back(ReadStatement());
      while (AtPunctuation(";")) { Advance(); }
      selector.body.push_back(std::move(branch));
    }
    if (AtKeyword("else")) {
      Advance();
      selector.otherwise = ReadBlock();
    }
    Expect("end");
    return selector;
  }

  Stmt ReadIf() {
    Expect("if");
    Stmt statement{.kind = StmtKind::If,
                   .expression = ReadTernary(),
                   .labels = {},
                   .body = {},
                   .otherwise = {},
                   .descending = false};
    Expect("then");
    statement.body.push_back(ReadStatement());
    // A SEMICOLON ENDS THE `if`, SO THE `else` AFTER ONE IS NOT ITS OWN. `Incoming Document` writes
    //
    //     case true of
    //         SalesCrMemoHeader.Get(DocNo):
    //             if SalesCrMemoHeader."Posting Date" = PostingDate then
    //                 exit("Document Type"::"Sales Credit Memo");
    //         else
    //             ...
    //
    // and taking that `else` for the `if` swallowed the case's own branch: the branch parser then
    // ran off the end of the tokens looking for the next label, which is where
    // "expected ':' but found ''" came from. AL's own `if ... then ... else` never carries a
    // semicolon in front of the `else`, so the presence of one is the signal.
    bool terminated = false;
    while (AtPunctuation(";")) {
      Advance();
      terminated = true;
    }
    if (!terminated && AtKeyword("else")) {
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

  /// AL's conditional operator, `cond ? a : b`, which BC 25 added and which binds loosest of all.
  /// It is read here rather than in the level table because it is right-associative and ternary,
  /// which a precedence climb over binary levels cannot express.
  Expr ReadTernary() {
    Expr condition = ReadExpression(1);
    if (!AtPunctuation("?")) { return condition; }
    Advance();
    Expr whenTrue = ReadTernary();
    if (!AtPunctuation(":")) { throw ParseError("a conditional expression needs its ':'"); }
    Advance();
    Expr whenFalse = ReadTernary();
    Expr conditional{.kind = ExprKind::Binary, .text = "?:", .children = {}};
    conditional.children.push_back(std::move(condition));
    conditional.children.push_back(std::move(whenTrue));
    conditional.children.push_back(std::move(whenFalse));
    return conditional;
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
        while (!AtEnd() && !AtPunctuation("]")) {
          index.children.push_back(ReadTernary());
          if (AtPunctuation(",")) { Advance(); }
        }
        Expect("]");
        value = std::move(index);
        continue;
      }
      if (AtPunctuation("(")) {
        Advance();
        Expr call{.kind = ExprKind::Call, .text = {}, .children = {}};
        call.children.push_back(std::move(value));
        while (!AtEnd() && !AtPunctuation(")")) {
          call.children.push_back(ReadTernary());
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
        Expr item = ReadTernary();
        if (AtPunctuation("..")) {
          Advance();
          Expr range{.kind = ExprKind::Range, .text = {}, .children = {}};
          range.children.push_back(std::move(item));
          range.children.push_back(ReadTernary());
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
      Expr inner = ReadTernary();
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
      // `0D`, `20011125D`, `080000T`, `0DT`. It is a LITERAL and it was falling through to a name,
      // so `0D` became the identifier `_0D` -- 6 383 of them in W1 alone.
      case TokenKind::DateTime:
        return Expr{.kind = ExprKind::TemporalLiteral, .text = token.text, .children = {}};
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

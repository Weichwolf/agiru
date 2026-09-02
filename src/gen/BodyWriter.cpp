#include "BodyWriter.h"

#include "TableWriter.h"

#include "Ast.h"
#include "Expr.h"
#include "Names.h"

#include <array>
#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

namespace {

// MEASURED, NOT GUESSED (2026-09-01, over all four apps): the deepest expression the BaseApp and
// its tests actually build is well inside this, and the guard exists to stop runaway recursion on a
// malformed tree rather than to cap a legitimate expression. BC really does split a base64 blob
// across hundreds of lines joined by `+`.
constexpr int kMaxDepth = 4096;

struct Operator {
  const char *al;
  const char *cpp;
  int precedence;
};

/// C++'s conditional operator binds looser than everything but assignment and comma, which is
/// where AL puts it too.
constexpr int kConditionalPrecedence = 1;
constexpr int kEqualityPrecedence = 3;
constexpr int kComparisonPrecedence = 4;
constexpr int kUnaryPrecedence = 8;
constexpr int kPrimaryPrecedence = 9;

// THIS TABLE IS C++'S PRECEDENCE AND THE ONE IN src/al/Statements.cpp IS AL'S. They are DIFFERENT
// ON PURPOSE and neither is a copy of the other: the parser's table decides what the AL text means,
// this one decides which parentheses the C++ text needs so that it means the same thing. AL binds
// AND like multiplication and its comparisons loosest; C++ does neither. Making them agree would
// break one of the two jobs.
constexpr std::array kOperators{
    Operator{.al = "or", .cpp = "||", .precedence = 1},
    Operator{.al = "and", .cpp = "&&", .precedence = 2},
    // AL `xor` on two booleans is C++ `!=` on two booleans, which sits with `==` and NOT with `||`.
    Operator{.al = "xor", .cpp = "!=", .precedence = 3},
    Operator{.al = "=", .cpp = "==", .precedence = 3},
    Operator{.al = "<>", .cpp = "!=", .precedence = 3},
    Operator{.al = "<", .cpp = "<", .precedence = 4},
    Operator{.al = "<=", .cpp = "<=", .precedence = 4},
    Operator{.al = ">", .cpp = ">", .precedence = 4},
    Operator{.al = ">=", .cpp = ">=", .precedence = 4},
    Operator{.al = "+", .cpp = "+", .precedence = 5},
    Operator{.al = "-", .cpp = "-", .precedence = 5},
    Operator{.al = "*", .cpp = "*", .precedence = 6},
    Operator{.al = "/", .cpp = "/", .precedence = 6},
    Operator{.al = "div", .cpp = "/", .precedence = 6},
    Operator{.al = "mod", .cpp = "%", .precedence = 6},
    Operator{.al = ".", .cpp = ".", .precedence = kPrimaryPrecedence},
    Operator{.al = ":=", .cpp = "=", .precedence = 0},
    Operator{.al = "+=", .cpp = "+=", .precedence = 0},
    Operator{.al = "-=", .cpp = "-=", .precedence = 0},
    Operator{.al = "*=", .cpp = "*=", .precedence = 0},
    Operator{.al = "/=", .cpp = "/=", .precedence = 0},
};

const Operator *Find(std::string_view al) {
  for (const Operator &op : kOperators) {
    if (al == op.al) { return &op; }
  }
  return nullptr;
}

bool SameName(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) { return false; }
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(a[i])) !=
        std::tolower(static_cast<unsigned char>(b[i]))) {
      return false;
    }
  }
  return true;
}

const al::FieldDecl *FieldNamed(const al::TableObject &table, std::string_view name) {
  for (const al::FieldDecl &field : table.fields) {
    if (SameName(field.name, name)) { return &field; }
  }
  return nullptr;
}

std::string Quoted(std::string_view text) {
  std::string out = "\"";
  for (const char c : text) {
    if (c == '"' || c == '\\') { out += '\\'; }
    out += c;
  }
  out += '"';
  return out;
}

// A DEPTH COUNTER THAT CANNOT LEAK. It was a `++` at the top and a `--` at the bottom, and one
// early return in the middle -- the `in` operator, which hands off to Membership() -- never reached
// the bottom. Every `in` expression therefore raised the counter permanently, and the guard fired
// on files that were not deep at all: 9 objects refused for a reason that was not true of them.
// Written as a scope, the mistake is not available.
class Deeper {
public:
  explicit Deeper(int &depth) : depth_(depth) {
    if (++depth_ > kMaxDepth) { throw std::runtime_error("an expression nests too deeply"); }
  }

  ~Deeper() { --depth_; }

  Deeper(const Deeper &) = delete;
  Deeper(Deeper &&) = delete;
  Deeper &operator=(const Deeper &) = delete;
  Deeper &operator=(Deeper &&) = delete;

private:
  int &depth_;
};

class Writer {
public:
  explicit Writer(const Names &scope) : scope_(scope) {}

  std::string Statements(const std::vector<al::Stmt> &body, int indent) {
    std::string out;
    for (const al::Stmt &statement : body) { out += Statement(statement, indent); }
    return out;
  }

private:
  static std::string Pad(int indent) {
    std::string pad;
    pad.resize(static_cast<std::size_t>(indent), ' ');
    return pad;
  }

  std::string CaseChain(const al::Stmt &statement, int indent) {
    const std::string subject = Expression(statement.expression, kPrimaryPrecedence);
    std::string out;
    for (const al::Stmt &branch : statement.body) {
      std::string condition;
      for (const al::Expr &label : branch.labels) {
        if (!condition.empty()) { condition += " || "; }
        if (label.kind == al::ExprKind::Range) {
          condition += subject;
          condition += " >= ";
          condition += Expression(label.children.front(), kComparisonPrecedence);
          condition += " && ";
          condition += subject;
          condition += " <= ";
          condition += Expression(label.children.back(), kComparisonPrecedence);
          continue;
        }
        condition += subject;
        condition += " == ";
        condition += Expression(label, kEqualityPrecedence);
      }
      out += out.empty() ? Pad(indent) + "if (" : " else if (";
      out += condition + ") {\n" + Statements(branch.body, indent + 2) + Pad(indent) + "}";
      if (&branch == &statement.body.back() && statement.otherwise.empty()) { out += "\n"; }
    }
    if (!statement.otherwise.empty()) {
      out += out.empty() ? Pad(indent) + "{\n" : " else {\n";
      out += Statements(statement.otherwise, indent + 2) + Pad(indent) + "}\n";
    }
    return out;
  }

  std::string Statement(const al::Stmt &statement, int indent) {
    const Deeper nested(depth_);
    std::string out;
    switch (statement.kind) {
      case al::StmtKind::Block:
        out = Pad(indent) + "{\n" + Statements(statement.body, indent + 2) + Pad(indent) + "}\n";
        break;
      case al::StmtKind::If:
        out = Pad(indent) + "if (" + Expression(statement.expression, 0) + ") {\n" +
              Statements(statement.body, indent + 2) + Pad(indent) + "}\n";
        if (!statement.otherwise.empty()) {
          out.pop_back();
          out += " else {\n" + Statements(statement.otherwise, indent + 2) + Pad(indent) + "}\n";
        }
        break;
      case al::StmtKind::Repeat:
        out = Pad(indent) + "do {\n" + Statements(statement.body, indent + 2) + Pad(indent) +
              "} while (!(" + Expression(statement.expression, 0) + "));\n";
        break;
      case al::StmtKind::While:
        out = Pad(indent) + "while (" + Expression(statement.expression, 0) + ") {\n" +
              Statements(statement.body, indent + 2) + Pad(indent) + "}\n";
        break;
      case al::StmtKind::For: {
        const std::string counter = Expression(statement.expression.children.front(), 0);
        const std::string first = Expression(statement.expression.children.back(), 0);
        const std::string last = Expression(statement.labels.front(), 0);
        out = Pad(indent) + "for (" + counter + " = " + first + "; " + counter +
              (statement.descending ? " >= " : " <= ") + last + "; " +
              (statement.descending ? "--" : "++") + counter + ") {\n" +
              Statements(statement.body, indent + 2) + Pad(indent) + "}\n";
        break;
      }
      case al::StmtKind::ForEach:
        out = Pad(indent) + "for (auto &" + Expression(statement.expression, 0) + " : " +
              Expression(statement.labels.front(), 0) + ") {\n" +
              Statements(statement.body, indent + 2) + Pad(indent) + "}\n";
        break;
      case al::StmtKind::Case: out = CaseChain(statement, indent); break;
      case al::StmtKind::CaseBranch:
        throw std::runtime_error("a case branch stands only inside a case");
      case al::StmtKind::With:
        throw std::runtime_error("AL `with` needs the members it opens to be resolved first");
      // `asserterror <stmt>` is the statement inside a boundary that EXPECTS it to raise: the text
      // is captured where GetLastErrorText reads it, the write set is discarded, and execution
      // carries on. A lambda is what carries the statement in, and it reads as what it is.
      case al::StmtKind::AssertError:
        out = Pad(indent) + "AssertError([&] {\n" + Statements(statement.body, indent + 2) +
              Pad(indent) + "});\n";
        break;
      case al::StmtKind::Exit:
        out = Pad(indent) + "return" +
              (statement.expression.kind == al::ExprKind::Name && statement.expression.text.empty()
                   ? ""
                   : " " + Expression(statement.expression, 0)) +
              ";\n";
        break;
      case al::StmtKind::Expression:
        out = Pad(indent) + Expression(statement.expression, 0) + ";\n";
        break;
    }
    return out;
  }

  std::string Scope(const al::Expr &expression) {
    const al::Expr &base = expression.children.front();
    if (base.kind == al::ExprKind::Name) {
      const std::string enumeration = scope_.Enumeration(base.text);
      if (!enumeration.empty()) { return enumeration + "::" + EnumeratorName(expression.text); }
    }
    return Expression(base, kPrimaryPrecedence) + "::" + EnumeratorName(expression.text);
  }

  std::string Call(const al::Expr &expression) {
    std::string out = Expression(expression.children.front(), kPrimaryPrecedence) + "(";
    for (std::size_t i = 1; i < expression.children.size(); ++i) {
      if (i != 1) { out += ", "; }
      out += Expression(expression.children[i], 0);
    }
    return out + ")";
  }

  std::string Name(const al::Expr &expression) {
    // AL'S BOOLEAN LITERALS ARE NOT IDENTIFIERS, and treating them as one capitalised them:
    // `IsHandled := false` became `IsHandled = False`, which is an unknown name in every body that
    // has one. AL is case-insensitive here and C++ is not.
    if (SameName(expression.text, "true")) { return "true"; }
    if (SameName(expression.text, "false")) { return "false"; }
    const std::string known = scope_.Resolve(expression.text);
    return known.empty() ? Identifier(expression.text) : known;
  }

  std::string Membership(const al::Expr &expression, int outer) {
    const al::Expr &value = expression.children.front();
    const al::Expr &set = expression.children.back();
    const std::string subject = Expression(value, kPrimaryPrecedence);
    std::string out;
    for (const al::Expr &item : set.children) {
      if (!out.empty()) { out += " || "; }
      if (item.kind == al::ExprKind::Range) {
        out += subject;
        out += " >= ";
        out += Expression(item.children.front(), kComparisonPrecedence);
        out += " && ";
        out += subject;
        out += " <= ";
        out += Expression(item.children.back(), kComparisonPrecedence);
        continue;
      }
      out += subject;
      out += " == ";
      out += Expression(item, kEqualityPrecedence);
    }
    if (out.empty()) { out = "false"; }
    if (outer > 1) { out = "(" + out + ")"; }
    return out;
  }

  std::string Binary(const al::Expr &expression, int outer) {
    if (expression.text == "in") { return Membership(expression, outer); }
    // AL'S CONDITIONAL OPERATOR IS C++'S, and it is the one Binary node with three children.
    // Without a case of its own the chain walk below found no chain, left `walk` pointing at this
    // very node, and called Expression on it again -- recursion with no bottom. It showed up as
    // nine objects "nesting too deeply" and, once the guard was lifted to measure it, as a
    // segmentation fault.
    if (expression.text == "?:") { return Conditional(expression, outer); }
    if (expression.children.size() != 2) {
      throw std::runtime_error("a binary operator with " +
                               std::to_string(expression.children.size()) +
                               " operands has no translation");
    }

    const Operator *op = Find(expression.text);
    const int precedence = op != nullptr ? op->precedence : 0;
    const std::string spelling = op != nullptr ? op->cpp : expression.text;

    std::vector<const al::Expr *> chain;
    const al::Expr *walk = &expression;
    while (walk->kind == al::ExprKind::Binary && walk->text == expression.text &&
           walk->children.size() == 2) {
      chain.push_back(&walk->children.back());
      walk = &walk->children.front();
    }
    // A HANDLE IS REACHED THROUGH WITH `->`, AND ONLY ON THE FIRST LINK. `A.B.C` where `A` is a
    // member object is `A->B.C`: what `A` yields is a value like any other.
    const bool handle = spelling == "." && walk->kind == al::ExprKind::Name &&
                        scope_.IsHandle(walk->text);
    std::string out = Expression(*walk, precedence);
    for (std::size_t i = chain.size(); i > 0; --i) {
      if (spelling == ".") {
        out += handle && i == chain.size() ? "->" : ".";
      } else {
        out += " ";
        out += spelling;
        out += " ";
      }
      out += Expression(*chain[i - 1], precedence + 1);
    }
    if (precedence < outer) { out = "(" + out + ")"; }
    return out;
  }

  std::string Conditional(const al::Expr &expression, int outer) {
    std::string out = Expression(expression.children[0], kConditionalPrecedence + 1) + " ? " +
                      Expression(expression.children[1], 0) + " : " +
                      Expression(expression.children[2], kConditionalPrecedence);
    if (kConditionalPrecedence < outer) { out = "(" + out + ")"; }
    return out;
  }

  std::string Expression(const al::Expr &expression, int outer) {
    const Deeper nested(depth_);
    std::string out;
    switch (expression.kind) {
      case al::ExprKind::StringLiteral: out = Quoted(expression.text); break;
      case al::ExprKind::NumberLiteral: out = expression.text; break;
      case al::ExprKind::Name: out = Name(expression); break;
      case al::ExprKind::Scope: out = Scope(expression); break;
      case al::ExprKind::Call: out = Call(expression); break;
      case al::ExprKind::Unary:
        out = (expression.text == "-" ? "-" : "!") +
              Expression(expression.children.front(), kUnaryPrecedence);
        break;
      case al::ExprKind::Set:
      case al::ExprKind::Range:
        throw std::runtime_error("a set literal stands only on the right of `in`");
      // AL COUNTS FROM ONE AND C++ FROM ZERO, and what is being indexed is not known here: an
      // array, a List or a string, depending on a declaration this translator does not resolve. So
      // it writes the call and the OVERLOAD SET decides, which is a compiler's job.
      case al::ExprKind::Index: {
        std::string call = "At(" + Expression(expression.children.front(), 0);
        for (std::size_t i = 1; i < expression.children.size(); ++i) {
          call += ", " + Expression(expression.children[i], 0);
        }
        out = call + ")";
        break;
      }
      // A LEFT-ASSOCIATIVE CHAIN IS EMITTED IN A LOOP, NOT BY RECURSION. `a + b + c + ...` builds a
      // left-deep tree, so walking it down recursively costs one frame per term -- and BC really
      // does split a base64 blob across hundreds of lines joined by `+`. The depth guard below
      // exists to stop runaway recursion on a malformed tree, not to cap a legitimate expression,
      // so the chain is flattened first and the guard keeps its job.
      case al::ExprKind::Binary: out = Binary(expression, outer); break;
    }
    return out;
  }

  const Names &scope_;
  int depth_ = 0;
};

} // namespace

std::string WriteStatements(const Names &scope, const std::vector<al::Stmt> &body, int indent) {
  return Writer(scope).Statements(body, indent);
}

/// A table trigger's scope: the table's own fields, then its labels. AL resolves a bare name in a
/// trigger against the record it belongs to before anything else, which is why `Code` inside
/// `Resource Cost` is the field and not a type.
class TableNames : public Names {
public:
  explicit TableNames(const al::TableObject &table) : table_(table) {}

  [[nodiscard]] bool IsHandle(std::string_view name) const override {
    for (const al::VarDecl &declared : table_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) { return DeclaresAnObject(declared); }
    }
    return false;
  }

  [[nodiscard]] std::string Resolve(std::string_view name) const override {
    for (const al::VarDecl &declared : table_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) {
        return VariableIdentifier(table_, declared.name);
      }
    }
    const al::FieldDecl *field = FieldNamed(table_, name);
    if (field != nullptr) { return Identifier(field->name); }
    for (const al::LabelDecl &label : table_.labels) {
      if (SameName(label.name, name)) { return label.name; }
    }
    for (const al::ProcedureDecl &procedure : table_.procedures) {
      if (SameName(procedure.name, name)) { return ProcedureIdentifier(table_, procedure.name); }
    }
    return {};
  }

  /// \note AN ENUM FIELD SCOPES THROUGH ITS ENUMERATION AND NOT THROUGH ITSELF. AL writes
  ///       `"SEPA Partner Type" = "SEPA Partner Type"::Blank` -- the same words on both sides, the
  ///       field on the left and the enum object on the right. Answering only for inline OPTIONS
  ///       left the right-hand side spelled as the field, which is not a scope and does not
  ///       compile. It is the same question with two answers depending on how the field was
  ///       declared.
  [[nodiscard]] std::string Enumeration(std::string_view name) const override {
    const al::FieldDecl *field = FieldNamed(table_, name);
    if (field == nullptr) { return {}; }
    if (Find(field->properties, "OptionMembers") != nullptr) {
      return OptionEnumName(table_.name, field->name);
    }
    if (TypeName(field->type) == "Enum" && !field->subtype.empty()) {
      return "enums::" + Identifier(field->subtype);
    }
    return {};
  }

private:
  const al::TableObject &table_;
};

std::string WriteSource(const al::TableObject &table,
                        const std::string &sourcePath,
                        const Objects &objects) {
  const std::string identifier = Identifier(table.name);
  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#include \"" + identifier + ".h\"\n\n";
  out += "#include \"agiru.h\"\n\n";
  out += SourceIncludesOf(table.variables, table.procedures, objects);
  out += "\nnamespace agiru::app::tables {\n\n";
  for (const al::FieldDecl &field : table.fields) {
    for (const al::Trigger &trigger : field.triggers) {
      out += "void " + identifier + "::" + trigger.name + Identifier(field.name) + "() {\n";
      out += WriteStatements(TableNames(table), trigger.body, 2);
      out += "}\n\n";
    }
  }
  // A TABLE CARRIES CODE, and its procedures are written the way a codeunit's are.
  for (const al::ProcedureDecl &procedure : table.procedures) {
    out += ProcedureSignature(procedure,
                               objects,
                               table.name,
                               identifier,
                               true,
                               {},
                               table.procedures,
                               ProcedureIdentifier(table, procedure.name)) + " {";
    const std::string locals = ProcedureLocals(procedure, objects, table.name, table.procedures);
    const std::string body = WriteStatements(TableNames(table), procedure.body, 2);
    if (locals.empty() && body.empty()) {
      out += "}\n\n";
      continue;
    }
    out += "\n" + locals;
    if (!locals.empty() && !body.empty()) { out += "\n"; }
    out += body + "}\n\n";
  }

  out += "} // namespace agiru::app::tables\n";
  return out;
}

} // namespace agiru::gen

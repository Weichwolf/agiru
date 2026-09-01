#include "BodyWriter.h"

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

constexpr int kMaxDepth = 128;

struct Operator {
  const char *al;
  const char *cpp;
  int precedence;
};

constexpr int kEqualityPrecedence = 3;
constexpr int kComparisonPrecedence = 4;
constexpr int kUnaryPrecedence = 8;
constexpr int kPrimaryPrecedence = 9;

constexpr std::array kOperators{
    Operator{.al = "or", .cpp = "||", .precedence = 1},
    Operator{.al = "xor", .cpp = "!=", .precedence = 1},
    Operator{.al = "and", .cpp = "&&", .precedence = 2},
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

class Writer {
public:
  explicit Writer(const al::TableObject &table) : table_(table) {}

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
    if (++depth_ > kMaxDepth) { throw std::runtime_error("a statement nests too deeply"); }
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
    --depth_;
    return out;
  }

  [[nodiscard]] std::string OptionEnum(const al::FieldDecl &field) const {
    return OptionEnumName(table_.name, field.name);
  }

  std::string Scope(const al::Expr &expression) {
    const al::Expr &base = expression.children.front();
    if (base.kind == al::ExprKind::Name) {
      const al::FieldDecl *field = FieldNamed(table_, base.text);
      if (field != nullptr) { return OptionEnum(*field) + "::" + EnumeratorName(expression.text); }
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
    const al::FieldDecl *field = FieldNamed(table_, expression.text);
    if (field != nullptr) { return Identifier(field->name); }
    for (const al::LabelDecl &label : table_.labels) {
      if (SameName(label.name, expression.text)) { return label.name; }
    }
    return Identifier(expression.text);
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

  std::string Expression(const al::Expr &expression, int outer) {
    if (++depth_ > kMaxDepth) { throw std::runtime_error("an expression nests too deeply"); }
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
      case al::ExprKind::Index:
        throw std::runtime_error("indexing a string is not translated yet: AL counts from one");
      case al::ExprKind::Binary: {
        if (expression.text == "in") { return Membership(expression, outer); }
        const Operator *op = Find(expression.text);
        const int precedence = op != nullptr ? op->precedence : 0;
        const std::string spelling = op != nullptr ? op->cpp : expression.text;
        const std::string left = Expression(expression.children.front(), precedence);
        const std::string right = Expression(expression.children.back(), precedence + 1);
        out = spelling == "." ? left + "." + right : left + " " + spelling + " " + right;
        if (precedence < outer) { out = "(" + out + ")"; }
        break;
      }
    }
    --depth_;
    return out;
  }

  const al::TableObject &table_;
  int depth_ = 0;
};

} // namespace

std::string
WriteStatements(const al::TableObject &table, const std::vector<al::Stmt> &body, int indent) {
  return Writer(table).Statements(body, indent);
}

std::string WriteSource(const al::TableObject &table, const std::string &sourcePath) {
  const std::string identifier = Identifier(table.name);
  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#include \"" + identifier + ".h\"\n\n";
  out += "#include \"agiru.h\"\n\n";
  out += "namespace agiru::app {\n\n";
  for (const al::FieldDecl &field : table.fields) {
    for (const al::Trigger &trigger : field.triggers) {
      out += "void " + identifier + "::" + trigger.name + Identifier(field.name) + "() {\n";
      out += WriteStatements(table, trigger.body, 2);
      out += "}\n\n";
    }
  }
  out += "} // namespace agiru::app\n";
  return out;
}

} // namespace agiru::gen

#include "BodyWriter.h"

#include "Ast.h"
#include "CodeunitWriter.h"
#include "Door.h"
#include "EnumWriter.h"
#include "Expr.h"
#include "Names.h"
#include "PageWriter.h"
#include "Scope.h"
#include "TableWriter.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::gen {

namespace {

constexpr int kMaxDepth = 4096;

struct Operator {
  const char *al;
  const char *cpp;
  int precedence;
};

constexpr int kConditionalPrecedence = 1;
constexpr int kEqualityPrecedence = 3;
constexpr int kComparisonPrecedence = 4;
constexpr int kUnaryPrecedence = 8;
constexpr int kPrimaryPrecedence = 9;

constexpr std::array kOperators{
    Operator{.al = "or", .cpp = "||", .precedence = 1},
    Operator{.al = "and", .cpp = "&&", .precedence = 2},
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

constexpr std::size_t kDateDigits = 8;
constexpr std::size_t kClockDigits = 6;
constexpr std::size_t kYearDigits = 4;
constexpr std::size_t kPairDigits = 2;
constexpr std::size_t kMilliDigits = 3;

std::string Temporal(const std::string &literal) {
  std::string upper = literal;
  for (char &c : upper) { c = static_cast<char>(std::toupper(static_cast<unsigned char>(c))); }
  std::size_t end = upper.size();
  while (end > 0 && (upper[end - 1] == 'D' || upper[end - 1] == 'T')) { --end; }
  const std::string suffix = upper.substr(end);
  const std::string digits = upper.substr(0, end);
  const bool zero = digits.find_first_not_of("0.") == std::string::npos;
  if (suffix == "DT") {
    return zero ? "DateTime{}" : "RefusedTemporal<DateTime>(\"" + literal + "\")";
  }
  if (suffix == "D") {
    if (zero) { return "Date{}"; }
    if (digits.size() != kDateDigits) { return "RefusedTemporal<Date>(\"" + literal + "\")"; }
    return "Date::FromYmd(" + digits.substr(0, kYearDigits) + ", " +
           digits.substr(kYearDigits, kPairDigits) + ", " +
           digits.substr(kYearDigits + kPairDigits, kPairDigits) + ")";
  }
  if (suffix == "T") {
    if (zero) { return "Time{}"; }
    const std::size_t point = digits.find('.');
    const std::string clock = point == std::string::npos ? digits : digits.substr(0, point);
    if (clock.size() != kClockDigits) { return "RefusedTemporal<Time>(\"" + literal + "\")"; }
    std::string milli = point == std::string::npos ? "0" : digits.substr(point + 1);
    while (milli.size() < kMilliDigits) { milli += '0'; }
    return "Time::FromHms(" + clock.substr(0, kPairDigits) + ", " +
           clock.substr(kPairDigits, kPairDigits) + ", " +
           clock.substr(2 * kPairDigits, kPairDigits) + ", " + milli.substr(0, kMilliDigits) + ")";
  }
  return "RefusedTemporal<Date>(\"" + literal + "\")";
}

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
      case al::StmtKind::AssertError:
        out = Pad(indent) + "AssertError([&] {\n" + Statements(statement.body, indent + 2) +
              Pad(indent) + "});\n";
        break;
      case al::StmtKind::Exit:
        out = Pad(indent) + "return" +
              (statement.expression.kind == al::ExprKind::Name && statement.expression.text.empty()
                   ? scope_.ExitValue()
                   : " " + Expression(statement.expression, 0)) +
              ";\n";
        break;
      case al::StmtKind::Break: out = Pad(indent) + "break;\n"; break;
      case al::StmtKind::Expression:
        out = Pad(indent) + Expression(statement.expression, 0) + ";\n";
        break;
    }
    return out;
  }

  static bool NamesATableNumber(std::string_view base) { return SameName(base, "Database"); }

  static std::string_view KindNamespace(std::string_view base) {
    if (SameName(base, "Codeunit")) { return "codeunits"; }
    if (SameName(base, "Page")) { return "pages"; }
    if (SameName(base, "Table")) { return "tables"; }
    if (SameName(base, "Report")) { return "reports"; }
    if (SameName(base, "Query")) { return "queries"; }
    if (SameName(base, "XmlPort")) { return "xmlports"; }
    if (SameName(base, "Enum")) { return "enums"; }
    if (SameName(base, "Interface")) { return "interfaces"; }
    return {};
  }

  static std::string_view NumberedKind(std::string_view space) {
    if (space == "codeunits" || space == "pages" || space == "reports" || space == "queries" ||
        space == "xmlports") {
      return space;
    }
    return {};
  }

  std::string Scope(const al::Expr &expression) {
    const al::Expr &base = expression.children.front();
    if (base.kind == al::ExprKind::Binary && base.text == "." && base.children.size() == 2 &&
        base.children[0].kind == al::ExprKind::Name &&
        base.children[1].kind == al::ExprKind::Name) {
      const std::string enumeration = scope_.FieldEnumeration(
          OfVariable{.variable = base.children[0].text, .field = base.children[1].text});
      if (!enumeration.empty()) { return enumeration + "::" + EnumeratorName(expression.text); }
      if (scope_.IsRecord(base.children[0].text)) {
        return "RefusedOption(\"" + base.children[0].text + "." + base.children[1].text +
               "::" + expression.text + "\")";
      }
    }
    if (base.kind == al::ExprKind::Name) {
      const std::string enumeration = scope_.Enumeration(base.text);
      if (!enumeration.empty()) { return enumeration + "::" + EnumeratorName(expression.text); }
      const std::string named = scope_.EnumObject(base.text);
      if (!named.empty()) { return named + "::" + EnumeratorName(expression.text); }
      const std::string_view kind =
          scope_.Resolve(base.text).empty() ? KindNamespace(base.text) : std::string_view{};
      if (!kind.empty()) {
        const std::string named = std::string(kind) + "::" + Identifier(expression.text);
        return NumberedKind(kind).empty() ? named : named + "::Id().Value()";
      }
      if (scope_.Resolve(base.text).empty() && IsAlTypeName(base.text)) {
        return "::agiru::" + TypeName(base.text) + "::" + EnumeratorName(expression.text);
      }
      if (scope_.Resolve(base.text).empty() && NamesATableNumber(base.text)) {
        return "tables::" + Identifier(expression.text) + "::kId.Value()";
      }
    }
    const std::string resolved = Expression(base, kPrimaryPrecedence);
    if (resolved.find("::") == std::string::npos) {
      return "RefusedOption(\"" + base.text + "::" + expression.text + "\")";
    }
    return resolved + "::" + EnumeratorName(expression.text);
  }

  static std::size_t FieldArguments(std::string_view method) {
    static constexpr auto kAll = static_cast<std::size_t>(-1);
    static const std::vector<std::pair<std::string_view, std::size_t>> kTakers{
        {"SetRange", 1},
        {"SetFilter", 1},
        {"TestField", 1},
        {"FieldError", 1},
        {"FieldCaption", 1},
        {"FieldName", 1},
        {"FieldNo", 1},
        {"Validate", 1},
        {"SetAscending", 1},
        {"CalcFields", kAll},
        {"CalcSums", kAll},
        {"SetCurrentKey", kAll},
        {"SetLoadFields", kAll},
        {"AddLoadFields", kAll},
    };
    for (const auto &[name, count] : kTakers) {
      if (SameName(name, method)) { return count; }
    }
    return 0;
  }

  std::string Raise(const al::Expr &expression) {
    std::string message = expression.children.size() > 2 ? "StrSubstNo(" : "";
    for (std::size_t i = 1; i < expression.children.size(); ++i) {
      if (i != 1) { message += ", "; }
      message += Expression(expression.children[i], 0);
    }
    if (expression.children.size() > 2) { message += ")"; }
    return "throw Error(" + message + ")";
  }

  std::string RunObject(const al::Expr &expression, const al::Expr &callee) {
    std::string out = Expression(expression.children[1], kPrimaryPrecedence) +
                      "::" + Identifier(callee.children[1].text) + "(";
    for (std::size_t i = 2; i < expression.children.size(); ++i) {
      if (i != 2) { out += ", "; }
      out += Expression(expression.children[i], 0);
    }
    return out + ")";
  }

  std::string Callee(const al::Expr &callee) {
    if (callee.kind == al::ExprKind::Binary) { return Binary(callee, kPrimaryPrecedence, true); }
    if (callee.kind != al::ExprKind::Name) { return Expression(callee, kPrimaryPrecedence); }
    std::string known = scope_.Resolve(callee.text);
    if (!known.empty()) { return known; }
    const std::string_view builtin = BareBuiltin(callee.text);
    return builtin.empty() ? Identifier(callee.text) : std::string(builtin);
  }

  std::string Call(const al::Expr &expression) {
    const al::Expr &callee = expression.children.front();
    if (callee.kind == al::ExprKind::Name && SameName(callee.text, "Error") &&
        scope_.Resolve(callee.text).empty()) {
      return Raise(expression);
    }
    if (callee.kind == al::ExprKind::Binary && callee.text == "." && callee.children.size() == 2 &&
        callee.children[0].kind == al::ExprKind::Name &&
        !KindNamespace(callee.children[0].text).empty() && expression.children.size() > 1 &&
        expression.children[1].kind == al::ExprKind::Scope) {
      return RunObject(expression, callee);
    }
    const std::string spelled = Callee(callee);
    std::string out = spelled + "(";
    std::string receiver;
    std::size_t fields = 0;
    if (callee.kind == al::ExprKind::Binary && callee.text == "." && callee.children.size() == 2 &&
        callee.children[1].kind == al::ExprKind::Name) {
      fields =
          scope_.IsRecord(callee.children[0].text) ? FieldArguments(callee.children[1].text) : 0;
      if (fields != 0) { receiver = Expression(callee.children.front(), kPrimaryPrecedence); }
    }
    for (std::size_t i = 1; i < expression.children.size(); ++i) {
      if (i != 1) { out += ", "; }
      const bool isField =
          !receiver.empty() && (fields == static_cast<std::size_t>(-1) || i <= fields);
      if (isField && expression.children[i].kind == al::ExprKind::Name) {
        out += receiver + "." + Identifier(expression.children[i].text);
        continue;
      }
      out += Expression(expression.children[i], 0);
    }
    return out + ")";
  }

  static std::string_view BareBuiltin(std::string_view name) {
    static constexpr std::array kNoArgument{
        std::string_view{"ApplicationIdentifier"},
        std::string_view{"ApplicationPath"},
        std::string_view{"ClearAll"},
        std::string_view{"ClearCollectedErrors"},
        std::string_view{"ClearLastError"},
        std::string_view{"CodeCoverageLoad"},
        std::string_view{"CodeCoverageRefresh"},
        std::string_view{"Commit"},
        std::string_view{"CompanyName"},
        std::string_view{"CreateEncryptionKey"},
        std::string_view{"CreateGuid"},
        std::string_view{"CurrentClientType"},
        std::string_view{"CurrentDateTime"},
        std::string_view{"CurrentExecutionMode"},
        std::string_view{"DefaultClientType"},
        std::string_view{"DeleteEncryptionKey"},
        std::string_view{"EncryptionEnabled"},
        std::string_view{"EncryptionKeyExists"},
        std::string_view{"GetCurrentModuleExecutionContext"},
        std::string_view{"GetExecutionContext"},
        std::string_view{"GetLastErrorCallStack"},
        std::string_view{"GetLastErrorCode"},
        std::string_view{"GetLastErrorObject"},
        std::string_view{"GetLastErrorText"},
        std::string_view{"GuiAllowed"},
        std::string_view{"HasCollectedErrors"},
        std::string_view{"IsCollectingErrors"},
        std::string_view{"IsInWriteTransaction"},
        std::string_view{"IsServiceTier"},
        std::string_view{"LastUsedRowVersion"},
        std::string_view{"MinimumActiveRowVersion"},
        std::string_view{"SelectLatestVersion"},
        std::string_view{"SerialNumber"},
        std::string_view{"ServiceInstanceId"},
        std::string_view{"SessionId"},
        std::string_view{"TemporaryPath"},
        std::string_view{"TenantId"},
        std::string_view{"Time"},
        std::string_view{"Today"},
        std::string_view{"UserId"},
        std::string_view{"UserSecurityId"},
        std::string_view{"WindowsLanguage"},
    };
    const auto *found = std::ranges::find_if(
        kNoArgument, [name](std::string_view known) { return SameName(known, name); });
    return found == kNoArgument.end() ? std::string_view{} : *found;
  }

  std::string Name(const al::Expr &expression) {
    if (SameName(expression.text, "true")) { return "true"; }
    if (SameName(expression.text, "false")) { return "false"; }
    const std::string known = scope_.Resolve(expression.text);
    if (known.empty()) {
      const std::string_view builtin = BareBuiltin(expression.text);
      if (!builtin.empty()) { return std::string(builtin) + "()"; }
    }
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

  [[nodiscard]] bool YieldsADoorType(const al::Expr &call) const {
    if (call.children.empty()) { return false; }
    const al::Expr &callee = call.children.front();
    if (callee.kind != al::ExprKind::Binary || callee.text != "." || callee.children.size() != 2) {
      return false;
    }
    return callee.children[0].kind == al::ExprKind::Name &&
           scope_.MembersAreCalls(callee.children[0].text);
  }

  bool Calls(std::string_view spelling, const al::Expr &base, const al::Expr &last) {
    if (spelling != ".") { return false; }
    if (base.kind == al::ExprKind::Call) { return YieldsADoorType(base); }
    if (base.kind != al::ExprKind::Name) { return false; }
    if (scope_.MembersAreCalls(base.text)) { return true; }
    return last.kind == al::ExprKind::Name &&
           scope_.MemberIsCall(OfVariable{.variable = base.text, .field = last.text});
  }

  std::string Binary(const al::Expr &expression, int outer, bool asCallee) {
    if (expression.text == "in") { return Membership(expression, outer); }
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
    const bool handle =
        spelling == "." && walk->kind == al::ExprKind::Name && scope_.IsHandle(walk->text);
    const bool calls = Calls(spelling, *walk, *chain.front());
    std::string out = Expression(*walk, precedence);
    for (std::size_t i = chain.size(); i > 0; --i) {
      if (spelling == ".") {
        out += handle && i == chain.size() ? "->" : ".";
      } else {
        out += " ";
        out += spelling;
        out += " ";
      }
      out += spelling == "." && chain[i - 1]->kind == al::ExprKind::Name
                 ? scope_.MemberSpelling(
                       OfVariable{.variable = walk->text, .field = Identifier(chain[i - 1]->text)})
                 : Expression(*chain[i - 1], precedence + 1);
      if (calls && i == chain.size() && !asCallee) { out += "()"; }
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
      case al::ExprKind::TemporalLiteral: out = Temporal(expression.text); break;
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
      case al::ExprKind::Index: {
        std::string call = "At(" + Expression(expression.children.front(), 0);
        for (std::size_t i = 1; i < expression.children.size(); ++i) {
          call += ", " + Expression(expression.children[i], 0);
        }
        out = call + ")";
        break;
      }
      case al::ExprKind::Binary: out = Binary(expression, outer, false); break;
    }
    return out;
  }

  const Names &scope_;
  int depth_ = 0;
};

}

std::string WriteStatements(const Names &scope, const std::vector<al::Stmt> &body, int indent) {
  return Writer(scope).Statements(body, indent);
}

namespace {

std::string NamedEnum(const Objects &objects, std::string_view name) {
  const auto found = objects.enums.find(LowerKey(std::string(name)));
  if (found == objects.enums.end()) { return {}; }
  return "enums::" + Identifier(name);
}

}

class TableNames : public Names {
public:
  TableNames(const al::TableObject &table, const Objects &objects)
      : table_(table), objects_(objects) {}

  [[nodiscard]] std::string EnumObject(std::string_view name) const override {
    return NamedEnum(objects_, name);
  }

  [[nodiscard]] bool IsHandle(std::string_view name) const override {
    for (const al::VarDecl &declared : table_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) {
        return DeclaresAnObject(declared);
      }
    }
    return false;
  }

  [[nodiscard]] std::string Resolve(std::string_view name) const override {
    for (const al::VarDecl &declared : table_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) {
        return "Var_Block->" + VariableIdentifier(table_, declared.name);
      }
    }
    const al::FieldDecl *field = FieldNamed(table_, name);
    if (field != nullptr) { return FieldIdentifier(table_, field->name); }
    for (const al::LabelDecl &label : table_.labels) {
      if (SameName(label.name, name)) { return label.name; }
    }
    for (const al::ProcedureDecl &procedure : table_.procedures) {
      if (SameName(procedure.name, name)) { return ProcedureIdentifier(table_, procedure.name); }
    }
    if (SameName("Rec", name)) { return "(*this)"; }
    return {};
  }

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

  [[nodiscard]] bool IsRecord(std::string_view variable) const override {
    return SameName("Rec", variable) || SameName("xRec", variable);
  }

  [[nodiscard]] bool MemberIsCall(const OfVariable &member) const override {
    return IsRecord(member.variable) && DoorCalls(member.field) &&
           FieldNamed(table_, member.field) == nullptr;
  }

  [[nodiscard]] std::string MemberSpelling(const OfVariable &member) const override {
    if (!IsRecord(member.variable) || FieldNamed(table_, member.field) != nullptr) {
      return std::string(member.field);
    }
    return AsTheDoorSpellsIt(member.field);
  }

  [[nodiscard]] std::string FieldEnumeration(const OfVariable &field) const override {
    if (!IsRecord(field.variable)) { return {}; }
    return Enumeration(field.field);
  }

private:
  const al::TableObject &table_;
  const Objects &objects_;
};

class PageNames : public Names {
public:
  PageNames(const al::PageObject &page, const al::TableObject *source, const Objects &objects)
      : page_(page), source_(source), objects_(objects) {}

  [[nodiscard]] std::string EnumObject(std::string_view name) const override {
    return NamedEnum(objects_, name);
  }

  [[nodiscard]] bool IsHandle(std::string_view name) const override {
    for (const al::VarDecl &declared : page_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) {
        return DeclaresAnObject(declared);
      }
    }
    return false;
  }

  [[nodiscard]] std::string Resolve(std::string_view name) const override {
    for (const al::VarDecl &declared : page_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) { return Identifier(name); }
    }
    for (const al::LabelDecl &label : page_.labels) {
      if (SameName(label.name, name)) { return label.name; }
    }
    for (const al::ProcedureDecl &procedure : page_.procedures) {
      if (SameName(procedure.name, name)) { return Identifier(procedure.name); }
    }
    if (source_ != nullptr) {
      const al::FieldDecl *field = FieldNamed(*source_, name);
      if (field != nullptr) { return "Rec." + FieldIdentifier(*source_, field->name); }
    }
    if (SameName("Rec", name)) { return "Rec"; }
    if (SameName("CurrPage", name)) { return "(*this)"; }
    return {};
  }

  [[nodiscard]] bool IsRecord(std::string_view variable) const override {
    return source_ != nullptr && (SameName("Rec", variable) || SameName("xRec", variable));
  }

  [[nodiscard]] bool MemberIsCall(const OfVariable &member) const override {
    return IsRecord(member.variable) && DoorCalls(member.field) &&
           FieldNamed(*source_, member.field) == nullptr;
  }

  [[nodiscard]] std::string MemberSpelling(const OfVariable &member) const override {
    if (!IsRecord(member.variable) || FieldNamed(*source_, member.field) != nullptr) {
      return std::string(member.field);
    }
    return AsTheDoorSpellsIt(member.field);
  }

  [[nodiscard]] std::string FieldEnumeration(const OfVariable &field) const override {
    if (!IsRecord(field.variable)) { return {}; }
    return Enumeration(field.field);
  }

  [[nodiscard]] std::string Enumeration(std::string_view name) const override {
    if (source_ == nullptr) { return {}; }
    const al::FieldDecl *field = FieldNamed(*source_, name);
    if (field == nullptr) { return {}; }
    if (Find(field->properties, "OptionMembers") != nullptr) {
      return OptionEnumName(source_->name, field->name);
    }
    if (TypeName(field->type) == "Enum" && !field->subtype.empty()) {
      return "enums::" + Identifier(field->subtype);
    }
    return {};
  }

private:
  const al::PageObject &page_;
  const al::TableObject *source_;
  const Objects &objects_;
};

namespace {
std::string BindsBefore(const std::string &body, const std::string &identifier) {
  return body.find("XRec") == std::string::npos
             ? std::string{}
             : "  const " + identifier + " &XRec = detail::Before<" + identifier + ">();\n\n";
}
}

std::string
WriteSource(const al::TableObject &table, const std::string &sourcePath, const Objects &objects) {
  const std::string identifier = Identifier(table.name);
  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#include \"" + identifier + ".h\"\n\n";
  out += kDoorMarker;
  out += "\n";
  out += SourceIncludesOf(table.variables, table.procedures, objects);
  out += "\nnamespace agiru::app::tables {\n\n";
  for (const al::FieldDecl &field : table.fields) {
    for (const al::Trigger &trigger : field.triggers) {
      const std::string body = WriteStatements(TableNames(table, objects), trigger.body, 2);
      out += "void " + identifier + "::" + trigger.name + Identifier(field.name) + "() {\n";
      out += ProcedureLocals(trigger, objects, table.name, table.procedures);
      out += BindsBefore(body, identifier);
      out += body;
      out += "}\n\n";
    }
  }
  for (const al::ProcedureDecl &procedure : table.procedures) {
    out += ProcedureSignature(procedure,
                              objects,
                              table.name,
                              identifier,
                              true,
                              {},
                              table.procedures,
                              ProcedureIdentifier(table, procedure.name)) +
           " {";
    const std::string body = WriteStatements(TableNames(table, objects), procedure.body, 2);
    const std::string locals = ProcedureLocals(procedure, objects, table.name, table.procedures) +
                               BindsBefore(body, identifier);
    if (locals.empty() && body.empty()) {
      out += "}\n\n";
      continue;
    }
    out += "\n" + locals;
    if (!locals.empty() && !body.empty()) { out += "\n"; }
    out += body + "}\n\n";
  }

  out += "namespace {\nconst RegisterTable<" + identifier + "> kInCatalogue;\n} // namespace\n\n";
  out += "} // namespace agiru::app::tables\n";
  return WithDoor(out, ObjectKind::Table);
}

std::string ControlTrigger(std::string_view trigger, std::string_view controlIdentifier) {
  return Identifier(trigger) + std::string(controlIdentifier);
}

namespace {

void ControlBodies(std::string &out,
                   const std::vector<al::PageControl> &controls,
                   const std::string &identifier,
                   const al::PageObject &page,
                   const al::TableObject *source,
                   const Objects &objects,
                   const std::map<std::string, std::string> &named) {
  for (const al::PageControl &control : controls) {
    for (const al::ProcedureDecl &trigger : control.triggers) {
      const std::string name = ControlTrigger(trigger.name, ControlIdentifier(named, control.name));
      const std::string body = WriteStatements(PageNames(page, source, objects), trigger.body, 2);
      const std::string locals =
          ProcedureLocals(trigger, objects, page.name, page.procedures) +
          (source == nullptr ? std::string{}
                             : BindsBefore(body, "tables::" + Identifier(source->name)));
      out += "void ";
      out += identifier;
      out += "::";
      out += name;
      out += "() {";
      if (locals.empty() && body.empty()) {
        out += "}\n\n";
        continue;
      }
      out += "\n" + locals;
      if (!locals.empty() && !body.empty()) { out += "\n"; }
      out += body + "}\n\n";
    }
    ControlBodies(out, control.children, identifier, page, source, objects, named);
  }
}

}

std::string WriteSource(const al::PageObject &page,
                        const std::string &sourcePath,
                        const Objects &objects,
                        const al::TableObject *source) {
  const std::string identifier = Identifier(page.name);
  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#include \"" + identifier + ".h\"\n\n";
  out += kDoorMarker;
  out += "\n";
  out += SourceIncludesOf(page.variables, page.procedures, objects);
  out += "\nnamespace agiru::app::pages {\n\n";
  const std::map<std::string, std::string> named = ControlIdentifiers(page);
  ControlBodies(out, page.layout, identifier, page, source, objects, named);
  ControlBodies(out, page.actions, identifier, page, source, objects, named);
  for (const al::ProcedureDecl &procedure : page.procedures) {
    out += ProcedureSignature(procedure,
                              objects,
                              page.name,
                              identifier,
                              true,
                              {},
                              page.procedures,
                              Identifier(procedure.name)) +
           " {";
    const std::string body = WriteStatements(PageNames(page, source, objects), procedure.body, 2) +
                             FallsOffEnd(procedure, PageNames(page, source, objects));
    const std::string locals =
        ProcedureLocals(procedure, objects, page.name, page.procedures) +
        (source == nullptr ? std::string{}
                           : BindsBefore(body, "tables::" + Identifier(source->name)));
    if (locals.empty() && body.empty()) {
      out += "}\n\n";
      continue;
    }
    out += "\n" + locals;
    if (!locals.empty() && !body.empty()) { out += "\n"; }
    out += body + "}\n\n";
  }
  out += "} // namespace agiru::app::pages\n";
  return WithDoor(out, ObjectKind::Page);
}

}

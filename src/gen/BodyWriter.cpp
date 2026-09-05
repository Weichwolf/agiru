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
#include <cstdint>
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

constexpr int kEqualityPrecedence = 3;
constexpr int kComparisonPrecedence = 4;
constexpr int kUnaryPrecedence = 8;

constexpr int kAdditivePrecedence = 5;
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
    return zero ? "::agiru::DateTime{}"
                : "::agiru::RefusedTemporal<::agiru::DateTime>(\"" + literal + "\")";
  }
  if (suffix == "D") {
    if (zero) { return "::agiru::Date{}"; }
    if (digits.size() != kDateDigits) {
      return "::agiru::RefusedTemporal<::agiru::Date>(\"" + literal + "\")";
    }
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
          condition += "(";
          condition += subject;
          condition += " >= ";
          condition += Expression(label.children.front(), kComparisonPrecedence);
          condition += " && ";
          condition += subject;
          condition += " <= ";
          condition += Expression(label.children.back(), kComparisonPrecedence);
          condition += ")";
          continue;
        }
        condition += subject;
        condition += " == ";
        condition += Expression(label, kEqualityPrecedence + 1);
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
        out = Pad(indent) + "for ([[maybe_unused]] auto &" + Expression(statement.expression, 0) +
              " : " + Expression(statement.labels.front(), 0) + ") {\n" +
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
      case al::StmtKind::Expression: {
        const bool was = discarded_;
        discarded_ = true;
        out = Pad(indent) + Expression(statement.expression, 0) + ";\n";
        discarded_ = was;
        break;
      }
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

  std::string AsOption(const std::string &enumeration, std::string_view member) const {
    const std::string member_ = scope_.EnumMember(enumeration, member);
    if (!enumeration.starts_with("::agiru::")) {
      const bool isEnum = enumeration.starts_with("enums::") || enumeration.contains("::enums::");
      return std::string(isEnum ? "::agiru::Enum<" : "::agiru::Option<") + enumeration + ">{" +
             enumeration + "::" + member_ + "}";
    }
    return "::agiru::Option<" + enumeration + ">{" + enumeration +
           "::" + AsTheDoorSpellsIt(member_) + "}";
  }

  static const al::Expr &Indexed(const al::Expr &expression) {
    return expression.kind == al::ExprKind::Index && !expression.children.empty()
               ? expression.children.front()
               : expression;
  }

  std::string Kinded(std::string_view kind, const std::string &name) {
    if (kind == "enums" && scope_.EnumObject(name).empty()) {
      return "RefusedOption(\"" + name + "\")";
    }
    const std::string named = scope_.ObjectNamed(kind, name);
    if (named.starts_with("absent::") && !NumberedKind(kind).empty()) {
      return "::agiru::AbsentObjectId(\"" + name + "\")";
    }
    return NumberedKind(kind).empty() ? named : named + "::Id().Value()";
  }

  std::string Scope(const al::Expr &expression) {
    const al::Expr &base = expression.children.front();
    if (base.kind == al::ExprKind::Binary && base.text == "." && base.children.size() == 2 &&
        Indexed(base.children[0]).kind == al::ExprKind::Name &&
        base.children[1].kind == al::ExprKind::Name) {
      const std::string enumeration = scope_.FieldEnumeration(
          OfVariable{.variable = Indexed(base.children[0]).text, .field = base.children[1].text});
      if (!enumeration.empty()) { return AsOption(enumeration, expression.text); }
      if (scope_.IsRecord(Indexed(base.children[0]).text)) {
        return "RefusedOption(\"" + Indexed(base.children[0]).text + "." + base.children[1].text +
               "::" + expression.text + "\")";
      }
    }
    if (base.kind == al::ExprKind::Name) {
      const std::string enumeration = scope_.Enumeration(base.text);
      if (!enumeration.empty()) { return AsOption(enumeration, expression.text); }
      const std::string named = scope_.EnumObject(base.text);
      if (!named.empty()) { return AsOption(named, expression.text); }
      const std::string_view kind =
          scope_.Resolve(base.text).empty() ? KindNamespace(base.text) : std::string_view{};
      if (!kind.empty()) { return Kinded(kind, expression.text); }
      if (scope_.Resolve(base.text).empty() && IsAlTypeName(base.text)) {
        return "::agiru::" + TypeName(base.text) +
               "::" + AsTheDoorSpellsIt(EnumeratorName(expression.text));
      }
      if (scope_.Resolve(base.text).empty()) {
        static constexpr std::array<std::pair<std::string_view, std::string_view>, 3>
            kMethodOptions{{{"securityfiltering", "SecurityFilter"},
                            {"readisolation", "IsolationLevel"},
                            {"currenttransactiontype", "TransactionType"}}};
        const std::string lowered = LowerKey(base.text);
        for (const auto &[method, option] : kMethodOptions) {
          if (lowered == method) {
            return "::agiru::" + std::string(option) +
                   "::" + AsTheDoorSpellsIt(EnumeratorName(expression.text));
          }
        }
      }
      if (scope_.Resolve(base.text).empty() && NamesATableNumber(base.text)) {
        const std::string table = scope_.ObjectNamed("tables", expression.text);
        if (table.starts_with("absent::")) {
          return "::agiru::AbsentObjectId(\"" + expression.text + "\")";
        }
        return "tables::" + Identifier(expression.text) + "::kId.Value()";
      }
    }
    const std::string resolved = Expression(base, kPrimaryPrecedence);
    if (resolved.find("::") == std::string::npos) {
      return "RefusedOption(\"" + base.text + "::" + expression.text + "\")";
    }
    if (resolved.starts_with("::agiru::")) {
      return resolved + "::" + AsTheDoorSpellsIt(EnumeratorName(expression.text));
    }
    return resolved + "::" + EnumeratorName(expression.text);
  }

  static std::size_t FieldArguments(std::string_view method) {
    static constexpr auto kAll = static_cast<std::size_t>(-1);
    static const std::vector<std::pair<std::string_view, std::size_t>> kTakers{
        {"SetRange", 1},           {"SetFilter", 1},
        {"FindFirstField", 1},     {"FindNextField", 1},
        {"FindPreviousField", 1},  {"TestField", 1},
        {"FieldError", 1},         {"FieldCaption", 1},
        {"FieldName", 1},          {"FieldNo", 1},
        {"Validate", 1},           {"SetAscending", 1},
        {"CalcFields", kAll},      {"CalcSums", kAll},
        {"SetCurrentKey", kAll},   {"SetLoadFields", kAll},
        {"AddLoadFields", kAll},   {"GetRangeMin", 1},
        {"GetRangeMax", 1},        {"GetFilter", 1},
        {"GetAscending", 1},       {"CopyFilter", kAll},
        {"FieldActive", 1},        {"ModifyAll", 1},
        {"Relation", 1},           {"SetAutoCalcFields", kAll},
        {"AreFieldsLoaded", kAll},
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
    const al::Expr &named = expression.children[1];
    std::string subject = Expression(named, kPrimaryPrecedence);
    if (named.kind == al::ExprKind::Scope && !named.children.empty() &&
        named.children.front().kind == al::ExprKind::Name) {
      const std::string_view kind = KindNamespace(named.children.front().text);
      if (!kind.empty()) { subject = scope_.ObjectNamed(kind, named.text) + "{}"; }
    }
    const std::string member = Identifier(callee.children[1].text);
    std::string out =
        subject + "." + (DoorCalls(member) ? AsTheDoorSpellsIt(member) : member) + "(";
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
    if (!known.empty() && !scope_.IsVariable(callee.text) && !scope_.ThisTable().empty() &&
        scope_.HasField(OfVariable{.variable = "Rec", .field = callee.text}) &&
        HiddenByABaseMember(callee.text)) {
      return "this->::agiru::Table<::agiru::app::tables::" + scope_.ThisTable() +
             ">::" + AsTheDoorSpellsIt(Identifier(callee.text));
    }
    if (!known.empty() && !scope_.IsVariable(callee.text) && scope_.IsRecord("Rec") &&
        scope_.HasField(OfVariable{.variable = "Rec", .field = callee.text}) &&
        !IsAlTypeName(callee.text) && DoorCalls(callee.text)) {
      return "::agiru::" + AsTheDoorSpellsIt(Identifier(callee.text));
    }
    if (!known.empty() && scope_.IsVariable(callee.text) && DoorCalls(callee.text)) {
      const std::string rec = scope_.Resolve("Rec");
      if (!rec.empty()) { return rec + "." + AsTheDoorSpellsIt(Identifier(callee.text)); }
    }
    if (!known.empty()) { return known; }
    const std::string_view builtin = BareBuiltin(callee.text);
    return builtin.empty() ? AsTheDoorSpellsIt(Identifier(callee.text)) : std::string(builtin);
  }

  std::string Tried(const al::Expr &expression) {
    const al::Expr &callee = expression.children.front();
    if (discarded_ || callee.kind != al::ExprKind::Name || !scope_.IsTryFunction(callee.text)) {
      return {};
    }
    const bool was = discarded_;
    discarded_ = true;
    const std::string inner = Call(expression);
    discarded_ = was;
    return "::agiru::Tried([&] { return " + inner + "; })";
  }

  std::string Call(const al::Expr &expression) {
    const al::Expr &callee = expression.children.front();
    if (const std::string tried = Tried(expression); !tried.empty()) { return tried; }
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
    std::string reach = ".";
    std::size_t fields = 0;
    const al::Expr *holder = nullptr;
    if (callee.kind == al::ExprKind::Binary && callee.text == "." && callee.children.size() == 2 &&
        callee.children[1].kind == al::ExprKind::Name) {
      fields = FieldArguments(callee.children[1].text);
      if (fields != 0) {
        const al::Expr *owner = &callee.children.front();
        if (owner->kind == al::ExprKind::Binary && owner->text == "." &&
            owner->children.size() == 2 && owner->children[1].kind == al::ExprKind::Name &&
            DoorDeclares(owner->children[1].text) && !DoorCalls(owner->children[1].text)) {
          owner = &owner->children.front();
        }
        receiver = Expression(*owner, kPrimaryPrecedence);
        reach = owner->kind == al::ExprKind::Name && scope_.IsHandle(owner->text) ? "->" : ".";
        holder = owner;
      }
    }
    const std::vector<bool> publisherVars = callee.kind == al::ExprKind::Name
                                                ? scope_.VarParametersOfPublisher(callee.text)
                                                : std::vector<bool>{};
    for (std::size_t i = 1; i < expression.children.size(); ++i) {
      if (i != 1) { out += ", "; }
      const al::Expr &argument = expression.children[i];
      const bool named =
          argument.kind == al::ExprKind::Name &&
          (!scope_.Resolve(argument.text).empty() || IsSystemFieldName(argument.text) ||
           SameName(argument.text, "Rec") || SameName(argument.text, "xRec"));
      const bool lvalue = named || argument.kind == al::ExprKind::Index ||
                          (argument.kind == al::ExprKind::Binary && argument.text == ".");
      if (i - 1 < publisherVars.size() && publisherVars[i - 1] && !lvalue) {
        out += "::agiru::Materialised(" + Expression(argument, 0) + ")";
        continue;
      }
      const bool isField =
          !receiver.empty() && (fields == static_cast<std::size_t>(-1) || i <= fields);
      if (isField && expression.children[i].kind == al::ExprKind::Name &&
          (scope_.Resolve(expression.children[i].text).empty() ||
           (holder != nullptr && holder->kind == al::ExprKind::Name &&
            scope_.HasField(
                OfVariable{.variable = holder->text, .field = expression.children[i].text})))) {
        out += receiver + reach;
        out += holder != nullptr && holder->kind == al::ExprKind::Name
                   ? scope_.MemberSpelling(
                         OfVariable{.variable = holder->text, .field = expression.children[i].text})
                   : Identifier(expression.children[i].text);
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
        std::string_view{"CurrFieldNo"},
        std::string_view{"CurrentClientType"},
        std::string_view{"CurrentDateTime"},
        std::string_view{"CurrentExecutionMode"},
        std::string_view{"DefaultClientType"},
        std::string_view{"DeleteEncryptionKey"},
        std::string_view{"EncryptionEnabled"},
        std::string_view{"GlobalLanguage"},
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
    if (SameName(expression.text, "this")) { return "(*this)"; }
    if (SameName(expression.text, "true")) { return "true"; }
    if (SameName(expression.text, "false")) { return "false"; }
    const std::string known = scope_.Resolve(expression.text);
    if (known.empty()) {
      const std::string_view builtin = BareBuiltin(expression.text);
      if (!builtin.empty()) { return std::string(builtin) + "()"; }
      if (IsSystemFieldName(expression.text)) { return Identifier(expression.text); }
      if (scope_.MemberIsCall(OfVariable{.variable = "Rec", .field = expression.text})) {
        if (!scope_.ThisTable().empty() && HiddenByABaseMember(expression.text) &&
            scope_.HasField(OfVariable{.variable = "Rec", .field = expression.text})) {
          return "this->::agiru::Table<::agiru::app::tables::" + scope_.ThisTable() +
                 ">::" + AsTheDoorSpellsIt(Identifier(expression.text)) + "()";
        }
        return Identifier(expression.text) + "()";
      }
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
        out += "(";
        out += subject;
        out += " >= ";
        out += Expression(item.children.front(), kComparisonPrecedence);
        out += " && ";
        out += subject;
        out += " <= ";
        out += Expression(item.children.back(), kComparisonPrecedence);
        out += ")";
        continue;
      }
      out += subject;
      out += " == ";
      out += Expression(item, kEqualityPrecedence + 1);
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

  enum class Parens : std::uint8_t { None, First, Last };

  Parens Calls(std::string_view spelling, const al::Expr &base, const al::Expr &last) {
    if (spelling != ".") { return Parens::None; }
    if (base.kind == al::ExprKind::Call) {
      const al::Expr &callee = base.children.front();
      if (callee.kind == al::ExprKind::Name && SameName(callee.text, "At") &&
          base.children.size() > 1 && base.children[1].kind == al::ExprKind::Name &&
          scope_.MembersAreCalls(base.children[1].text)) {
        return Parens::First;
      }
      if (YieldsADoorType(base)) { return Parens::First; }
      if (last.kind == al::ExprKind::Name && DoorCalls(last.text)) { return Parens::Last; }
      return Parens::None;
    }
    if (base.kind == al::ExprKind::Index && !base.children.empty() &&
        base.children.front().kind == al::ExprKind::Name &&
        scope_.MembersAreCalls(base.children.front().text)) {
      return Parens::First;
    }
    if (base.kind != al::ExprKind::Name) { return Parens::None; }
    if (scope_.MembersAreCalls(base.text)) { return Parens::First; }
    if (last.kind == al::ExprKind::Name &&
        scope_.MemberIsCall(OfVariable{.variable = base.text, .field = last.text})) {
      return Parens::Last;
    }
    return Parens::None;
  }

  std::string PropertyAssignment(const al::Expr &expression) {
    if (expression.text != ":=" || expression.children.size() != 2) { return {}; }
    const al::Expr &target = expression.children.front();
    if (target.kind == al::ExprKind::Name && scope_.Resolve(target.text).empty() &&
        DoorCalls(target.text) && !IsSystemFieldName(target.text)) {
      return AsTheDoorSpellsIt(Identifier(target.text)) + "(" +
             Expression(expression.children.back(), 0) + ")";
    }
    if (target.kind == al::ExprKind::Name && !scope_.Resolve(target.text).empty() &&
        !scope_.IsVariable(target.text) &&
        !scope_.HasField(OfVariable{.variable = "Rec", .field = target.text}) &&
        !scope_.ProcedureOf(OfVariable{.variable = "Rec", .field = target.text}).empty()) {
      return scope_.Resolve(target.text) + "(" + Expression(expression.children.back(), 0) + ")";
    }
    if (target.kind != al::ExprKind::Binary || target.text != "." || target.children.size() != 2 ||
        target.children[0].kind != al::ExprKind::Name ||
        target.children[1].kind != al::ExprKind::Name) {
      return {};
    }
    if (!scope_.MemberIsCall(
            OfVariable{.variable = target.children[0].text, .field = target.children[1].text})) {
      return {};
    }
    return Binary(target, kPrimaryPrecedence, true) + "(" +
           Expression(expression.children.back(), 0) + ")";
  }

  struct Reach {
    std::string_view spelling;
    const al::Expr &base;
    const al::Expr &link;
  };

  struct How {
    bool arrow;
    bool parens;
    int precedence;
    bool callee = false;
  };

  void Link(std::string &out, const Reach &reach, const How &how) {
    if (reach.spelling == ".") {
      out += how.arrow ? "->" : ".";
    } else {
      out += " ";
      out += reach.spelling;
      out += " ";
    }
    const bool andUnderOr = reach.spelling == "||" && reach.link.kind == al::ExprKind::Binary &&
                            reach.link.text == "and";
    const OfVariable member{.variable = reach.base.text, .field = reach.link.text};
    const bool calledBesideAField =
        how.callee && reach.spelling == "." && reach.link.kind == al::ExprKind::Name &&
        reach.base.kind == al::ExprKind::Name && scope_.HasField(member);
    std::string besideAField;
    if (calledBesideAField) {
      besideAField = scope_.ProcedureOf(member);
      if (besideAField.empty() && HiddenByABaseMember(reach.link.text)) {
        const std::string table = scope_.TableOf(reach.base.text);
        if (!table.empty()) {
          besideAField = "::agiru::Table<::agiru::app::tables::" + table +
                         ">::" + AsTheDoorSpellsIt(Identifier(reach.link.text));
        }
      }
    }
    out += !besideAField.empty() ? besideAField
           : reach.spelling == "." && reach.link.kind == al::ExprKind::Name
               ? scope_.MemberSpelling(member)
           : andUnderOr ? "(" + Expression(reach.link, how.precedence + 1) + ")"
                        : Expression(reach.link, how.precedence + 1);
    if (how.parens && !IsSystemFieldName(reach.link.text)) { out += "()"; }
  }

  static std::string Number(std::string_view text) {
    if (text.find('.') == std::string_view::npos && text.find('e') == std::string_view::npos &&
        text.find('E') == std::string_view::npos) {
      return std::string(text);
    }
    return "::agiru::Decimal::FromInvariantString(\"" + std::string(text) + "\")";
  }

  std::string Added(const al::Expr &expression, int precedence) {
    const std::string rendered = Expression(expression, precedence);
    const bool wrap = expression.kind == al::ExprKind::StringLiteral ||
                      (expression.kind == al::ExprKind::Name && scope_.IsLabel(expression.text));
    return wrap ? "std::string(" + rendered + ")" : rendered;
  }

  [[nodiscard]] bool IsText(const al::Expr &expression) const {
    if (expression.kind == al::ExprKind::StringLiteral) { return true; }
    if (expression.kind == al::ExprKind::Name) { return scope_.IsLabel(expression.text); }
    return expression.kind == al::ExprKind::Binary && expression.text == "+" &&
           (IsText(expression.children.front()) || IsText(expression.children.back()));
  }

  static bool IsSystemFieldName(std::string_view name) {
    static constexpr std::array kSystem{std::string_view{"SystemId"},
                                        std::string_view{"SystemCreatedAt"},
                                        std::string_view{"SystemCreatedBy"},
                                        std::string_view{"SystemModifiedAt"},
                                        std::string_view{"SystemModifiedBy"},
                                        std::string_view{"SystemRowVersion"}};
    return std::ranges::any_of(kSystem,
                               [name](std::string_view known) { return SameName(known, name); });
  }

  static bool IsEnumMethod(std::string_view name) {
    static constexpr std::array kMethods{std::string_view{"FromInteger"},
                                         std::string_view{"Names"},
                                         std::string_view{"Ordinals"},
                                         std::string_view{"AsInteger"}};
    return std::ranges::any_of(kMethods,
                               [name](std::string_view known) { return SameName(known, name); });
  }

  static bool IsBuiltinFamily(std::string_view name) {
    static constexpr std::array kFamilies{std::string_view{"System"},
                                          std::string_view{"Text"},
                                          std::string_view{"Database"},
                                          std::string_view{"Session"},
                                          std::string_view{"Dialog"},
                                          std::string_view{"File"},
                                          std::string_view{"SecretText"}};
    return std::ranges::any_of(kFamilies,
                               [name](std::string_view known) { return SameName(known, name); });
  }

  std::string TypeStatic(const al::Expr &walk,
                         const std::vector<const al::Expr *> &chain,
                         std::string_view spelling,
                         int precedence) {
    if (spelling != "." || chain.empty() || chain.back()->kind != al::ExprKind::Name) { return {}; }
    if (walk.kind == al::ExprKind::Name && IsEnumMethod(chain.back()->text) &&
        scope_.Resolve(walk.text).empty()) {
      const std::string named = scope_.EnumObject(walk.text);
      if (!named.empty()) {
        return "::agiru::Enum<" + named + ">::" + Identifier(chain.back()->text);
      }
    }
    if (walk.kind == al::ExprKind::Scope && IsEnumMethod(chain.back()->text)) {
      const std::string named = Expression(walk, kPrimaryPrecedence);
      if (!named.starts_with("enums::")) { return {}; }
      const std::size_t member = named.find("::", std::string_view{"enums::"}.size());
      if (member == std::string::npos) {
        return "Enum<" + named + ">::" + Identifier(chain.back()->text);
      }
      return "::agiru::Enum<" + named.substr(0, member) + ">{" + named + "}." +
             Identifier(chain.back()->text);
    }
    if (walk.kind != al::ExprKind::Name || !scope_.Resolve(walk.text).empty() ||
        !DoorCalls(chain.back()->text)) {
      return {};
    }
    if (IsBuiltinFamily(walk.text)) {
      return chain.size() == 1 ? AsTheDoorSpellsIt(Identifier(chain.back()->text)) : std::string{};
    }
    if (!IsAlTypeName(walk.text)) { return {}; }
    const std::string holder = KindNamespace(walk.text).empty() ? "" : "<>";
    std::string out = "::agiru::" + TypeName(walk.text) + holder +
                      "::" + AsTheDoorSpellsIt(Identifier(chain.back()->text));
    for (std::size_t i = chain.size() - 1; i > 0; --i) {
      Link(out,
           {.spelling = spelling, .base = walk, .link = *chain[i - 1]},
           {.arrow = false, .parens = false, .precedence = precedence});
    }
    return out;
  }

  std::string Binary(const al::Expr &expression, int outer, bool asCallee) {
    if (expression.text == "in") { return Membership(expression, outer); }
    if (expression.text == "?:") { return Conditional(expression, outer); }
    if (expression.children.size() != 2) {
      throw std::runtime_error("a binary operator with " +
                               std::to_string(expression.children.size()) +
                               " operands has no translation");
    }

    if (const std::string assigned = PropertyAssignment(expression); !assigned.empty()) {
      return assigned;
    }

    if (expression.text == ":=") {
      const std::string left = Expression(expression.children.front(), kPrimaryPrecedence);
      if (left == Expression(expression.children.back(), kPrimaryPrecedence)) {
        return "static_cast<void>(" + left + ")";
      }
    }

    if (expression.text == "+" &&
        (IsText(expression.children.front()) || IsText(expression.children.back()))) {
      return Added(expression.children.front(), kAdditivePrecedence) + " + " +
             Added(expression.children.back(), kAdditivePrecedence + 1);
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
    if (const std::string reached = TypeStatic(*walk, chain, spelling, precedence);
        !reached.empty()) {
      return reached;
    }

    const bool handle =
        spelling == "." && ((walk->kind == al::ExprKind::Name && scope_.IsHandle(walk->text)) ||
                            (walk->kind == al::ExprKind::Call && !walk->children.empty() &&
                             walk->children.front().kind == al::ExprKind::Name &&
                             scope_.ReturnsAHandle(walk->children.front().text)));
    const Parens calls = Calls(spelling, *walk, *chain.front());
    std::string out = Expression(*walk, precedence);
    if (spelling == "||" && walk->kind == al::ExprKind::Binary && walk->text == "and") {
      out = "(" + out + ")";
    }
    for (std::size_t i = chain.size(); i > 0; --i) {
      Link(out,
           {.spelling = spelling, .base = *walk, .link = *chain[i - 1]},
           {.arrow = handle && i == chain.size(),
            .parens = calls != Parens::None && (calls == Parens::First || i == 1) &&
                      !(asCallee && i == 1),
            .precedence = precedence,
            .callee = asCallee && i == 1});
    }
    if (precedence < outer) { out = "(" + out + ")"; }
    return out;
  }

  std::string Conditional(const al::Expr &expression, int outer) {
    static_cast<void>(outer);
    const std::string whenTrue = Expression(expression.children[1], 0);
    return "[&]() -> decltype(" + whenTrue + ") { if (" + Expression(expression.children[0], 0) +
           ") { return " + whenTrue + "; } return " + Expression(expression.children[2], 0) +
           "; }()";
  }

  std::string Expression(const al::Expr &expression, int outer) {
    const Deeper nested(depth_);
    std::string out;
    switch (expression.kind) {
      case al::ExprKind::StringLiteral: out = Quoted(expression.text); break;
      case al::ExprKind::NumberLiteral: out = Number(expression.text); break;
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
  bool discarded_ = false;
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
  TableNames(const al::TableObject &table,
             const Objects &objects,
             const al::ProcedureDecl *running = nullptr)
      : table_(table), objects_(objects), running_(running) {}

  [[nodiscard]] bool ShadowedByALocal(const std::string &identifier) const {
    if (running_ == nullptr) { return false; }
    for (const std::vector<al::VarDecl> *group : {&running_->variables, &running_->parameters}) {
      for (const al::VarDecl &declared : *group) {
        if (Identifier(declared.name) == identifier) { return true; }
      }
    }
    return !running_->returnName.empty() && Identifier(running_->returnName) == identifier;
  }

  [[nodiscard]] const al::VarDecl *Local(std::string_view name) const {
    if (running_ == nullptr) { return nullptr; }
    for (const al::VarDecl &declared : running_->variables) {
      if (SameName(declared.name, name)) { return &declared; }
    }
    for (const al::VarDecl &declared : running_->parameters) {
      if (SameName(declared.name, name)) { return &declared; }
    }
    if (!running_->returnName.empty() && SameName(running_->returnName, name)) {
      return &running_->returned;
    }
    return nullptr;
  }

  [[nodiscard]] std::string ExitValue() const override {
    if (running_ == nullptr) { return {}; }
    if (!running_->returnName.empty()) { return " " + Identifier(running_->returnName); }
    return running_->returnType.empty() ? std::string{} : std::string(" {}");
  }

  [[nodiscard]] bool HasField(const OfVariable &member) const override {
    if (IsRecord(member.variable)) {
      if (FieldNamed(table_, member.field) != nullptr) { return true; }
      const std::string spelled = LowerKey(Identifier(member.field));
      return std::ranges::any_of(table_.fields, [&](const al::FieldDecl &field) {
        return LowerKey(Identifier(field.name)) == spelled;
      });
    }
    const auto *fields = FieldsOf(member.variable);
    if (fields == nullptr) { return false; }
    if (fields->contains(LowerKey(std::string(member.field)))) { return true; }
    const std::string spelled = LowerKey(Identifier(member.field));
    return std::ranges::any_of(
        *fields, [&](const auto &field) { return LowerKey(Identifier(field.second)) == spelled; });
  }

  [[nodiscard]] bool IsVariable(std::string_view name) const override {
    return Local(name) != nullptr || Global(name) != nullptr;
  }

  [[nodiscard]] std::string ThisTable() const override { return Identifier(table_.name); }

  [[nodiscard]] std::string ProcedureOf(const OfVariable &member) const override {
    if (IsRecord(member.variable)) {
      for (const al::ProcedureDecl &procedure : table_.procedures) {
        if (SameName(procedure.name, member.field)) {
          return ProcedureIdentifier(table_, procedure.name);
        }
      }
      return {};
    }
    for (const al::VarDecl *where : {Local(member.variable), Global(member.variable)}) {
      if (where == nullptr || TypeName(where->type) != "Record") { continue; }
      const auto table = objects_.tables.find(LowerKey(where->subtype));
      if (table == objects_.tables.end()) { break; }
      const auto found = table->second.procedures.find(LowerKey(std::string(member.field)));
      return found == table->second.procedures.end() ? std::string{} : found->second;
    }
    return {};
  }

  [[nodiscard]] std::string TableOf(std::string_view variable) const override {
    if (IsRecord(variable)) { return Identifier(table_.name); }
    for (const al::VarDecl *where : {Local(variable), Global(variable)}) {
      if (where == nullptr || TypeName(where->type) != "Record") { continue; }
      const auto table = objects_.tables.find(LowerKey(where->subtype));
      if (table == objects_.tables.end()) { break; }
      const std::string &id = table->second.identifier;
      return id.starts_with("tables::") ? id.substr(8) : std::string{};
    }
    return {};
  }

  [[nodiscard]] std::string EnumMember(std::string_view enumeration,
                                       std::string_view member) const override {
    return DeclaredEnumMember(objects_, enumeration, member);
  }

  [[nodiscard]] std::vector<bool> VarParametersOfPublisher(std::string_view name) const override {
    for (const al::ProcedureDecl &procedure : table_.procedures) {
      if (!SameName(procedure.name, name) || !IsPublisher(procedure)) { continue; }
      std::vector<bool> vars;
      for (const al::VarDecl &parameter : procedure.parameters) {
        vars.push_back(parameter.byReference);
      }
      return vars;
    }
    return {};
  }

  [[nodiscard]] bool MembersAreCalls(std::string_view variable) const override {
    const al::VarDecl *declared = Local(variable);
    if (declared == nullptr) { declared = Global(variable); }
    if (declared == nullptr) { return false; }
    const std::string type = TypeName(declared->type);
    return IsAlTypeName(type) && type != "Option" && type != "Enum" && declared->subtype.empty();
  }

  [[nodiscard]] const std::map<std::string, std::string> *
  FieldsOf(std::string_view variable) const {
    if (IsRecord(variable)) { return nullptr; }
    const al::VarDecl *local = Local(variable);
    if (local == nullptr) { local = Global(variable); }
    if (local == nullptr || TypeName(local->type) != "Record" || local->subtype.empty()) {
      return nullptr;
    }
    const auto found = objects_.tables.find(LowerKey(local->subtype));
    return found == objects_.tables.end() || found->second.fields.empty() ? nullptr
                                                                          : &found->second.fields;
  }

  [[nodiscard]] bool MemberIsCall(const OfVariable &member) const override {
    const al::VarDecl *local = Local(member.variable);
    if (local != nullptr && !DeclaresAnObject(*local)) { return DoorCalls(member.field); }
    if (local == nullptr) {
      if (const al::VarDecl *global = Global(member.variable);
          global != nullptr && TypeName(global->type) == "Record" && !global->subtype.empty()) {
        const auto found = objects_.tables.find(LowerKey(global->subtype));
        const bool field =
            found != objects_.tables.end() &&
            (found->second.fields.contains(LowerKey(std::string(member.field))) ||
             (found->second.fields.empty() &&
              PlatformFieldNamed(PlatformField{.table = global->subtype, .field = member.field})));
        return !field && DoorCalls(member.field);
      }
    }
    if (local != nullptr &&
        (TypeName(local->type) == "Page" || TypeName(local->type) == "TestPage" ||
         TypeName(local->type) == "TestRequestPage")) {
      const auto page = objects_.pages.find(LowerKey(local->subtype));
      const bool control = page != objects_.pages.end() &&
                           page->second.fields.contains(LowerKey(std::string(member.field)));
      return !control && DoorCalls(member.field);
    }
    if (const auto *fields = FieldsOf(member.variable); fields != nullptr) {
      return DoorCalls(member.field) && !fields->contains(LowerKey(std::string(member.field)));
    }
    if (local != nullptr && TypeName(local->type) == "Record") {
      return DoorCalls(member.field) &&
             !PlatformFieldNamed(PlatformField{.table = local->subtype, .field = member.field});
    }
    return IsRecord(member.variable) && DoorCalls(member.field) &&
           FieldNamed(table_, member.field) == nullptr;
  }

  [[nodiscard]] std::string ObjectNamed(std::string_view kind,
                                        std::string_view name) const override {
    const TableIndex *index = nullptr;
    if (kind == "codeunits") { index = &objects_.codeunits; }
    if (kind == "tables") { index = &objects_.tables; }
    if (kind == "pages") { index = &objects_.pages; }
    if (kind == "interfaces") { index = &objects_.interfaces; }
    if (index == nullptr) { return std::string(kind) + "::" + AsTheDoorSpellsIt(Identifier(name)); }
    const auto found = index->find(LowerKey(std::string(name)));
    if (found != index->end()) { return found->second.identifier; }
    return "absent::" + Identifier(name);
  }

  [[nodiscard]] std::string EnumObject(std::string_view name) const override {
    return NamedEnum(objects_, name);
  }

  [[nodiscard]] bool IsHandle(std::string_view name) const override {
    if (const al::VarDecl *local = Local(name); local != nullptr) {
      return TypeName(local->type) == "Interface";
    }
    for (const al::VarDecl &declared : table_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) {
        return DeclaresAnObject(declared) || TypeName(declared.type) == "Interface";
      }
    }
    return false;
  }

  [[nodiscard]] std::string Resolve(std::string_view name) const override {
    if (const al::VarDecl *local = Local(name); local != nullptr) {
      return Identifier(local->name);
    }
    if (LocalLabel(name)) { return Identifier(std::string(name)); }
    for (const al::VarDecl &declared : table_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) {
        return "Var_Block->" + VariableIdentifier(table_, declared.name);
      }
    }
    const al::FieldDecl *field = FieldNamed(table_, name);
    if (field != nullptr) {
      const std::string spelled = FieldIdentifier(table_, field->name);
      return ShadowedByALocal(spelled) ? "this->" + spelled : spelled;
    }
    for (const al::LabelDecl &label : table_.labels) {
      if (SameName(label.name, name)) { return label.name; }
    }
    for (const al::ProcedureDecl &procedure : table_.procedures) {
      if (SameName(procedure.name, name)) { return ProcedureIdentifier(table_, procedure.name); }
    }
    if (SameName("Rec", name)) { return "(*this)"; }
    return {};
  }

  [[nodiscard]] const al::VarDecl *Global(std::string_view name) const {
    for (const al::VarDecl &declared : table_.variables) {
      if (SameName(declared.name, name)) { return &declared; }
    }
    return nullptr;
  }

  [[nodiscard]] bool LocalLabel(std::string_view name) const {
    return running_ != nullptr &&
           std::ranges::any_of(running_->labels, [name](const al::LabelDecl &label) {
             return SameName(label.name, name);
           });
  }

  [[nodiscard]] bool IsLabel(std::string_view name) const override {
    return LocalLabel(name) ||
           std::ranges::any_of(table_.labels, [name](const al::LabelDecl &label) {
             return SameName(label.name, name);
           });
  }

  [[nodiscard]] std::string Enumeration(std::string_view name) const override {
    const al::FieldDecl *field = FieldNamed(table_, name);
    if (field == nullptr) {
      for (const al::VarDecl *where : {Local(name), Global(name)}) {
        if (where == nullptr) { continue; }
        if (TypeName(where->type) == "Enum" && !where->subtype.empty()) {
          return "enums::" + Identifier(where->subtype);
        }
        if (TypeName(where->type) == "Option" && !where->members.empty()) {
          return OptionTypeName(table_.name,
                                running_ != nullptr ? running_->name : std::string{},
                                *where,
                                table_.procedures);
        }
      }
      return {};
    }
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

  [[nodiscard]] std::string MemberSpelling(const OfVariable &member) const override {
    for (const al::VarDecl *where : {Local(member.variable), Global(member.variable)}) {
      if (where == nullptr || TypeName(where->type) != "Codeunit" || where->subtype.empty()) {
        continue;
      }
      const auto unit = objects_.codeunits.find(LowerKey(where->subtype));
      if (unit == objects_.codeunits.end()) { break; }
      const auto found = unit->second.procedures.find(LowerKey(std::string(member.field)));
      if (found != unit->second.procedures.end()) { return found->second; }
      break;
    }
    if (const auto *fields = FieldsOf(member.variable); fields != nullptr) {
      const auto field = fields->find(LowerKey(std::string(member.field)));
      if (field != fields->end()) { return field->second; }
      if (const std::string declaredThere = ProcedureOf(member); !declaredThere.empty()) {
        return declaredThere;
      }
      return AsTheDoorSpellsIt(Identifier(member.field));
    }
    if (IsRecord(member.variable) && FieldNamed(table_, member.field) != nullptr) {
      return Identifier(member.field);
    }
    for (const al::VarDecl *where : {Local(member.variable), Global(member.variable)}) {
      if (where == nullptr) { continue; }
      const std::string platform =
          PlatformFieldSpelling(PlatformField{.table = where->subtype, .field = member.field});
      if (!platform.empty()) { return platform; }
    }
    if (const std::string declaredThere = ProcedureOf(member); !declaredThere.empty()) {
      return declaredThere;
    }
    return MemberIsCall(member) ? AsTheDoorSpellsIt(Identifier(member.field))
                                : Identifier(member.field);
  }

  [[nodiscard]] std::string FieldEnumeration(const OfVariable &field) const override {
    if (IsRecord(field.variable)) { return Enumeration(field.field); }
    for (const al::VarDecl *where : {Local(field.variable), Global(field.variable)}) {
      if (where == nullptr || TypeName(where->type) != "Record") { continue; }
      const auto table = objects_.fieldEnums.find(LowerKey(where->subtype));
      if (table == objects_.fieldEnums.end()) { break; }
      const auto found = table->second.find(LowerKey(std::string(field.field)));
      if (found != table->second.end()) { return found->second; }
    }
    for (const al::VarDecl *where : {Local(field.variable), Global(field.variable)}) {
      if (where == nullptr || TypeName(where->type) != "Record") { continue; }
      const auto table = objects_.fieldEnums.find(LowerKey(where->subtype));
      if (table == objects_.fieldEnums.end()) { return {}; }
      const auto found = table->second.find(LowerKey(std::string(field.field)));
      return found == table->second.end() ? std::string{} : found->second;
    }
    return {};
  }

private:
  const al::TableObject &table_;
  const Objects &objects_;
  const al::ProcedureDecl *running_;
};

class PageNames : public Names {
public:
  [[nodiscard]] std::string EnumMember(std::string_view enumeration,
                                       std::string_view member) const override {
    return DeclaredEnumMember(objects_, enumeration, member);
  }

  PageNames(const al::PageObject &page, const al::TableObject *source, const Objects &objects)
      : page_(page), source_(source), objects_(objects) {}

  [[nodiscard]] std::string ObjectNamed(std::string_view kind,
                                        std::string_view name) const override {
    const TableIndex *index = nullptr;
    if (kind == "codeunits") { index = &objects_.codeunits; }
    if (kind == "tables") { index = &objects_.tables; }
    if (kind == "pages") { index = &objects_.pages; }
    if (kind == "interfaces") { index = &objects_.interfaces; }
    if (index == nullptr) { return std::string(kind) + "::" + AsTheDoorSpellsIt(Identifier(name)); }
    const auto found = index->find(LowerKey(std::string(name)));
    if (found != index->end()) { return found->second.identifier; }
    return "absent::" + Identifier(name);
  }

  [[nodiscard]] std::string EnumObject(std::string_view name) const override {
    return NamedEnum(objects_, name);
  }

  [[nodiscard]] bool IsHandle(std::string_view name) const override {
    for (const al::VarDecl &declared : page_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) {
        return DeclaresAnObject(declared) || TypeName(declared.type) == "Interface";
      }
    }
    return false;
  }

  [[nodiscard]] bool IsLabel(std::string_view name) const override {
    return std::ranges::any_of(
        page_.labels, [name](const al::LabelDecl &label) { return SameName(label.name, name); });
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
    if (IsRecord(member.variable)) {
      return DoorCalls(member.field) && FieldNamed(*source_, member.field) == nullptr;
    }
    if (SameName("CurrPage", member.variable)) {
      return DoorCalls(member.field) && ControlOf(member.field).empty();
    }
    const auto *fields = FieldsOfRecord(member.variable);
    return fields != nullptr && DoorCalls(member.field) &&
           !fields->contains(LowerKey(std::string(member.field)));
  }

  [[nodiscard]] std::string MemberSpelling(const OfVariable &member) const override {
    if (SameName("CurrPage", member.variable)) {
      const std::string control = ControlOf(member.field);
      return control.empty() ? AsTheDoorSpellsIt(Identifier(member.field)) : control;
    }
    if (const auto *fields = FieldsOfRecord(member.variable); fields != nullptr) {
      const auto field = fields->find(LowerKey(std::string(member.field)));
      return field != fields->end() ? field->second : AsTheDoorSpellsIt(Identifier(member.field));
    }
    if (!IsRecord(member.variable) || FieldNamed(*source_, member.field) != nullptr) {
      return Identifier(member.field);
    }
    return AsTheDoorSpellsIt(member.field);
  }

  [[nodiscard]] std::string ControlOf(std::string_view name) const {
    const auto page = objects_.pages.find(LowerKey(page_.name));
    if (page == objects_.pages.end()) { return {}; }
    const auto found = page->second.fields.find(LowerKey(std::string(name)));
    return found == page->second.fields.end() ? std::string{} : found->second;
  }

  [[nodiscard]] const std::map<std::string, std::string> *
  FieldsOfRecord(std::string_view variable) const {
    for (const al::VarDecl &declared : page_.variables) {
      if (!SameName(declared.name, variable) || TypeName(declared.type) != "Record") { continue; }
      const auto table = objects_.tables.find(LowerKey(declared.subtype));
      if (table == objects_.tables.end()) { return nullptr; }
      return &table->second.fields;
    }
    return nullptr;
  }

  [[nodiscard]] std::string FieldEnumeration(const OfVariable &field) const override {
    if (IsRecord(field.variable)) { return Enumeration(field.field); }
    for (const al::VarDecl &where : page_.variables) {
      if (!SameName(where.name, field.variable) || TypeName(where.type) != "Record") { continue; }
      const auto table = objects_.fieldEnums.find(LowerKey(where.subtype));
      if (table == objects_.fieldEnums.end()) { break; }
      const auto found = table->second.find(LowerKey(std::string(field.field)));
      if (found != table->second.end()) { return found->second; }
    }
    return {};
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
bool MentionsXRec(const std::string &body) {
  for (std::size_t at = body.find("XRec"); at != std::string::npos;
       at = body.find("XRec", at + 1)) {
    const bool before = at > 0 && (std::isalnum(static_cast<unsigned char>(body[at - 1])) != 0 ||
                                   body[at - 1] == '_');
    const std::size_t after = at + 4;
    const bool behind =
        after < body.size() &&
        (std::isalnum(static_cast<unsigned char>(body[after])) != 0 || body[after] == '_');
    if (!before && !behind) { return true; }
  }
  return false;
}

std::string BindsBefore(const std::string &body, const std::string &identifier) {
  return !MentionsXRec(body) ? std::string{}
                             : "  tables::" + identifier +
                                   " &XRec = detail::Before<tables::" + identifier + ">();\n\n";
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
  std::vector<al::ProcedureDecl> reaching = table.procedures;
  for (const al::FieldDecl &field : table.fields) {
    for (const al::ProcedureDecl &trigger : field.triggers) { reaching.push_back(trigger); }
  }
  out += SourceIncludesOf(table.variables, reaching, objects);
  out += "\n" + TableDefinitions(table, objects.enums);
  const std::size_t bodyAt = out.size();
  const std::set<std::string> shadowedByFields = Shadowed(table);
  out += "\nnamespace agiru::app::tables {\n\n";
  for (const al::FieldDecl &field : table.fields) {
    for (const al::Trigger &trigger : field.triggers) {
      const std::string body =
          WriteStatements(TableNames(table, objects, &trigger), trigger.body, 2);
      out += "void " + identifier + "::" + trigger.name + Identifier(field.name) + "() {\n";
      out +=
          ProcedureLocals(trigger, objects, table.name, table.procedures, shadowedByFields, body);
      out += BindsBefore(body, identifier);
      out += body;
      out += "}\n\n";
    }
  }
  for (const al::ProcedureDecl &procedure : table.procedures) {
    const std::string traits = "::agiru::TableTraits<::agiru::app::tables::" + identifier + ">";
    const std::string body =
        IsPublisher(procedure)
            ? RaisingBody(procedure,
                          "EventObject::Table",
                          traits + "::kTable.id.Value()",
                          traits + "::kTable.name")
            : WriteStatements(TableNames(table, objects, &procedure), procedure.body, 2) +
                  FallsOffEnd(procedure, TableNames(table, objects, &procedure));
    const std::string locals =
        IsPublisher(procedure)
            ? std::string{}
            : ProcedureLocals(
                  procedure, objects, table.name, table.procedures, shadowedByFields, body) +
                  BindsBefore(body, identifier);
    out += ProcedureSignature(
               procedure,
               objects,
               table.name,
               identifier,
               !(locals.empty() && body.empty()),
               shadowedByFields,
               table.procedures,
               Spelling{.spelled = ProcedureIdentifier(table, procedure.name), .body = body}) +
           " {";
    if (locals.empty() && body.empty()) {
      out += "}\n\n";
      continue;
    }
    out += "\n" + locals;
    if (!locals.empty() && !body.empty()) { out += "\n"; }
    out += body + "}\n\n";
  }

  out += "namespace {\nnamespace " + identifier + "_unit {\nconst RegisterTable<" + identifier +
         "> kInCatalogue;\n} // namespace " + identifier + "_unit\n} // namespace\n\n";
  out += "} // namespace agiru::app::tables\n";
  out.insert(bodyAt, BodyIncludes(out.substr(bodyAt), objects));
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
          ProcedureLocals(trigger, objects, page.name, page.procedures, {}, body) +
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
    const std::string traits = "::agiru::PageTraits<::agiru::app::pages::" + identifier + ">";
    const std::string body =
        IsPublisher(procedure)
            ? RaisingBody(
                  procedure, "EventObject::Page", traits + "::kId.Value()", traits + "::kName")
            : WriteStatements(PageNames(page, source, objects), procedure.body, 2) +
                  FallsOffEnd(procedure, PageNames(page, source, objects));
    out += ProcedureSignature(procedure,
                              objects,
                              page.name,
                              identifier,
                              true,
                              {},
                              page.procedures,
                              Spelling{.spelled = Identifier(procedure.name), .body = body}) +
           " {";
    const std::string locals =
        IsPublisher(procedure)
            ? std::string{}
            : ProcedureLocals(procedure, objects, page.name, page.procedures, {}, body) +
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

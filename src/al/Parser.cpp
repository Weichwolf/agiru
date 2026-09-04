#include "Parser.h"

#include "Ast.h"
#include "Lexer.h"
#include "Statements.h"
#include "Token.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::al {

namespace {

char Lower(char c) {
  return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

bool SameName(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) { return false; }
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (Lower(a[i]) != Lower(b[i])) { return false; }
  }
  return true;
}

class Parser {
public:
  explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

  TableObject ParseTable() {
    TableObject table;
    table.nameSpace = ReadHeaderNamespace("table");
    Expect("table");
    table.id = ExpectInteger();
    table.name = ExpectName();
    ParseTableBody(table);
    return table;
  }

  TableExtensionObject ParseTableExtension() {
    TableExtensionObject extension;
    extension.nameSpace = ReadHeaderNamespace("tableextension");
    Expect("tableextension");
    extension.id = ExpectInteger();
    extension.name = ExpectName();
    Expect("extends");
    extension.extends = ExpectName();
    TableObject body;
    ParseTableBody(body);
    extension.fields = std::move(body.fields);
    extension.modified = std::move(body.modified);
    extension.keys = std::move(body.keys);
    extension.labels = std::move(body.labels);
    extension.variables = std::move(body.variables);
    extension.procedures = std::move(body.procedures);
    return extension;
  }

  void ParseTableBody(TableObject &table) {
    Expect("{");
    std::vector<std::string> attributes;
    while (!AtPunctuation("}") && !AtEnd()) {
      if (AtKeyword("fields")) {
        Advance();
        ParseFields(table);
      } else if (AtKeyword("keys")) {
        Advance();
        ParseKeys(table);
      } else if (AtKeyword("var")) {
        Advance();
        ParseVarsInto(table.labels, table.variables);
      } else if (AtPunctuation("[")) {
        attributes.push_back(ReadAttribute());
      } else if (AtProtectedVar()) {
        Advance();
        Advance();
        ParseVarsInto(table.labels, table.variables);
      } else if (AtKeyword("trigger") || AtKeyword("procedure") || AtKeyword("local") ||
                 AtKeyword("internal") || AtKeyword("protected")) {
        table.procedures.push_back(ParseProcedure(attributes));
        attributes.clear();
      } else if (Peek().kind == TokenKind::Identifier && IsPunctuation(Peek(1), "{")) {
        Advance();
        SkipBracedBlock();
      } else {
        table.properties.push_back(ParseProperty());
      }
    }
    Expect("}");
  }

  CodeunitObject ParseCodeunit() {
    CodeunitObject unit;
    unit.nameSpace = ReadHeaderNamespace("codeunit");
    Expect("codeunit");
    unit.id = ExpectInteger();
    unit.name = ExpectName();
    if (AtKeyword("implements")) {
      Advance();
      while (!AtEnd() && !AtPunctuation("{")) {
        unit.implements.push_back(ExpectName());
        if (AtPunctuation(",")) { Advance(); }
      }
    }
    Expect("{");
    std::vector<std::string> attributes;
    while (!AtPunctuation("}")) {
      if (AtPunctuation("[")) {
        attributes.push_back(ReadAttribute());
        continue;
      }
      if (AtProtectedVar()) {
        Advance();
        Advance();
        ParseVarsInto(unit.labels, unit.variables);
        continue;
      }
      if (AtKeyword("var")) {
        Advance();
        ParseVarsInto(unit.labels, unit.variables);
        continue;
      }
      if (AtKeyword("trigger") || AtKeyword("procedure") || AtKeyword("local") ||
          AtKeyword("internal") || AtKeyword("protected")) {
        unit.procedures.push_back(ParseProcedure(attributes));
        attributes.clear();
        continue;
      }
      unit.properties.push_back(ParseProperty());
    }
    Expect("}");
    return unit;
  }

  EnumObject ParseEnum() {
    EnumObject object;
    object.nameSpace = ReadHeaderNamespace("enum");
    Expect("enum");
    object.id = ExpectInteger();
    object.name = ExpectName();
    if (AtKeyword("implements")) {
      Advance();
      while (!AtEnd() && !AtPunctuation("{")) {
        object.implements.push_back(ExpectName());
        if (AtPunctuation(",")) { Advance(); }
      }
    }
    ParseEnumBody(object.values, object.properties);
    return object;
  }

  EnumExtensionObject ParseEnumExtension() {
    EnumExtensionObject extension;
    extension.nameSpace = ReadHeaderNamespace("enumextension");
    Expect("enumextension");
    extension.id = ExpectInteger();
    extension.name = ExpectName();
    Expect("extends");
    extension.extends = ExpectName();
    std::vector<Property> properties;
    ParseEnumBody(extension.values, properties);
    return extension;
  }

  void ParseEnumBody(std::vector<EnumValueDecl> &values, std::vector<Property> &properties) {
    Expect("{");
    while (!AtPunctuation("}") && !AtEnd()) {
      if (AtKeyword("value")) {
        Advance();
        Expect("(");
        EnumValueDecl value;
        value.ordinal = ExpectInteger();
        Expect(";");
        value.name = ExpectName();
        Expect(")");
        Expect("{");
        while (!AtPunctuation("}")) { value.properties.push_back(ParseProperty()); }
        Expect("}");
        values.push_back(std::move(value));
        continue;
      }
      properties.push_back(ParseProperty());
    }
    Expect("}");
  }

private:
public:
  ProcedureDecl ParseSignature() {
    ProcedureDecl procedure;
    while (AtKeyword("local") || AtKeyword("internal") || AtKeyword("protected")) {
      procedure.isLocal = procedure.isLocal || AtKeyword("local");
      Advance();
    }
    procedure.isTrigger = AtKeyword("trigger");
    if (AtKeyword("trigger") || AtKeyword("procedure") || AtKeyword("event")) { Advance(); }
    procedure.name = ExpectName();
    while (AtPunctuation("::")) {
      Advance();
      procedure.name = ExpectName();
    }
    Expect("(");
    while (!AtPunctuation(")")) {
      Parameter parameter;
      if (AtKeyword("var")) {
        parameter.byReference = true;
        Advance();
      }
      std::string name = ExpectName();
      Expect(":");
      const bool byReference = parameter.byReference;
      parameter = ReadType();
      parameter.byReference = byReference;
      parameter.name = std::move(name);
      procedure.parameters.push_back(std::move(parameter));
      if (AtPunctuation(";")) { Advance(); }
    }
    Expect(")");
    if (!AtPunctuation(":") && !AtKeyword("var") && !AtKeyword("begin") && !AtPunctuation(";") &&
        !AtKeyword("procedure") && !AtPunctuation("}")) {
      procedure.returnName = ExpectName();
    }
    if (AtPunctuation(":")) {
      Advance();
      const VarDecl returned = ReadType();
      procedure.returnType = returned.type;
      procedure.returnSubtype = returned.subtype;
      procedure.returned = returned;
      procedure.returned.name = procedure.returnName;
    }
    if (AtPunctuation(";")) { Advance(); }
    return procedure;
  }

  InterfaceObject ParseInterface() {
    InterfaceObject object;
    object.nameSpace = ReadHeaderNamespace("interface");
    Expect("interface");
    if (Peek().kind == TokenKind::Integer) { object.id = ExpectInteger(); }
    object.name = ExpectName();
    if (AtKeyword("extends")) {
      Advance();
      while (!AtEnd() && !AtPunctuation("{")) {
        (void)ExpectName();
        if (AtPunctuation(",")) { Advance(); }
      }
    }
    Expect("{");
    while (!AtEnd() && !AtPunctuation("}")) {
      if (AtKeyword("procedure")) {
        object.procedures.push_back(ParseSignature());
        continue;
      }
      Advance();
    }
    Expect("}");
    return object;
  }

  PageObject ParsePage() {
    PageObject object;
    object.nameSpace = ReadHeaderNamespace("page");
    Expect("page");
    object.id = ExpectInteger();
    object.name = ExpectName();
    ParsePageBody(object);
    return object;
  }

  PageExtensionObject ParsePageExtension() {
    PageExtensionObject extension;
    extension.nameSpace = ReadHeaderNamespace("pageextension");
    Expect("pageextension");
    extension.id = ExpectInteger();
    extension.name = ExpectName();
    Expect("extends");
    extension.extends = ExpectName();
    PageObject body;
    ParsePageBody(body);
    extension.layout = std::move(body.layout);
    extension.actions = std::move(body.actions);
    extension.procedures = std::move(body.procedures);
    extension.variables = std::move(body.variables);
    extension.labels = std::move(body.labels);
    return extension;
  }

  void ParsePageBody(PageObject &object) {
    Expect("{");
    std::vector<std::string> attributes;
    while (!AtPunctuation("}") && !AtEnd()) {
      if (AtPunctuation("[")) {
        attributes.push_back(ReadAttribute());
        continue;
      }
      if (AtKeyword("layout")) {
        Advance();
        ParseControlsInto(object.layout);
        continue;
      }
      if (AtKeyword("actions")) {
        Advance();
        ParseControlsInto(object.actions);
        continue;
      }
      if (Peek().kind == TokenKind::Identifier && IsPunctuation(Peek(1), "{")) {
        Advance();
        SkipBracedBlock();
        continue;
      }
      if (AtProtectedVar()) {
        Advance();
        Advance();
        ParseVarsInto(object.labels, object.variables);
        continue;
      }
      if (AtKeyword("var")) {
        Advance();
        ParseVarsInto(object.labels, object.variables);
        continue;
      }
      if (AtKeyword("trigger") || AtKeyword("procedure") || AtKeyword("local") ||
          AtKeyword("internal") || AtKeyword("protected")) {
        object.procedures.push_back(ParseProcedure(attributes));
        attributes.clear();
        continue;
      }
      object.properties.push_back(ParseProperty());
    }
    Expect("}");
  }

  void ParseControlsInto(std::vector<PageControl> &into) {
    Expect("{");
    while (!AtPunctuation("}") && !AtEnd()) { into.push_back(ParseControl()); }
    Expect("}");
  }

  void ParseControlHead(PageControl &control) {
    if (!AtPunctuation("(")) { return; }
    Advance();
    if (!AtPunctuation(")")) { control.name = ExpectName(); }
    if (AtPunctuation(";")) {
      Advance();
      int depth = 0;
      while (!AtEnd() && (depth != 0 || !AtPunctuation(")"))) {
        if (AtPunctuation("(")) { ++depth; }
        if (AtPunctuation(")")) { --depth; }
        control.source.push_back(Peek());
        Advance();
      }
    }
    Expect(")");
  }

  PageControl ParseControl() {
    PageControl control;
    control.kind = Peek().text;
    Advance();
    ParseControlHead(control);
    if (AtPunctuation(";")) {
      Advance();
      return control;
    }
    if (!AtPunctuation("{")) { return control; }
    Expect("{");
    while (!AtPunctuation("}") && !AtEnd()) {
      if (AtKeyword("trigger") || AtKeyword("procedure") || AtKeyword("local") ||
          AtKeyword("internal") || AtKeyword("protected")) {
        control.triggers.push_back(ParseProcedure({}));
        continue;
      }
      if (Peek(1).kind == TokenKind::Punctuation && (Peek(1).text == "(" || Peek(1).text == "{")) {
        control.children.push_back(ParseControl());
        continue;
      }
      control.properties.push_back(ParseProperty());
    }
    Expect("}");
    return control;
  }

private:
  ProcedureDecl ParseProcedure(const std::vector<std::string> &attributes) {
    ProcedureDecl procedure;
    procedure.attributes = attributes;
    while (AtKeyword("local") || AtKeyword("internal") || AtKeyword("protected")) {
      procedure.isLocal = procedure.isLocal || AtKeyword("local");
      Advance();
    }
    procedure.isTrigger = AtKeyword("trigger");
    if (AtKeyword("trigger") || AtKeyword("procedure") || AtKeyword("event")) { Advance(); }
    procedure.name = ExpectName();
    while (AtPunctuation("::")) {
      Advance();
      procedure.name = ExpectName();
    }
    Expect("(");
    while (!AtPunctuation(")")) {
      Parameter parameter;
      if (AtKeyword("var")) {
        parameter.byReference = true;
        Advance();
      }
      std::string name;
      name = ExpectName();
      Expect(":");
      const bool byReference = parameter.byReference;
      parameter = ReadType();
      parameter.byReference = byReference;
      parameter.name = std::move(name);
      procedure.parameters.push_back(std::move(parameter));
      if (AtPunctuation(";")) { Advance(); }
    }
    Expect(")");
    if (!AtPunctuation(":") && !AtKeyword("var") && !AtKeyword("begin") && !AtPunctuation(";")) {
      procedure.returnName = ExpectName();
    }
    if (AtPunctuation(":")) {
      Advance();
      const VarDecl returned = ReadType();
      procedure.returnType = returned.type;
      procedure.returnSubtype = returned.subtype;
      procedure.returned = returned;
      procedure.returned.name = procedure.returnName;
    }
    if (AtPunctuation(";")) { Advance(); }
    std::vector<LabelDecl> locals;
    procedure.tokens = SkipBeginEnd(locals, procedure.variables);
    procedure.body = ParseStatements(procedure.tokens);
    return procedure;
  }

  void SkipBracketed() {
    Expect("[");
    int depth = 1;
    while (!AtEnd() && depth > 0) {
      if (AtPunctuation("[")) { ++depth; }
      if (AtPunctuation("]")) { --depth; }
      Advance();
    }
  }

  std::vector<std::string> ReadOptionMembers() {
    std::vector<std::string> members;
    bool expecting = true;
    while (Peek().kind == TokenKind::Identifier || Peek().kind == TokenKind::QuotedIdentifier ||
           AtPunctuation(",")) {
      if (AtKeyword("var") || AtKeyword("begin") || AtKeyword("temporary")) { break; }
      if (AtPunctuation(",")) {
        if (expecting) { members.emplace_back(); }
        expecting = true;
        Advance();
        continue;
      }
      members.push_back(Peek().text);
      expecting = false;
      Advance();
    }
    if (expecting && !members.empty()) { members.emplace_back(); }
    return members;
  }

  std::string ReadSubtypeName() {
    if (AtKeyword("var") || AtKeyword("begin") || AtKeyword("temporary")) { return {}; }
    if (Peek().kind == TokenKind::Integer) {
      std::string number = Peek().text;
      Advance();
      return number;
    }
    std::string name = ExpectName();
    while (AtPunctuation(".")) {
      Advance();
      name = ExpectName();
    }
    return name;
  }

  void ReadDimensions(VarDecl &declared) {
    while (AtKeyword("array")) {
      Advance();
      if (AtPunctuation("[")) {
        Advance();
        while (!AtEnd() && !AtPunctuation("]")) {
          if (Peek().kind == TokenKind::Integer) {
            declared.dimensions.push_back(ExpectInteger());
          } else {
            Advance();
          }
        }
        Expect("]");
      }
      Expect("of");
    }
  }

  static bool TakesASubtype(std::string_view type) {
    static constexpr std::array kWithOne{
        std::string_view{"Record"},
        std::string_view{"Codeunit"},
        std::string_view{"Page"},
        std::string_view{"Report"},
        std::string_view{"Query"},
        std::string_view{"XmlPort"},
        std::string_view{"Enum"},
        std::string_view{"Interface"},
        std::string_view{"TestPage"},
        std::string_view{"TestRequestPage"},
        std::string_view{"ControlAddIn"},
        std::string_view{"DotNet"},
        std::string_view{"RecordRef"},
        std::string_view{"FilterPageBuilder"},
        std::string_view{"TestAction"},
        std::string_view{"TestField"},
        std::string_view{"Automation"},
        std::string_view{"Label"},
    };
    return std::ranges::any_of(kWithOne,
                               [type](std::string_view known) { return SameName(known, type); });
  }

  VarDecl ReadType() {
    VarDecl declared;
    ReadDimensions(declared);
    declared.type = ExpectName();
    if (AtKeyword("of")) {
      Advance();
      Expect("[");
      while (!AtEnd() && !AtPunctuation("]")) {
        if (AtPunctuation(",")) {
          Advance();
          continue;
        }
        declared.arguments.push_back(ReadType());
      }
      Expect("]");
      return declared;
    }
    if (SameName(declared.type, "Option")) {
      declared.members = ReadOptionMembers();
      return declared;
    }
    if (AtPunctuation("[")) {
      Advance();
      if (Peek().kind == TokenKind::Integer) {
        declared.length = std::stoi(Peek().text);
        Advance();
      }
      while (!AtEnd() && !AtPunctuation("]")) { Advance(); }
      Expect("]");
    } else if (TakesASubtype(declared.type) &&
               (Peek().kind == TokenKind::Identifier ||
                Peek().kind == TokenKind::QuotedIdentifier || Peek().kind == TokenKind::Integer)) {
      declared.subtype = ReadSubtypeName();
    }
    while (AtKeyword("temporary")) {
      declared.temporary = true;
      Advance();
    }
    return declared;
  }

  std::string ReadTypeName() { return ReadType().type; }

  std::string ReadAttribute() {
    Expect("[");
    std::string written;
    int depth = 1;
    while (!AtEnd() && depth > 0) {
      if (AtPunctuation("[")) { ++depth; }
      if (AtPunctuation("]")) { --depth; }
      if (depth > 0) { written += Peek().text; }
      Advance();
    }
    return written;
  }

  void ParseVarsInto(std::vector<LabelDecl> &labels) {
    std::vector<VarDecl> discarded;
    ParseVarsInto(labels, discarded);
  }

  void ParseVarsInto(std::vector<LabelDecl> &labels, std::vector<VarDecl> &variables) {
    while (!AtEnd() && !AtPunctuation("}") && !AtKeyword("var") && !AtKeyword("begin") &&
           !AtKeyword("trigger") && !AtKeyword("procedure") && !AtKeyword("local") &&
           !AtKeyword("internal") && !AtKeyword("protected")) {
      if (AtPunctuation("[")) {
        if (!VariableFollowsAttribute()) { return; }
        (void)ReadAttribute();
        continue;
      }
      std::vector<std::string> names{ExpectName()};
      while (AtPunctuation(",")) {
        Advance();
        names.push_back(ExpectName());
      }
      Expect(":");
      if (AtKeyword("Label")) {
        Advance();
        if (Peek().kind == TokenKind::String) {
          for (const std::string &name : names) {
            labels.push_back(LabelDecl{.name = name, .text = Peek().text});
          }
          Advance();
        }
      } else {
        const VarDecl declared = ReadType();
        for (const std::string &name : names) {
          VarDecl one = declared;
          one.name = name;
          variables.push_back(std::move(one));
        }
      }
      while (!AtEnd() && !AtPunctuation(";")) { Advance(); }
      Expect(";");
    }
  }

  [[nodiscard]] const Token &Peek(std::size_t ahead = 0) const {
    const std::size_t index = position_ + ahead;
    return index < tokens_.size() ? tokens_[index] : tokens_.back();
  }

  [[nodiscard]] bool AtProtectedVar() const {
    return (AtKeyword("protected") || AtKeyword("internal") || AtKeyword("local")) &&
           IsKeyword(Peek(1), "var");
  }

  void Advance() {
    if (position_ + 1 < tokens_.size()) { ++position_; }
  }

  [[nodiscard]] bool AtKeyword(std::string_view keyword) const {
    return IsKeyword(Peek(), keyword);
  }

  [[nodiscard]] bool AtPunctuation(std::string_view text) const {
    return IsPunctuation(Peek(), text);
  }

  [[nodiscard]] bool AtEnd() const { return Peek().kind == TokenKind::EndOfFile; }

  void Expect(std::string_view what) {
    if (AtKeyword(what) || AtPunctuation(what)) {
      Advance();
      return;
    }
    throw ParseError("expected '" + std::string(what) + "' but found '" + Peek().text +
                     "' on line " + std::to_string(Peek().line));
  }

  int ExpectInteger() {
    if (Peek().kind != TokenKind::Integer) {
      throw ParseError("expected a number on line " + std::to_string(Peek().line));
    }
    const int value = std::stoi(Peek().text);
    Advance();
    return value;
  }

  std::string ExpectName() {
    if (Peek().kind != TokenKind::Identifier && Peek().kind != TokenKind::QuotedIdentifier) {
      throw ParseError("expected a name on line " + std::to_string(Peek().line));
    }
    std::string value = Peek().text;
    Advance();
    return value;
  }

  std::string ReadHeaderNamespace(std::string_view objectWord) {
    std::string dotted;
    while (!AtEnd() && !AtKeyword(objectWord)) {
      if (AtEnd()) { break; }
      if (AtKeyword("namespace")) {
        Advance();
        dotted.clear();
        while (!AtEnd() && !AtPunctuation(";")) {
          dotted += Peek().text;
          Advance();
        }
      }
      Advance();
    }
    if (AtEnd()) {
      throw ParseError("this file declares no " + std::string(objectWord) + " object");
    }
    return dotted;
  }

  void SkipBracedBlock() {
    Expect("{");
    int depth = 1;
    while (!AtEnd() && depth > 0) {
      if (AtPunctuation("{")) { ++depth; }
      if (AtPunctuation("}")) { --depth; }
      Advance();
    }
  }

  [[nodiscard]] bool IsVariableAhead() const {
    const bool named =
        Peek().kind == TokenKind::Identifier || Peek().kind == TokenKind::QuotedIdentifier;
    return named && IsPunctuation(Peek(1), ":");
  }

  [[nodiscard]] bool VariableFollowsAttribute() const {
    std::size_t ahead = 0;
    while (IsPunctuation(Peek(ahead), "[")) {
      int depth = 0;
      do {
        if (IsPunctuation(Peek(ahead), "[")) { ++depth; }
        if (IsPunctuation(Peek(ahead), "]")) { --depth; }
        ++ahead;
      } while (depth > 0 && Peek(ahead).kind != TokenKind::EndOfFile);
    }
    while (Peek(ahead).kind == TokenKind::Identifier ||
           Peek(ahead).kind == TokenKind::QuotedIdentifier) {
      if (IsPunctuation(Peek(ahead + 1), ":")) { return true; }
      if (!IsPunctuation(Peek(ahead + 1), ",")) { return false; }
      ahead += 2;
    }
    return false;
  }

  void SkipAttribute() {
    Expect("[");
    int depth = 1;
    while (!AtEnd() && depth > 0) {
      if (AtPunctuation("[")) { ++depth; }
      if (AtPunctuation("]")) { --depth; }
      Advance();
    }
  }

  void SkipMember() {
    while (!AtEnd() && !AtKeyword("begin")) { Advance(); }
    (void)SkipBeginEnd();
  }

  void ReadLocalVars(std::vector<LabelDecl> &labels, std::vector<VarDecl> &variables) {
    if (!AtKeyword("var")) { return; }
    Advance();
    ParseVarsInto(labels, variables);
  }

  std::vector<Token> SkipBeginEnd() {
    std::vector<LabelDecl> labels;
    std::vector<VarDecl> variables;
    return SkipBeginEnd(labels, variables);
  }

  std::vector<Token> SkipBeginEnd(std::vector<LabelDecl> &labels, std::vector<VarDecl> &variables) {
    ReadLocalVars(labels, variables);
    Expect("begin");
    std::vector<Token> body;
    int depth = 1;
    while (!AtEnd()) {
      if (AtKeyword("begin") || AtKeyword("case")) { ++depth; }
      if (AtKeyword("end")) {
        --depth;
        if (depth == 0) {
          Advance();
          if (AtPunctuation(";")) { Advance(); }
          return body;
        }
      }
      body.push_back(Peek());
      Advance();
    }
    throw ParseError("unterminated begin block");
  }

  Property ParseProperty() {
    Property property;
    property.name = ExpectName();
    Expect("=");
    while (!AtEnd() && !AtPunctuation(";")) {
      property.value.push_back(Peek());
      if (!property.text.empty()) { property.text += ' '; }
      property.text += Peek().text;
      Advance();
    }
    Expect(";");
    return property;
  }

  void ParseFields(TableObject &table) {
    Expect("{");
    while (!AtPunctuation("}")) {
      if (AtKeyword("modify")) {
        Advance();
        Expect("(");
        FieldDecl changed;
        changed.name = ExpectName();
        Expect(")");
        ParseFieldBody(changed);
        table.modified.push_back(std::move(changed));
        continue;
      }
      Expect("field");
      Expect("(");
      FieldDecl field;
      field.number = ExpectInteger();
      Expect(";");
      field.name = ExpectName();
      Expect(";");
      field.type = ExpectName();
      if (AtPunctuation("[")) {
        Advance();
        field.length = ExpectInteger();
        Expect("]");
      } else if (!AtPunctuation(")")) {
        if (Peek().kind == TokenKind::Integer) {
          field.subtype = Peek().text;
          Advance();
        } else {
          field.subtype = ExpectName();
          while (AtPunctuation(".")) {
            Advance();
            field.subtype = ExpectName();
          }
        }
      }
      Expect(")");
      ParseFieldBody(field);
      table.fields.push_back(std::move(field));
    }
    Expect("}");
  }

  void ParseFieldBody(FieldDecl &field) {
    Expect("{");
    while (!AtPunctuation("}")) {
      if (AtKeyword("trigger")) {
        Advance();
        Trigger trigger;
        trigger.name = ExpectName();
        Expect("(");
        Expect(")");
        if (AtKeyword("var")) {
          Advance();
          std::vector<LabelDecl> labels;
          ParseVarsInto(labels, trigger.variables);
        }
        trigger.tokens = SkipBeginEnd();
        trigger.body = ParseStatements(trigger.tokens);
        field.triggers.push_back(std::move(trigger));
      } else {
        field.properties.push_back(ParseProperty());
      }
    }
    Expect("}");
  }

  void ParseKeys(TableObject &table) {
    Expect("{");
    while (!AtPunctuation("}")) {
      Expect("key");
      Expect("(");
      KeyDecl key;
      key.name = ExpectName();
      Expect(";");
      while (!AtPunctuation(")")) {
        key.fields.push_back(ExpectName());
        if (AtPunctuation(",")) { Advance(); }
      }
      Expect(")");
      Expect("{");
      while (!AtPunctuation("}")) { key.properties.push_back(ParseProperty()); }
      Expect("}");
      table.keys.push_back(std::move(key));
    }
    Expect("}");
  }

  std::vector<Token> tokens_;
  std::size_t position_ = 0;
};

}

const Property *Find(const std::vector<Property> &properties, std::string_view name) {
  for (const Property &p : properties) {
    if (SameName(p.name, name)) { return &p; }
  }
  return nullptr;
}

const Trigger *Find(const std::vector<Trigger> &triggers, std::string_view name) {
  for (const Trigger &t : triggers) {
    if (SameName(t.name, name)) { return &t; }
  }
  return nullptr;
}

std::vector<std::string> ListValue(const Property &property) {
  std::vector<std::string> items;
  std::string current;
  bool pending = false;
  for (const Token &token : property.value) {
    if (IsPunctuation(token, ",")) {
      items.push_back(current);
      current.clear();
      pending = false;
      continue;
    }
    current += token.text;
    pending = true;
  }
  if (pending || !items.empty()) { items.push_back(current); }
  return items;
}

TableObject ParseTable(std::string_view source) {
  return Parser(Tokenize(source)).ParseTable();
}

CodeunitObject ParseCodeunit(std::string_view source) {
  return Parser(Tokenize(source)).ParseCodeunit();
}

EnumObject ParseEnum(std::string_view source) {
  return Parser(Tokenize(source)).ParseEnum();
}

InterfaceObject ParseInterface(std::string_view source) {
  return Parser(Tokenize(source)).ParseInterface();
}

PageObject ParsePage(std::string_view source) {
  return Parser(Tokenize(source)).ParsePage();
}

TableExtensionObject ParseTableExtension(std::string_view source) {
  return Parser(Tokenize(source)).ParseTableExtension();
}

EnumExtensionObject ParseEnumExtension(std::string_view source) {
  return Parser(Tokenize(source)).ParseEnumExtension();
}

PageExtensionObject ParsePageExtension(std::string_view source) {
  return Parser(Tokenize(source)).ParsePageExtension();
}

bool HasAttribute(const ProcedureDecl &procedure, std::string_view name) {
  return std::ranges::any_of(procedure.attributes, [name](const std::string &a) {
    const std::size_t open = a.find('(');
    return SameName(open == std::string::npos ? a : a.substr(0, open), name);
  });
}

}

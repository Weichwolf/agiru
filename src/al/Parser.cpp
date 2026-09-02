#include "Parser.h"

#include "Ast.h"
#include "Lexer.h"
#include "Statements.h"
#include "Token.h"

#include <algorithm>
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
    Expect("{");
    while (!AtPunctuation("}")) {
      if (AtKeyword("fields")) {
        Advance();
        ParseFields(table);
      } else if (AtKeyword("keys")) {
        Advance();
        ParseKeys(table);
      } else if (AtKeyword("fieldgroups")) {
        Advance();
        SkipBracedBlock();
      } else if (AtKeyword("var")) {
        Advance();
        ParseVars(table);
      } else if (AtPunctuation("[")) {
        SkipAttribute();
      } else if (AtKeyword("trigger") || AtKeyword("procedure") || AtKeyword("local") ||
                 AtKeyword("internal") || AtKeyword("protected")) {
        SkipMember();
      } else {
        table.properties.push_back(ParseProperty());
      }
    }
    Expect("}");
    return table;
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
        (void)ExpectName();
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
    // An enum may implement interfaces, and 28 of the BaseApp's 576 do. The interface list is read
    // and dropped here rather than skipped past: `implements` is the only thing that may stand
    // between the name and the brace, so anything else is still an error.
    if (AtKeyword("implements")) {
      Advance();
      while (!AtEnd() && !AtPunctuation("{")) {
        (void)ExpectName();
        if (AtPunctuation(",")) { Advance(); }
      }
    }
    Expect("{");
    while (!AtPunctuation("}")) {
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
        object.values.push_back(std::move(value));
        continue;
      }
      object.properties.push_back(ParseProperty());
    }
    Expect("}");
    return object;
  }

private:
public:
  // AN INTERFACE'S PROCEDURE IS A SIGNATURE AND NOTHING ELSE -- no `begin`, no `var`, no body. It
  // is the same header a codeunit's procedure has, which is why the two share this and only the
  // BODY is read separately.
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
    }
    if (AtPunctuation(";")) { Advance(); }
    return procedure;
  }

  InterfaceObject ParseInterface() {
    InterfaceObject object;
    object.nameSpace = ReadHeaderNamespace("interface");
    Expect("interface");
    // AN INTERFACE MAY CARRY NO NUMBER. `interface "No. Series - Single"` is how BCApps writes
    // them, and the id is optional where every other object kind requires one.
    if (Peek().kind == TokenKind::Integer) { object.id = ExpectInteger(); }
    object.name = ExpectName();
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
    // A DotNet event receiver names its trigger through the variable it listens on:
    // `trigger EventReceiver::OnPermissionCheckEvent(sender: Variant; e: DotNet ...)`. The
    // qualifier belongs to the SUBSCRIPTION rather than to the signature, so the name kept here is
    // the event's.
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
        // `Option A,,B` -- a blank member holds an ordinal nobody filled in, and it counts.
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
    // AL names an object by NAME or by NUMBER, and both are legal in a subtype:
    // `var GLEntry: Record 17` is `Record "G/L Entry"`. Test code uses the number freely.
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

  // ONE READER FOR A TYPE, AND IT KEEPS WHAT IT READS. The subtype is what turns `Record` into a
  // class and `Codeunit` into another object, the length is what turns `Text` into `Text<N>`, and
  // `temporary` is what turns a record into one with no database behind it. Discarding them made a
  // type name enough for counting and not enough for emitting.
  VarDecl ReadType() {
    VarDecl declared;
    while (AtKeyword("array")) {
      Advance();
      if (AtPunctuation("[")) { SkipBracketed(); }
      Expect("of");
    }
    declared.type = ExpectName();
    // `List of [Text]` AND `Dictionary of [Text, Integer]` NAME THEIR ELEMENT TYPES, and they were
    // being skipped. A generic with its arguments thrown away is a class template with none.
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
    // AN INLINE OPTION DECLARES ITS MEMBERS AND HAS NO NAME, and they were being skipped. They are
    // the only thing that says what `Type::All` means where `Type` is a local; without them the
    // declaration is an integer with a lost vocabulary.
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
    } else if (Peek().kind == TokenKind::Identifier || Peek().kind == TokenKind::QuotedIdentifier ||
               Peek().kind == TokenKind::Integer) {
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
    const std::string name = Peek().text;
    int depth = 1;
    while (!AtEnd() && depth > 0) {
      if (AtPunctuation("[")) { ++depth; }
      if (AtPunctuation("]")) { --depth; }
      Advance();
    }
    return name;
  }

  // A `var` BLOCK ENDS AT THE NEXT `var` AS WELL AS AT THE NEXT MEMBER. A codeunit may declare
  // several, separated by nothing but a comment, and `#pragma` lines between them vanish in the
  // lexer. Without `var` in this list the keyword was read as a variable NAME and the block after
  // it was lost with the rest of the file.
  void ParseVarsInto(std::vector<LabelDecl> &labels) {
    std::vector<VarDecl> discarded;
    ParseVarsInto(labels, discarded);
  }

  void ParseVarsInto(std::vector<LabelDecl> &labels, std::vector<VarDecl> &variables) {
    // AND AT `begin`, because this now reads a PROCEDURE'S OWN var block as well as a codeunit's.
    // The codeunit-level block never meets one; a local block always does, and without it the
    // declarations ran straight into the body -- 3877 codeunits down to 380 in one run, which the
    // population baseline caught on the spot.
    while (!AtEnd() && !AtPunctuation("}") && !AtKeyword("var") && !AtKeyword("begin") &&
           !AtKeyword("trigger") && !AtKeyword("procedure") && !AtKeyword("local") &&
           !AtKeyword("internal") && !AtKeyword("protected")) {
      if (AtPunctuation("[")) {
        if (!VariableFollowsAttribute()) { return; }
        (void)ReadAttribute();
        continue;
      }
      // ONE TYPE CAN CARRY SEVERAL NAMES: `HasFileContent, HasTextContent : Boolean;`. Reading one
      // name and expecting a colon lost 9 codeunits over the tree, all with the same message.
      std::vector<std::string> names{ExpectName()};
      while (AtPunctuation(",")) {
        Advance();
        names.push_back(ExpectName());
      }
      Expect(":");
      // A LABEL IS NOT A VARIABLE, and AL declares them in the same block. A Label is a constant
      // string with a caption and translations behind it; everything else is state.
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

  /// Whether a variable declaration follows the bracket group the cursor sits on, WITHOUT
  /// consuming it.
  ///
  /// A `var` block ends where the next member begins, and the next member usually begins with its
  /// attributes: `var ... [Test] procedure X()`. Reading the attribute to find that out threw it
  /// away, and the procedure was then declared without it -- one lost `[Test]` per test codeunit,
  /// 67 of them, invisible because the count came from the same parser that had dropped them.
  [[nodiscard]] bool VariableFollowsAttribute() const {
    std::size_t ahead = 0;
    // ATTRIBUTES STACK: `[NonDebuggable] [WithEvents] AzureMLRequest: DotNet ...`. Skipping one
    // group and looking at the next `[` decided there was no variable and ended the var block.
    while (IsPunctuation(Peek(ahead), "[")) {
      int depth = 0;
      do {
        if (IsPunctuation(Peek(ahead), "[")) { ++depth; }
        if (IsPunctuation(Peek(ahead), "]")) { --depth; }
        ++ahead;
      } while (depth > 0 && Peek(ahead).kind != TokenKind::EndOfFile);
    }
    // AND ONE TYPE CAN CARRY SEVERAL NAMES: `[NonDebuggable] A, B : Text;`. Looking for a single
    // `name :` after the attributes missed those too.
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

  /// A body whose local declarations are thrown away -- a table trigger, or a member being skipped.
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
      Expect("{");
      while (!AtPunctuation("}")) {
        if (AtKeyword("trigger")) {
          Advance();
          Trigger trigger;
          trigger.name = ExpectName();
          Expect("(");
          Expect(")");
          trigger.tokens = SkipBeginEnd();
          trigger.body = ParseStatements(trigger.tokens);
          field.triggers.push_back(std::move(trigger));
        } else {
          field.properties.push_back(ParseProperty());
        }
      }
      Expect("}");
      table.fields.push_back(std::move(field));
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

  void ParseVars(TableObject &table) {
    while (!AtEnd() && !AtPunctuation("}") && !AtKeyword("var") && !AtKeyword("trigger") &&
           !AtKeyword("procedure") && !AtKeyword("local") && !AtKeyword("internal") &&
           !AtKeyword("protected")) {
      if (AtPunctuation("[")) {
        if (!VariableFollowsAttribute()) { return; }
        SkipAttribute();
        continue;
      }
      std::vector<std::string> names{ExpectName()};
      while (AtPunctuation(",")) {
        Advance();
        names.push_back(ExpectName());
      }
      Expect(":");
      const std::string type = ExpectName();
      if (SameName(type, "Label") && Peek().kind == TokenKind::String) {
        for (const std::string &name : names) {
          table.labels.push_back(LabelDecl{.name = name, .text = Peek().text});
        }
        Advance();
      }
      while (!AtEnd() && !AtPunctuation(";")) { Advance(); }
      Expect(";");
    }
  }

  std::vector<Token> tokens_;
  std::size_t position_ = 0;
};

} // namespace

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

bool HasAttribute(const ProcedureDecl &procedure, std::string_view name) {
  return std::ranges::any_of(procedure.attributes,
                             [name](const std::string &a) { return SameName(a, name); });
}

} // namespace agiru::al

#pragma once

#include "Expr.h"
#include "Token.h"

#include <string>
#include <string_view>
#include <vector>

namespace agiru::al {

struct Property {
  std::string name;
  std::vector<Token> value;
  std::string text;
};

struct KeyDecl {
  std::string name;
  std::vector<std::string> fields;
  std::vector<Property> properties;
};

struct LabelDecl {
  std::string name;
  std::string text;
};

struct VarDecl {
  bool byReference = false;
  bool temporary = false;
  std::string name;
  std::string type;
  std::string subtype;
  int length = 0;
  std::vector<std::string> members;
  std::vector<VarDecl> arguments;
  std::vector<int> dimensions;
};

using Parameter = VarDecl;

struct ProcedureDecl {
  std::vector<std::string> attributes;
  bool isLocal = false;
  bool isTrigger = false;
  std::string name;
  std::vector<Parameter> parameters;
  std::vector<VarDecl> variables;
  std::string returnName;
  std::string returnType;
  std::string returnSubtype;

  VarDecl returned;
  std::vector<Token> tokens;
  std::vector<Stmt> body;
};

using Trigger = ProcedureDecl;

struct FieldDecl {
  int number = 0;
  std::string name;
  std::string type;
  std::string subtype;
  int length = 0;
  std::vector<Property> properties;
  std::vector<ProcedureDecl> triggers;
};

struct CodeunitObject {
  int id = 0;
  std::string name;
  std::string nameSpace;
  std::vector<Property> properties;
  std::vector<ProcedureDecl> procedures;
  std::vector<VarDecl> variables;
  std::vector<LabelDecl> labels;
};

struct EnumValueDecl {
  int ordinal = 0;
  std::string name;
  std::vector<Property> properties;
};

struct EnumObject {
  int id = 0;
  std::string name;
  std::string nameSpace;
  std::vector<Property> properties;
  std::vector<EnumValueDecl> values;
};

struct InterfaceObject {
  int id = 0;
  std::string name;
  std::string nameSpace;
  std::vector<ProcedureDecl> procedures;
};

struct PageControl {
  std::string kind;
  std::string name;
  std::vector<Token> source;
  std::vector<Property> properties;
  std::vector<ProcedureDecl> triggers;
  std::vector<PageControl> children;
};

struct PageObject {
  int id = 0;
  std::string name;
  std::string nameSpace;
  std::vector<Property> properties;
  std::vector<PageControl> layout;
  std::vector<PageControl> actions;
  std::vector<ProcedureDecl> procedures;
  std::vector<VarDecl> variables;
  std::vector<LabelDecl> labels;
};

struct PageExtensionObject {
  int id = 0;
  std::string name;
  std::string extends;
  std::string nameSpace;
  std::vector<PageControl> layout;
  std::vector<PageControl> actions;
  std::vector<ProcedureDecl> procedures;
  std::vector<VarDecl> variables;
  std::vector<LabelDecl> labels;
};

struct EnumExtensionObject {
  int id = 0;
  std::string name;
  std::string extends;
  std::string nameSpace;
  std::vector<EnumValueDecl> values;
};

struct TableExtensionObject {
  int id = 0;
  std::string name;
  std::string extends;
  std::string nameSpace;
  std::vector<FieldDecl> fields;
  std::vector<FieldDecl> modified;
  std::vector<KeyDecl> keys;
  std::vector<LabelDecl> labels;
  std::vector<VarDecl> variables;
  std::vector<ProcedureDecl> procedures;
};

struct TableObject {
  int id = 0;
  std::string name;
  std::string nameSpace;
  std::vector<Property> properties;
  std::vector<FieldDecl> fields;
  std::vector<FieldDecl> modified;
  std::vector<KeyDecl> keys;
  std::vector<LabelDecl> labels;
  std::vector<VarDecl> variables;
  std::vector<ProcedureDecl> procedures;
};

const Property *Find(const std::vector<Property> &properties, std::string_view name);

const Trigger *Find(const std::vector<Trigger> &triggers, std::string_view name);

bool HasAttribute(const ProcedureDecl &procedure, std::string_view name);

std::vector<std::string> ListValue(const Property &property);

}

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

struct Trigger {
  std::string name;
  std::vector<Token> tokens;
  std::vector<Stmt> body;
};

struct FieldDecl {
  int number = 0;
  std::string name;
  std::string type;
  std::string subtype;
  int length = 0;
  std::vector<Property> properties;
  std::vector<Trigger> triggers;
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

struct TableObject {
  int id = 0;
  std::string name;
  std::string nameSpace;
  std::vector<Property> properties;
  std::vector<FieldDecl> fields;
  std::vector<KeyDecl> keys;
  std::vector<LabelDecl> labels;
};

const Property *Find(const std::vector<Property> &properties, std::string_view name);

const Trigger *Find(const std::vector<Trigger> &triggers, std::string_view name);

std::vector<std::string> ListValue(const Property &property);

} // namespace agiru::al

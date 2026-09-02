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

// AL DECLARES A PARAMETER AND A VARIABLE WITH THE SAME GRAMMAR, and the only difference is that a
// parameter may carry `var`. One shape for both, so that the generator emits them the same way.
struct VarDecl {
  bool byReference = false; ///< AL `var` -- only a parameter can be.
  bool temporary = false;   ///< AL `temporary` -- a record with no database behind it.
  std::string name;
  std::string type;
  std::string subtype;
  int length = 0;
  std::vector<std::string> members; ///< An inline `Option A,B,C` declares its own.
  /// `List of [Text]`, `Dictionary of [Text, Integer]` -- and they NEST: the BaseApp writes
  /// `Dictionary of [Integer, List of [Text]]` and `Dictionary of [Enum "X", Text]`. An argument is
  /// therefore a declaration and not a name, because a name cannot carry the inner arguments, the
  /// enum's subtype or a `Text[50]`'s length.
  std::vector<VarDecl> arguments;
};

using Parameter = VarDecl;

struct ProcedureDecl {
  std::vector<std::string> attributes;
  bool isLocal = false;
  bool isTrigger = false; ///< AL `trigger` -- the platform calls it, nobody else does.
  std::string name;
  std::vector<Parameter> parameters;
  std::vector<VarDecl> variables;
  std::string returnName;
  std::string returnType;
  std::string returnSubtype;

  /// The return's WHOLE declaration. `returnType` and `returnSubtype` are two of its fields, and
  /// rebuilding a `VarDecl` from just those two dropped the length of a `Text[50]`, the members of
  /// an inline `Option` and the arguments of a `Dictionary of [Text, Text]` -- which is a class
  /// template with no arguments, and not a type.
  VarDecl returned;
  std::vector<Token> tokens;
  std::vector<Stmt> body;
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

/// AL `interface` -- procedure SIGNATURES and no bodies.
///
/// It is the one AL object kind that maps onto C++ without a deviation: an abstract class with
/// pure virtual functions, and `codeunit X implements I` as inheritance (board:0027).
struct InterfaceObject {
  int id = 0;
  std::string name;
  std::string nameSpace;
  std::vector<ProcedureDecl> procedures;
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

bool HasAttribute(const ProcedureDecl &procedure, std::string_view name);

std::vector<std::string> ListValue(const Property &property);

} // namespace agiru::al

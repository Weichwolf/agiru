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
  /// AL `array[6] of Record "Dimension Value"` -- the dimensions, outermost first.
  ///
  /// \note THEY ARE PART OF THE SIGNATURE. `ERMDimensionShortcuts` declares `CreateDimSet` twice,
  ///       once over an `array[6] of Record "Dimension Value"` and once over one record; dropping
  ///       the dimension made them the same C++ member declared twice.
  std::vector<int> dimensions;
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

/// AL page layout and actions -- one node of the control tree.
///
/// THE TREE IS ONE SHAPE AND NOT TWENTY-FOUR. `field`, `group`, `area`, `part`, `repeater`,
/// `action`, `actionref`, `systempart`, `cuegroup`, `addafter` -- every one of them is
/// `<kind>(<name>[; <source>]) { <properties> <triggers> <children> }`, so the parser reads that
/// shape and records the KIND rather than growing a case per keyword. What separates a control from
/// a property is the token after the name: `(` opens a control, `=` opens a property.
struct PageControl {
  std::string kind;
  std::string name;
  std::vector<Token> source; ///< What follows the `;` inside the parentheses, verbatim.
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

/// AL `pageextension` -- controls, actions and code ADDED to a page that is declared elsewhere.
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

/// AL `enumextension` -- values ADDED to an enumeration that is declared elsewhere.
struct EnumExtensionObject {
  int id = 0;
  std::string name;
  std::string extends;
  std::string nameSpace;
  std::vector<EnumValueDecl> values;
};

/// AL `tableextension` -- fields, keys and code ADDED to a table that is declared elsewhere.
///
/// \note IT IS MERGED AT TRANSLATION TIME, NOT LINKED AT RUN TIME, and BC does the same: the added
///       columns land in the SAME SQL table. A C++ class is closed, so the alternative does not
///       exist -- which is what makes which apps are installed a transpile-time decision
///       (board:0033).
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
  /// What a `tableextension` MODIFIES rather than adds -- the field's own declaration is elsewhere,
  /// so only its name, its changed properties and its added triggers are here.
  std::vector<FieldDecl> modified;
  std::vector<KeyDecl> keys;
  std::vector<LabelDecl> labels;
  /// A TABLE CARRIES CODE, and AL says so: `Tracking Specification` declares
  /// `procedure SetSourceFilter(...)` beside its fields, and the BaseApp calls it on a record.
  std::vector<VarDecl> variables;
  std::vector<ProcedureDecl> procedures;
};

const Property *Find(const std::vector<Property> &properties, std::string_view name);

const Trigger *Find(const std::vector<Trigger> &triggers, std::string_view name);

bool HasAttribute(const ProcedureDecl &procedure, std::string_view name);

std::vector<std::string> ListValue(const Property &property);

} // namespace agiru::al

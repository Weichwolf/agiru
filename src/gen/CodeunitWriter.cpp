#include "CodeunitWriter.h"

#include "Ast.h"
#include "BodyWriter.h"
#include "EnumWriter.h"
#include "Names.h"
#include "Scope.h"
#include "Token.h"

#include <algorithm>
#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace agiru::gen {

namespace {

bool IsPublisher(const al::ProcedureDecl &procedure) {
  return al::HasAttribute(procedure, "IntegrationEvent") ||
         al::HasAttribute(procedure, "BusinessEvent") ||
         al::HasAttribute(procedure, "InternalEvent");
}

bool NamesAnObject(const al::VarDecl &declared) {
  const std::string type = TypeName(declared.type);
  return (type == "Record" || type == "Codeunit") && !declared.subtype.empty();
}

// A type as it stands in a DECLARATION. `Record "X"` is the generated class, `Record "X" temporary`
// is that class with no database behind it, `Text[50]` carries its length, and everything else is
// the door's own name for the AL type.
const TableRef *Reach(const al::VarDecl &declared, const Objects &objects) {
  const std::string type = TypeName(declared.type);
  const TableIndex &index = type == "Codeunit" ? objects.codeunits : objects.tables;
  const auto found = index.find(LowerKey(declared.subtype));
  return found != index.end() ? &found->second : nullptr;
}

// AN INLINE `Option A,B,C` DECLARES ITS OWN MEMBERS AND HAS NO NAME, so the generator gives it
// one: the codeunit, the procedure it stands in, and the variable. A table does the same for its
// own inline options, and the alternative -- an Integer -- would compile and lose the vocabulary
// `Type::All` is written in.
std::string InterfaceType(const al::VarDecl &declared, const Objects &objects) {
  const auto found = objects.interfaces.find(LowerKey(declared.subtype));
  return (found != objects.interfaces.end() ? found->second.identifier
                                            : "absent::" + Identifier(declared.subtype)) +
         " *";
}

/// Whether a declaration names a type that is neither AL's nor an object -- a bare platform enum.
///
/// AL writes the platform's own enums as bare type names: `Verbosity: Verbosity`,
/// `DataClassification: DataClassification`. Emitting the word unchanged put it beside the door,
/// where nothing declares it.
bool NamesAbsentType(const al::VarDecl &declared) {
  // AN EMPTY TYPE IS NOT A TYPE. A procedure with no return has one, and calling it absent named a
  // struct with no name at all.
  return !declared.type.empty() && !IsAlTypeName(declared.type) && declared.subtype.empty() &&
         declared.members.empty() && declared.arguments.empty();
}

std::string ObjectType(const al::VarDecl &declared, const Objects &objects) {
  const TableRef *ref = Reach(declared, objects);
  // AN OBJECT THIS RUN DOES NOT HAVE IS NAMED AS ABSENT. It used to be emitted as a bare
  // identifier, which named nothing; and putting the stub beside the real objects shadowed the AL
  // TYPES, because the virtual table `Integer` and the AL type `Integer` are one word.
  if (ref == nullptr) { return "absent::" + Identifier(declared.subtype); }
  return declared.temporary ? "Temporary<" + ref->identifier + ">" : ref->identifier;
}

std::string
TypeOf(const al::VarDecl &declared, const Objects &objects, const std::string &owner = {});

/// A generic's element types, each read by the same function that read the generic.
std::string Generic(const std::string &type,
                    const std::vector<al::VarDecl> &arguments,
                    const Objects &objects) {
  std::string out = type + "<";
  for (std::size_t i = 0; i < arguments.size(); ++i) {
    if (i != 0) { out += ", "; }
    out += TypeOf(arguments[i], objects);
  }
  return out + ">";
}

std::string TypeOf(const al::VarDecl &declared, const Objects &objects, const std::string &owner) {
  std::string type = TypeName(declared.type);
  if (type == "Record" || type == "Codeunit") { return ObjectType(declared, objects); }
  // AN ENUM VARIABLE NAMES ITS ENUMERATION, and without it `Enum` is a class template with no
  // arguments -- which is not a type at all. The index is the same one a table field uses.
  // A DotNet VARIABLE IS TYPED BY ITS SUBTYPE, and the subtype was being thrown away: every one of
  // the 7 117 declarations came out as the bare word `DotNet`, so the compiler said "unknown type
  // name 'DotNet'" 1 971 times and named none of the 577 classes actually wanted. A .NET class
  // cannot be papered over the way an enum ordinal can -- it is used by CALLING it, and no
  // placeholder carries 577 method sets -- so what this buys is not a compile, it is a MEASUREMENT:
  // the roots now name `dotnet::XmlDocument` and the classes can be ranked by what they stop.
  if (type == "DotNet") {
    return declared.subtype.empty() ? "dotnet::Unnamed" : "dotnet::" + Identifier(declared.subtype);
  }
  // AN ENUMERATION THIS RUN NEVER SAW BECOMES `Enum<>`, which carries the ordinal and names no
  // member. `Copilot Capability` exists in BCApps only as enumextensions of something that is not
  // there -- the platform declares it. Emitting `Enum<enums::CopilotCapability>` named a type
  // nobody declares and stopped the file; inventing ordinals from the extensions would be a wrong
  // number that looks like a right one. The transpiler names every unresolved enumeration in its
  // summary, so this is reported rather than swallowed.
  if (type == "Enum") {
    if (declared.subtype.empty()) { return "Enum<>"; }
    const auto found = objects.enums.find(LowerKey(declared.subtype));
    return found != objects.enums.end() ? "Enum<enums::" + found->second.identifier + ">"
                                        : "Enum<>";
  }
  // AN OPTION WITH NO MEMBERS IS AL'S OWN DECLARATION AND NOT A GAP. `procedure P(ChangeType:
  // Option)` takes any option value at all, and the BaseApp calls it with a member of some other
  // enumeration entirely. `Option<>` is that: the ordinal, without a vocabulary.
  // AN INTERFACE VARIABLE HOLDS A POINTER TO THE ABSTRACT CLASS. AL writes `I.Method()` and C++
  // writes `I->Method()`, which is the deviation board:0027 names: an interface variable IS a
  // handle to something else, and 822 signatures cannot be forwarded by a wrapper.
  if (type == "Interface") { return InterfaceType(declared, objects); }
  if (type == "Option") {
    return declared.members.empty() || owner.empty() ? "Option<>" : "Option<" + owner + ">";
  }
  // `List of [Text]` and `Dictionary of [Text, Integer]` carry their element types with them, AND
  // THOSE NEST: `Dictionary of [Integer, List of [Text]]` is one the BaseApp writes. An argument is
  // read by this same function, so an inner generic, an enum's subtype and a `Text[50]`'s length
  // all survive.
  if ((type == "List" || type == "Dictionary") && !declared.arguments.empty()) {
    return Generic(type, declared.arguments, objects);
  }

  // A NAME THAT IS NEITHER AN AL TYPE NOR AN OBJECT THIS RUN READ IS ABSENT. AL writes the
  // platform's own enums as bare type names -- `Verbosity: Verbosity` -- and emitting the word
  // unchanged put it beside the door, where nothing declares it.
  if (NamesAbsentType(declared)) { return "absent::" + Identifier(declared.type); }
  // A BARE `Text` IS UNBOUNDED, and it lands here as a length of zero -- which the string types
  // read as no limit rather than a limit of nothing. `Text[50]` carries its 50.
  if (type == "Code" || type == "Text") {
    return type + "<" + std::to_string(declared.length) + ">";
  }
  return type;
}

// A PARAMETER MAY BE NAMED AFTER ITS TYPE, and AL writes it constantly: `Item: Record Item`,
// `SalesLine: Record "Sales Line"`. In C++ the name then hides the class and the declaration needs
// an elaborated specifier or a qualification. Qualified, because `class Item &Item` reads like a
// C++ puzzle and `agiru::app::Item &Item` reads like what it is.
std::string
OptionName(const std::string &unit, const std::string &within, const std::string &name) {
  return Identifier(unit) + Identifier(within) + Identifier(name);
}

// THE NAMESPACE IS DECIDED BY WHAT THE TYPE IS, and getting it wrong cost more than anything else
// in this tree: `agiru::app::RecordRef` was the FIRST diagnostic of 1 375 of 3 123 failing headers,
// measured 2026-09-02, because `LibraryAssert` writes `RecordRef: RecordRef` and one bad
// qualification in it stops every header that includes it. An AL object -- a Record or a Codeunit
// -- becomes a class in `agiru::app`; every other AL type is a DOOR type and lives in `agiru`.
std::string Qualified(const al::VarDecl &declared, const std::string &type) {
  const std::string alType = TypeName(declared.type);
  const bool object = alType == "Record" || alType == "Codeunit";
  return (object ? "agiru::app::" : "agiru::") + type;
}

std::string
Signature(const al::VarDecl &declared, const Objects &objects, const std::string &owner = {}) {
  std::string type = TypeOf(declared, objects, owner);
  if (type == Identifier(declared.name)) { type = Qualified(declared, type); }
  return type + (declared.byReference ? " &" : " ");
}

std::string Parameters(const al::ProcedureDecl &procedure,
                       const Objects &objects,
                       bool named,
                       const std::string &unit) {
  std::string out;
  for (std::size_t i = 0; i < procedure.parameters.size(); ++i) {
    if (i != 0) { out += ", "; }
    out += Signature(procedure.parameters[i],
                     objects,
                     OptionName(unit, procedure.name, procedure.parameters[i].name));
    if (named) { out += Identifier(procedure.parameters[i].name); }
  }
  return out;
}

/// Every inline option a codeunit declares, as its own enumeration with its own traits.
std::string InlineOptions(const al::CodeunitObject &unit) {
  const std::string identifier = Identifier(unit.name);
  std::string out;
  // AN OVERLOAD DECLARES THE SAME INLINE OPTION TWICE. `UserPermissionsImpl` has two
  // `GetEffectivePermission`, both taking `PermissionObjectType: Option "Table Data",...` -- one
  // AL parameter written twice, and the generated name is built from the codeunit, the procedure
  // and the parameter, so it is the same name both times. It is ONE enumeration; emitting it twice
  // is a redefinition.
  //
  // THE MEMBERS DECIDE, not the count. Two options that share a generated name and differ in what
  // they declare are two enumerations wearing one name, and that is a translation error rather
  // than something to pick a winner for.
  std::map<std::string, std::vector<std::string>> emitted;
  const auto declare = [&](const std::string &within, const al::VarDecl &declared) {
    if (TypeName(declared.type) != "Option" || declared.members.empty()) { return; }
    const std::string name = OptionName(unit.name, within, declared.name);
    const auto seen = emitted.find(name);
    if (seen != emitted.end()) {
      if (seen->second != declared.members) {
        throw std::runtime_error("codeunit \"" + unit.name +
                                 "\" declares two different options "
                                 "under the name " +
                                 name);
      }
      return;
    }
    emitted.insert_or_assign(name, declared.members);
    const std::vector<std::string> names = EnumeratorNames(declared.members);
    out += "namespace agiru::app::codeunits {\n\nenum class " + name + " : std::int32_t {\n";
    for (std::size_t i = 0; i < names.size(); ++i) {
      out += "  " + names[i] + " = " + std::to_string(i) + ",\n";
    }
    out += "};\n\n} // namespace agiru::app::codeunits\n\n";
    out += "template <> struct agiru::OptionTraits<agiru::app::codeunits::" + name + "> {\n";
    out += "  static constexpr std::array<EnumValueDef, " +
           std::to_string(declared.members.size()) + "> kValues{{\n";
    for (std::size_t i = 0; i < declared.members.size(); ++i) {
      out += "      EnumValueDef{.ordinal = " + std::to_string(i) +
             ", .name = " + Literal(declared.members[i]) +
             ", .caption = " + Literal(declared.members[i]) + "},\n";
    }
    out += "  }};\n};\n\n";
  };
  static_cast<void>(identifier);
  for (const al::VarDecl &declared : unit.variables) { declare(std::string{}, declared); }
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { declare(procedure.name, declared); }
    for (const al::VarDecl &declared : procedure.variables) { declare(procedure.name, declared); }
  }
  return out;
}

std::string Returns(const al::ProcedureDecl &procedure, const Objects &objects) {
  if (procedure.returnType.empty()) { return "void"; }
  return TypeOf(procedure.returned, objects);
}

std::string
Declaration(const al::ProcedureDecl &procedure, const Objects &objects, const std::string &unit) {
  return "  " + Returns(procedure, objects) + " " + Identifier(procedure.name) + "(" +
         Parameters(procedure, objects, true, unit) + ");\n";
}

bool NamesAbsent(const al::CodeunitObject &unit, const Objects &objects) {
  const auto absent = [&objects](const al::VarDecl &declared) {
    const std::string type = TypeName(declared.type);
    if (type == "DotNet") { return true; }
    return (type == "Record" || type == "Codeunit") && !declared.subtype.empty() &&
           Reach(declared, objects) == nullptr;
  };
  if (std::ranges::any_of(unit.variables, absent)) { return true; }
  return std::ranges::any_of(unit.procedures, [&absent](const al::ProcedureDecl &procedure) {
    return std::ranges::any_of(procedure.parameters, absent) ||
           std::ranges::any_of(procedure.variables, absent);
  });
}

std::string SourceIncludes(const al::CodeunitObject &unit, const Objects &objects) {
  std::set<std::string> headers;
  const auto reach = [&](const al::VarDecl &declared) {
    if (!NamesAnObject(declared)) { return; }
    const TableRef *ref = Reach(declared, objects);
    if (ref != nullptr && !ref->header.empty()) { headers.insert(ref->header); }
  };
  const auto reachInterface = [&](const al::VarDecl &declared) {
    if (TypeName(declared.type) != "Interface") { return; }
    const auto found = objects.interfaces.find(LowerKey(declared.subtype));
    if (found != objects.interfaces.end() && !found->second.header.empty()) {
      headers.insert(found->second.header);
    }
  };
  const auto named = [&](const al::VarDecl &declared) {
    reach(declared);
    reachInterface(declared);
  };
  for (const al::VarDecl &declared : unit.variables) { named(declared); }
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { named(declared); }
    for (const al::VarDecl &declared : procedure.variables) { named(declared); }
    named(procedure.returned);
  }
  std::string out;
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }
  return out;
}

/// What a declaration needs the header to KNOW: a name for an object, a name for an interface --
/// which is a pointer -- and the enumeration itself, which is a template argument and not a name.
template <typename Ahead, typename Enum>
void Declared(const al::VarDecl &declared, const Objects &objects, Ahead ahead, Enum reachEnum) {
  if (NamesAnObject(declared)) {
    const TableRef *ref = Reach(declared, objects);
    if (ref != nullptr) { ahead(ref->identifier); }
  }
  if (TypeName(declared.type) == "Interface") {
    const auto found = objects.interfaces.find(LowerKey(declared.subtype));
    if (found != objects.interfaces.end()) { ahead(found->second.identifier); }
  }
  if (TypeName(declared.type) == "Enum") { reachEnum(declared.subtype); }
}

/// Adds the header of an enumeration, when the run has one.
///
/// AN ENUM NEEDS ITS HEADER even where an object needs only its name: `Enum<enums::X>` is a
/// TEMPLATE ARGUMENT and a template argument must be complete. Missing it made `LibraryNoSeries`
/// the first diagnostic of 1 159 failing headers.
void EnumHeader(const EnumIndex &enums,
                const std::string &subtype,
                std::set<std::string> &headers) {
  if (subtype.empty()) { return; }
  const auto found = enums.find(LowerKey(subtype));
  if (found != enums.end() && !found->second.header.empty()) {
    headers.insert(found->second.header);
  }
}

std::string Includes(const al::CodeunitObject &unit, const Objects &objects) {
  std::set<std::string> headers;
  const auto reach = [&](const al::VarDecl &declared) {
    if (!NamesAnObject(declared)) { return; }
    const TableRef *ref = Reach(declared, objects);
    // A PLATFORM TABLE HAS NO HEADER OF ITS OWN TO NAME: it arrives with the door, so its entry
    // carries an empty path and an empty path would emit `#include ""`.
    if (ref != nullptr && !ref->header.empty()) { headers.insert(ref->header); }
  };
  // AN ENUM NEEDS ITS HEADER TOO, and it was the only kind of object this did not ask for:
  // `LibraryNoSeries` names `Enum<enums::NoSeriesImplementation>` and included the table beside it
  // but not the enumeration, which made it the FIRST diagnostic of 1 159 failing headers.
  const auto reachEnum = [&](const std::string &subtype) {
    EnumHeader(objects.enums, subtype, headers);
  };
  // AN INTERFACE NEEDS ITS HEADER TOO, and it is the third kind this walk has had to learn: a
  // variable names it, the abstract class is somewhere else, and nothing else pulls it in.
  std::map<std::string, std::set<std::string>> forward;
  const auto ahead = [&forward](const std::string &qualified) {
    const std::size_t colons = qualified.find("::");
    if (colons == std::string::npos) { return; }
    forward[qualified.substr(0, colons)].insert(qualified.substr(colons + 2));
  };
  // INCLUDE WHAT YOU CONTAIN, DECLARE WHAT YOU NAME -- AND A HEADER CONTAINS ONLY ITS MEMBERS.
  // A function DECLARATION may take and return incomplete types; only a definition or a call needs
  // the layout, and both of those live in the `.cpp`. Including for parameters too made the include
  // graph far larger than the containment graph and produced cycles the containment graph does not
  // have: `Language` holds `LanguageImpl` and `LanguageImpl` holds no codeunit at all, yet their
  // headers included each other because a parameter named the other.
  const auto both = [&](const al::VarDecl &declared) {
    Declared(declared, objects, ahead, reachEnum);
  };
  // A MEMBER IS A GLOBAL, so this is the containment graph and nothing else.
  for (const al::VarDecl &declared : unit.variables) { reach(declared); }
  for (const al::VarDecl &declared : unit.variables) { both(declared); }
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { both(declared); }
    // AND A PROCEDURE'S OWN VARIABLES, which were never walked at all: a local `Record` or `Enum`
    // is as much a declaration as a parameter is.
    for (const al::VarDecl &declared : procedure.variables) { both(declared); }
    if (TypeName(procedure.returnType) == "Enum") { reachEnum(procedure.returnSubtype); }
  }
  // A CODEUNIT THAT NAMES A .NET TYPE INCLUDES THE GENERATED SURFACE. It is one file for all of
  // them, because the stubs reference nothing but `Refused` and splitting them would be 499 headers
  // for no reader's benefit.
  std::string out = "#include \"agiru.h\"\n";
  // ONLY WHERE SOMETHING IS ACTUALLY ABSENT. An unconditional include would make every generated
  // codeunit depend on a file that exists because something is MISSING, which is the wrong way
  // round -- and it would make the hand-written target image depend on a transpiler run.
  if (NamesAbsent(unit, objects)) { out += "#include \"absent/Types.h\"\n"; }
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }

  // AL LETS TWO CODEUNITS NAME EACH OTHER, AND C++ HEADERS CANNOT. `AOAIFunctionResponse` includes
  // `AOAIChatMessages` and `AOAIChatMessages` includes it back; `#pragma once` makes the second
  // include a no-op, so the inner file sees the outer one half-written and the type is not there.
  // A forward declaration of every object this file NAMES settles it, because a parameter taken by
  // reference needs the name and not the layout -- and AL passes objects by `var` where it passes
  // them at all.
  if (!forward.empty()) { out += "\n"; }
  for (const auto &[space, named] : forward) {
    out += "namespace agiru::app::" + space + " {\n";
    for (const std::string &one : named) { out += "class " + one + ";\n"; }
    out += "} // namespace agiru::app::" + space + "\n";
  }
  out += "\n";
  return out;
}

// THE .NET SURFACE EXISTS ONLY AT THE CALL SITES. A `dotnet` package declares an assembly and a
// type alias and no members at all, so nothing in the AL source says what `XmlDocument` can do --
// only what this corpus ASKS it to do. That set is gathered here, per type, from the tokens of
// every body: a name that is a DotNet variable, a dot, and the member after it (board:0035).
//
// TOKENS AND NOT THE EXPRESSION TREE, because a property read and a call are the same thing here
// and the token pair is the same for both -- `UserInfo.ObjectId` and `Doc.SelectNodes(x)` differ
// only in what follows, which this does not need.
using DotNetNames = std::map<std::string, std::string>;

void NoteDotNet(const al::VarDecl &declared, DotNetNames &named) {
  if (TypeName(declared.type) == "DotNet" && !declared.subtype.empty()) {
    named.insert_or_assign(LowerKey(declared.name), Identifier(declared.subtype));
  }
}

void GatherCalls(const al::ProcedureDecl &procedure, const DotNetNames &named, DotNetUse &use) {
  for (std::size_t i = 0; i + 2 < procedure.tokens.size(); ++i) {
    if (procedure.tokens[i].kind != al::TokenKind::Identifier) { continue; }
    if (procedure.tokens[i + 1].text != ".") { continue; }
    if (procedure.tokens[i + 2].kind != al::TokenKind::Identifier) { continue; }
    const auto found = named.find(LowerKey(procedure.tokens[i].text));
    if (found != named.end()) {
      use[found->second].insert(Identifier(procedure.tokens[i + 2].text));
    }
  }
}

// AN AL OBJECT THIS RUN DOES NOT HAVE IS THE SAME QUESTION AS A .NET TYPE. `Record "Windows
// Language"` names a platform table no source root declares, so the generator emitted the bare
// identifier and the file stopped. What the corpus asks of it is in the call sites, exactly as it
// is for `DotNet` -- so it is gathered by the same walk and answered by the same shape.
// A TYPE THAT IS NAMED BUT NEVER ASKED ANYTHING STILL NEEDS TO EXIST. `Verbosity` appears as a
// parameter type and nothing is ever called on it, so gathering only members would leave the
// declaration pointing at nothing. The entry is created empty and filled if a member turns up.
void NoteAbsent(const al::VarDecl &declared,
                const Objects &objects,
                DotNetNames &named,
                DotNetUse &use) {
  if (NamesAbsentType(declared)) {
    const std::string bare = Identifier(declared.type);
    named.insert_or_assign(LowerKey(declared.name), bare);
    use.try_emplace(bare);
    return;
  }
  const std::string type = TypeName(declared.type);
  if (type != "Record" && type != "Codeunit") { return; }
  if (declared.subtype.empty() || Reach(declared, objects) != nullptr) { return; }
  const std::string subtype = Identifier(declared.subtype);
  named.insert_or_assign(LowerKey(declared.name), subtype);
  use.try_emplace(subtype);
}

void GatherDotNet(const al::CodeunitObject &unit,
                  const Objects &objects,
                  DotNetUse &use,
                  DotNetUse &absent) {
  DotNetNames named;
  DotNetNames missing;
  for (const al::VarDecl &declared : unit.variables) {
    NoteDotNet(declared, named);
    NoteAbsent(declared, objects, missing, absent);
  }

  for (const al::ProcedureDecl &procedure : unit.procedures) {
    DotNetNames inner = named;
    DotNetNames innerMissing = missing;
    for (const al::VarDecl &declared : procedure.parameters) {
      NoteDotNet(declared, inner);
      NoteAbsent(declared, objects, innerMissing, absent);
    }
    for (const al::VarDecl &declared : procedure.variables) {
      NoteDotNet(declared, inner);
      NoteAbsent(declared, objects, innerMissing, absent);
    }
    GatherCalls(procedure, inner, use);
    GatherCalls(procedure, innerMissing, absent);
  }
}

std::vector<std::string> Unresolved(const al::CodeunitObject &unit, const Objects &objects) {
  std::vector<std::string> missing;
  const auto note = [&](const al::VarDecl &declared) {
    if (!NamesAnObject(declared) || Reach(declared, objects) != nullptr) { return; }
    if (std::ranges::find(missing, declared.subtype) == missing.end()) {
      missing.push_back(declared.subtype);
    }
  };
  for (const al::VarDecl &declared : unit.variables) { note(declared); }
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { note(declared); }
  }
  return missing;
}

// A PROCEDURE'S SCOPE, INNERMOST FIRST: its own locals, then its parameters, then the codeunit's
// variables, labels and other procedures. AL resolves a bare name that way and so does C++ once the
// locals are declared, so every one of them spells itself -- what this settles is that the name IS
// known, which is what keeps an unknown one from being emitted as if it were.
class CodeunitNames : public Names {
public:
  CodeunitNames(const al::CodeunitObject &unit, const al::ProcedureDecl &procedure)
      : unit_(unit), procedure_(procedure) {}

  [[nodiscard]] std::string Resolve(std::string_view name) const override {
    for (const al::VarDecl &declared : procedure_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) { return Identifier(name); }
    }
    for (const al::VarDecl &declared : procedure_.parameters) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) { return Identifier(name); }
    }
    if (!procedure_.returnName.empty() &&
        LowerKey(procedure_.returnName) == LowerKey(std::string(name))) {
      return Identifier(name);
    }
    for (const al::VarDecl &declared : unit_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) { return Identifier(name); }
    }
    for (const al::LabelDecl &label : unit_.labels) {
      if (LowerKey(label.name) == LowerKey(std::string(name))) { return Identifier(label.name); }
    }
    for (const al::ProcedureDecl &other : unit_.procedures) {
      if (LowerKey(other.name) == LowerKey(std::string(name))) { return Identifier(other.name); }
    }
    return {};
  }

  /// \note EMPTY UNTIL A VARIABLE'S ENUM CAN BE NAMED. `X::Member` where `X` is an option or enum
  ///       variable needs the enumeration that declared it, which is an index this writer does not
  ///       carry yet. Returning nothing emits `X::Member` against a variable, which does not
  ///       compile -- loud, and at build time, rather than a plausible wrong type.
  [[nodiscard]] std::string Enumeration(std::string_view name) const override {
    static_cast<void>(name);
    return {};
  }

private:
  const al::CodeunitObject &unit_;
  const al::ProcedureDecl &procedure_;
};

std::string
Locals(const al::ProcedureDecl &procedure, const Objects &objects, const std::string &unit) {
  std::string out;
  // THE NAMED RETURN VALUE IS A LOCAL, and it comes first because AL declares it in the signature,
  // ahead of the var block. `exit;` with no argument returns it, zero-initialised if nothing wrote.
  if (!procedure.returnName.empty()) {
    out += "  " + Returns(procedure, objects) + " " + Identifier(procedure.returnName) + "{};\n";
  }
  for (const al::VarDecl &declared : procedure.variables) {
    out += "  " + TypeOf(declared, objects, OptionName(unit, procedure.name, declared.name)) + " " +
           Identifier(declared.name) + "{};\n";
  }
  return out;
}

} // namespace

std::string WriteCodeunitSource(const al::CodeunitObject &unit,
                                const std::string &sourcePath,
                                const Objects &objects) {
  const std::string identifier = Identifier(unit.name);
  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#include \"" + identifier + ".h\"\n\n";
  out += "#include \"agiru.h\"\n";
  // THE SOURCE INCLUDES WHAT THE HEADER ONLY DECLARED. The header carries the containment graph and
  // forward-declares everything else, which is what keeps its include graph acyclic; a BODY calls
  // methods on those objects and needs their layout, so the definitions bring them in. Header
  // declares, source includes -- and the cycle the header cannot have, the source can, because a
  // `.cpp` is nobody's dependency.
  out += SourceIncludes(unit, objects);
  out += "\nnamespace agiru::app::codeunits {\n\n";

  for (const al::ProcedureDecl &procedure : unit.procedures) {
    // AN EVENT PUBLISHER'S BODY IS EMPTY BY DESIGN -- the platform fires subscribers at the CALL
    // SITE -- so its definition has nothing to do with its parameters and drops their names.
    const bool publisher = IsPublisher(procedure);
    out += Returns(procedure, objects) + " " + identifier + "::" + Identifier(procedure.name) +
           "(" + Parameters(procedure, objects, !publisher, unit.name) + ") {";
    const std::string locals = publisher ? std::string{} : Locals(procedure, objects, unit.name);
    const std::string body =
        publisher ? std::string{}
                  : WriteStatements(CodeunitNames(unit, procedure), procedure.body, 2);
    if (locals.empty() && body.empty()) {
      out += "}\n\n";
      continue;
    }
    out += "\n";
    out += locals;
    if (!locals.empty() && !body.empty()) { out += "\n"; }
    out += body;
    out += "}\n\n";
  }

  out += "} // namespace agiru::app::codeunits\n";
  return out;
}

std::string CodeunitHeaderPath(const al::CodeunitObject &unit) {
  return OutputDirectory(unit.nameSpace, ObjectKind::Codeunit) + "/" + Identifier(unit.name) + ".h";
}

TableIndex PlatformTables() {
  TableIndex tables;
  tables.insert_or_assign("field", TableRef{.identifier = "platform::Field", .header = {}});
  tables.insert_or_assign("2000000041", TableRef{.identifier = "platform::Field", .header = {}});
  return tables;
}

InterfaceHeader WriteInterface(const al::InterfaceObject &object,
                               const std::string &sourcePath,
                               const Objects &objects) {
  const std::string identifier = Identifier(object.name);
  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n#pragma once\n\n#include \"agiru.h\"\n";
  // AN INTERFACE INCLUDES WHAT ITS SIGNATURES NAME, exactly as a codeunit includes what its
  // declarations name -- a signature is a declaration and a `Verbosity` in one is an enum the file
  // has to have seen.
  std::set<std::string> headers;
  const auto reach = [&](const al::VarDecl &declared) {
    if (NamesAnObject(declared)) {
      const TableRef *ref = Reach(declared, objects);
      if (ref != nullptr && !ref->header.empty()) { headers.insert(ref->header); }
    }
    if (TypeName(declared.type) == "Enum" && !declared.subtype.empty()) {
      const auto found = objects.enums.find(LowerKey(declared.subtype));
      if (found != objects.enums.end() && !found->second.header.empty()) {
        headers.insert(found->second.header);
      }
    }
  };
  for (const al::ProcedureDecl &procedure : object.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { reach(declared); }
    reach(procedure.returned);
  }
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }
  out += "\nnamespace agiru::app::interfaces {\n\n";
  out += "class " + identifier + " {\n";
  out += "public:\n";
  // A CLASS SOMEBODY DERIVES FROM NEEDS A VIRTUAL DESTRUCTOR, and an interface is only ever
  // derived from.
  out += "  virtual ~" + identifier + "() = default;\n\n";
  for (const al::ProcedureDecl &procedure : object.procedures) {
    out += "  virtual " + Returns(procedure, objects) + " " + Identifier(procedure.name) + "(" +
           Parameters(procedure, objects, true, object.name) + ") = 0;\n";
  }
  out += "};\n\n} // namespace agiru::app::interfaces\n";
  DotNetUse missing;
  DotNetNames named;
  for (const al::ProcedureDecl &procedure : object.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) {
      NoteAbsent(declared, objects, named, missing);
    }
    NoteAbsent(procedure.returned, objects, named, missing);
  }
  return InterfaceHeader{.text = out, .absent = std::move(missing)};
}

CodeunitHeader WriteCodeunit(const al::CodeunitObject &unit,
                             const std::string &sourcePath,
                             const Objects &objects) {
  const std::string identifier = Identifier(unit.name);

  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#pragma once\n\n";
  out += Includes(unit, objects);
  out += "#include <array>\n#include <cstdint>\n#include <string_view>\n\n";
  out += InlineOptions(unit);

  out += "namespace agiru::app::codeunits {\n\n";
  out += "class " + identifier + " : public Codeunit<" + identifier + "> {\npublic:\n";

  // PUBLIC BEFORE PRIVATE, AND `local` IS WHAT DECIDES IT. AL's `local procedure` is exactly C++'s
  // private, and an event publisher is local too -- nobody calls it but the object that raises it.
  //
  // THE ORDER IS AL'S ORDER, with a blank line where the KIND changes. A `trigger` is called by the
  // platform and a `procedure` by whoever holds the object; AL says which, and a reader scanning
  // the class wants that boundary. Reordering to group them would read less like the .al than the
  // blank line costs.
  bool previousWasTrigger = false;
  bool first = true;
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    if (procedure.isLocal) { continue; }
    if (!first && previousWasTrigger != procedure.isTrigger) { out += "\n"; }
    out += Declaration(procedure, objects, unit.name);
    previousWasTrigger = procedure.isTrigger;
    first = false;
  }

  std::string hidden;
  for (const al::VarDecl &declared : unit.variables) {
    hidden += "  " + TypeOf(declared, objects, OptionName(unit.name, {}, declared.name)) + " " +
              Identifier(declared.name) + ";\n";
  }
  if (!unit.labels.empty() && !hidden.empty()) { hidden += "\n"; }
  for (const al::LabelDecl &label : unit.labels) {
    hidden += "  static constexpr std::string_view " + Identifier(label.name) + "{" +
              Literal(label.text) + "};\n";
  }
  std::string locals;
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    if (!procedure.isLocal) { continue; }
    locals += Declaration(procedure, objects, unit.name);
  }
  if (!hidden.empty() || !locals.empty()) {
    out += "\nprivate:\n";
    out += hidden;
    if (!hidden.empty() && !locals.empty()) { out += "\n"; }
    out += locals;
  }
  out += "};\n\n";
  out += "} // namespace agiru::app::codeunits\n\n";

  out += "template <> struct agiru::CodeunitTraits<agiru::app::codeunits::" + identifier + "> {\n";
  out += "  static constexpr CodeunitId kId{" + std::to_string(unit.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(unit.name) + "};\n";
  out += "};\n";
  DotNetUse dotnet;
  DotNetUse absent;
  GatherDotNet(unit, objects, dotnet, absent);
  return CodeunitHeader{.text = out,
                        .unresolvedTables = Unresolved(unit, objects),
                        .dotnet = std::move(dotnet),
                        .absent = std::move(absent)};
}

} // namespace agiru::gen

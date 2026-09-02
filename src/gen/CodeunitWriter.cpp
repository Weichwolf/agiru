#include "CodeunitWriter.h"

#include "Ast.h"
#include "BodyWriter.h"
#include "EnumWriter.h"
#include "Names.h"
#include "Scope.h"

#include <algorithm>
#include <cstddef>
#include <set>
#include <string>
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

std::string TypeOf(const al::VarDecl &declared, const Objects &objects) {
  std::string type = TypeName(declared.type);
  if (type == "Record" || type == "Codeunit") {
    const TableRef *ref = Reach(declared, objects);
    const std::string named = ref != nullptr ? ref->identifier : Identifier(declared.subtype);
    return declared.temporary ? "Temporary<" + named + ">" : named;
  }
  // AN ENUM VARIABLE NAMES ITS ENUMERATION, and without it `Enum` is a class template with no
  // arguments -- which is not a type at all. The index is the same one a table field uses.
  if (type == "Enum" && !declared.subtype.empty()) {
    const auto found = objects.enums.find(LowerKey(declared.subtype));
    return "Enum<enums::" +
           (found != objects.enums.end() ? found->second.identifier
                                         : Identifier(declared.subtype)) +
           ">";
  }
  // AN INLINE `Option A,B,C` DECLARES ITS OWN MEMBERS AND HAS NO NAME. A table's version becomes a
  // generated enumeration beside the table; a procedure's has nowhere to live yet, so it is left as
  // the bare template -- which does not compile, loudly, rather than becoming an Integer that
  // silently loses the member names.

  if (type == "Code" || type == "Text") {
    return type + "<" + std::to_string(declared.length) + ">";
  }
  return type;
}

// A PARAMETER MAY BE NAMED AFTER ITS TYPE, and AL writes it constantly: `Item: Record Item`,
// `SalesLine: Record "Sales Line"`. In C++ the name then hides the class and the declaration needs
// an elaborated specifier or a qualification. Qualified, because `class Item &Item` reads like a
// C++ puzzle and `agiru::app::Item &Item` reads like what it is.
std::string Signature(const al::VarDecl &declared, const Objects &objects) {
  std::string type = TypeOf(declared, objects);
  if (type == Identifier(declared.name)) { type = "agiru::app::" + type; }
  return type + (declared.byReference ? " &" : " ");
}

std::string Parameters(const al::ProcedureDecl &procedure, const Objects &objects, bool named) {
  std::string out;
  for (std::size_t i = 0; i < procedure.parameters.size(); ++i) {
    if (i != 0) { out += ", "; }
    out += Signature(procedure.parameters[i], objects);
    if (named) { out += Identifier(procedure.parameters[i].name); }
  }
  return out;
}

std::string Returns(const al::ProcedureDecl &procedure, const Objects &objects) {
  if (procedure.returnType.empty()) { return "void"; }
  const al::VarDecl returned{.byReference = false,
                             .temporary = false,
                             .name = {},
                             .type = procedure.returnType,
                             .subtype = procedure.returnSubtype,
                             .length = 0};
  return TypeOf(returned, objects);
}

std::string Declaration(const al::ProcedureDecl &procedure, const Objects &objects) {
  return "  " + Returns(procedure, objects) + " " + Identifier(procedure.name) + "(" +
         Parameters(procedure, objects, true) + ");\n";
}

std::string Includes(const al::CodeunitObject &unit, const Objects &objects) {
  std::set<std::string> headers;
  const auto reach = [&](const al::VarDecl &declared) {
    if (!NamesAnObject(declared)) { return; }
    const TableRef *ref = Reach(declared, objects);
    if (ref != nullptr) { headers.insert(ref->header); }
  };
  for (const al::VarDecl &declared : unit.variables) { reach(declared); }
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { reach(declared); }
  }
  std::string out = "#include \"agiru.h\"\n";
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }
  out += "\n";
  return out;
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

std::string Locals(const al::ProcedureDecl &procedure, const Objects &objects) {
  std::string out;
  // THE NAMED RETURN VALUE IS A LOCAL, and it comes first because AL declares it in the signature,
  // ahead of the var block. `exit;` with no argument returns it, zero-initialised if nothing wrote.
  if (!procedure.returnName.empty()) {
    out += "  " + Returns(procedure, objects) + " " + Identifier(procedure.returnName) + "{};\n";
  }
  for (const al::VarDecl &declared : procedure.variables) {
    out += "  " + TypeOf(declared, objects) + " " + Identifier(declared.name) + "{};\n";
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
  out += "#include \"agiru.h\"\n\n";
  out += "namespace agiru::app {\n\n";

  for (const al::ProcedureDecl &procedure : unit.procedures) {
    // AN EVENT PUBLISHER'S BODY IS EMPTY BY DESIGN -- the platform fires subscribers at the CALL
    // SITE -- so its definition has nothing to do with its parameters and drops their names.
    const bool publisher = IsPublisher(procedure);
    out += Returns(procedure, objects) + " " + identifier + "::" + Identifier(procedure.name) +
           "(" + Parameters(procedure, objects, !publisher) + ") {";
    const std::string locals = publisher ? std::string{} : Locals(procedure, objects);
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

  out += "} // namespace agiru::app\n";
  return out;
}

std::string CodeunitHeaderPath(const al::CodeunitObject &unit) {
  return OutputDirectory(unit.nameSpace, ObjectKind::Codeunit) + "/" + Identifier(unit.name) + ".h";
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
  out += "#include <string_view>\n\n";

  out += "namespace agiru::app {\n\n";
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
    out += Declaration(procedure, objects);
    previousWasTrigger = procedure.isTrigger;
    first = false;
  }

  std::string hidden;
  for (const al::VarDecl &declared : unit.variables) {
    hidden += "  " + TypeOf(declared, objects) + " " + Identifier(declared.name) + ";\n";
  }
  if (!unit.labels.empty() && !hidden.empty()) { hidden += "\n"; }
  for (const al::LabelDecl &label : unit.labels) {
    hidden += "  static constexpr std::string_view " + Identifier(label.name) + "{" +
              Literal(label.text) + "};\n";
  }
  std::string locals;
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    if (!procedure.isLocal) { continue; }
    locals += Declaration(procedure, objects);
  }
  if (!hidden.empty() || !locals.empty()) {
    out += "\nprivate:\n";
    out += hidden;
    if (!hidden.empty() && !locals.empty()) { out += "\n"; }
    out += locals;
  }
  out += "};\n\n";
  out += "} // namespace agiru::app\n\n";

  out += "template <> struct agiru::CodeunitTraits<agiru::app::" + identifier + "> {\n";
  out += "  static constexpr CodeunitId kId{" + std::to_string(unit.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(unit.name) + "};\n";
  out += "};\n";
  return CodeunitHeader{.text = out, .unresolvedTables = Unresolved(unit, objects)};
}

} // namespace agiru::gen

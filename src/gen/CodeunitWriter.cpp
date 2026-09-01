#include "CodeunitWriter.h"

#include "Ast.h"
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

bool NamesAnObject(const al::VarDecl &declared) {
  const std::string type = TypeName(declared.type);
  return (type == "Record" || type == "Codeunit") && !declared.subtype.empty();
}

// A type as it stands in a DECLARATION. `Record "X"` is the generated class, `Record "X" temporary`
// is that class with no database behind it, `Text[50]` carries its length, and everything else is
// the door's own name for the AL type.
std::string TypeOf(const al::VarDecl &declared, const TableIndex &tables) {
  std::string type = TypeName(declared.type);
  if (type == "Record" || type == "Codeunit") {
    const auto found = tables.find(LowerKey(declared.subtype));
    const std::string named =
        found != tables.end() ? found->second.identifier : Identifier(declared.subtype);
    return declared.temporary ? "Temporary<" + named + ">" : named;
  }
  if (type == "Code" || type == "Text") {
    return type + "<" + std::to_string(declared.length) + ">";
  }
  return type;
}

std::string Signature(const al::VarDecl &declared, const TableIndex &tables) {
  return TypeOf(declared, tables) + (declared.byReference ? " &" : " ");
}

std::string Parameters(const al::ProcedureDecl &procedure, const TableIndex &tables, bool named) {
  std::string out;
  for (std::size_t i = 0; i < procedure.parameters.size(); ++i) {
    if (i != 0) { out += ", "; }
    out += Signature(procedure.parameters[i], tables);
    if (named) { out += Identifier(procedure.parameters[i].name); }
  }
  return out;
}

std::string Returns(const al::ProcedureDecl &procedure, const TableIndex &tables) {
  if (procedure.returnType.empty()) { return "void"; }
  const al::VarDecl returned{.byReference = false,
                             .temporary = false,
                             .name = {},
                             .type = procedure.returnType,
                             .subtype = procedure.returnSubtype,
                             .length = 0};
  return TypeOf(returned, tables);
}

std::string Declaration(const al::ProcedureDecl &procedure, const TableIndex &tables) {
  return "  " + Returns(procedure, tables) + " " + Identifier(procedure.name) + "(" +
         Parameters(procedure, tables, true) + ");\n";
}

std::string Includes(const al::CodeunitObject &unit, const TableIndex &tables) {
  std::set<std::string> headers;
  const auto reach = [&](const al::VarDecl &declared) {
    if (!NamesAnObject(declared)) { return; }
    const auto found = tables.find(LowerKey(declared.subtype));
    if (found != tables.end()) { headers.insert(found->second.header); }
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

std::vector<std::string> Unresolved(const al::CodeunitObject &unit, const TableIndex &tables) {
  std::vector<std::string> missing;
  const auto note = [&](const al::VarDecl &declared) {
    if (!NamesAnObject(declared) || tables.contains(LowerKey(declared.subtype))) { return; }
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

} // namespace

std::string CodeunitHeaderPath(const al::CodeunitObject &unit) {
  return OutputDirectory(unit.nameSpace, ObjectKind::Codeunit) + "/" + Identifier(unit.name) + ".h";
}

CodeunitHeader WriteCodeunit(const al::CodeunitObject &unit,
                             const std::string &sourcePath,
                             const TableIndex &tables) {
  const std::string identifier = Identifier(unit.name);

  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#pragma once\n\n";
  out += Includes(unit, tables);
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
    out += Declaration(procedure, tables);
    previousWasTrigger = procedure.isTrigger;
    first = false;
  }

  std::string hidden;
  for (const al::VarDecl &declared : unit.variables) {
    hidden += "  " + TypeOf(declared, tables) + " " + Identifier(declared.name) + ";\n";
  }
  if (!unit.labels.empty() && !hidden.empty()) { hidden += "\n"; }
  for (const al::LabelDecl &label : unit.labels) {
    hidden += "  static constexpr std::string_view " + Identifier(label.name) + "{" +
              Literal(label.text) + "};\n";
  }
  std::string locals;
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    if (!procedure.isLocal) { continue; }
    locals += Declaration(procedure, tables);
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
  return CodeunitHeader{.text = out, .unresolvedTables = Unresolved(unit, tables)};
}

} // namespace agiru::gen

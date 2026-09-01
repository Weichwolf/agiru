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

std::string Locals(const al::ProcedureDecl &procedure, const TableIndex &tables) {
  std::string out;
  // THE NAMED RETURN VALUE IS A LOCAL, and it comes first because AL declares it in the signature,
  // ahead of the var block. `exit;` with no argument returns it, zero-initialised if nothing wrote.
  if (!procedure.returnName.empty()) {
    out += "  " + Returns(procedure, tables) + " " + Identifier(procedure.returnName) + "{};\n";
  }
  for (const al::VarDecl &declared : procedure.variables) {
    out += "  " + TypeOf(declared, tables) + " " + Identifier(declared.name) + "{};\n";
  }
  return out;
}

} // namespace

std::string WriteCodeunitSource(const al::CodeunitObject &unit,
                                const std::string &sourcePath,
                                const TableIndex &tables) {
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
    out += Returns(procedure, tables) + " " + identifier + "::" + Identifier(procedure.name) + "(" +
           Parameters(procedure, tables, !publisher) + ") {";
    const std::string locals = publisher ? std::string{} : Locals(procedure, tables);
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

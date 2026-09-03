#include "PageWriter.h"

#include "Ast.h"
#include "BodyWriter.h"
#include "CodeunitWriter.h"
#include "Door.h"
#include "EnumWriter.h"
#include "Names.h"
#include "Scope.h"
#include "Token.h"

#include <cctype>
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

namespace {

bool IsField(const std::string &kind) {
  return kind == "field" || kind == "part" || kind == "usercontrol" || kind == "label" ||
         kind == "systempart" || kind == "chartpart";
}

bool IsAction(const std::string &kind) {
  return kind == "action" || kind == "actionref" || kind == "fileuploadaction" ||
         kind == "systemaction";
}

std::string Lowered(std::string_view text) {
  std::string out;
  for (const char c : text) {
    out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return out;
}

void Flatten(const std::vector<al::PageControl> &controls,
             std::vector<const al::PageControl *> &fields,
             std::vector<const al::PageControl *> &actions) {
  for (const al::PageControl &control : controls) {
    const std::string kind = Lowered(control.kind);
    if (!control.name.empty() && IsField(kind)) { fields.push_back(&control); }
    if (!control.name.empty() && IsAction(kind)) { actions.push_back(&control); }
    Flatten(control.children, fields, actions);
  }
}

void WriteControls(std::string &out,
                   const std::vector<const al::PageControl *> &controls,
                   std::string_view type,
                   std::set<std::string> &taken) {
  for (const al::PageControl *control : controls) {
    const std::string identifier = Identifier(control->name);
    if (identifier.empty() || !taken.insert(identifier).second) { continue; }
    out += "  " + std::string(type) + " " + identifier + "{" + Literal(control->name) + "};\n";
  }
}

struct Reached {
  std::set<std::string> headers;
  std::map<std::string, std::set<std::string>> forward;
};

void Ahead(Reached &reached, const std::string &qualified) {
  const std::size_t colons = qualified.find("::");
  if (colons == std::string::npos) { return; }
  reached.forward[qualified.substr(0, colons)].insert(qualified.substr(colons + 2));
}

void Named(Reached &reached, const al::VarDecl &declared, const Objects &objects, bool complete) {
  const std::string type = TypeName(declared.type);
  if (type == "Enum" && !declared.subtype.empty()) {
    const auto found = objects.enums.find(LowerKey(declared.subtype));
    if (found != objects.enums.end() && !found->second.header.empty()) {
      reached.headers.insert(found->second.header);
    }
  }
  if (type == "Interface") {
    const auto found = objects.interfaces.find(LowerKey(declared.subtype));
    if (found != objects.interfaces.end()) { Ahead(reached, found->second.identifier); }
    return;
  }
  if (type == "TestPage" || type == "TestRequestPage") {
    const auto found = objects.pages.find(LowerKey(declared.subtype));
    if (found != objects.pages.end() && !found->second.header.empty()) {
      reached.headers.insert(found->second.header);
    }
  }
  for (const al::VarDecl &argument : declared.arguments) {
    Named(reached, argument, objects, complete);
  }
  const TableRef *ref = ReachObject(declared, objects);
  if (ref == nullptr || ref->header.empty()) { return; }
  if (complete || declared.temporary) {
    reached.headers.insert(ref->header);
    return;
  }
  Ahead(reached, ref->identifier);
}

std::string Includes(const al::PageObject &object, const Objects &objects) {
  Reached reached;
  std::set<std::string> &headers = reached.headers;
  const auto reach = [&](const al::VarDecl &declared) { Named(reached, declared, objects, true); };
  for (const al::VarDecl &declared : object.variables) { reach(declared); }
  for (const al::ProcedureDecl &procedure : object.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) {
      Named(reached, declared, objects, false);
    }
    Named(reached, procedure.returned, objects, false);
  }
  const al::Property *source = al::Find(object.properties, "SourceTable");
  if (source != nullptr && !source->value.empty()) {
    std::string name;
    for (const al::Token &token : source->value) { name += token.text; }
    const auto found = objects.tables.find(LowerKey(name));
    if (found != objects.tables.end() && !found->second.header.empty()) {
      headers.insert(found->second.header);
    }
  }
  std::string out;
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }
  if (!reached.forward.empty()) { out += "\n"; }
  for (const auto &[space, named] : reached.forward) {
    const ObjectKind kind = KindOfNamespace(space);
    out += "namespace agiru::app::" + space + " {\n";
    for (const std::string &one : named) {
      out += "class " + ClassName(one, kind) + ";\n" + ClassAlias(one, kind);
    }
    out += "} // namespace agiru::app::" + space + "\n";
  }
  return out;
}

void ControlTriggerDeclarations(std::string &out, const std::vector<al::PageControl> &controls) {
  for (const al::PageControl &control : controls) {
    for (const al::ProcedureDecl &trigger : control.triggers) {
      out += "  void " + ControlTrigger(trigger.name, control.name) + "();\n";
    }
    ControlTriggerDeclarations(out, control.children);
  }
}

std::string SourceTable(const al::PageObject &object, const Objects &objects) {
  const al::Property *source = al::Find(object.properties, "SourceTable");
  if (source == nullptr || source->value.empty()) { return {}; }
  std::string name;
  for (const al::Token &token : source->value) { name += token.text; }
  const auto found = objects.tables.find(LowerKey(name));
  return found == objects.tables.end() ? std::string{} : found->second.identifier;
}

}

std::string PageHeaderPath(const al::PageObject &object) {
  return OutputDirectory(object.nameSpace, ObjectKind::Page) + "/" + Identifier(object.name) + ".h";
}

PageHeader
WritePage(const al::PageObject &object, const std::string &source, const Objects &objects) {
  const std::string identifier = Identifier(object.name);
  const std::string pageClass = ClassName(identifier, ObjectKind::Page);
  const std::string controlsClass = identifier + "_Controls";

  std::string out = "// Generated from " + source + ". Do not edit.\n#pragma once\n\n";
  out += kDoorMarker;
  if (NamesAbsentIn(object.variables, object.procedures, objects)) {
    out += "#include \"absent/Types.h\"\n";
  }
  const std::string includes = Includes(object, objects);
  if (!includes.empty()) { out += "\n" + includes; }
  out += "\n#include <array>\n#include <cstdint>\n#include <string_view>\n\n";
  out += InlineOptionsOf(object.name, "pages", object.variables, object.procedures);

  std::vector<const al::PageControl *> fields;
  std::vector<const al::PageControl *> actions;
  Flatten(object.layout, fields, actions);
  Flatten(object.actions, fields, actions);

  out += "namespace agiru::app::pages {\n\n";
  out += "template <typename Field, typename Action> class " + controlsClass + " {\npublic:\n";
  std::set<std::string> taken{"OpenNew", "OpenEdit", "OpenView", "Close", "First", "Next", "New"};
  WriteControls(out, fields, "Field", taken);
  if (!fields.empty() && !actions.empty()) { out += "\n"; }
  WriteControls(out, actions, "Action", taken);
  out += "};\n\n";
  out += "class " + pageClass + ";\n" + ClassAlias(identifier, ObjectKind::Page) + "\n";
  out += "class " + pageClass + " : public Page<" + pageClass + "> {\npublic:\n";
  out += "  static constexpr PageId kId{" + std::to_string(object.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(object.name) + "};\n\n";

  const std::string table = SourceTable(object, objects);
  if (!table.empty()) { out += "  " + table + " Rec;\n\n"; }

  const std::string members =
      MemberDeclarations(object.name, object.variables, object.labels, object.procedures, objects);
  if (!members.empty()) { out += members + "\n"; }

  std::string triggers;
  ControlTriggerDeclarations(triggers, object.layout);
  ControlTriggerDeclarations(triggers, object.actions);
  if (!triggers.empty()) { out += "\n" + triggers; }

  std::string publics;
  std::string locals;
  for (const al::ProcedureDecl &procedure : object.procedures) {
    (procedure.isLocal ? locals : publics) +=
        ProcedureDeclaration(procedure, objects, object.name, {}, object.procedures);
  }
  if (!publics.empty()) { out += "\n" + publics; }
  if (!locals.empty()) { out += "\nprivate:\n" + locals; }

  out += "};\n\n";
  out += "} // namespace agiru::app::pages\n\n";
  out += "template <> struct agiru::PageTraits<agiru::app::pages::" + identifier + "> {\n";
  out += "  static constexpr PageId kId{" + std::to_string(object.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(object.name) + "};\n";
  out += "  template <typename Field, typename Action>\n  using Controls = agiru::app::pages::" +
         controlsClass + "<Field, Action>;\n";
  out += "};\n";
  DotNetUse dotnet;
  DotNetUse absent;
  GatherAbsentIn(object.variables, object.procedures, objects, dotnet, absent);
  return PageHeader{.text = WithDoor(out, ObjectKind::Page), .dotnet = dotnet, .absent = absent};
}

}

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
  return kind == "field" || kind == "usercontrol" || kind == "label" || kind == "systempart" ||
         kind == "chartpart";
}

bool IsPart(const std::string &kind) {
  return kind == "part";
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

struct Controls {
  std::vector<const al::PageControl *> fields;
  std::vector<const al::PageControl *> actions;
  std::vector<const al::PageControl *> parts;
};

void Flatten(const std::vector<al::PageControl> &controls, Controls &into) {
  for (const al::PageControl &control : controls) {
    const std::string kind = Lowered(control.kind);
    if (!control.name.empty() && IsField(kind)) { into.fields.push_back(&control); }
    if (!control.name.empty() && IsAction(kind)) { into.actions.push_back(&control); }
    if (!control.name.empty() && IsPart(kind)) { into.parts.push_back(&control); }
    Flatten(control.children, into);
  }
}

std::set<std::string> TestPageSurface() {
  static const std::set<std::string> surface{"OpenNew",
                                             "OpenEdit",
                                             "OpenView",
                                             "Close",
                                             "First",
                                             "Next",
                                             "New",
                                             "Last",
                                             "Previous",
                                             "Prev",
                                             "GoToRecord",
                                             "GoToKey",
                                             "Trap",
                                             "OK",
                                             "Cancel",
                                             "Yes",
                                             "No",
                                             "Caption",
                                             "Editable",
                                             "Expand",
                                             "IsExpanded",
                                             "GetField",
                                             "Filter",
                                             "GetValidationError",
                                             "ValidationErrorCount",
                                             "FindFirstField",
                                             "FindNextField",
                                             "FindPreviousField",
                                             "Edit",
                                             "RunPageBackgroundTask"};
  return surface;
}

std::string PartSource(const al::PageControl &control) {
  std::string named;
  for (const al::Token &token : control.source) { named += token.text; }
  return named;
}

void WriteParts(std::string &out,
                const std::vector<const al::PageControl *> &parts,
                const Objects &objects,
                const std::map<std::string, std::string> &named,
                std::set<std::string> &taken) {
  if (parts.empty()) { return; }
  out += "\n";
  for (const al::PageControl *control : parts) {
    const std::string identifier = ControlIdentifier(named, control->name);
    if (identifier.empty() || !taken.insert(identifier).second) { continue; }
    const auto found = objects.pages.find(LowerKey(PartSource(*control)));
    if (found == objects.pages.end()) {
      out += "  Field_Kind " + identifier + "{" + Literal(control->name) + "};\n";
      continue;
    }
    out += "  Part_Kind<" + found->second.identifier + "> " + identifier + ";\n";
  }
}

void WriteControls(std::string &out,
                   const std::vector<const al::PageControl *> &controls,
                   std::string_view type,
                   const std::map<std::string, std::string> &named,
                   std::set<std::string> &taken) {
  for (const al::PageControl *control : controls) {
    const std::string identifier = ControlIdentifier(named, control->name);
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
  for (const al::VarDecl &declared : object.variables) { Named(reached, declared, objects, false); }
  for (const al::ProcedureDecl &procedure : object.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) {
      Named(reached, declared, objects, false);
    }
    Named(reached, procedure.returned, objects, false);
  }
  {
    Controls all;
    Flatten(object.layout, all);
    Flatten(object.actions, all);
    std::vector<const al::PageControl *> everywhere = all.parts;
    everywhere.insert(everywhere.end(), all.fields.begin(), all.fields.end());
    everywhere.insert(everywhere.end(), all.actions.begin(), all.actions.end());
    for (const al::PageControl *control : everywhere) {
      for (const al::ProcedureDecl &trigger : control->triggers) {
        for (const al::VarDecl &declared : trigger.parameters) {
          Named(reached, declared, objects, false);
        }
        Named(reached, trigger.returned, objects, false);
      }
    }
  }
  {
    Controls all;
    Flatten(object.layout, all);
    Flatten(object.actions, all);
    for (const al::PageControl *control : all.parts) {
      const auto found = objects.pages.find(LowerKey(PartSource(*control)));
      if (found != objects.pages.end() && !found->second.header.empty()) {
        headers.insert(found->second.header);
      }
    }
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
  if (DeclaresAnOption(object.variables, object.procedures)) {
    out += "#include \"options/Types.h\"\n";
  }
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

void ControlTriggerDeclarations(std::string &out,
                                const std::vector<al::PageControl> &controls,
                                const std::map<std::string, std::string> &named,
                                const al::PageObject &page,
                                const Objects &objects) {
  for (const al::PageControl &control : controls) {
    for (const al::ProcedureDecl &trigger : control.triggers) {
      out += ProcedureDeclaration(trigger,
                                  objects,
                                  page.name,
                                  Shadowing(page.variables, page.procedures, page.labels),
                                  page.procedures,
                                  ControlTrigger(trigger.name,
                                                 ControlIdentifier(named, control.name),
                                                 page.procedures));
    }
    ControlTriggerDeclarations(out, control.children, named, page, objects);
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

std::map<std::string, std::string> ControlIdentifiers(const al::PageObject &object) {
  Controls all;
  Flatten(object.layout, all);
  Flatten(object.actions, all);
  std::vector<std::string> alNames;
  alNames.reserve(all.fields.size() + all.actions.size() + all.parts.size());
  for (const std::vector<const al::PageControl *> *group :
       {&all.fields, &all.actions, &all.parts}) {
    for (const al::PageControl *control : *group) { alNames.push_back(control->name); }
  }
  const std::vector<std::string> made = Distinct(alNames, TestPageSurface());
  std::map<std::string, std::string> named;
  std::size_t at = 0;
  for (const std::vector<const al::PageControl *> *group :
       {&all.fields, &all.actions, &all.parts}) {
    for (const al::PageControl *control : *group) {
      named.emplace(Lowered(control->name), made[at++]);
    }
  }
  return named;
}

std::map<std::string, std::string> ControlIdentifiers(const al::PageObject &object,
                                                      const Objects &objects) {
  std::map<std::string, std::string> named = ControlIdentifiers(object);
  const al::Property *source = al::Find(object.properties, "SourceTable");
  if (source == nullptr) { return named; }
  std::string table;
  for (const al::Token &token : source->value) { table += token.text; }
  const auto found = objects.tables.find(LowerKey(table));
  if (found == objects.tables.end()) { return named; }
  std::set<std::string> taken;
  for (const auto &[key, identifier] : named) { taken.insert(identifier); }
  for (const auto &[field, identifier] : found->second.fields) {
    if (named.contains(field) || !taken.insert(identifier).second) { continue; }
    named.emplace(field, identifier);
  }
  return named;
}

std::string ControlIdentifier(const std::map<std::string, std::string> &named,
                              std::string_view alName) {
  const auto found = named.find(Lowered(alName));
  return found == named.end() ? Identifier(alName) : found->second;
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

  Controls all;
  Flatten(object.layout, all);
  Flatten(object.actions, all);

  out += "namespace agiru::app::pages {\n\n";
  out +=
      "template <typename Field_Kind, typename Action_Kind, template <typename> class Part_Kind>\n"
      "class " +
      controlsClass + " {\npublic:\n";
  std::set<std::string> taken{"OpenNew", "OpenEdit", "OpenView", "Close", "First", "Next", "New"};
  const std::map<std::string, std::string> named = ControlIdentifiers(object, objects);
  WriteControls(out, all.fields, "Field_Kind", named, taken);
  if (!all.fields.empty() && !all.actions.empty()) { out += "\n"; }
  WriteControls(out, all.actions, "Action_Kind", named, taken);
  WriteParts(out, all.parts, objects, named, taken);
  for (const auto &[field, identifier] : named) {
    if (!taken.insert(identifier).second) { continue; }
    out += "  Field_Kind " + identifier + "{" + Literal(field) + "};\n";
  }
  out += "};\n\n";
  out += "class " + pageClass + ";\n" + ClassAlias(identifier, ObjectKind::Page) + "\n";
  out += "class " + pageClass + " : public Page<" + pageClass + "> {\npublic:\n";
  out += "  static constexpr PageId kId{" + std::to_string(object.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(object.name) + "};\n\n";

  const std::string table = SourceTable(object, objects);
  if (!table.empty()) { out += "  " + table + " Rec;\n\n"; }

  {
    Controls all;
    Flatten(object.layout, all);
    Flatten(object.actions, all);
    std::set<std::string> written;
    for (const al::PageControl *control : all.parts) {
      const std::string member = ControlIdentifier(named, control->name);
      if (member.empty() || !written.insert(member).second) { continue; }
      const auto found = objects.pages.find(LowerKey(PartSource(*control)));
      const std::string sub =
          found == objects.pages.end() ? std::string{"::agiru::Page<>"} : found->second.identifier;
      out += "  ::agiru::PartRef<" + sub + "> " + member + ";\n";
    }
    if (!written.empty()) { out += "\n"; }
  }

  const std::set<std::string> shadowed =
      Shadowing(object.variables, object.procedures, object.labels);
  const std::string members =
      MemberDeclarations(object.name, object.variables, object.labels, object.procedures, objects);
  if (!members.empty()) { out += members + "\n"; }

  std::string triggers;
  ControlTriggerDeclarations(triggers, object.layout, named, object, objects);
  ControlTriggerDeclarations(triggers, object.actions, named, object, objects);
  if (!triggers.empty()) { out += "\n" + triggers; }

  std::string publics;
  std::string locals;
  for (const al::ProcedureDecl &procedure : object.procedures) {
    (procedure.isLocal ? locals : publics) +=
        ProcedureDeclaration(procedure, objects, object.name, shadowed, object.procedures);
  }
  if (!publics.empty()) { out += "\n" + publics; }
  if (!locals.empty()) { out += "\nprivate:\n" + locals; }

  out += "};\n\n";
  out += "} // namespace agiru::app::pages\n\n";
  out += "template <> struct agiru::PageTraits<agiru::app::pages::" + identifier + "> {\n";
  out += "  static constexpr PageId kId{" + std::to_string(object.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(object.name) + "};\n";
  out += "  template <typename Field_Kind, typename Action_Kind, template <typename> class "
         "Part_Kind>\n"
         "  using Controls = agiru::app::pages::" +
         controlsClass + "<Field_Kind, Action_Kind, Part_Kind>;\n";
  out += "};\n";
  DotNetUse dotnet;
  DotNetUse absent;
  GatherAbsentIn(object.variables, object.procedures, objects, dotnet, absent);
  return PageHeader{.text = WithDoor(out, ObjectKind::Page), .dotnet = dotnet, .absent = absent};
}

}

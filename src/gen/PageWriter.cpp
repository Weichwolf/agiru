#include "PageWriter.h"

#include "Ast.h"
#include "BodyWriter.h"
#include "CodeunitWriter.h"
#include "Door.h"
#include "EnumWriter.h"
#include "Names.h"
#include "Scope.h"
#include "Token.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <map>
#include <set>
#include <span>
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
  const std::string reachable = Unprefixed(qualified);
  const std::size_t colons = reachable.rfind("::");
  reached.forward[colons == std::string::npos ? std::string{} : reachable.substr(0, colons)].insert(
      colons == std::string::npos ? reachable : reachable.substr(colons + 2));
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
    const std::string within = space.empty() ? "agiru" : "agiru::" + space;
    out += "namespace " + within + " {\n";
    for (const std::string &one : named) { out += "class " + one + ";\n"; }
    out += "} // namespace " + within + "\n";
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
      out += ProcedureDeclaration(
          trigger,
          objects,
          page.name,
          Shadowing(page.variables, page.procedures, page.labels),
          page.procedures,
          ControlTrigger(trigger.name, ControlIdentifier(named, control.name), page.procedures));
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
  if (found != objects.tables.end()) { return found->second.identifier; }
  return "absent::" + Identifier(name);
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

namespace {

struct KindName {
  std::string_view spelled;
  std::string_view kind;
};

constexpr std::array kControlKinds{
    KindName{.spelled = "area", .kind = "Area"},
    KindName{.spelled = "group", .kind = "Group"},
    KindName{.spelled = "repeater", .kind = "Repeater"},
    KindName{.spelled = "cuegroup", .kind = "CueGroup"},
    KindName{.spelled = "grid", .kind = "Grid"},
    KindName{.spelled = "fixed", .kind = "Fixed"},
    KindName{.spelled = "field", .kind = "Field"},
    KindName{.spelled = "label", .kind = "Label"},
    KindName{.spelled = "part", .kind = "Part"},
    KindName{.spelled = "systempart", .kind = "SystemPart"},
    KindName{.spelled = "chartpart", .kind = "ChartPart"},
    KindName{.spelled = "usercontrol", .kind = "UserControl"},
    KindName{.spelled = "view", .kind = "View"},
    KindName{.spelled = "action", .kind = "Action"},
    KindName{.spelled = "actionref", .kind = "ActionRef"},
    KindName{.spelled = "separator", .kind = "Separator"},
    KindName{.spelled = "fileuploadaction", .kind = "FileUploadAction"},
    KindName{.spelled = "systemaction", .kind = "SystemAction"},
    KindName{.spelled = "actions", .kind = "Actions"},
};

constexpr std::array kAreaKinds{
    KindName{.spelled = "content", .kind = "Content"},
    KindName{.spelled = "factboxes", .kind = "FactBoxes"},
    KindName{.spelled = "sections", .kind = "Sections"},
    KindName{.spelled = "rolecenter", .kind = "RoleCenter"},
    KindName{.spelled = "embedding", .kind = "Embedding"},
    KindName{.spelled = "processing", .kind = "Processing"},
    KindName{.spelled = "navigation", .kind = "Navigation"},
    KindName{.spelled = "reporting", .kind = "Reporting"},
    KindName{.spelled = "creation", .kind = "Creation"},
    KindName{.spelled = "promoted", .kind = "Promoted"},
    KindName{.spelled = "systemactions", .kind = "SystemActions"},
    KindName{.spelled = "prompt", .kind = "Prompt"},
    KindName{.spelled = "promptoptions", .kind = "PromptOptions"},
    KindName{.spelled = "prompting", .kind = "Prompting"},
    KindName{.spelled = "promptguide", .kind = "PromptGuide"},
};

std::string_view KindOf(std::span<const KindName> known, const std::string &spelled) {
  for (const KindName &one : known) {
    if (one.spelled == spelled) { return one.kind; }
  }
  return {};
}

std::string_view AreaOf(const std::string &spelled) {
  for (const KindName &one : kAreaKinds) {
    if (one.spelled == spelled) { return one.kind; }
  }
  return {};
}

std::string ControlText(const al::PageControl &control, std::string_view name) {
  const al::Property *found = Find(control.properties, name);
  return found == nullptr ? std::string{} : found->text;
}

bool ControlIs(const al::PageControl &control, std::string_view name, bool absent) {
  const al::Property *found = Find(control.properties, name);
  return found == nullptr ? absent : LowerKey(found->text) == "true";
}

std::string ControlSource(const al::PageControl &control) {
  std::string named;
  for (const al::Token &token : control.source) { named += token.text; }
  return named;
}

std::string DeclaredControl(const al::PageControl &control,
                            const Objects &objects,
                            const al::TableObject *source,
                            const std::string &children) {
  const std::string kind = Lowered(control.kind);
  const std::string_view known = KindOf(kControlKinds, kind);
  std::string out = ".kind = ControlKind::";
  out += known.empty() ? "Unknown" : std::string(known);
  const bool area = kind == "area";
  if (!area && !control.name.empty()) { out += ", .name = " + Literal(control.name); }
  if (area) {
    const std::string_view named = AreaOf(Lowered(control.name));
    out += ", .area = AreaKind::" + std::string(named.empty() ? "None" : named);
  }
  const auto text = [&out, &control](std::string_view member, std::string_view property) {
    const std::string said = ControlText(control, property);
    if (said.empty()) { return; }
    out += ", ." + std::string(member) + " = " + Literal(said);
  };
  const auto flag = [&out,
                     &control](std::string_view member, std::string_view property, bool absent) {
    const bool said = ControlIs(control, property, absent);
    if (said == absent) { return; }
    out += ", ." + std::string(member) + " = " + (said ? "true" : "false");
  };
  text("caption", "Caption");
  if (known.empty()) { out += ", .source = " + Literal(control.kind); }
  const std::string expression = ControlSource(control);
  if (!expression.empty() && !known.empty() && kind != "part" && kind != "systempart") {
    out += ", .source = " + Literal(expression);
  }
  if (source != nullptr && kind == "field") {
    for (const al::FieldDecl &field : source->fields) {
      if (LowerKey(field.name) != LowerKey(expression) &&
          LowerKey("Rec." + field.name) != LowerKey(expression) &&
          LowerKey("Rec.\"" + field.name + "\"") != LowerKey(expression)) {
        continue;
      }
      out += ", .field = ::agiru::FieldNo{" + std::to_string(field.number) + "}";
      break;
    }
  }
  if (kind == "part") {
    const auto found = objects.pages.find(LowerKey(expression));
    if (found != objects.pages.end() && found->second.id != 0) {
      out += ", .page = ::agiru::PageId{" + std::to_string(found->second.id) + "}";
    }
  }
  text("toolTip", "ToolTip");
  text("applicationArea", "ApplicationArea");
  text("visible", "Visible");
  text("enabled", "Enabled");
  text("editable", "Editable");
  text("importance", "Importance");
  text("style", "Style");
  text("styleExpr", "StyleExpr");
  text("image", "Image");
  text("shortcutKey", "ShortcutKey");
  text("runObject", "RunObject");
  text("runPageLink", "RunPageLink");
  text("runPageView", "RunPageView");
  text("runPageMode", "RunPageMode");
  text("subPageLink", "SubPageLink");
  text("subPageView", "SubPageView");
  flag("showCaption", "ShowCaption", true);
  flag("ellipsis", "Ellipsis", false);
  flag("multiLine", "MultiLine", false);
  flag("quickEntry", "QuickEntry", true);
  flag("showMandatory", "ShowMandatory", false);
  text("hideValue", "HideValue");
  const std::string width = ControlText(control, "Width");
  if (!width.empty() && width.find_first_not_of("0123456789") == std::string::npos &&
      width != "0") {
    out += ", .width = " + width;
  }
  text("freezeColumn", "FreezeColumn");
  text("lookup", "Lookup");
  text("drillDown", "DrillDown");
  text("assistEdit", "AssistEdit");
  text("extendedDataType", "ExtendedDataType");
  text("instructionalText", "InstructionalText");
  {
    const std::string lookup = ControlText(control, "LookupPageId");
    const std::string drill = ControlText(control, "DrillDownPageId");
    const auto page = [&objects](const std::string &said) {
      if (said.empty()) { return std::string{}; }
      if (said.find_first_not_of("0123456789") == std::string::npos) { return said; }
      const auto found = objects.pages.find(LowerKey(said));
      return found == objects.pages.end() || found->second.id == 0
                 ? std::string{}
                 : std::to_string(found->second.id);
    };
    const std::string lookupNo = page(lookup);
    const std::string drillNo = page(drill);
    if (!lookupNo.empty()) { out += ", .lookupPageId = ::agiru::PageId{" + lookupNo + "}"; }
    if (!drillNo.empty()) { out += ", .drillDownPageId = ::agiru::PageId{" + drillNo + "}"; }
  }
  text("updatePropagation", "UpdatePropagation");
  text("optionCaption", "OptionCaption");
  text("aboutTitle", "AboutTitle");
  text("aboutText", "AboutText");
  text("indentationColumn", "IndentationColumn");
  text("indentationControls", "IndentationControls");
  text("showAs", "ShowAs");
  flag("inFooterBar", "InFooterBar", false);
  flag("runPageOnRec", "RunPageOnRec", false);
  flag("showFilter", "ShowFilter", true);
  flag("notBlank", "NotBlank", false);
  flag("showAsTree", "ShowAsTree", false);
  flag("isHeader", "IsHeader", false);
  text("provider", "Provider");
  text("minValue", "MinValue");
  text("maxValue", "MaxValue");
  text("blankNumbers", "BlankNumbers");
  text("maskType", "MaskType");
  {
    const auto span = [&out, &control](std::string_view member, std::string_view property) {
      const std::string said = ControlText(control, property);
      if (said.empty() || said.find_first_not_of("0123456789") != std::string::npos ||
          said == "0") {
        return;
      }
      out += ", ." + std::string(member) + " = " + said;
    };
    span("columnSpan", "ColumnSpan");
    span("rowSpan", "RowSpan");
  }
  flag("closingDates", "ClosingDates", false);
  flag("numeric", "Numeric", false);
  text("valuesAllowed", "ValuesAllowed");
  text("treeInitialState", "TreeInitialState");
  text("cueGroupLayout", "CueGroupLayout");
  flag("allowMultipleFiles", "AllowMultipleFiles", false);
  text("allowedFileExtensions", "AllowedFileExtensions");
  text("entityName", "EntityName");
  text("entitySetName", "EntitySetName");
  text("description", "Description");
  text("gridLayout", "GridLayout");
  text("gesture", "Gesture");
  text("flowTemplateCategoryName", "FlowTemplateCategoryName");
  text("autoFormatType", "AutoFormatType");
  text("autoFormatExpression", "AutoFormatExpression");
  text("captionClass", "CaptionClass");
  text("decimalPlaces", "DecimalPlaces");
  text("tableRelation", "TableRelation");
  flag("blankZero", "BlankZero", false);
  text("scope", "Scope");
  text("accessByPermission", "AccessByPermission");
  text("obsoleteState", "ObsoleteState");
  if (!children.empty()) { out += ", .children = " + children; }
  return out;
}

std::string ControlArrays(const std::vector<al::PageControl> &controls,
                          const std::string &prefix,
                          const std::string &name,
                          const Objects &objects,
                          const al::TableObject *source,
                          int &counter,
                          std::string &out) {
  if (controls.empty()) { return {}; }
  std::vector<std::string> below;
  below.reserve(controls.size());
  for (const al::PageControl &control : controls) {
    const std::string mine = prefix + "_C" + std::to_string(++counter);
    below.push_back(ControlArrays(control.children, prefix, mine, objects, source, counter, out));
  }
  out +=
      "constexpr std::array<ControlDef, " + std::to_string(controls.size()) + "> " + name + "{{\n";
  for (std::size_t i = 0; i < controls.size(); ++i) {
    out += "    ControlDef{" + DeclaredControl(controls[i], objects, source, below[i]) + "},\n";
  }
  out += "}};\n\n";
  return name;
}

std::size_t DepthOf(const std::vector<al::PageControl> &controls) {
  std::size_t deepest = 0;
  for (const al::PageControl &control : controls) {
    const std::size_t below = DepthOf(control.children) + 1;
    deepest = std::max(deepest, below);
  }
  return deepest;
}

std::string PageTypeOf(const al::PageObject &page) {
  static constexpr std::array kTypes{"Card",
                                     "List",
                                     "RoleCenter",
                                     "CardPart",
                                     "ListPart",
                                     "Document",
                                     "Worksheet",
                                     "ListPlus",
                                     "ConfirmationDialog",
                                     "NavigatePage",
                                     "StandardDialog",
                                     "Api",
                                     "ReportPreview",
                                     "ReportProcessingOnly",
                                     "XmlPort",
                                     "HeadlinePart",
                                     "PromptDialog",
                                     "ConfigurationDialog",
                                     "UserControlHost"};
  const al::Property *found = Find(page.properties, "PageType");
  if (found == nullptr) { return "Card"; }
  const std::string said = LowerKey(found->text);
  for (const char *one : kTypes) {
    if (LowerKey(one) == said) { return one; }
  }
  return "Card";
}

}

std::string
PageDefinition(const al::PageObject &page, const Objects &objects, const al::TableObject *source) {
  const std::string identifier = ClassName(Identifier(page.name), ObjectKind::Page);
  const std::string space = NamespaceOf(page.nameSpace);
  const std::string prefix = "k" + Identifier(page.name);
  std::string out = "namespace " + space + " {\n\n";
  int counter = 0;
  const std::string layout =
      ControlArrays(page.layout, prefix, prefix + "Layout", objects, source, counter, out);
  const std::string actions =
      ControlArrays(page.actions, prefix, prefix + "Actions", objects, source, counter, out);
  out += "constexpr PageDef " + prefix + "Page{\n";
  out += "    .id = " + identifier + "::kId,\n";
  out += "    .name = " + identifier + "::kName,\n";
  const al::Property *caption = Find(page.properties, "Caption");
  out += "    .caption = ";
  out += caption == nullptr ? identifier + "::kName" : Literal(caption->text);
  out += ",\n";
  out += "    .type = PageType::" + PageTypeOf(page) + ",\n";
  if (source != nullptr) {
    out += "    .source = ::agiru::TableId{" + std::to_string(source->id) + "},\n";
  }
  const auto said = [&page](std::string_view name) {
    const al::Property *found = Find(page.properties, name);
    return found == nullptr ? std::string{} : found->text;
  };
  const auto text = [&out, &said](std::string_view member, std::string_view property) {
    const std::string value = said(property);
    if (value.empty()) { return; }
    out += "    ." + std::string(member) + " = " + Literal(value) + ",\n";
  };
  const auto flag = [&out, &page](std::string_view member, std::string_view property, bool absent) {
    const al::Property *found = Find(page.properties, property);
    if (found == nullptr) { return; }
    const bool value = LowerKey(found->text) == "true";
    if (value == absent) { return; }
    out += "    ." + std::string(member) + " = " + (value ? "true" : "false") + ",\n";
  };
  text("sourceTableView", "SourceTableView");
  if (!layout.empty()) { out += "    .layout = " + layout + ",\n"; }
  if (!actions.empty()) { out += "    .actions = " + actions + ",\n"; }
  text("editable", "Editable");
  text("insertAllowed", "InsertAllowed");
  text("modifyAllowed", "ModifyAllowed");
  text("deleteAllowed", "DeleteAllowed");
  flag("delayedInsert", "DelayedInsert", false);
  flag("linksAllowed", "LinksAllowed", true);
  flag("showFilter", "ShowFilter", true);
  flag("refreshOnActivate", "RefreshOnActivate", false);
  flag("saveValues", "SaveValues", false);
  flag("analysisModeEnabled", "AnalysisModeEnabled", true);
  const std::string card = said("CardPageId");
  if (!card.empty()) {
    const auto found = objects.pages.find(LowerKey(card));
    if (found != objects.pages.end() && found->second.id != 0) {
      out += "    .cardPageId = ::agiru::PageId{" + std::to_string(found->second.id) + "},\n";
    }
  }
  text("dataCaptionExpression", "DataCaptionExpression");
  text("dataCaptionFields", "DataCaptionFields");
  text("applicationArea", "ApplicationArea");
  text("usageCategory", "UsageCategory");
  text("additionalSearchTerms", "AdditionalSearchTerms");
  text("instructionalText", "InstructionalText");
  text("promotedActionCategories", "PromotedActionCategories");
  text("permissions", "Permissions");
  flag("sourceTableTemporary", "SourceTableTemporary", false);
  flag("autoSplitKey", "AutoSplitKey", false);
  text("aboutTitle", "AboutTitle");
  text("aboutText", "AboutText");
  flag("populateAllFields", "PopulateAllFields", false);
  text("inherentPermissions", "InherentPermissions");
  text("inherentEntitlements", "InherentEntitlements");
  text("accessByPermission", "AccessByPermission");
  text("queryCategory", "QueryCategory");
  text("odataKeyFields", "ODataKeyFields");
  text("contextSensitiveHelpPage", "ContextSensitiveHelpPage");
  text("apiPublisher", "APIPublisher");
  text("apiGroup", "APIGroup");
  text("apiVersion", "APIVersion");
  text("entityName", "EntityName");
  text("entitySetName", "EntitySetName");
  text("entityCaption", "EntityCaption");
  text("entitySetCaption", "EntitySetCaption");
  flag("changeTrackingAllowed", "ChangeTrackingAllowed", false);
  flag("isPreview", "IsPreview", false);
  text("dataAccessIntent", "DataAccessIntent");
  text("helpLink", "HelpLink");
  text("description", "Description");
  text("extensible", "Extensible");
  text("access", "Access");
  text("obsoleteState", "ObsoleteState");
  out += "};\n\n";
  for (const auto &[named, controls] :
       {std::pair{layout, &page.layout}, std::pair{actions, &page.actions}}) {
    if (named.empty()) { continue; }
    out += "static_assert(Depth(" + named + ") == " + std::to_string(DepthOf(*controls)) +
           ",\n              \"a page's layout is a TREE and the generator keeps it -- a "
           "flattened one is one level deep (board:0553)\");\n";
  }
  out += "\n} // namespace " + space + "\n";
  return out;
}

std::string PageHeaderPath(const al::PageObject &object) {
  return OutputDirectory(object.nameSpace, ObjectKind::Page) + "/" + Identifier(object.name) + ".h";
}

static std::vector<al::ProcedureDecl> WithControlTriggers(const al::PageObject &page) {
  std::vector<al::ProcedureDecl> all = page.procedures;
  const auto walk = [&all](auto &&self, const std::vector<al::PageControl> &controls) -> void {
    for (const al::PageControl &control : controls) {
      all.insert(all.end(), control.triggers.begin(), control.triggers.end());
      self(self, control.children);
    }
  };
  walk(walk, page.layout);
  walk(walk, page.actions);
  return all;
}

PageHeader
WritePage(const al::PageObject &object, const std::string &source, const Objects &objects) {
  const std::string identifier = Identifier(object.name);
  const std::vector<al::ProcedureDecl> bodies = WithControlTriggers(object);
  const std::string pageClass = ClassName(identifier, ObjectKind::Page);
  const std::string controlsClass = identifier + "_Controls";

  std::string out = "// Generated from " + source + ". Do not edit.\n#pragma once\n\n";
  out += kDoorMarker;
  if (NamesAbsentIn(object.variables, bodies, objects) ||
      SourceTable(object, objects).starts_with("absent::")) {
    out += "#include \"absent/Types.h\"\n";
  }
  const std::string includes = Includes(object, objects);
  if (!includes.empty()) { out += "\n" + includes; }
  out += "\n#include <array>\n#include <cstdint>\n#include <string_view>\n\n";
  out += InlineOptionsOf(object.name, "pages", object.variables, bodies);

  Controls all;
  Flatten(object.layout, all);
  Flatten(object.actions, all);

  const std::string space = NamespaceOf(object.nameSpace);
  out += "namespace " + space + " {\n\n";
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
  out += "class " + pageClass + ";\n\n";
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
  out += "extern const PageDef k" + identifier + "Page;\n\n";
  out += "} // namespace " + space + "\n\n";
  out += "template <> struct agiru::PageTraits<" + space + "::" + pageClass + "> {\n";
  out += "  static constexpr PageId kId{" + std::to_string(object.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(object.name) + "};\n";
  out += "  static constexpr const PageDef &kPage = " + space + "::k" + identifier + "Page;\n";
  out += "  template <typename Field_Kind, typename Action_Kind, template <typename> class "
         "Part_Kind>\n"
         "  using Controls = " +
         space + "::" + controlsClass + "<Field_Kind, Action_Kind, Part_Kind>;\n";
  out += "};\n";
  DotNetUse dotnet;
  DotNetUse absent;
  GatherAbsentIn(object.variables, bodies, objects, dotnet, absent);
  return PageHeader{.text = WithDoor(out, ObjectKind::Page), .dotnet = dotnet, .absent = absent};
}

}

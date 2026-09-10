#include "TableWriter.h"

#include "meta/Declare.h"
#include "meta/TableDef.h"

#include "Ast.h"
#include "CodeunitWriter.h"
#include "Door.h"
#include "EnumWriter.h"
#include "Names.h"
#include "Scope.h"
#include "Token.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace agiru::gen {

namespace {

struct OptionField {
  const al::FieldDecl *field;
  std::string enumName;
  std::vector<std::string> members;
  std::vector<std::string> captions;
};

std::string Caption(const al::FieldDecl &field) {
  const al::Property *property = Find(field.properties, "Caption");
  return property != nullptr ? property->text : field.name;
}

std::vector<std::string> Captions(const al::FieldDecl &field,
                                  const std::vector<std::string> &members) {
  const al::Property *caption = Find(field.properties, "OptionCaption");
  if (caption == nullptr) { return members; }
  std::vector<std::string> parts;
  std::string current;
  for (const al::Token &token : caption->value) {
    if (token.kind == al::TokenKind::String) { current = token.text; }
  }
  std::string item;
  for (const char c : current) {
    if (c == ',') {
      parts.push_back(item);
      item.clear();
      continue;
    }
    item += c;
  }
  parts.push_back(item);
  return parts.size() == members.size() ? parts : members;
}

std::vector<OptionField> OptionFields(const al::TableObject &table) {
  std::vector<OptionField> options;
  for (const al::FieldDecl &field : table.fields) {
    const al::Property *members = Find(field.properties, "OptionMembers");
    if (members == nullptr) { continue; }
    OptionField option;
    option.field = &field;
    option.members = ListValue(*members);
    option.enumName = OptionEnumName(table.name, field.name, option.members);
    option.captions = Captions(field, option.members);
    options.push_back(std::move(option));
  }
  return options;
}

const OptionField *OptionOf(const std::vector<OptionField> &options, const al::FieldDecl &field) {
  for (const OptionField &option : options) {
    if (option.field == &field) { return &option; }
  }
  return nullptr;
}

}

namespace {

struct FieldIdentifiers {
  const al::TableObject *table = nullptr;
  std::size_t fields = 0;
  int id = 0;
  std::string name;
  std::unordered_map<std::string, std::string> byName;
  std::unordered_map<std::string, std::string> byBare;
};

const FieldIdentifiers &IdentifiersOf(const al::TableObject &table) {
  static FieldIdentifiers cached;
  if (cached.table == &table && cached.fields == table.fields.size() && cached.id == table.id &&
      cached.name == table.name) {
    return cached;
  }
  cached = FieldIdentifiers{.table = &table,
                            .fields = table.fields.size(),
                            .id = table.id,
                            .name = table.name,
                            .byName = {},
                            .byBare = {}};
  std::set<std::string> taken;
  for (const SystemFieldDecl &system : kSystemFields) {
    taken.insert(LowerKey(std::string(system.name)));
  }
  if (al::Find(table.properties, "QueryColumns") != nullptr) {
    static constexpr std::array kQueryMembers{std::string_view{"open"},
                                              std::string_view{"read"},
                                              std::string_view{"close"},
                                              std::string_view{"setrange"},
                                              std::string_view{"setfilter"},
                                              std::string_view{"getfilter"},
                                              std::string_view{"getfilters"},
                                              std::string_view{"topnumberofrows"},
                                              std::string_view{"columnname"},
                                              std::string_view{"columncaption"},
                                              std::string_view{"columnno"},
                                              std::string_view{"saveascsv"},
                                              std::string_view{"saveasjson"},
                                              std::string_view{"saveasxml"},
                                              std::string_view{"securityfiltering"},
                                              std::string_view{"id"},
                                              std::string_view{"state_block"}};
    for (const std::string_view member : kQueryMembers) { taken.insert(std::string(member)); }
  }
  for (const al::FieldDecl &field : table.fields) {
    const std::string bare = Identifier(field.name);
    const std::string lowerBare = LowerKey(bare);
    const bool platform = field.number >= kSystemFields.front().no.Value();
    const bool collides = !taken.insert(lowerBare).second && !platform;
    const std::string spelled = collides ? bare + "_" + std::to_string(field.number) : bare;
    cached.byName.emplace(LowerKey(field.name), spelled);
    cached.byBare.emplace(lowerBare, spelled);
  }
  return cached;
}

}

std::string FieldIdentifier(const al::TableObject &table, const std::string &name) {
  const FieldIdentifiers &known = IdentifiersOf(table);
  if (const auto found = known.byName.find(LowerKey(name)); found != known.byName.end()) {
    return found->second;
  }
  std::string bare = Identifier(name);
  if (const auto found = known.byBare.find(LowerKey(bare)); found != known.byBare.end()) {
    return found->second;
  }
  return bare;
}

namespace {

std::string Trim(std::string_view text) {
  const auto first = text.find_first_not_of(" \t");
  if (first == std::string_view::npos) { return {}; }
  return std::string(text.substr(first, text.find_last_not_of(" \t") - first + 1));
}

std::vector<std::string> CommaSeparatedNames(std::string_view text) {
  std::vector<std::string> names;
  std::string current;
  bool quoted = false;
  for (const char c : text) {
    if (c == '"') {
      quoted = !quoted;
      continue;
    }
    if (c == ',' && !quoted) {
      names.push_back(Trim(current));
      current.clear();
      continue;
    }
    current += c;
  }
  if (!Trim(current).empty()) { names.push_back(Trim(current)); }
  return names;
}

std::string KeyArrayName(std::size_t position) {
  return "kKey" + std::to_string(position + 1);
}

bool IsEnumField(const al::FieldDecl &field) {
  return TypeName(field.type) == "Enum" && !field.subtype.empty();
}

bool ShadowedByAField(const al::TableObject &table, std::string_view type) {
  return std::ranges::any_of(
      table.fields, [type](const al::FieldDecl &field) { return Identifier(field.name) == type; });
}

std::string Reach(const al::TableObject &table, const std::string &type, const std::string &bare) {
  if (type == "FieldNo" || HiddenByABaseMember(type) || ShadowsADoorType(type)) {
    return "::agiru::" + bare;
  }
  return ShadowedByAField(table, type) ? "::agiru::" + bare : bare;
}

std::string MemberType(const al::TableObject &table,
                       const al::FieldDecl &field,
                       const OptionField *option,
                       const EnumIndex &enums) {
  if (option != nullptr) { return Reach(table, "Option", "Option<" + option->enumName + ">"); }
  if (IsEnumField(field)) {
    const auto found = enums.find(LowerKey(field.subtype));
    return found != enums.end() ? Reach(table, "Enum", "Enum<" + found->second.identifier + ">")
                                : Reach(table, "Enum", "Enum<>");
  }
  const std::string type = TypeName(field.type);
  if (type == "Code" || type == "Text") {
    return Reach(table, type, type + "<" + std::to_string(field.length) + ">");
  }
  return Reach(table, type, type);
}

std::vector<const al::FieldDecl *> ByNumber(const al::TableObject &table) {
  std::vector<const al::FieldDecl *> fields;
  fields.reserve(table.fields.size());
  for (const al::FieldDecl &field : table.fields) { fields.push_back(&field); }
  std::ranges::sort(
      fields, [](const al::FieldDecl *a, const al::FieldDecl *b) { return a->number < b->number; });
  return fields;
}

std::string Includes(const al::TableObject &table,
                     const std::vector<OptionField> &options,
                     const EnumIndex &enums) {
  std::set<std::string> headers;
  for (const al::FieldDecl &field : table.fields) {
    if (OptionOf(options, field) != nullptr || !IsEnumField(field)) { continue; }
    const auto found = enums.find(LowerKey(field.subtype));
    if (found != enums.end()) { headers.insert(found->second.header); }
  }
  std::string out = std::string(kDoorMarker);
  for (const std::string &header : headers) {
    out += "#include \"";
    out += header;
    out += "\"\n";
  }
  out += "\n";
  return out;
}

bool IsSystemField(const al::FieldDecl &field) {
  return field.number >= kSystemFields.front().no.Value();
}

al::TableObject WithSystemFields(al::TableObject table) {
  for (const SystemFieldDecl &system : kSystemFields) {
    table.fields.push_back(al::FieldDecl{.number = system.no.Value(),
                                         .name = std::string(system.name),
                                         .type = std::string(system.alType),
                                         .subtype = {},
                                         .length = 0,
                                         .properties = {},
                                         .triggers = {}});
  }
  return table;
}

std::optional<std::string>
InitValue(const al::FieldDecl &field, const OptionField *option, const EnumIndex &enums) {
  const al::Property *declared = Find(field.properties, "InitValue");
  if (declared == nullptr) { return std::nullopt; }
  std::string written = declared->text;
  for (const al::Token &token : declared->value) {
    if (token.kind == al::TokenKind::String) { written = token.text; }
  }
  if (option != nullptr) {
    for (std::size_t i = 0; i < option->members.size(); ++i) {
      if (LowerKey(option->members[i]) == LowerKey(written)) { return std::to_string(i); }
    }
    return std::nullopt;
  }
  if (!IsEnumField(field)) { return written; }
  const auto found = enums.find(LowerKey(field.subtype));
  if (found == enums.end()) { return std::nullopt; }
  const auto value = found->second.ordinals.find(LowerKey(written));
  return value == found->second.ordinals.end()
             ? std::nullopt
             : std::optional<std::string>(std::to_string(value->second));
}

bool CarriesInitValues(const al::TableObject &table,
                       const std::vector<OptionField> &options,
                       const EnumIndex &enums) {
  return std::ranges::any_of(table.fields, [&](const al::FieldDecl &field) {
    return InitValue(field, OptionOf(options, field), enums).has_value();
  });
}

bool DeclaredTemporary(const al::TableObject &table) {
  const al::Property *kind = Find(table.properties, "TableType");
  return kind != nullptr && LowerKey(kind->text) == "temporary";
}

bool CarriesConstructor(const al::TableObject &table,
                        const std::vector<OptionField> &options,
                        const EnumIndex &enums) {
  return CarriesInitValues(table, options, enums) || DeclaredTemporary(table);
}

std::string PropertyText(const al::FieldDecl &field, std::string_view name) {
  const al::Property *found = Find(field.properties, name);
  if (found == nullptr) { return {}; }
  for (const al::Token &token : found->value) {
    if (token.kind == al::TokenKind::String) { return token.text; }
  }
  return found->text;
}

std::string FormulaText(const al::FieldDecl &field) {
  const al::Property *found = Find(field.properties, "CalcFormula");
  if (found == nullptr) { return {}; }
  std::string out;
  for (const al::Token &token : found->value) {
    if (!out.empty()) { out += ' '; }
    if (token.kind == al::TokenKind::QuotedIdentifier) {
      out += '"';
      for (const char c : token.text) {
        if (c == '"') { out += '"'; }
        out += c;
      }
      out += '"';
    } else if (token.kind == al::TokenKind::String) {
      out += '\'' + token.text + '\'';
    } else {
      out += token.text;
    }
  }
  return out;
}

bool PropertyIs(const al::FieldDecl &field, std::string_view name, bool absent) {
  const std::string text = PropertyText(field, name);
  if (text.empty()) { return absent; }
  return LowerKey(text) == "true";
}

std::string PageNumber(const TableIndex &pages, std::string text) {
  while (!text.empty() && (text.back() == ';' || text.back() == ' ' || text.back() == '"')) {
    text.pop_back();
  }
  while (!text.empty() && (text.front() == ' ' || text.front() == '"')) { text.erase(0, 1); }
  if (text.empty()) { return {}; }
  if (text.find_first_not_of("0123456789") == std::string::npos) { return text; }
  const auto found = pages.find(LowerKey(text));
  if (found == pages.end() || found->second.id == 0) { return {}; }
  return std::to_string(found->second.id);
}

std::string DeclaredBlock(const al::FieldDecl &field,
                          const OptionField *option,
                          const EnumIndex &enums,
                          const TableIndex &pages) {
  std::string out;
  const auto text = [&out](std::string_view member, const std::string &value) {
    if (value.empty()) { return; }
    if (!out.empty()) { out += ", "; }
    out += "." + std::string(member) + " = " + Literal(value);
  };
  const auto number =
      [&out](std::string_view member, const std::string &value, std::string_view cast) {
        if (value.empty()) { return; }
        if (value.find_first_not_of("0123456789") != std::string::npos) { return; }
        if (value == "0") { return; }
        if (!out.empty()) { out += ", "; }
        out += "." + std::string(member) + " = ";
        out += cast.empty() ? value : std::string(cast) + "{" + value + "}";
      };
  const auto flag = [&out](std::string_view member, bool value, bool absent) {
    if (value == absent) { return; }
    if (!out.empty()) { out += ", "; }
    out += "." + std::string(member) + " = " + (value ? "true" : "false");
  };
  const std::optional<std::string> initial = InitValue(field, option, enums);
  if (initial.has_value()) { out += ".initValue = " + Literal(*initial); }
  const std::string kind = LowerKey(PropertyText(field, "FieldClass"));
  if (kind == "flowfield" || kind == "flowfilter") {
    if (!out.empty()) { out += ", "; }
    out += ".fieldClass = ::agiru::FieldClass::";
    out += kind == "flowfield" ? "FlowField" : "FlowFilter";
  }
  text("calcFormula", FormulaText(field));
  flag("notBlank", PropertyIs(field, "NotBlank", false), false);
  flag("autoIncrement", PropertyIs(field, "AutoIncrement", false), false);
  flag("editable", PropertyIs(field, "Editable", true), true);
  flag("validateTableRelation", PropertyIs(field, "ValidateTableRelation", true), true);
  {
    const std::string relation = PropertyText(field, "TableRelation");
    const std::string lowered = LowerKey(relation);
    const bool simple = !relation.empty() && lowered.find("where") == std::string::npos &&
                        lowered.find("if") == std::string::npos &&
                        lowered.find("else") == std::string::npos;
    if (simple) {
      const auto unquote = [](std::string one) {
        while (!one.empty() && (one.front() == '"' || one.front() == ' ')) { one.erase(0, 1); }
        while (!one.empty() && (one.back() == '"' || one.back() == ' ' || one.back() == ';')) {
          one.pop_back();
        }
        return one;
      };
      std::string target = relation;
      std::string named;
      const std::size_t dot = target.find("\".\"");
      if (dot != std::string::npos) {
        named = target.substr(dot + 2);
        target = target.substr(0, dot + 1);
      } else if (const std::size_t bare = target.find('.');
                 bare != std::string::npos && target.front() != '"') {
        named = target.substr(bare + 1);
        target = target.substr(0, bare);
      }
      text("relationTable", unquote(target));
      text("relationField", unquote(named));
    }
  }
  flag("blankZero", PropertyIs(field, "BlankZero", false), false);
  text("minValue", PropertyText(field, "MinValue"));
  text("maxValue", PropertyText(field, "MaxValue"));
  text("decimalPlaces", PropertyText(field, "DecimalPlaces"));
  text("blankNumbers", PropertyText(field, "BlankNumbers"));
  flag("compressed", PropertyIs(field, "Compressed", true), true);
  flag("numeric", PropertyIs(field, "Numeric", false), false);
  text("charAllowed", PropertyText(field, "CharAllowed"));
  text("valuesAllowed", PropertyText(field, "ValuesAllowed"));
  flag("closingDates", PropertyIs(field, "ClosingDates", false), false);
  text("extendedDataType", PropertyText(field, "ExtendedDataType"));
  text("maskType", PropertyText(field, "MaskType"));
  text("toolTip", PropertyText(field, "ToolTip"));
  text("accessByPermission", PropertyText(field, "AccessByPermission"));
  number("lookupPageId", PageNumber(pages, PropertyText(field, "LookupPageId")), "::agiru::PageId");
  number("drillDownPageId",
         PageNumber(pages, PropertyText(field, "DrillDownPageId")),
         "::agiru::PageId");
  flag("optimizeForTextSearch", PropertyIs(field, "OptimizeForTextSearch", false), false);
  text("captionClass", PropertyText(field, "CaptionClass"));
  number("width", PropertyText(field, "Width"), "");
  text("autoFormatType", PropertyText(field, "AutoFormatType"));
  text("autoFormatExpression", PropertyText(field, "AutoFormatExpression"));
  text("allowInCustomizations", PropertyText(field, "AllowInCustomizations"));
  text("access", PropertyText(field, "Access"));
  text("subtype", PropertyText(field, "Subtype"));
  flag("enabled", PropertyIs(field, "Enabled", true), true);
  text("movedFrom", PropertyText(field, "MovedFrom"));
  text("movedTo", PropertyText(field, "MovedTo"));
  text("description", PropertyText(field, "Description"));
  text("obsoleteState", PropertyText(field, "ObsoleteState"));
  text("obsoleteReason", PropertyText(field, "ObsoleteReason"));
  text("obsoleteTag", PropertyText(field, "ObsoleteTag"));
  text("externalName", PropertyText(field, "ExternalName"));
  text("optionOrdinalValues", PropertyText(field, "OptionOrdinalValues"));
  flag("sqlTimestamp", PropertyIs(field, "SqlTimestamp", false), false);
  return out;
}

std::string FieldTable(const al::TableObject &table,
                       const std::vector<const al::FieldDecl *> &sorted,
                       const std::string &tableIdentifier,
                       const std::vector<OptionField> &options,
                       const EnumIndex &enums,
                       const TableIndex &pages) {
  const std::size_t declaredCount = sorted.size() - kSystemFieldCount;
  const std::string tableClass = ClassName(tableIdentifier, ObjectKind::Table);
  std::string out = "constexpr auto k" + tableIdentifier + "Fields = WithSystemFields<" +
                    tableClass + ">(std::array<FieldDef, " + std::to_string(declaredCount) +
                    ">{{\n";
  for (const al::FieldDecl *field : sorted) {
    if (IsSystemField(*field)) { continue; }
    const std::string identifier = FieldIdentifier(table, field->name);
    out += "    Declare<&";
    out += tableClass;
    out += "::";
    out += identifier;
    out += ">(";
    out += tableClass;
    out += "::Field_No::";
    out += identifier;
    out += ", ";
    out += Literal(field->name);
    out += ", ";
    out += Literal(Caption(*field));
    out += ", offsetof(";
    out += tableClass;
    out += ", ";
    out += identifier;
    out += ")";
    const std::string declared = DeclaredBlock(*field, OptionOf(options, *field), enums, pages);
    if (!declared.empty()) { out += ", Declared{" + declared + "}"; }
    out += "),\n";
  }
  return out + "}});\n\n";
}

std::vector<std::string> Unresolved(const al::TableObject &table, const EnumIndex &enums) {
  std::vector<std::string> missing;
  for (const al::FieldDecl &field : table.fields) {
    if (!IsEnumField(field)) { continue; }
    if (enums.contains(LowerKey(field.subtype))) { continue; }
    if (std::ranges::find(missing, field.subtype) == missing.end()) {
      missing.push_back(field.subtype);
    }
  }
  return missing;
}

}

namespace {

std::string Disambiguated(const std::string &bare,
                          std::string_view seam,
                          const std::function<bool(const std::string &)> &taken) {
  if (!taken(bare)) { return bare; }
  for (int n = 0; n < 100; ++n) {
    const std::string spelled =
        bare + std::string(seam) + (n == 0 ? std::string{} : std::to_string(n));
    if (!taken(spelled)) { return spelled; }
  }
  return bare + std::string(seam);
}

bool NamedByAField(const al::TableObject &table, const std::string &spelled) {
  return IdentifiersOf(table).byBare.contains(LowerKey(spelled));
}

}

std::string VariableIdentifier(const al::TableObject &table, const std::string &name) {
  const auto taken = [&](const std::string &spelled) {
    if (NamedByAField(table, spelled)) { return true; }
    for (const al::VarDecl &declared : table.variables) {
      if (declared.name == name) { break; }
      if (LowerKey(Identifier(declared.name)) == LowerKey(spelled)) { return true; }
    }
    return false;
  };
  return Disambiguated(Identifier(name), "_Var", taken);
}

std::string ProcedureIdentifier(const al::TableObject &table, const std::string &name) {
  const auto taken = [&](const std::string &spelled) {
    if (NamedByAField(table, spelled)) { return true; }
    for (const al::VarDecl &declared : table.variables) {
      if (LowerKey(VariableIdentifier(table, declared.name)) == LowerKey(spelled)) { return true; }
    }
    for (const al::ProcedureDecl &other : table.procedures) {
      if (other.name == name) { break; }
      if (LowerKey(Identifier(other.name)) == LowerKey(spelled)) { return true; }
    }
    return false;
  };
  return Disambiguated(Identifier(name), "_Proc", taken);
}

namespace {

template <typename Index>
void Indexed(const Index &index, const std::string &subtype, std::set<std::string> &headers) {
  if (subtype.empty()) { return; }
  const auto found = index.find(LowerKey(subtype));
  if (found != index.end() && !found->second.header.empty()) {
    headers.insert(found->second.header);
  }
}

struct Reached {
  std::set<std::string> headers;
  std::map<std::string, std::set<std::string>> ahead;
};

void Name(Reached &reached, const al::VarDecl &declared, const Objects &objects) {
  for (const al::VarDecl &argument : declared.arguments) { Name(reached, argument, objects); }
  const std::string alType = TypeName(declared.type);
  if (alType == "TestPage" || alType == "TestRequestPage") {
    Indexed(PageIndexFor(objects, alType), declared.subtype, reached.headers);
    return;
  }
  if (alType == "Interface") {
    const auto found = objects.interfaces.find(LowerKey(declared.subtype));
    if (found != objects.interfaces.end()) {
      const std::string reachable = Unprefixed(found->second.identifier);
      const std::size_t colons = reachable.rfind("::");
      reached.ahead[colons == std::string::npos ? std::string{} : reachable.substr(0, colons)]
          .insert(colons == std::string::npos ? reachable : reachable.substr(colons + 2));
    }
    return;
  }
  if (alType == "Enum") {
    Indexed(objects.enums, declared.subtype, reached.headers);
    return;
  }
  const TableRef *ref = ReachObject(declared, objects);
  if (ref == nullptr || ref->header.empty()) { return; }
  if (alType == "Interface") {
    reached.headers.insert(ref->header);
    return;
  }
  const std::string reachable = Unprefixed(ref->identifier);
  const std::size_t colons = reachable.rfind("::");
  reached.ahead[colons == std::string::npos ? std::string{} : reachable.substr(0, colons)].insert(
      colons == std::string::npos ? reachable : reachable.substr(colons + 2));
}

std::string Declarations(const al::TableObject &table, const Objects &objects) {
  std::string out;
  Reached reached;
  const std::map<std::string, std::set<std::string>> &ahead = reached.ahead;
  std::set<std::string> &memberHeaders = reached.headers;
  const auto named = [&](const al::VarDecl &declared) { Name(reached, declared, objects); };
  for (const al::VarDecl &declared : table.variables) {
    if (DeclaresAnObject(declared)) {
      named(declared);
      continue;
    }
    const TableRef *ref = ReachObject(declared, objects);
    if (ref != nullptr && !ref->header.empty()) { memberHeaders.insert(ref->header); }
    named(declared);
  }
  for (const al::ProcedureDecl &procedure : table.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { named(declared); }
    for (const al::VarDecl &declared : procedure.variables) { named(declared); }
    named(procedure.returned);
    if (!procedure.returned.byReference &&
        LowerKey(procedure.returned.subtype) != LowerKey(table.name)) {
      const TableRef *returned = ReachObject(procedure.returned, objects);
      if (returned != nullptr && !returned->header.empty()) {
        memberHeaders.insert(returned->header);
      }
    }
  }
  for (const std::string &header : memberHeaders) { out += "#include \"" + header + "\"\n"; }
  if (!memberHeaders.empty()) { out += "\n"; }
  for (const auto &[space, objectNames] : ahead) {
    const std::string named = space.empty() ? "agiru" : "agiru::" + space;
    out += "namespace " + named + " {\n";
    for (const std::string &one : objectNames) { out += "class " + one + ";\n"; }
    out += "} // namespace " + named + "\n";
  }
  if (!ahead.empty()) { out += "\n"; }
  return out;
}

std::string ClassConstants(const al::TableObject &table) {
  std::string out;
  const std::string fieldNo = Reach(table, "FieldNo", "FieldNo");
  out += "\n  struct Field_No : SystemFieldNumbers {\n";
  for (const al::FieldDecl &field : table.fields) {
    if (IsSystemField(field)) { continue; }
    out += "    static constexpr " + fieldNo + " " + FieldIdentifier(table, field.name) + "{" +
           std::to_string(field.number) + "};\n";
  }
  out += "  };\n\n";

  for (std::size_t i = 0; i < table.keys.size(); ++i) {
    out += "  static constexpr std::array<" + Reach(table, "FieldNo", "FieldNo") + ", " +
           std::to_string(table.keys[i].fields.size()) + "> " + KeyArrayName(i) + "{{";
    for (std::size_t f = 0; f < table.keys[i].fields.size(); ++f) {
      if (f != 0) { out += ", "; }
      out += "Field_No::" + FieldIdentifier(table, table.keys[i].fields[f]);
    }
    out += "}};\n";
    const al::Property *sums = Find(table.keys[i].properties, "SumIndexFields");
    if (sums == nullptr) { continue; }
    const std::vector<std::string> names = CommaSeparatedNames(sums->text);
    out += "  static constexpr std::array<" + Reach(table, "FieldNo", "FieldNo") + ", " +
           std::to_string(names.size()) + "> " + KeyArrayName(i) + "Sums{{";
    for (std::size_t f = 0; f < names.size(); ++f) {
      if (f != 0) { out += ", "; }
      out += "Field_No::" + FieldIdentifier(table, names[f]);
    }
    out += "}};\n";
  }

  if (const al::Property *caption = Find(table.properties, "DataCaptionFields");
      caption != nullptr) {
    const std::vector<std::string> names = CommaSeparatedNames(caption->text);
    out += "  static constexpr std::array<" + Reach(table, "FieldNo", "FieldNo") + ", " +
           std::to_string(names.size()) + "> kDataCaptionFields{{";
    for (std::size_t f = 0; f < names.size(); ++f) {
      if (f != 0) { out += ", "; }
      out += "Field_No::" + FieldIdentifier(table, names[f]);
    }
    out += "}};\n";
  }

  if (!table.labels.empty()) { out += "\n"; }
  for (const al::LabelDecl &label : table.labels) {
    out += "  static constexpr std::string_view " + label.name + "{" + Literal(label.text) + "};\n";
  }
  return out;
}

std::string ClassBody(const al::TableObject &table,
                      const std::string &tableIdentifier,
                      const std::vector<OptionField> &options,
                      const EnumIndex &enums,
                      const Objects &objects) {
  const std::string space = NamespaceOf(table.nameSpace);
  std::string out;
  out += "namespace " + space + " {\n\n";
  const std::string tableClass = ClassName(tableIdentifier, ObjectKind::Table);
  out += "class " + tableClass + ";\n\n";
  out += "class " + tableClass + " : public Table<" + tableClass + "> {\npublic:\n";
  out += "  using Table<" + tableClass + ">::operator=;\n\n";
  out += "  static constexpr " + Reach(table, "TableId", "TableId") + " kId{" +
         std::to_string(table.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(table.name) + "};\n\n";

  out += "  detail::StateHandle State_Block;\n\n";

  for (const al::FieldDecl &field : table.fields) {
    out += "  " + MemberType(table, field, OptionOf(options, field), enums) + " " +
           FieldIdentifier(table, field.name) + "{};\n";
  }
  if (CarriesConstructor(table, options, enums)) { out += "\n  " + tableClass + "();\n"; }

  out += ClassConstants(table);

  out += "\n";
  for (const al::FieldDecl &field : table.fields) {
    for (const al::Trigger &trigger : field.triggers) {
      out += "  void " + trigger.name + FieldIdentifier(table, field.name) + "();\n";
    }
  }
  const std::set<std::string> shadowed = Shadowed(table);
  std::string publics;
  std::string locals;
  for (const al::ProcedureDecl &procedure : table.procedures) {
    (procedure.isLocal ? locals : publics) +=
        ProcedureDeclaration(procedure,
                             objects,
                             table.name,
                             shadowed,
                             table.procedures,
                             ProcedureIdentifier(table, procedure.name));
  }
  if (!table.variables.empty()) {
    out += "\n  struct Variables {\n";
    for (const al::VarDecl &declared : table.variables) {
      std::string type = QualifiedType(DeclaredType(declared, objects), shadowed);
      if (DeclaresAnObject(declared)) { type.insert(0, "Instance<").append(">"); }
      out += "    " + type + " " + VariableIdentifier(table, declared.name) + ";\n";
    }
    out += "  };\n\n  Globals<Variables> Var_Block;\n";
  }
  if (!publics.empty()) { out += "\n" + publics; }
  if (!locals.empty()) { out += "\nprivate:\n" + locals; }
  out += "};\n\n";
  return out;
}

}

std::set<std::string> Shadowed(const al::TableObject &table) {
  std::set<std::string> hidden;
  for (const al::FieldDecl &field : table.fields) {
    hidden.insert(FieldIdentifier(table, field.name));
  }
  for (const al::VarDecl &declared : table.variables) { hidden.insert(Identifier(declared.name)); }
  for (const al::ProcedureDecl &procedure : table.procedures) {
    hidden.insert(Identifier(procedure.name));
  }
  for (const al::LabelDecl &label : table.labels) { hidden.insert(Identifier(label.name)); }
  return hidden;
}

std::string TableDefinitions(const al::TableObject &declared, const Objects &objects) {
  const std::string space = NamespaceOf(declared.nameSpace);
  const EnumIndex &enums = objects.enums;
  const al::TableObject table = WithSystemFields(declared);
  const std::string tableIdentifier = Identifier(table.name);
  const std::vector<OptionField> options = OptionFields(table);
  const std::vector<const al::FieldDecl *> sorted = ByNumber(table);
  const std::string tableClass = ClassName(tableIdentifier, ObjectKind::Table);
  const std::string qualified = space + "::" + tableClass;
  std::string out = "namespace " + space + " {\n\n";
  out += FieldTable(table, sorted, tableIdentifier, options, enums, objects.pages);

  out += "constexpr std::array<KeyDef, " + std::to_string(table.keys.size()) + "> k" +
         tableIdentifier + "Keys{{\n";
  for (std::size_t i = 0; i < table.keys.size(); ++i) {
    const auto said = [&](std::string_view name, bool absent) {
      const al::Property *found = Find(table.keys[i].properties, name);
      return found == nullptr ? absent : LowerKey(found->text) == "true";
    };
    const al::Property *sums = Find(table.keys[i].properties, "SumIndexFields");
    out += "    KeyDef{.name = " + Literal(table.keys[i].name) + ", .fields = " + tableClass +
           "::" + KeyArrayName(i) +
           ", .clustered = " + (said("Clustered", false) ? "true" : "false");
    if (!said("Enabled", true)) { out += ", .enabled = false"; }
    if (sums != nullptr) {
      out += ", .sumIndexFields = " + tableClass + "::" + KeyArrayName(i) + "Sums";
    }
    if (!said("MaintainSiftIndex", true)) { out += ", .maintainSiftIndex = false"; }
    if (!said("MaintainSqlIndex", true)) { out += ", .maintainSqlIndex = false"; }
    if (said("Unique", false)) { out += ", .unique = true"; }
    const al::Property *included = Find(table.keys[i].properties, "IncludedFields");
    if (included != nullptr) { out += ", .includedFields = " + Literal(included->text); }
    for (const auto &[name, member] :
         {std::pair<std::string_view, std::string_view>{"Description", "description"},
          std::pair<std::string_view, std::string_view>{"ObsoleteState", "obsoleteState"}}) {
      const al::Property *found = Find(table.keys[i].properties, name);
      if (found != nullptr) { out += ", ." + std::string(member) + " = " + Literal(found->text); }
    }
    out += "},\n";
  }
  out += "}};\n\n";

  out += "constexpr TableDef k" + tableIdentifier + "Table{\n";
  out += "    .id = " + tableClass + "::kId,\n";
  out += "    .name = " + tableClass + "::kName,\n";
  const al::Property *named = Find(table.properties, "Caption");
  out += "    .caption = ";
  out += named == nullptr ? tableClass + "::kName" : Literal(named->text);
  out += ",\n";
  out += "    .fields = k" + tableIdentifier + "Fields,\n";
  out += "    .keys = k" + tableIdentifier + "Keys,\n";
  const auto property = [&table](std::string_view name) {
    const al::Property *found = Find(table.properties, name);
    return found == nullptr ? std::string{} : found->text;
  };
  for (const auto &[name, member] :
       {std::pair<std::string_view, std::string_view>{"ExternalName", "externalName"},
        std::pair<std::string_view, std::string_view>{"ExternalSchema", "externalSchema"}}) {
    const std::string said = property(name);
    if (!said.empty()) { out += "    ." + std::string(member) + " = " + Literal(said) + ",\n"; }
  }
  const std::string kind = property("TableType");
  if (!kind.empty()) {
    static constexpr std::array kTableTypes{
        "Normal", "CRM", "CDS", "ExternalSQL", "Exchange", "MicrosoftGraph", "Temporary"};
    const auto *const spelled = std::ranges::find_if(
        kTableTypes, [&](const char *name) { return LowerKey(name) == LowerKey(kind); });
    out += "    .tableType = TableType::" +
           (spelled == kTableTypes.end() ? kind : std::string(*spelled)) + ",\n";
  }
  const std::string perCompany = property("DataPerCompany");
  if (LowerKey(perCompany) == "false") { out += "    .dataPerCompany = false,\n"; }
  const std::string replicate = property("ReplicateData");
  if (LowerKey(replicate) == "false") { out += "    .replicateData = false,\n"; }
  for (const auto &[name, member] :
       {std::pair<std::string_view, std::string_view>{"DataAccessIntent", "dataAccessIntent"},
        std::pair<std::string_view, std::string_view>{"CompressionType", "compressionType"}}) {
    const std::string said = property(name);
    if (!said.empty()) { out += "    ." + std::string(member) + " = " + Literal(said) + ",\n"; }
  }
  if (!property("DataCaptionFields").empty()) {
    out += "    .dataCaptionFields = " + tableClass + "::kDataCaptionFields,\n";
  }
  for (const auto &[name, member] :
       {std::pair<std::string_view, std::string_view>{"Permissions", "permissions"},
        std::pair<std::string_view, std::string_view>{"InherentPermissions", "inherentPermissions"},
        std::pair<std::string_view, std::string_view>{"InherentEntitlements",
                                                      "inherentEntitlements"},
        std::pair<std::string_view, std::string_view>{"Extensible", "extensible"},
        std::pair<std::string_view, std::string_view>{"Access", "access"},
        std::pair<std::string_view, std::string_view>{"MovedFrom", "movedFrom"},
        std::pair<std::string_view, std::string_view>{"MovedTo", "movedTo"}}) {
    const std::string said = property(name);
    if (!said.empty()) { out += "    ." + std::string(member) + " = " + Literal(said) + ",\n"; }
  }
  for (const auto &[name, member] :
       {std::pair<std::string_view, std::string_view>{"LookupPageId", "lookupPageId"},
        std::pair<std::string_view, std::string_view>{"DrillDownPageId", "drillDownPageId"}}) {
    const std::string said = PageNumber(objects.pages, property(name));
    if (!said.empty()) {
      out += "    ." + std::string(member) + " = ::agiru::PageId{" + said + "},\n";
    }
  }
  {
    if (LowerKey(property("PasteIsValid")) == "false") { out += "    .pasteIsValid = false,\n"; }
    const std::string describe = property("Description");
    if (!describe.empty()) { out += "    .description = " + Literal(describe) + ",\n"; }
    const std::string said = property("AllowInCustomizations");
    if (!said.empty()) { out += "    .allowInCustomizations = " + Literal(said) + ",\n"; }
  }
  const std::string obsolete = property("ObsoleteState");
  if (!obsolete.empty()) { out += "    .obsoleteState = " + Literal(obsolete) + ",\n"; }
  out += "};\n\n";

  out += "static_assert(FieldsAreSorted(k";
  out += tableIdentifier;
  out += "Table),\n";
  out += "              \"the field table is emitted sorted by field number, which is what lets ";
  out += "Field() \"\n";
  out += "              \"binary-search it\");\n";
  out += "static_assert(offsetof(" + qualified;
  out += ", State_Block) == 0,\n";
  out += "              \"the record variable's state is the FIRST member, which is how the base ";
  out += "reaches it \"\n";
  out += "              \"through the address of the object\");\n";
  out += "static_assert(std::is_standard_layout_v<";
  out += tableClass;
  out += ">,\n";
  out += "              \"offsetof over the field table requires standard layout. The base ";
  out += "carries NO data, \"\n";
  out += "              \"which is what keeps it so\");\n";
  out += "static_assert(k";
  out += tableIdentifier;
  out += "Fields.size() == ";
  out += std::to_string(sorted.size() - kSystemFieldCount);
  out += " + kSystemFieldCount, \"table ";
  out += std::to_string(table.id);
  out += " declares ";
  out += std::to_string(sorted.size() - kSystemFieldCount);
  out += " fields, and the platform adds its own\");\n\n";
  out += "static_assert(k";
  out += tableIdentifier;
  out += "Keys.size() <= ::agiru::kMaximumKeys,\n";
  out += "              \"a table declares at most 40 keys (devenv-table-keys.md)\");\n";
  if (!table.keys.empty()) {
    out += "static_assert(";
    out += tableClass;
    out += "::" + KeyArrayName(0) + ".size() <= ::agiru::kMaximumPrimaryKeyFields,\n";
    out += "              \"a primary key names at most 16 fields "
           "(devenv-table-keys.md)\");\n";
    out += "static_assert(!k";
    out += tableIdentifier;
    out += "Keys.empty(), \"keys[0] IS the primary key, so a table has one\");\n";
  }
  out += "\n";
  out += "} // namespace " + space + "\n\n";
  return out;
}

TableHeader WriteHeader(const al::TableObject &declared,
                        const std::string &sourcePath,
                        const EnumIndex &enums,
                        const Objects &objects) {
  const al::TableObject table = WithSystemFields(declared);
  const std::string tableIdentifier = Identifier(table.name);
  const std::string space = NamespaceOf(table.nameSpace);
  const std::string qualified = space + "::" + ClassName(tableIdentifier, ObjectKind::Table);
  const std::vector<OptionField> options = OptionFields(table);
  const std::vector<const al::FieldDecl *> sorted = ByNumber(table);

  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#pragma once\n\n";
  out += Includes(table, options, enums);
  out += "#include <array>\n#include <cstddef>\n#include <cstdint>\n";
  out += "#include <string_view>\n#include <type_traits>\n\n";
  std::vector<al::ProcedureDecl> bodies = table.procedures;
  for (const al::FieldDecl &field : table.fields) {
    bodies.insert(bodies.end(), field.triggers.begin(), field.triggers.end());
  }
  if (NamesAbsentIn(table.variables, bodies, objects)) { out += "#include \"absent/Types.h\"\n\n"; }
  out += Declarations(table, objects);

  std::map<std::string, std::vector<std::string>> fieldOptions;
  for (const OptionField &option : options) {
    fieldOptions.emplace(option.enumName, option.members);
  }
  out += InlineOptionsOf(table.name, "tables", table.variables, bodies, fieldOptions);

  if (!options.empty() || DeclaresAnOption(table.variables, bodies)) {
    out += "#include \"options/Types.h\"\n\n";
  }

  out += ClassBody(table, tableIdentifier, options, enums, objects);

  out += "extern const TableDef k" + tableIdentifier + "Table;\n\n";
  out += "} // namespace " + space + "\n\n";

  out += "template <> struct agiru::TableTraits<" + qualified + "> {\n";
  out +=
      "  static constexpr const TableDef &kTable = " + space + "::k" + tableIdentifier + "Table;\n";
  std::string validators;
  std::size_t validated = 0;
  for (const al::FieldDecl &field : table.fields) {
    for (const al::Trigger &trigger : field.triggers) {
      if (LowerKey(trigger.name) != "onvalidate") { continue; }
      const std::string member = FieldIdentifier(table, field.name);
      validators += "      {.field = " + qualified;
      validators += "::Field_No::";
      validators += member;
      validators += ",\n       .run = [](" + qualified;
      validators += " &record) { record.OnValidate";
      validators += member;
      validators += "(); }},\n";
      ++validated;
    }
  }
  if (!validators.empty()) {
    out += "  static constexpr std::array<agiru::OnValidateOf<" + qualified + ">, " +
           std::to_string(validated) + "> kOnValidate{{\n" + validators + "  }};\n";
  }
  std::string lookups;
  std::size_t lookedUp = 0;
  for (const al::FieldDecl &field : table.fields) {
    for (const al::Trigger &trigger : field.triggers) {
      if (LowerKey(trigger.name) != "onlookup") { continue; }
      const std::string member = FieldIdentifier(table, field.name);
      lookups += "      {.field = " + qualified + "::Field_No::" + member + ",\n       .run = [](" +
                 qualified + " &record) { record.OnLookup" + member + "(); }},\n";
      ++lookedUp;
    }
  }
  if (!lookups.empty()) {
    out += "  static constexpr std::array<agiru::OnLookupOf<" + qualified + ">, " +
           std::to_string(lookedUp) + "> kOnLookup{{\n" + lookups + "  }};\n";
  }
  out += "};\n";
  if (CarriesConstructor(table, options, enums)) {
    out +=
        "\ninline " + qualified + "::" + ClassName(tableIdentifier, ObjectKind::Table) + "() {\n";
    if (CarriesInitValues(table, options, enums)) {
      out += "  ::agiru::detail::RuntimeInitValues(this, ::agiru::TableTraits<" + qualified +
             ">::kTable);\n";
    }
    if (DeclaredTemporary(table)) {
      out +=
          "  ::agiru::detail::RuntimeMakeTemporary(this, &::agiru::kTempOps<" + qualified + ">);\n";
    }
    out += "}\n";
  }
  DotNetUse dotnet;
  DotNetUse absent;
  GatherAbsentIn(table.variables, bodies, objects, dotnet, absent);
  return TableHeader{.text = WithDoor(out, ObjectKind::Table),
                     .unresolvedEnums = Unresolved(table, enums),
                     .dotnet = dotnet,
                     .absent = absent};
}

}

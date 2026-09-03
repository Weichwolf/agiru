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
#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <string_view>
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
    option.enumName = OptionEnumName(table.name, field.name);
    option.members = ListValue(*members);
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

} // namespace

std::string FieldIdentifier(const al::TableObject &table, const std::string &name) {
  std::set<std::string> taken;
  for (const SystemFieldDecl &system : kSystemFields) {
    taken.insert(LowerKey(std::string(system.name)));
  }
  std::string collapsed;
  for (const al::FieldDecl &field : table.fields) {
    const std::string bare = Identifier(field.name);
    const bool platform = field.number >= kSystemFields.front().no.Value();
    const bool collides = !taken.insert(LowerKey(bare)).second && !platform;
    const std::string spelled = collides ? bare + "_" + std::to_string(field.number) : bare;
    if (LowerKey(field.name) == LowerKey(name)) { return spelled; }
    if (collapsed.empty() && LowerKey(bare) == LowerKey(Identifier(name))) { collapsed = spelled; }
  }
  return collapsed.empty() ? Identifier(name) : collapsed;
}

namespace {

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
  if (type == "FieldNo" || HiddenByABaseMember(type)) { return "::agiru::" + bare; }
  return ShadowedByAField(table, type) ? "::agiru::" + bare : bare;
}

std::string MemberType(const al::TableObject &table,
                       const al::FieldDecl &field,
                       const OptionField *option,
                       const EnumIndex &enums) {
  if (option != nullptr) { return Reach(table, "Option", "Option<" + option->enumName + ">"); }
  if (IsEnumField(field)) {
    const auto found = enums.find(LowerKey(field.subtype));
    return found != enums.end()
               ? Reach(table, "Enum", "Enum<enums::" + found->second.identifier + ">")
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

void WriteOptionTraits(std::string &out, const OptionField &option) {
  out += "template <> struct agiru::OptionTraits<agiru::app::tables::" + option.enumName + "> {\n";
  out += "  static constexpr std::array<EnumValueDef, " + std::to_string(option.members.size()) +
         "> kValues{{\n";
  for (std::size_t i = 0; i < option.members.size(); ++i) {
    out += "      EnumValueDef{.ordinal = " + std::to_string(i) +
           ", .name = " + Literal(option.members[i]) +
           ", .caption = " + Literal(option.captions[i]) + "},\n";
  }
  out += "  }};\n";
  out += "};\n";
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

std::string FieldTable(const al::TableObject &table,
                       const std::vector<const al::FieldDecl *> &sorted,
                       const std::string &tableIdentifier) {
  const std::size_t declaredCount = sorted.size() - kSystemFieldCount;
  std::string out = "inline constexpr auto k" + tableIdentifier + "Fields = WithSystemFields<" +
                    tableIdentifier + ">(std::array<FieldDef, " + std::to_string(declaredCount) +
                    ">{{\n";
  for (const al::FieldDecl *field : sorted) {
    if (IsSystemField(*field)) { continue; }
    const std::string identifier = FieldIdentifier(table, field->name);
    out += "    Declare<&";
    out += tableIdentifier;
    out += "::";
    out += identifier;
    out += ">(";
    out += tableIdentifier;
    out += "::Field_No::";
    out += identifier;
    out += ", ";
    out += Literal(field->name);
    out += ", ";
    out += Literal(Caption(*field));
    out += ", offsetof(";
    out += tableIdentifier;
    out += ", ";
    out += identifier;
    out += ")),\n";
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

} // namespace

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
  return std::ranges::any_of(table.fields, [&](const al::FieldDecl &field) {
    return LowerKey(Identifier(field.name)) == LowerKey(spelled);
  });
}

} // namespace

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
    Indexed(objects.pages, declared.subtype, reached.headers);
    return;
  }
  if (alType == "Interface") {
    const auto found = objects.interfaces.find(LowerKey(declared.subtype));
    if (found != objects.interfaces.end()) {
      reached.ahead["interfaces"].insert(
          found->second.identifier.substr(std::size("interfaces::") - 1));
    }
    return;
  }
  if (alType == "Enum") {
    Indexed(objects.enums, declared.subtype, reached.headers);
    return;
  }
  const TableRef *ref = ReachObject(declared, objects);
  if (ref == nullptr || ref->header.empty()) { return; }
  const std::size_t colons = ref->identifier.find("::");
  if (colons == std::string::npos) { return; }
  reached.ahead[ref->identifier.substr(0, colons)].insert(ref->identifier.substr(colons + 2));
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
  }
  for (const std::string &header : memberHeaders) { out += "#include \"" + header + "\"\n"; }
  if (!memberHeaders.empty()) { out += "\n"; }
  for (const auto &[space, objectNames] : ahead) {
    const ObjectKind kind = KindOfNamespace(space);
    out += "namespace agiru::app::" + space + " {\n";
    for (const std::string &one : objectNames) {
      out += "class " + ClassName(one, kind) + ";\n" + ClassAlias(one, kind);
    }
    out += "} // namespace agiru::app::" + space + "\n";
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
  std::string out;
  out += "namespace agiru::app::tables {\n\n";
  const std::string tableClass = ClassName(tableIdentifier, ObjectKind::Table);
  out += "class " + tableClass + ";\n" + ClassAlias(tableIdentifier, ObjectKind::Table) + "\n";
  out += "class " + tableClass + " : public Table<" + tableClass + "> {\npublic:\n";
  out += "  static constexpr " + Reach(table, "TableId", "TableId") + " kId{" +
         std::to_string(table.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(table.name) + "};\n\n";

  for (const al::FieldDecl &field : table.fields) {
    out += "  " + MemberType(table, field, OptionOf(options, field), enums) + " " +
           FieldIdentifier(table, field.name) + "{};\n";
  }

  out += ClassConstants(table);

  out += "\n";
  for (const al::FieldDecl &field : table.fields) {
    for (const al::Trigger &trigger : field.triggers) {
      out += "  void " + trigger.name + FieldIdentifier(table, field.name) + "();\n";
    }
  }
  std::set<std::string> shadowed;
  for (const al::FieldDecl &field : table.fields) {
    shadowed.insert(FieldIdentifier(table, field.name));
  }
  for (const al::VarDecl &declared : table.variables) {
    shadowed.insert(Identifier(declared.name));
  }
  for (const al::ProcedureDecl &procedure : table.procedures) {
    shadowed.insert(Identifier(procedure.name));
  }
  for (const al::LabelDecl &label : table.labels) { shadowed.insert(Identifier(label.name)); }
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
    out += "  };\n\n  Instance<Variables> Var_Block;\n";
  }
  if (!publics.empty()) { out += "\n" + publics; }
  if (!locals.empty()) { out += "\nprivate:\n" + locals; }
  out += "};\n\n";
  return out;
}

} // namespace

TableHeader WriteHeader(const al::TableObject &declared,
                        const std::string &sourcePath,
                        const EnumIndex &enums,
                        const Objects &objects) {
  const al::TableObject table = WithSystemFields(declared);
  const std::string tableIdentifier = Identifier(table.name);
  const std::vector<OptionField> options = OptionFields(table);
  const std::vector<const al::FieldDecl *> sorted = ByNumber(table);

  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#pragma once\n\n";
  out += Includes(table, options, enums);
  out += "#include <array>\n#include <cstddef>\n#include <cstdint>\n";
  out += "#include <string_view>\n#include <type_traits>\n\n";
  if (NamesAbsentIn(table.variables, table.procedures, objects)) {
    out += "#include \"absent/Types.h\"\n\n";
  }
  out += Declarations(table, objects);

  out += InlineOptionsOf(table.name, "tables", table.variables, table.procedures);

  out += "namespace agiru::app::tables {\n\n";
  for (const OptionField &option : options) {
    const std::vector<std::string> names = EnumeratorNames(option.members);
    out += "enum class " + option.enumName + " : std::int32_t {\n";
    for (std::size_t i = 0; i < names.size(); ++i) {
      out += "  " + names[i] + " = " + std::to_string(i) + ",\n";
    }
    out += "};\n\n";
  }
  out += "} // namespace agiru::app::tables\n\n";

  for (const OptionField &option : options) {
    WriteOptionTraits(out, option);
    out += "\n";
  }

  out += ClassBody(table, tableIdentifier, options, enums, objects);

  out += FieldTable(table, sorted, tableIdentifier);

  out += "inline constexpr std::array<KeyDef, " + std::to_string(table.keys.size()) + "> k" +
         tableIdentifier + "Keys{{\n";
  for (std::size_t i = 0; i < table.keys.size(); ++i) {
    const al::Property *clustered = Find(table.keys[i].properties, "Clustered");
    out += "    KeyDef{.name = " + Literal(table.keys[i].name) + ", .fields = " + tableIdentifier +
           "::" + KeyArrayName(i) + ", .clustered = " +
           (clustered != nullptr && clustered->text == "true" ? "true" : "false") + "},\n";
  }
  out += "}};\n\n";

  out += "inline constexpr TableDef k" + tableIdentifier + "Table{\n";
  out += "    .id = " + tableIdentifier + "::kId,\n";
  out += "    .name = " + tableIdentifier + "::kName,\n";
  out += "    .caption = " + tableIdentifier + "::kName,\n";
  out += "    .fields = k" + tableIdentifier + "Fields,\n";
  out += "    .keys = k" + tableIdentifier + "Keys,\n};\n\n";

  out += "static_assert(FieldsAreSorted(k";
  out += tableIdentifier;
  out += "Table),\n";
  out += "              \"the field table is emitted sorted by field number, which is what lets ";
  out += "Field() \"\n";
  out += "              \"binary-search it\");\n";
  out += "static_assert(std::is_standard_layout_v<";
  out += tableIdentifier;
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
  out += "} // namespace agiru::app::tables\n\n";

  out += "template <> struct agiru::TableTraits<agiru::app::tables::" + tableIdentifier + "> {\n";
  out += "  static constexpr const TableDef &kTable = agiru::app::tables::k" + tableIdentifier +
         "Table;\n";
  out += "};\n";
  DotNetUse dotnet;
  DotNetUse absent;
  GatherAbsentIn(table.variables, table.procedures, objects, dotnet, absent);
  return TableHeader{.text = WithDoor(out, ObjectKind::Table),
                     .unresolvedEnums = Unresolved(table, enums),
                     .dotnet = dotnet,
                     .absent = absent};
}

} // namespace agiru::gen

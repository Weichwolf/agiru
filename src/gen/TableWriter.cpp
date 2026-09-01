#include "TableWriter.h"

#include "Ast.h"
#include "EnumWriter.h"
#include "Names.h"
#include "Token.h"

#include <algorithm>
#include <cstddef>
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

bool IsEnumField(const al::FieldDecl &field) {
  return TypeName(field.type) == "Enum" && !field.subtype.empty();
}

bool ShadowedByAField(const al::TableObject &table, std::string_view type) {
  return std::ranges::any_of(
      table.fields, [type](const al::FieldDecl &field) { return Identifier(field.name) == type; });
}

std::string Reach(const al::TableObject &table, const std::string &type, const std::string &bare) {
  return ShadowedByAField(table, type) ? "::agiru::" + bare : bare;
}

std::string MemberType(const al::TableObject &table,
                       const al::FieldDecl &field,
                       const OptionField *option,
                       const EnumIndex &enums) {
  if (option != nullptr) { return Reach(table, "Option", "Option<" + option->enumName + ">"); }
  if (IsEnumField(field)) {
    const auto found = enums.find(LowerKey(field.subtype));
    const std::string named =
        found != enums.end() ? found->second.identifier : Identifier(field.subtype);
    return Reach(table, "Enum", "Enum<enums::" + named + ">");
  }
  const std::string type = TypeName(field.type);
  if (type == "Code" || type == "Text") {
    return Reach(table, type, type + "<" + std::to_string(field.length) + ">");
  }
  return Reach(table, type, type);
}

// SORTED BY FIELD NUMBER, NOT AS AL DECLARED IT. Field() binary-searches and the emitted
// static_assert holds it to that, and AL does not always oblige: 19 of the BaseApp's 1 545 tables
// declare their fields out of order, Sales Line among them (measured 2026-09-01). The MEMBERS keep
// AL's order, because that is what a reader compares against the .al file and because `offsetof`
// does not care; only the field table moves.
std::vector<const al::FieldDecl *> ByNumber(const al::TableObject &table) {
  std::vector<const al::FieldDecl *> fields;
  fields.reserve(table.fields.size());
  for (const al::FieldDecl &field : table.fields) { fields.push_back(&field); }
  std::ranges::sort(
      fields, [](const al::FieldDecl *a, const al::FieldDecl *b) { return a->number < b->number; });
  return fields;
}

void WriteOptionTraits(std::string &out, const OptionField &option) {
  out += "template <> struct agiru::OptionTraits<agiru::app::" + option.enumName + "> {\n";
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
  std::set<std::string> headers{
      "agiru/Declare.h", "agiru/Ids.h", "agiru/Table.h", "agiru/TableDef.h"};
  for (const al::FieldDecl &field : table.fields) {
    if (OptionOf(options, field) != nullptr) {
      headers.insert("agiru/Option.h");
      continue;
    }
    if (IsEnumField(field)) {
      headers.insert("agiru/Enum.h");
      const auto found = enums.find(LowerKey(field.subtype));
      if (found != enums.end()) { headers.insert(found->second.header); }
      continue;
    }
    headers.insert("agiru/" + TypeName(field.type) + ".h");
  }
  std::string out;
  for (const std::string &header : headers) {
    out += "#include \"";
    out += header;
    out += "\"\n";
  }
  out += "\n";
  return out;
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

TableHeader
WriteHeader(const al::TableObject &table, const std::string &sourcePath, const EnumIndex &enums) {
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

  out += "namespace agiru::app {\n\n";
  for (const OptionField &option : options) {
    out += "enum class " + option.enumName + " : std::int32_t {\n";
    for (std::size_t i = 0; i < option.members.size(); ++i) {
      out += "  " + EnumeratorName(option.members[i]) + " = " + std::to_string(i) + ",\n";
    }
    out += "};\n\n";
  }
  out += "} // namespace agiru::app\n\n";

  for (const OptionField &option : options) {
    WriteOptionTraits(out, option);
    out += "\n";
  }

  out += "namespace agiru::app {\n\n";
  out += "class " + tableIdentifier + " : public Table<" + tableIdentifier + "> {\npublic:\n";
  out += "  static constexpr TableId kId{" + std::to_string(table.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(table.name) + "};\n\n";

  for (const al::FieldDecl &field : table.fields) {
    out += "  " + MemberType(table, field, OptionOf(options, field), enums) + " " +
           Identifier(field.name) + ";\n";
  }

  out += "\n  struct FieldNumber {\n";
  for (const al::FieldDecl &field : table.fields) {
    out += "    static constexpr FieldNo " + Identifier(field.name) + "{" +
           std::to_string(field.number) + "};\n";
  }
  out += "  };\n\n";

  for (const al::KeyDecl &key : table.keys) {
    out += "  static constexpr std::array k" + Identifier(key.name) + "{";
    for (std::size_t i = 0; i < key.fields.size(); ++i) {
      if (i != 0) { out += ", "; }
      out += "FieldNumber::" + Identifier(key.fields[i]);
    }
    out += "};\n";
  }

  if (!table.labels.empty()) { out += "\n"; }
  for (const al::LabelDecl &label : table.labels) {
    out += "  static constexpr std::string_view " + label.name + "{" + Literal(label.text) + "};\n";
  }

  out += "\n";
  for (const al::FieldDecl &field : table.fields) {
    for (const al::Trigger &trigger : field.triggers) {
      out += "  void " + trigger.name + Identifier(field.name) + "();\n";
    }
  }
  out += "};\n\n";

  out += "inline constexpr std::array k" + tableIdentifier + "Fields{\n";
  for (const al::FieldDecl *field : sorted) {
    const std::string identifier = Identifier(field->name);
    out += "    Declare<&";
    out += tableIdentifier;
    out += "::";
    out += identifier;
    out += ">(";
    out += tableIdentifier;
    out += "::FieldNumber::";
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
  out += "};\n\n";

  out += "inline constexpr std::array k" + tableIdentifier + "Keys{\n";
  for (const al::KeyDecl &key : table.keys) {
    const al::Property *clustered = Find(key.properties, "Clustered");
    out += "    KeyDef{.name = " + Literal(key.name) + ", .fields = " + tableIdentifier + "::k" +
           Identifier(key.name) + ", .clustered = " +
           (clustered != nullptr && clustered->text == "true" ? "true" : "false") + "},\n";
  }
  out += "};\n\n";

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
  out += std::to_string(table.fields.size());
  out += ", \"table ";
  out += std::to_string(table.id);
  out += " declares ";
  out += std::to_string(table.fields.size());
  out += " fields\");\n\n";
  out += "} // namespace agiru::app\n\n";

  out += "template <> struct agiru::TableTraits<agiru::app::" + tableIdentifier + "> {\n";
  out += "  static constexpr const TableDef &kTable = agiru::app::k" + tableIdentifier + "Table;\n";
  out += "};\n";
  return TableHeader{.text = out, .unresolvedEnums = Unresolved(table, enums)};
}

} // namespace agiru::gen

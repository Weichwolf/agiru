#include "TableWriter.h"

#include "Ast.h"
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

std::string Quoted(std::string_view text) {
  return "\"" + std::string(text) + "\"";
}

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

bool ShadowedByAField(const al::TableObject &table, std::string_view type) {
  return std::ranges::any_of(
      table.fields, [type](const al::FieldDecl &field) { return Identifier(field.name) == type; });
}

std::string
MemberType(const al::TableObject &table, const al::FieldDecl &field, const OptionField *option) {
  if (option != nullptr) { return "Option<" + option->enumName + ">"; }
  if (field.type == "Code" || field.type == "Text") {
    const std::string bare = field.type + "<" + std::to_string(field.length) + ">";
    return ShadowedByAField(table, field.type) ? "::agiru::" + bare : bare;
  }
  return ShadowedByAField(table, field.type) ? "::agiru::" + field.type : field.type;
}

void WriteOptionTraits(std::string &out, const OptionField &option) {
  out += "template <> struct agiru::OptionTraits<agiru::app::" + option.enumName + "> {\n";
  for (const std::string_view which : {"kMembers", "kCaptions"}) {
    const std::vector<std::string> &values = which == "kMembers" ? option.members : option.captions;
    out += "  static constexpr std::array<std::string_view, " + std::to_string(values.size()) +
           "> " + std::string(which) + "{";
    for (std::size_t i = 0; i < values.size(); ++i) {
      if (i != 0) { out += ", "; }
      out += Quoted(values[i]);
    }
    out += "};\n";
  }
  out += "};\n";
}

std::string Includes(const al::TableObject &table, const std::vector<OptionField> &options) {
  std::set<std::string> headers{
      "agiru/Declare.h", "agiru/Ids.h", "agiru/Table.h", "agiru/TableDef.h"};
  for (const al::FieldDecl &field : table.fields) {
    const std::string type =
        OptionOf(options, field) != nullptr ? std::string("Option") : field.type;
    headers.insert("agiru/" + type + ".h");
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

} // namespace

std::string WriteHeader(const al::TableObject &table, const std::string &sourcePath) {
  const std::string tableIdentifier = Identifier(table.name);
  const std::vector<OptionField> options = OptionFields(table);

  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#pragma once\n\n";
  out += Includes(table, options);
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
  out += "  static constexpr std::string_view kName{" + Quoted(table.name) + "};\n\n";

  for (const al::FieldDecl &field : table.fields) {
    out += "  " + MemberType(table, field, OptionOf(options, field)) + " " +
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
    out += "  static constexpr std::string_view " + label.name + "{" + Quoted(label.text) + "};\n";
  }

  out += "\n";
  for (const al::FieldDecl &field : table.fields) {
    for (const al::Trigger &trigger : field.triggers) {
      out += "  void " + trigger.name + Identifier(field.name) + "();\n";
    }
  }
  out += "};\n\n";

  out += "inline constexpr std::array k" + tableIdentifier + "Fields{\n";
  for (const al::FieldDecl &field : table.fields) {
    const std::string identifier = Identifier(field.name);
    out += "    Declare<&";
    out += tableIdentifier;
    out += "::";
    out += identifier;
    out += ">(";
    out += tableIdentifier;
    out += "::FieldNumber::";
    out += identifier;
    out += ", ";
    out += Quoted(field.name);
    out += ", ";
    out += Quoted(Caption(field));
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
    out += "    KeyDef{.name = " + Quoted(key.name) + ", .fields = " + tableIdentifier + "::k" +
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
  return out;
}

} // namespace agiru::gen

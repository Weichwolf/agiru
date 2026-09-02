#include "TableWriter.h"

#include "Ast.h"
#include "EnumWriter.h"
#include "Names.h"
#include "Token.h"
#include "meta/Declare.h"
#include "meta/TableDef.h"

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

// AL IS CASE-INSENSITIVE AND A KEY MAY SPELL A FIELD DIFFERENTLY FROM THE FIELD ITSELF. `Default
// Dimension` declares `field(8000; ParentId; Guid)` and then `key(Key3; "Parent Type", ParentID)`:
// one AL field, two C++ identifiers, and the second names nothing. The key is therefore resolved
// against the DECLARED fields rather than spelled out as written, which is the collapse-match the
// generator already does for type names.
std::string FieldIdentifier(const al::TableObject &table, const std::string &name) {
  const std::string wanted = LowerKey(Identifier(name));
  for (const al::FieldDecl &field : table.fields) {
    if (LowerKey(Identifier(field.name)) == wanted) { return Identifier(field.name); }
  }
  return Identifier(name);
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

// A FIELD NAME CAN SHADOW A RUNTIME TYPE, AND NOT ONLY ITS OWN. `Change Log Setup (Field)` declares
// a field called `Field No.`, whose member is `FieldNo` -- and from that member onward `FieldNo`
// names the member rather than `agiru::FieldNo`, so every entry of the FieldNumber struct below it
// fails to compile. 122 of the BaseApp's 1 545 tables hit this (measured 2026-09-01). Every runtime
// name the class body uses after its members therefore goes through here, not just the field types.
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

// ONE LINE FOR THE DOOR AND ONE PER APP HEADER IT NEEDS. An AL file declares no includes, so the
// translation writes none either beyond what the C++ compiler cannot do without: `agiru.h`, and the
// enum objects this table's fields name, which live in other files of this same app.
std::string Includes(const al::TableObject &table,
                     const std::vector<OptionField> &options,
                     const EnumIndex &enums) {
  std::set<std::string> headers;
  for (const al::FieldDecl &field : table.fields) {
    if (OptionOf(options, field) != nullptr || !IsEnumField(field)) { continue; }
    const auto found = enums.find(LowerKey(field.subtype));
    if (found != enums.end()) { headers.insert(found->second.header); }
  }
  std::string out = "#include \"agiru.h\"\n";
  for (const std::string &header : headers) {
    out += "#include \"";
    out += header;
    out += "\"\n";
  }
  out += "\n";
  return out;
}

// EVERY TABLE CARRIES THE SYSTEM FIELDS AND THE PLATFORM PUTS THEM THERE, not the AL author.
// `devenv-table-system-fields.md` names five with their numbers, says the range 2000000000-
// 2147483647 is reserved for them, and says "system fields are fields that are automatically
// included in every table object by the platform". The BaseApp KEYS on them -- 121 keys name
// SystemModifiedAt, 26 name SystemId, 15 name SystemCreatedAt -- so a table without them has keys
// that name nothing, and that is what blocked those files.
//
// SystemRowVersion IS NOT HERE. AL exposes the SQL rowversion under that name and the page gives it
// no field NUMBER, unlike the five it tabulates. Inventing one would put a number in the metadata
// that nothing can check, and the rowversion needs its database half anyway (board:0013).
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

// AN EMPTY BRACED LIST CANNOT DEDUCE ITS ELEMENT TYPE, and a table with no fields is legal AL --
// BC declares several as pure event containers. `std::array k{}` is not a declaration the compiler
// can complete, so the element type is named when there is nothing to deduce it from.
std::string FieldTable(const std::vector<const al::FieldDecl *> &sorted,
                       const std::string &tableIdentifier) {
  const std::size_t declaredCount = sorted.size() - kSystemFieldCount;
  std::string out = "inline constexpr auto k" + tableIdentifier + "Fields = WithSystemFields<" +
                    tableIdentifier + ">(std::array<FieldDef, " + std::to_string(declaredCount) +
                    ">{{\n";
  for (const al::FieldDecl *field : sorted) {
    if (IsSystemField(*field)) { continue; }
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

TableHeader WriteHeader(const al::TableObject &declared,
                        const std::string &sourcePath,
                        const EnumIndex &enums) {
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

  out += "namespace agiru::app {\n\n";
  for (const OptionField &option : options) {
    const std::vector<std::string> names = EnumeratorNames(option.members);
    out += "enum class " + option.enumName + " : std::int32_t {\n";
    for (std::size_t i = 0; i < names.size(); ++i) {
      out += "  " + names[i] + " = " + std::to_string(i) + ",\n";
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
  out += "  static constexpr " + Reach(table, "TableId", "TableId") + " kId{" +
         std::to_string(table.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(table.name) + "};\n\n";

  // A RECORD STARTS BLANK, AND AL GUARANTEES IT. `var Rec: Record X` gives every field its zero --
  // an empty Code, a zero Decimal, the undefined date -- and AL code reads a fresh record without
  // writing to it first. C++ default-initialises a member of built-in type to nothing at all, so
  // without the braces an Integer field of a fresh record holds whatever was on the stack.
  for (const al::FieldDecl &field : table.fields) {
    out += "  " + MemberType(table, field, OptionOf(options, field), enums) + " " +
           Identifier(field.name) + "{};\n";
  }

  const std::string fieldNo = Reach(table, "FieldNo", "FieldNo");
  out += "\n  struct FieldNumber : SystemFieldNumbers {\n";
  for (const al::FieldDecl &field : table.fields) {
    if (IsSystemField(field)) { continue; }
    out += "    static constexpr " + fieldNo + " " + Identifier(field.name) + "{" +
           std::to_string(field.number) + "};\n";
  }
  out += "  };\n\n";

  // A KEY ARRAY IS NAMED BY ITS POSITION AND NOT BY ITS AL NAME. 19 of the BaseApp's keys are
  // called `Name`, whose array would be `kName` -- which is already the table's own name constant,
  // and a duplicate member the compiler refuses (measured 2026-09-01). A position cannot collide:
  // the class declares exactly two other `k` constants, and no FIELD member can ever start with a
  // lower-case k, since Identifier() upper-cases the first letter of every AL name. The AL name is
  // not lost -- it stands beside the array in the KeyDef, which is where a reader looks for it.
  for (std::size_t i = 0; i < table.keys.size(); ++i) {
    out += "  static constexpr std::array<FieldNo, " + std::to_string(table.keys[i].fields.size()) +
           "> " + KeyArrayName(i) + "{{";
    for (std::size_t f = 0; f < table.keys[i].fields.size(); ++f) {
      if (f != 0) { out += ", "; }
      out += "FieldNumber::" + FieldIdentifier(table, table.keys[i].fields[f]);
    }
    out += "}};\n";
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

  out += FieldTable(sorted, tableIdentifier);

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
  out += "} // namespace agiru::app\n\n";

  out += "template <> struct agiru::TableTraits<agiru::app::" + tableIdentifier + "> {\n";
  out += "  static constexpr const TableDef &kTable = agiru::app::k" + tableIdentifier + "Table;\n";
  out += "};\n";
  return TableHeader{.text = out, .unresolvedEnums = Unresolved(table, enums)};
}

} // namespace agiru::gen

#include "TableWriter.h"

#include "Ast.h"
#include "EnumWriter.h"
#include "Names.h"
#include "Token.h"
#include "meta/Declare.h"
#include "meta/TableDef.h"

#include <algorithm>
#include <functional>
#include <cstddef>
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

// AL IS CASE-INSENSITIVE AND A KEY MAY SPELL A FIELD DIFFERENTLY FROM THE FIELD ITSELF. `Default
// Dimension` declares `field(8000; ParentId; Guid)` and then `key(Key3; "Parent Type", ParentID)`:
// one AL field, two C++ identifiers, and the second names nothing. The key is therefore resolved
// against the DECLARED fields rather than spelled out as written, which is the collapse-match the
// generator already does for type names.
// AND TWO DIFFERENT AL NAMES MAY COLLAPSE INTO ONE IDENTIFIER. `Email Related Record` declares
// `field(3; "System Id"; Guid)` -- the SystemId of the record the mail relates to -- beside the
// platform's own `SystemId`, and both are legal AL because the space makes them different names.
// Identifier() removes the space, so the second one carries its FIELD NUMBER: a visible, uniform
// deviation rather than a silently dropped field, and the number is what AL itself would use to
// tell them apart.
std::string FieldIdentifier(const al::TableObject &table, const std::string &name) {
  // THE PLATFORM'S FIVE KEEP THEIR NAMES AND THE AL FIELD YIELDS, which is the opposite of the
  // order the fields are walked in. `Cost Adjmt. Action Message` declares `field(6; SystemID; Guid)`
  // -- one letter of case away from the platform's `SystemId` -- and letting the AL field win left
  // the table with no `SystemId` at all, which `WithSystemFields<T>` addresses by name and the door
  // promises to every client.
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
// names the member rather than `agiru::FieldNo`, so every entry of the Field_No struct below it
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
  // AN ENUMERATION THIS RUN NEVER SAW BECOMES `Enum<>`, for the reason the codeunit writer gives:
  // the platform declares some, BCApps holds only their extensions, and a named type nobody
  // declares stops the file.
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
std::string FieldTable(const al::TableObject &table,
                       const std::vector<const al::FieldDecl *> &sorted,
                       const std::string &tableIdentifier) {
  const std::size_t declaredCount = sorted.size() - kSystemFieldCount;
  std::string out = "inline constexpr auto k" + tableIdentifier + "Fields = WithSystemFields<" +
                    tableIdentifier + ">(std::array<FieldDef, " + std::to_string(declaredCount) +
                    ">{{\n";
  for (const al::FieldDecl *field : sorted) {
    if (IsSystemField(*field)) { continue; }
    // THE SAME SPELLING THE CLASS USES, and not `Identifier` again: a field whose name collapses
    // onto another's carries a seam, and a row that named the bare form pointed at a different
    // field's number -- which the sortedness assertion caught, three files away from the cause.
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

// A TABLE'S VARIABLE SITS IN THE SAME CLASS AS ITS FIELDS AND MAY COLLIDE WITH ONE. `Campaign`
// has a field `"No. Series"` and a variable `NoSeries: Codeunit "No. Series"` -- two different AL
// names that Identifier() spells the same. The variable yields, because a field is what the field
// table addresses by `offsetof` and what AL code names far more often, and it carries the interior
// underscore that no AL name can reach.
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

// A PROCEDURE MAY CARRY A FIELD'S NAME TOO. `General Ledger Setup` has a field
// `"Use Concurrent Posting"` and a procedure `UseConcurrentPosting()`; in AL they are a field and a
// procedure and never confusable, in C++ they are two members of one class. The FIELD keeps its
// spelling, because the field table addresses it by `offsetof` and AL code names it far more often.
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
  // A TABLE'S PROCEDURES NAME OBJECTS, and a declaration needs the NAME and not the layout:
  // `Currency.GetGainLossAccount` takes a `Record "Detailed CV Ledg. Entry Buffer"`. Including it
  // would put two tables' headers in a cycle the moment each names the other, which AL allows.
  std::map<std::string, std::set<std::string>> ahead;
  // A MEMBER NEEDS THE LAYOUT AND A DECLARATION NEEDS THE NAME -- the same rule the codeunits
  // follow. An object member is the exception, because it is a handle (board:0037). AN ENUM IS
  // ALWAYS INCLUDED: `Enum<enums::X>` is a template argument and a template argument is complete.
  std::set<std::string> memberHeaders;
  const std::function<void(const al::VarDecl &)> named = [&](const al::VarDecl &declared) {
    for (const al::VarDecl &argument : declared.arguments) { named(argument); }
    // A PAGE IS A TEMPLATE ARGUMENT AND A BASE CLASS, so `TestPage<pages::X>` needs the header.
    const std::string alType = TypeName(declared.type);
    if (alType == "TestPage" || alType == "TestRequestPage") {
      const auto found = objects.pages.find(LowerKey(declared.subtype));
      if (found != objects.pages.end() && !found->second.header.empty()) {
        memberHeaders.insert(found->second.header);
      }
      return;
    }
    // AN INTERFACE VARIABLE IS A POINTER, so the header needs the NAME and not the class.
    if (TypeName(declared.type) == "Interface") {
      const auto found = objects.interfaces.find(LowerKey(declared.subtype));
      if (found != objects.interfaces.end()) {
        ahead["interfaces"].insert(found->second.identifier.substr(std::size("interfaces::") - 1));
      }
      return;
    }
    if (TypeName(declared.type) == "Enum" && !declared.subtype.empty()) {
      const auto found = objects.enums.find(LowerKey(declared.subtype));
      if (found != objects.enums.end() && !found->second.header.empty()) {
        memberHeaders.insert(found->second.header);
      }
      return;
    }
    const TableRef *ref = ReachObject(declared, objects);
    if (ref == nullptr || ref->header.empty()) { return; }
    const std::size_t colons = ref->identifier.find("::");
    if (colons == std::string::npos) { return; }
    ahead[ref->identifier.substr(0, colons)].insert(ref->identifier.substr(colons + 2));
  };
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

  // A TABLE'S PROCEDURES DECLARE INLINE OPTIONS TOO, the same way a codeunit's do.
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

  out += "namespace agiru::app::tables {\n\n";
  const std::string tableClass = ClassName(tableIdentifier, ObjectKind::Table);
  // THE ALIAS COMES BEFORE THE CLASS, because a member may name the object's own type: a codeunit
  // returns `Codeunit "Http Request Message Impl."` and a table holds a filter on itself. Declared
  // after the class, the AL name is not yet known inside it.
  out += "class " + tableClass + ";\n" + ClassAlias(tableIdentifier, ObjectKind::Table) + "\n";
  out += "class " + tableClass + " : public Table<" + tableClass + "> {\npublic:\n";
  out += "  static constexpr " + Reach(table, "TableId", "TableId") + " kId{" +
         std::to_string(table.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(table.name) + "};\n\n";

  // A RECORD STARTS BLANK, AND AL GUARANTEES IT. `var Rec: Record X` gives every field its zero --
  // an empty Code, a zero Decimal, the undefined date -- and AL code reads a fresh record without
  // writing to it first. C++ default-initialises a member of built-in type to nothing at all, so
  // without the braces an Integer field of a fresh record holds whatever was on the stack.
  for (const al::FieldDecl &field : table.fields) {
    out += "  " + MemberType(table, field, OptionOf(options, field), enums) + " " +
           FieldIdentifier(table, field.name) + "{};\n";
  }

  const std::string fieldNo = Reach(table, "FieldNo", "FieldNo");
  out += "\n  struct Field_No : SystemFieldNumbers {\n";
  for (const al::FieldDecl &field : table.fields) {
    if (IsSystemField(field)) { continue; }
    out += "    static constexpr " + fieldNo + " " + FieldIdentifier(table, field.name) + "{" +
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

  out += "\n";
  for (const al::FieldDecl &field : table.fields) {
    for (const al::Trigger &trigger : field.triggers) {
      out += "  void " + trigger.name + FieldIdentifier(table, field.name) + "();\n";
    }
  }
  // A TABLE CARRIES CODE. `Tracking Specification` declares `procedure SetSourceFilter(...)` beside
  // its fields and the BaseApp calls it on a record; skipping them made every such call name a
  // member that is not there.
  // A FIELD NAME HIDES A TYPE FOR THE WHOLE CLASS, and a table's procedures sit below its fields:
  // `Currency` has a field `Code` and a procedure returning `Code[20]`.
  std::set<std::string> shadowed;
  for (const al::FieldDecl &field : table.fields) {
    shadowed.insert(FieldIdentifier(table, field.name));
  }
  for (const al::VarDecl &declared : table.variables) { shadowed.insert(Identifier(declared.name)); }
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
  // A TABLE'S VARIABLES LIVE BEHIND ONE POINTER, AND THE STANDARD-LAYOUT INVARIANT DECIDES IT.
  // `offsetof` over the field table requires standard layout, which requires every member to be
  // standard-layout too -- and `Item Application Entry` declares `Dictionary of [Integer, Boolean]`,
  // whose std::map is not. Fields are the class; everything AL puts in the table's `var` block goes
  // into one nested struct reached through `Var_Block`, which is two pointers and standard-layout.
  // The interior underscore is the seam no AL name can reach.
  if (!table.variables.empty()) {
    out += "\n  struct Variables {\n";
    for (const al::VarDecl &declared : table.variables) {
      // The nested struct is inside the class, so a field name hides a type here as well.
      std::string type = QualifiedType(DeclaredType(declared, objects), shadowed);
      if (DeclaresAnObject(declared)) { type = "Instance<" + type + ">"; }
      out += "    " + type + " " + VariableIdentifier(table, declared.name) + ";\n";
    }
    out += "  };\n\n  Instance<Variables> Var_Block;\n";
  }
  if (!publics.empty()) { out += "\n" + publics; }
  if (!locals.empty()) { out += "\nprivate:\n" + locals; }
  out += "};\n\n";

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
  return TableHeader{.text = out,
                     .unresolvedEnums = Unresolved(table, enums),
                     .dotnet = dotnet,
                     .absent = absent};
}

} // namespace agiru::gen

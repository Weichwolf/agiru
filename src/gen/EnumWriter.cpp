#include "EnumWriter.h"

#include "Ast.h"
#include "Door.h"
#include "Names.h"
#include "Scope.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::gen {

namespace {

std::string Caption(const al::EnumValueDecl &value) {
  const al::Property *property = Find(value.properties, "Caption");
  return property != nullptr ? property->text : value.name;
}

// SORTED BY ORDINAL, NOT AS AL DECLARED IT. ValueOf() binary-searches and the emitted
// static_assert holds it to that, and AL does not always oblige: FlushingMethodFilter declares
// 0, 1, 2, 3, 4, 6, 50, 5 (measured over the BaseApp, 2026-09-01). The C++ ENUMERATORS keep AL's
// order, because that is what a reader compares against the .al file; only the value table moves.
std::vector<const al::EnumValueDecl *> ByOrdinal(const al::EnumObject &object) {
  std::vector<const al::EnumValueDecl *> values;
  values.reserve(object.values.size());
  for (const al::EnumValueDecl &value : object.values) { values.push_back(&value); }
  std::ranges::sort(values, [](const al::EnumValueDecl *a, const al::EnumValueDecl *b) {
    return a->ordinal < b->ordinal;
  });
  return values;
}

} // namespace

std::string LowerKey(const std::string &alName) {
  std::string out;
  out.reserve(alName.size());
  for (const char c : alName) {
    out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return out;
}

std::string EnumHeaderPath(const al::EnumObject &object) {
  return OutputDirectory(object.nameSpace, ObjectKind::Enum) + "/" + Identifier(object.name) + ".h";
}

std::string WriteEnum(const al::EnumObject &object, const std::string &sourcePath) {
  const std::string identifier = Identifier(object.name);
  const std::string qualified = "agiru::app::enums::" + identifier;
  const std::vector<const al::EnumValueDecl *> sorted = ByOrdinal(object);

  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#pragma once\n\n";
  out += kDoorMarker;
  out += "\n";
  out += "#include <array>\n#include <cstdint>\n\n";

  // ENUM OBJECTS GET A NAMESPACE OF THEIR OWN AND TABLES DO NOT. AL tells an object's kind apart at
  // every use site -- `Record "Reminder Action"` against `Enum "Reminder Action"` -- and C++ has
  // one name space for types, so the kind has to become a namespace somewhere. It becomes one HERE
  // because table names carry the bulk of generated code and stay closest to AL unqualified: 4 of
  // the BaseApp's table names collide with an enum name and 1 with a generated option name
  // (measured 2026-09-01), and the rule is total rather than applied on collision, since a rule
  // that fires only on collision renames an object when an unrelated one is added.
  out += "namespace agiru::app::enums {\n\n";
  out += "enum class " + identifier + " : std::int32_t {\n";
  for (const al::EnumValueDecl &value : object.values) {
    out += "  " + EnumeratorName(value.name) + " = " + std::to_string(value.ordinal) + ",\n";
  }
  out += "};\n\n";
  out += "} // namespace agiru::app::enums\n\n";

  out += "template <> struct agiru::EnumTraits<" + qualified + "> {\n";
  if (sorted.empty()) {
    out += "  static constexpr std::array<EnumValueDef, 0> kValues{};\n";
  } else {
    out += "  static constexpr std::array<EnumValueDef, " + std::to_string(sorted.size()) +
           "> kValues{{\n";
    for (const al::EnumValueDecl *value : sorted) {
      out += "      EnumValueDef{.ordinal = " + std::to_string(value->ordinal) +
             ", .name = " + Literal(value->name) + ", .caption = " + Literal(Caption(*value)) +
             "},\n";
    }
    out += "  }};\n";
  }
  out += "};\n\n";

  out += "static_assert(agiru::ValuesAreSorted(agiru::EnumTraits<" + qualified + ">::kValues),\n";
  out += "              \"the value table is emitted sorted by ordinal, which is what lets "
         "ValueOf() \"\n";
  out += "              \"binary-search it\");\n";
  out += "static_assert(agiru::EnumTraits<" + qualified +
         ">::kValues.size() == " + std::to_string(object.values.size()) + ",\n";
  out += "              \"enum " + std::to_string(object.id) + " declares " +
         std::to_string(object.values.size()) + " values\");\n";
  return WithDoor(out, ObjectKind::Enum);
}

} // namespace agiru::gen

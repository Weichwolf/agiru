#include "EnumWriter.h"

#include "Ast.h"
#include "CodeunitWriter.h"
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

std::vector<const al::EnumValueDecl *> ByOrdinal(const al::EnumObject &object) {
  std::vector<const al::EnumValueDecl *> values;
  values.reserve(object.values.size());
  for (const al::EnumValueDecl &value : object.values) { values.push_back(&value); }
  std::ranges::sort(values, [](const al::EnumValueDecl *a, const al::EnumValueDecl *b) {
    return a->ordinal < b->ordinal;
  });
  return values;
}

}

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

std::string EnumSourcePath(const al::EnumObject &object) {
  return OutputDirectory(object.nameSpace, ObjectKind::Enum) + "/" + Identifier(object.name) +
         ".cpp";
}

std::string ImplementationDeclarations(const al::EnumObject &object,
                                       const std::string &identifier,
                                       const Objects &objects) {
  std::string out;
  for (const std::string &face : object.implements) {
    const auto known = objects.interfaces.find(LowerKey(face));
    if (known == objects.interfaces.end()) { continue; }
    const std::string bare =
        known->second.identifier.substr(known->second.identifier.find("::") + 2);
    out += "\nnamespace agiru::app::interfaces {\nclass ";
    out += ClassName(bare, ObjectKind::Interface);
    out += ";\n";
    out += ClassAlias(bare, ObjectKind::Interface);
    out += "}\n\nnamespace agiru::app::enums {\n\nagiru::app::";
    out += known->second.identifier;
    out += " *ImplementationOf(";
    out += identifier;
    out += " value, agiru::app::";
    out += known->second.identifier;
    out += " *);\n\n}\n";
  }
  return out;
}

std::string ImplementationBodies(const al::EnumObject &object,
                                 const std::string &identifier,
                                 const Objects &objects) {
  std::string out;
  for (const std::string &face : object.implements) {
    const auto known = objects.interfaces.find(LowerKey(face));
    if (known == objects.interfaces.end()) { continue; }
    const std::string faceType = "agiru::app::" + known->second.identifier;
    out += "\nnamespace agiru::app::enums {\n\n";
    out += faceType;
    out += " *ImplementationOf(";
    out += identifier;
    out += " value, ";
    out += faceType;
    out += " *) {\n  switch (value) {\n";
    for (const al::EnumValueDecl &value : object.values) {
      const al::Property *bound = al::Find(value.properties, "Implementation");
      if (bound == nullptr) { continue; }
      const std::size_t at = bound->text.find('=');
      if (at == std::string::npos) { continue; }
      std::string named = bound->text.substr(at + 1);
      while (!named.empty() && named.front() == ' ') { named.erase(0, 1); }
      while (!named.empty() && named.back() == ' ') { named.pop_back(); }
      const auto unit = objects.codeunits.find(LowerKey(named));
      if (unit == objects.codeunits.end()) { continue; }
      out += "    case ";
      out += identifier;
      out += "::";
      out += EnumeratorName(value.name);
      out += ":\n      return new agiru::app::";
      out += unit->second.identifier;
      out += "{};\n";
    }
    out += "  }\n  throw agiru::Error(\"this value of ";
    out += object.name;
    out += " names no implementation of ";
    out += face;
    out += "\");\n}\n\n}\n";
  }
  return out;
}

std::string WriteEnumSource(const al::EnumObject &object,
                            const std::string &sourcePath,
                            const Objects &objects) {
  const std::string identifier = Identifier(object.name);
  const std::string bodies = ImplementationBodies(object, identifier, objects);
  if (bodies.empty()) { return {}; }
  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n\n";
  out += "#include \"" + identifier + ".h\"\n\n";
  out += kDoorMarker;
  out += BodyIncludes(bodies, objects);
  out += bodies;
  return WithDoor(out, ObjectKind::Enum);
}

std::string
WriteEnum(const al::EnumObject &object, const std::string &sourcePath, const Objects &objects) {
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
  out += ImplementationDeclarations(object, identifier, objects);
  return WithDoor(out, ObjectKind::Enum);
}

}

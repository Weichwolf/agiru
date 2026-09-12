#include "EnumWriter.h"

#include "Ast.h"
#include "CodeunitWriter.h"
#include "Door.h"
#include "Names.h"
#include "Scope.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <set>
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

namespace {

std::string ImplementationDeclarations(const al::EnumObject &object,
                                       const std::string &identifier,
                                       const std::string &space,
                                       const Objects &objects) {
  std::string out;
  for (const std::string &face : object.implements) {
    const auto known = objects.interfaces.find(LowerKey(face));
    if (known == objects.interfaces.end()) { continue; }
    const std::string reachable = Unprefixed(known->second.identifier);
    const std::size_t colons = reachable.rfind("::");
    const std::string faceSpace =
        colons == std::string::npos ? "agiru" : "agiru::" + reachable.substr(0, colons);
    const std::string bare = colons == std::string::npos ? reachable : reachable.substr(colons + 2);
    out += "\nnamespace " + faceSpace + " {\nclass ";
    out += bare;
    out += ";\n";
    out += "}\n\nnamespace " + space + " {\n\n";
    out += known->second.identifier;
    out += " *ImplementationOf(";
    out += identifier;
    out += " value, ";
    out += known->second.identifier;
    out += " *);\n\n";
    out += "auto CloneOf(" + identifier + " value, " + known->second.identifier + " *) -> " +
           known->second.identifier + " *(*)(const " + known->second.identifier + " *);\n\n}\n";
  }
  return out;
}
}

namespace {

std::string Unquoted(std::string_view text) {
  while (!text.empty() && text.front() == ' ') { text.remove_prefix(1); }
  while (!text.empty() && text.back() == ' ') { text.remove_suffix(1); }
  if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
    text = text.substr(1, text.size() - 2);
  }
  return std::string(text);
}
}

namespace {

std::string ImplementationFor(std::string_view text, std::string_view face) {
  std::string current;
  bool quoted = false;
  std::vector<std::string> pairs;
  for (const char c : text) {
    if (c == '"') { quoted = !quoted; }
    if (c == ',' && !quoted) {
      pairs.push_back(current);
      current.clear();
      continue;
    }
    current += c;
  }
  pairs.push_back(current);
  for (const std::string &pair : pairs) {
    const std::size_t at = pair.find('=');
    if (at == std::string::npos) { continue; }
    if (LowerKey(Unquoted(pair.substr(0, at))) == LowerKey(std::string(face))) {
      return Unquoted(pair.substr(at + 1));
    }
  }
  return {};
}
}

namespace {

std::string ImplementationBodies(const al::EnumObject &object,
                                 const std::string &identifier,
                                 const std::string &space,
                                 const Objects &objects) {
  std::string out;
  for (const std::string &face : object.implements) {
    const auto known = objects.interfaces.find(LowerKey(face));
    if (known == objects.interfaces.end()) { continue; }
    const std::string faceType = known->second.identifier;
    out += "\nnamespace " + space + " {\n\n";
    out += faceType;
    out += " *ImplementationOf(";
    out += identifier;
    out += " value, ";
    out += faceType;
    out += " *) {\n  switch (value) {\n";
    std::vector<std::pair<std::string, std::string>> cloneable;
    for (const al::EnumValueDecl &value : object.values) {
      const al::Property *bound = al::Find(value.properties, "Implementation");
      if (bound == nullptr) { continue; }
      const std::string named = ImplementationFor(bound->text, face);
      if (named.empty()) { continue; }
      const auto unit = objects.codeunits.find(LowerKey(named));
      if (unit == objects.codeunits.end()) { continue; }
      out += "    case ";
      out += identifier;
      out += "::";
      out += EnumeratorName(value.name);
      out += ":\n      return new ";
      out += unit->second.identifier;
      out += "{};\n";
      cloneable.emplace_back(EnumeratorName(value.name), unit->second.identifier);
    }
    std::string fallback;
    for (const char *property : {"DefaultImplementation", "UnknownValueImplementation"}) {
      if (!fallback.empty()) { break; }
      const al::Property *given = al::Find(object.properties, property);
      if (given == nullptr) { continue; }
      const std::string named = ImplementationFor(given->text, face);
      if (named.empty()) { continue; }
      const auto unit = objects.codeunits.find(LowerKey(named));
      if (unit != objects.codeunits.end()) { fallback = unit->second.identifier; }
    }
    const std::string registered = "::agiru::detail::FindImplementation(" + Literal(object.name) +
                                   ", static_cast<std::int32_t>(value), " + Literal(face) + ")";
    out += "    default: break;\n  }\n";
    out += "  if (const ::agiru::detail::ForeignImplementation *foreign = " + registered +
           ";\n      foreign != nullptr) {\n    return static_cast<" + faceType +
           " *>(foreign->make());\n  }\n";
    if (!fallback.empty()) { out += "  return new " + fallback + "{};\n"; }
    out += "  throw agiru::Error(\"this value of ";
    out += object.name;
    out += " names no implementation of ";
    out += face;
    out += "\");\n}\n\n";
    const auto cloner = [&faceType](const std::string &unit) {
      return "[](const " + faceType + " *held) -> " + faceType + " * { return new " + unit +
             "(*dynamic_cast<const " + unit + " *>(held)); }";
    };
    out += "auto CloneOf(" + identifier + " value, " + faceType + " *) -> " + faceType +
           " *(*)(const " + faceType + " *) {\n  switch (value) {\n";
    for (const auto &[enumerator, unit] : cloneable) {
      out += "    case " + identifier + "::" + enumerator + ": return " + cloner(unit) + ";\n";
    }
    out += "    default: break;\n  }\n";
    out += "  if (" + registered + " != nullptr) {\n    return [](const " + faceType +
           " *held) -> " + faceType + " * {\n      return static_cast<" + faceType +
           " *>(::agiru::detail::CloneForeign(typeid(*held), " + Literal(face) +
           ", held));\n    };\n  }\n";
    out += fallback.empty() ? "  return nullptr;\n" : "  return " + cloner(fallback) + ";\n";
    out += "}\n\n}\n";
  }
  return out;
}
}

std::string ImplementationKey(std::string_view enumName, int ordinal, std::string_view face) {
  return LowerKey(std::string(enumName)) + "|" + std::to_string(ordinal) + "|" +
         LowerKey(std::string(face));
}

std::vector<ForeignImplementationRef> UnresolvedImplementations(const al::EnumObject &object,
                                                                const Objects &objects) {
  std::vector<ForeignImplementationRef> unresolved;
  for (const std::string &face : object.implements) {
    if (!objects.interfaces.contains(LowerKey(face))) { continue; }
    for (const al::EnumValueDecl &value : object.values) {
      const al::Property *bound = al::Find(value.properties, "Implementation");
      if (bound == nullptr) { continue; }
      const std::string named = ImplementationFor(bound->text, face);
      if (named.empty() || objects.codeunits.contains(LowerKey(named))) { continue; }
      unresolved.push_back(ForeignImplementationRef{
          .enumName = object.name, .ordinal = value.ordinal, .face = face, .codeunit = named});
    }
  }
  return unresolved;
}

std::string EnumExtensionSourcePath(const al::EnumExtensionObject &extension) {
  return OutputDirectory(extension.nameSpace, ObjectKind::Enum) + "/" + Identifier(extension.name) +
         ".cpp";
}

std::string WriteEnumExtensionSource(const al::EnumExtensionObject &extension,
                                     const std::string &sourcePath,
                                     const std::set<std::string> &pending,
                                     const Objects &objects,
                                     std::vector<std::string> &registered) {
  std::string bodies;
  for (const al::EnumValueDecl &value : extension.values) {
    const al::Property *bound = al::Find(value.properties, "Implementation");
    if (bound == nullptr) { continue; }
    for (const auto &[faceKey, faceRef] : objects.interfaces) {
      const std::string named = ImplementationFor(bound->text, faceKey);
      if (named.empty()) { continue; }
      const std::string key = ImplementationKey(extension.extends, value.ordinal, faceKey);
      if (!pending.contains(key)) { continue; }
      const auto unit = objects.codeunits.find(LowerKey(named));
      if (unit == objects.codeunits.end()) { continue; }
      const std::string &faceType = faceRef.identifier;
      const std::string &unitType = unit->second.identifier;
      bodies += "const ::agiru::detail::RegisterForeignImplementation k" +
                EnumeratorName(value.name) + "_" +
                Identifier(faceRef.name.empty() ? faceKey : faceRef.name) + "{\n";
      bodies += "    " + Literal(extension.extends) + ",\n    " + std::to_string(value.ordinal) +
                ",\n    " + Literal(faceRef.name.empty() ? faceKey : faceRef.name) +
                ",\n    typeid(" + unitType + "),\n";
      bodies += "    {.make = []() -> void * { return static_cast<" + faceType + " *>(new " +
                unitType + "{}); },\n";
      bodies += "     .clone = [](const void *held) -> void * {\n       return static_cast<" +
                faceType + " *>(new " + unitType + "(*dynamic_cast<const " + unitType +
                " *>(static_cast<const " + faceType + " *>(held))));\n     }}};\n\n";
      registered.push_back(key);
    }
  }
  if (bodies.empty()) { return {}; }
  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n\n";
  out += kDoorMarker;
  out += BodyIncludes(bodies, objects);
  out += "\n#include <typeinfo>\n\nnamespace {\n\n";
  out += bodies;
  out += "}\n";
  return WithDoor(out, ObjectKind::Enum);
}

std::string WriteEnumSource(const al::EnumObject &object,
                            const std::string &sourcePath,
                            const Objects &objects) {
  const std::string fileName = Identifier(object.name);
  const std::string identifier = ClassName(fileName, ObjectKind::Enum);
  const std::string bodies =
      ImplementationBodies(object, identifier, NamespaceOf(object.nameSpace), objects);
  if (bodies.empty()) { return {}; }
  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n\n";
  out += "#include \"" + fileName + ".h\"\n\n";
  out += kDoorMarker;
  out += BodyIncludes(bodies, objects);
  out += bodies;
  return WithDoor(out, ObjectKind::Enum);
}

std::string
WriteEnum(const al::EnumObject &object, const std::string &sourcePath, const Objects &objects) {
  const std::string identifier = ClassName(Identifier(object.name), ObjectKind::Enum);
  const std::string space = NamespaceOf(object.nameSpace);
  const std::string qualified = space + "::" + identifier;
  const std::vector<const al::EnumValueDecl *> sorted = ByOrdinal(object);

  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#pragma once\n\n";
  out += kDoorMarker;
  out += "\n";
  out += "#include <array>\n#include <cstdint>\n\n";

  out += "namespace " + space + " {\n\n";
  out += "enum class " + identifier + " : std::int32_t {\n";
  for (const al::EnumValueDecl &value : object.values) {
    out += "  " + EnumeratorName(value.name) + " = " + std::to_string(value.ordinal) + ",\n";
  }
  out += "};\n\n";
  out += "} // namespace " + space + "\n\n";

  out += "template <> struct agiru::EnumTraits<" + qualified + "> {\n";
  const al::Property *unknown = al::Find(object.properties, "UnknownValueImplementation");
  out += "  static constexpr std::string_view kUnknownValueImplementation{";
  out += unknown == nullptr ? "\"\"" : Literal(unknown->text);
  out += "};\n";
  const al::Property *compatible = al::Find(object.properties, "AssignmentCompatibility");
  out += "  static constexpr bool kAssignmentCompatibility = ";
  out += compatible != nullptr && LowerKey(compatible->text) == "true" ? "true" : "false";
  out += ";\n";
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
  out += ImplementationDeclarations(object, identifier, space, objects);
  return WithDoor(out, ObjectKind::Enum);
}

}

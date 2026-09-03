#include "CodeunitWriter.h"

#include "Ast.h"
#include "BodyWriter.h"
#include "Door.h"
#include "EnumWriter.h"
#include "Expr.h"
#include "Names.h"
#include "Scope.h"
#include "Token.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace agiru::gen {

namespace {

bool IsPublisher(const al::ProcedureDecl &procedure) {
  return al::HasAttribute(procedure, "IntegrationEvent") ||
         al::HasAttribute(procedure, "BusinessEvent") ||
         al::HasAttribute(procedure, "InternalEvent");
}

std::string TableNoOf(const al::CodeunitObject &unit) {
  const al::Property *source = al::Find(unit.properties, "TableNo");
  if (source == nullptr || source->value.empty()) { return {}; }
  std::string name;
  for (const al::Token &token : source->value) { name += token.text; }
  return name;
}

std::string SourceTableOf(const al::CodeunitObject &unit, const Objects &objects) {
  const std::string name = TableNoOf(unit);
  if (name.empty()) { return {}; }
  const auto found = objects.tables.find(LowerKey(name));
  return found == objects.tables.end() ? std::string{} : found->second.identifier;
}

std::string SubtypeOf(const al::CodeunitObject &unit) {
  const al::Property *subtype = al::Find(unit.properties, "Subtype");
  if (subtype == nullptr) { return "Normal"; }
  const std::string named = LowerKey(subtype->text);
  if (named == "test") { return "Test"; }
  if (named == "testrunner") { return "TestRunner"; }
  if (named == "upgrade") { return "Upgrade"; }
  if (named == "install") { return "Install"; }
  return "Normal";
}

bool IsTestCodeunit(const al::CodeunitObject &unit) {
  const al::Property *subtype = al::Find(unit.properties, "Subtype");
  return subtype != nullptr && LowerKey(subtype->text) == "test";
}

bool IsTest(const al::ProcedureDecl &procedure) {
  return al::HasAttribute(procedure, "Test");
}

std::vector<const al::ProcedureDecl *> TestsOf(const al::CodeunitObject &unit) {
  std::vector<const al::ProcedureDecl *> tests;
  if (!IsTestCodeunit(unit)) { return tests; }
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    if (IsTest(procedure)) { tests.push_back(&procedure); }
  }
  return tests;
}

bool DeclaresOnRun(const al::CodeunitObject &unit) {
  return std::ranges::any_of(unit.procedures, [](const al::ProcedureDecl &procedure) {
    return procedure.isTrigger && Identifier(procedure.name) == "OnRun";
  });
}

std::string TestCatalogueOf(const al::CodeunitObject &unit, const std::string &identifier) {
  const std::vector<const al::ProcedureDecl *> tests = TestsOf(unit);
  if (tests.empty()) { return {}; }
  std::string out = "\nnamespace {\n\nconstexpr std::array<TestMethod, ";
  out += std::to_string(tests.size());
  out += "> kTestMethods{{\n";
  for (const al::ProcedureDecl *test : tests) {
    out += "    {\"";
    out += test->name;
    out += "\", &InvokeTest<";
    out += identifier;
    out += ", &";
    out += identifier;
    out += "::";
    out += Identifier(test->name);
    out += ">},\n";
  }
  out += "}};\n\nconst TestCatalogue kTestCatalogue{CodeunitTraits<";
  out += identifier;
  out += ">::kId,\n                                  CodeunitTraits<";
  out += identifier;
  out += ">::kName,\n";
  out += DeclaresOnRun(unit) ? "                                  &InvokeTest<" + identifier +
                                   ", &" + identifier + "::OnRun>,\n"
                             : "                                  nullptr,\n";
  out += "                                  kTestMethods};\n\n} // namespace\n";
  return out;
}

bool NamesAnObject(const al::VarDecl &declared) {
  const std::string type = TypeName(declared.type);
  return (type == "Record" || type == "Codeunit" || type == "Page" || type == "Report" ||
          type == "Query" || type == "XmlPort" || type == "ControlAddIn") &&
         !declared.subtype.empty();
}

const TableRef *Reach(const al::VarDecl &declared, const Objects &objects) {
  const std::string type = TypeName(declared.type);
  const TableIndex &index = type == "Codeunit" ? objects.codeunits
                            : type == "Page"   ? objects.pages
                                               : objects.tables;
  const auto found = index.find(LowerKey(declared.subtype));
  return found != index.end() ? &found->second : nullptr;
}

std::string InterfaceType(const al::VarDecl &declared, const Objects &objects) {
  const auto found = objects.interfaces.find(LowerKey(declared.subtype));
  return (found != objects.interfaces.end() ? found->second.identifier
                                            : "absent::" + Identifier(declared.subtype)) +
         " *";
}

constexpr std::size_t kEveryArgument = static_cast<std::size_t>(-1);

std::size_t FieldArguments(std::string_view method) {
  static const std::vector<std::pair<std::string_view, std::size_t>> kTakers{
      {"SetRange", 1},
      {"SetFilter", 1},
      {"TestField", 1},
      {"FieldError", 1},
      {"FieldCaption", 1},
      {"FieldName", 1},
      {"FieldNo", 1},
      {"Validate", 1},
      {"SetAscending", 1},
      {"CalcFields", kEveryArgument},
      {"CalcSums", kEveryArgument},
      {"SetCurrentKey", kEveryArgument},
      {"SetLoadFields", kEveryArgument},
      {"AddLoadFields", kEveryArgument},
  };
  for (const auto &[name, count] : kTakers) {
    if (LowerKey(std::string(name)) == LowerKey(std::string(method))) { return count; }
  }
  return 0;
}

bool NamesAbsentType(const al::VarDecl &declared) {
  return !declared.type.empty() && !IsAlTypeName(declared.type) && declared.subtype.empty() &&
         declared.members.empty() && declared.arguments.empty();
}

std::string ObjectType(const al::VarDecl &declared, const Objects &objects) {
  const TableRef *ref = Reach(declared, objects);
  if (ref == nullptr) { return "absent::" + Identifier(declared.subtype); }
  return declared.temporary ? "Temporary<" + ref->identifier + ">" : ref->identifier;
}

std::string
TypeOf(const al::VarDecl &declared, const Objects &objects, const std::string &owner = {});

std::string Generic(const std::string &type,
                    const std::vector<al::VarDecl> &arguments,
                    const Objects &objects) {
  std::string out = type + "<";
  for (std::size_t i = 0; i < arguments.size(); ++i) {
    if (i != 0) { out += ", "; }
    out += TypeOf(arguments[i], objects);
  }
  return out + ">";
}

std::string Element(const al::VarDecl &declared, const Objects &objects, const std::string &owner);

std::string TypeOf(const al::VarDecl &declared, const Objects &objects, const std::string &owner) {
  std::string inner = Element(declared, objects, owner);
  for (std::size_t i = declared.dimensions.size(); i > 0; --i) {
    std::string wrapped = "AlArray<";
    wrapped += inner;
    wrapped += ", ";
    wrapped += std::to_string(declared.dimensions[i - 1]);
    wrapped += ">";
    inner = std::move(wrapped);
  }
  return inner;
}

std::string Unhidden(const std::string &type) {
  return HiddenByABaseMember(type) ? "agiru::" + type : type;
}

struct Named {
  std::string type;
  std::string owner;
};

std::string Parameterised(const al::VarDecl &declared, const Objects &objects, const Named &named) {
  const std::string &type = named.type;
  const std::string &owner = named.owner;
  if (type == "TestPage" || type == "TestRequestPage") {
    if (declared.subtype.empty()) { return type + "<>"; }
    const auto found = objects.pages.find(LowerKey(declared.subtype));
    return found != objects.pages.end() ? type + "<" + found->second.identifier + ">" : type + "<>";
  }
  if (type == "Option") {
    return declared.members.empty() || owner.empty() ? "Option<>" : "Option<" + owner + ">";
  }
  if ((type == "List" || type == "Dictionary") && !declared.arguments.empty()) {
    return Generic(type, declared.arguments, objects);
  }

  if (NamesAbsentType(declared)) { return "absent::" + Identifier(declared.type); }
  if (type == "Code" || type == "Text") {
    return type + "<" + std::to_string(declared.length) + ">";
  }
  return Unhidden(type);
}

std::string Element(const al::VarDecl &declared, const Objects &objects, const std::string &owner) {
  const std::string type = TypeName(declared.type);
  if (NamesAnObject(declared)) { return ObjectType(declared, objects); }
  if (type == "DotNet") {
    return declared.subtype.empty() ? "dotnet::Unnamed" : "dotnet::" + Identifier(declared.subtype);
  }
  if (type == "Enum") {
    if (declared.subtype.empty()) { return "Enum<>"; }
    const auto found = objects.enums.find(LowerKey(declared.subtype));
    return found != objects.enums.end() ? "Enum<enums::" + found->second.identifier + ">"
                                        : "Enum<>";
  }
  if (type == "Interface") { return InterfaceType(declared, objects); }
  return Parameterised(declared, objects, Named{.type = type, .owner = owner});
}

std::string
OptionName(const std::string &unit, const std::string &within, const std::string &name) {
  return Identifier(unit) + Identifier(within) + Identifier(name);
}

std::string OptionNameOf(const std::string &owner,
                         const std::string &within,
                         const al::VarDecl &declared,
                         const std::vector<al::ProcedureDecl> &procedures) {
  std::string base = OptionName(owner, within, declared.name);
  if (declared.members.empty()) { return base; }
  const auto clashes = [&](const al::ProcedureDecl &procedure, const al::VarDecl &other) {
    return !other.members.empty() && other.members != declared.members &&
           OptionName(owner, procedure.name, other.name) == base;
  };
  for (const al::ProcedureDecl &procedure : procedures) {
    const bool found =
        std::ranges::any_of(procedure.parameters,
                            [&](const al::VarDecl &o) { return clashes(procedure, o); }) ||
        std::ranges::any_of(procedure.variables,
                            [&](const al::VarDecl &o) { return clashes(procedure, o); });
    if (found) { return base + EnumeratorName(declared.members.front()); }
  }
  return base;
}

bool Hidden(const std::string &type, const std::set<std::string> &names) {
  for (std::size_t i = 0; i < type.size();) {
    if (std::isalpha(static_cast<unsigned char>(type[i])) == 0 && type[i] != '_') {
      ++i;
      continue;
    }
    std::size_t end = i;
    while (end < type.size() &&
           (std::isalnum(static_cast<unsigned char>(type[end])) != 0 || type[end] == '_')) {
      ++end;
    }
    const bool qualified = i >= 2 && type[i - 1] == ':' && type[i - 2] == ':';
    if (!qualified && names.contains(type.substr(i, end - i))) { return true; }
    i = end;
  }
  return false;
}

std::set<std::string> Shadowing(const std::vector<al::VarDecl> &variables,
                                const std::vector<al::ProcedureDecl> &procedures,
                                const std::vector<al::LabelDecl> &labels) {
  std::set<std::string> names;
  for (const al::VarDecl &declared : variables) { names.insert(Identifier(declared.name)); }
  for (const al::ProcedureDecl &procedure : procedures) {
    names.insert(Identifier(procedure.name));
  }
  for (const al::LabelDecl &label : labels) { names.insert(Identifier(label.name)); }
  return names;
}

std::string Qualified(const std::string &type, const std::set<std::string> &names) {
  std::string out;
  for (std::size_t i = 0; i < type.size();) {
    if (std::isalpha(static_cast<unsigned char>(type[i])) == 0 && type[i] != '_') {
      out += type[i];
      ++i;
      continue;
    }
    std::size_t end = i;
    while (end < type.size() &&
           (std::isalnum(static_cast<unsigned char>(type[end])) != 0 || type[end] == '_')) {
      ++end;
    }
    const std::string word = type.substr(i, end - i);
    const bool qualified = i >= 2 && type[i - 1] == ':' && type[i - 2] == ':';
    if (!qualified && names.contains(word)) { out += "agiru::"; }
    out += word;
    i = end;
  }
  return out;
}

std::string Signature(const al::VarDecl &declared,
                      const Objects &objects,
                      const std::set<std::string> &names,
                      const std::string &owner = {}) {
  std::string type = TypeOf(declared, objects, owner);
  if (Hidden(type, names)) { type = Qualified(type, names); }
  return type + (declared.byReference ? " &" : " ");
}

std::string Parameters(const al::ProcedureDecl &procedure,
                       const Objects &objects,
                       bool named,
                       const std::string &unit,
                       const std::set<std::string> &shadowed = {},
                       const std::vector<al::ProcedureDecl> &all = {}) {
  std::set<std::string> names = shadowed;
  for (const al::VarDecl &parameter : procedure.parameters) {
    names.insert(Identifier(parameter.name));
  }
  std::string out;
  for (std::size_t i = 0; i < procedure.parameters.size(); ++i) {
    if (i != 0) { out += ", "; }
    out += Signature(procedure.parameters[i],
                     objects,
                     names,
                     OptionNameOf(unit, procedure.name, procedure.parameters[i], all));
    if (named) { out += Identifier(procedure.parameters[i].name); }
  }
  return out;
}

std::string InlineOptionsIn(const std::string &owner,
                            const std::string &space,
                            const std::vector<al::VarDecl> &variables,
                            const std::vector<al::ProcedureDecl> &procedures) {
  std::string out;
  std::map<std::string, std::vector<std::string>> emitted;
  const auto declare = [&](const std::string &within, const al::VarDecl &declared) {
    if (TypeName(declared.type) != "Option" || declared.members.empty()) { return; }
    const std::string name = OptionNameOf(owner, within, declared, procedures);
    const auto seen = emitted.find(name);
    if (seen != emitted.end()) {
      if (seen->second != declared.members) {
        throw std::runtime_error("\"" + owner +
                                 "\" declares two different options under the name " + name);
      }
      return;
    }
    emitted.insert_or_assign(name, declared.members);
    const std::vector<std::string> names = EnumeratorNames(declared.members);
    out += "namespace agiru::app::" + space + " {\n\nenum class " + name + " : std::int32_t {\n";
    for (std::size_t i = 0; i < names.size(); ++i) {
      out += "  " + names[i] + " = " + std::to_string(i) + ",\n";
    }
    out += "};\n\n} // namespace agiru::app::" + space + "\n\n";
    out += "template <> struct agiru::OptionTraits<agiru::app::" + space + "::" + name + "> {\n";
    out += "  static constexpr std::array<EnumValueDef, " +
           std::to_string(declared.members.size()) + "> kValues{{\n";
    for (std::size_t i = 0; i < declared.members.size(); ++i) {
      out += "      EnumValueDef{.ordinal = " + std::to_string(i) +
             ", .name = " + Literal(declared.members[i]) +
             ", .caption = " + Literal(declared.members[i]) + "},\n";
    }
    out += "  }};\n};\n\n";
  };
  for (const al::VarDecl &declared : variables) { declare(std::string{}, declared); }
  for (const al::ProcedureDecl &procedure : procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { declare(procedure.name, declared); }
    for (const al::VarDecl &declared : procedure.variables) { declare(procedure.name, declared); }
  }
  return out;
}

std::string InlineOptions(const al::CodeunitObject &unit) {
  return InlineOptionsIn(unit.name, "codeunits", unit.variables, unit.procedures);
}

std::string Returns(const al::ProcedureDecl &procedure,
                    const Objects &objects,
                    const std::set<std::string> &shadowed = {}) {
  if (procedure.returnType.empty()) { return "void"; }
  const std::string type = TypeOf(procedure.returned, objects);
  return Hidden(type, shadowed) ? Qualified(type, shadowed) : type;
}

std::string Declaration(const al::ProcedureDecl &procedure,
                        const Objects &objects,
                        const std::string &unit,
                        const std::set<std::string> &shadowed = {},
                        const std::vector<al::ProcedureDecl> &all = {}) {
  std::set<std::string> hiding = shadowed;
  hiding.insert(Identifier(procedure.name));
  return "  " + Returns(procedure, objects, hiding) + " " + Identifier(procedure.name) + "(" +
         Parameters(procedure, objects, true, unit, hiding, all) + ");\n";
}

bool NamesAbsent(const al::CodeunitObject &unit, const Objects &objects) {
  const auto absent = [&objects](const al::VarDecl &declared) {
    const std::string type = TypeName(declared.type);
    if (type == "DotNet") { return true; }
    return NamesAnObject(declared) && Reach(declared, objects) == nullptr;
  };
  if (std::ranges::any_of(unit.variables, absent)) { return true; }
  return std::ranges::any_of(unit.procedures, [&absent](const al::ProcedureDecl &procedure) {
    return std::ranges::any_of(procedure.parameters, absent) ||
           std::ranges::any_of(procedure.variables, absent);
  });
}

bool HandleMember(const al::VarDecl &declared) {
  return NamesAnObject(declared);
}

bool NamesAPage(std::string_view type) {
  return type == "TestPage" || type == "Page" || type == "TestRequestPage";
}

template <typename Index>
void IndexedHeader(const Index &index, const std::string &subtype, std::set<std::string> &headers) {
  if (subtype.empty()) { return; }
  const auto found = index.find(LowerKey(subtype));
  if (found != index.end() && !found->second.header.empty()) {
    headers.insert(found->second.header);
  }
}

std::string SourceIncludes(const al::CodeunitObject &unit, const Objects &objects) {
  std::set<std::string> headers;
  const auto reach = [&](const al::VarDecl &declared) {
    if (!NamesAnObject(declared)) { return; }
    const TableRef *ref = Reach(declared, objects);
    if (ref != nullptr && !ref->header.empty()) { headers.insert(ref->header); }
  };
  const auto reachInterface = [&](const al::VarDecl &declared) {
    if (TypeName(declared.type) != "Interface") { return; }
    const auto found = objects.interfaces.find(LowerKey(declared.subtype));
    if (found != objects.interfaces.end() && !found->second.header.empty()) {
      headers.insert(found->second.header);
    }
  };
  const auto named = [&](const al::VarDecl &declared) {
    reach(declared);
    reachInterface(declared);
    if (NamesAPage(TypeName(declared.type))) {
      IndexedHeader(objects.pages, declared.subtype, headers);
    }
  };
  for (const al::VarDecl &declared : unit.variables) { named(declared); }
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { named(declared); }
    for (const al::VarDecl &declared : procedure.variables) { named(declared); }
    named(procedure.returned);
  }
  std::string out;
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }
  return out;
}

template <typename Ahead, typename Enum, typename Page, typename Element>
void Declared(const al::VarDecl &declared,
              const Objects &objects,
              Ahead ahead,
              Enum reachEnum,
              Page reachPage,
              Element reachElement) {
  if (NamesAPage(TypeName(declared.type))) { reachPage(declared.subtype); }
  if (NamesAnObject(declared)) {
    const TableRef *ref = Reach(declared, objects);
    if (ref != nullptr && !ref->header.empty()) { ahead(ref->identifier); }
  }
  if (TypeName(declared.type) == "Interface") {
    const auto found = objects.interfaces.find(LowerKey(declared.subtype));
    if (found != objects.interfaces.end()) { ahead(found->second.identifier); }
  }
  if (TypeName(declared.type) == "Enum") { reachEnum(declared.subtype); }
  for (const al::VarDecl &argument : declared.arguments) {
    Declared(argument, objects, ahead, reachEnum, reachPage, reachElement);
    const TableRef *element = ReachObject(argument, objects);
    if (element != nullptr && !element->header.empty()) { reachElement(element->header); }
  }
}

void EnumHeader(const EnumIndex &enums,
                const std::string &subtype,
                std::set<std::string> &headers) {
  IndexedHeader(enums, subtype, headers);
}

std::string Includes(const al::CodeunitObject &unit, const Objects &objects) {
  std::set<std::string> headers;
  const auto reach = [&](const al::VarDecl &declared) {
    if (!NamesAnObject(declared) || HandleMember(declared)) { return; }
    const TableRef *ref = Reach(declared, objects);
    if (ref != nullptr && !ref->header.empty()) { headers.insert(ref->header); }
  };
  const auto reachEnum = [&](const std::string &subtype) {
    EnumHeader(objects.enums, subtype, headers);
  };
  std::map<std::string, std::set<std::string>> forward;
  const auto ahead = [&forward](const std::string &qualified) {
    const std::size_t colons = qualified.find("::");
    if (colons == std::string::npos) { return; }
    forward[qualified.substr(0, colons)].insert(qualified.substr(colons + 2));
  };
  const auto reachPage = [&](const std::string &subtype) {
    IndexedHeader(objects.pages, subtype, headers);
  };
  const auto reachElement = [&](const std::string &header) { headers.insert(header); };
  const auto both = [&](const al::VarDecl &declared) {
    Declared(declared, objects, ahead, reachEnum, reachPage, reachElement);
  };
  {
    const al::Property *source = al::Find(unit.properties, "TableNo");
    if (source != nullptr && !source->value.empty()) {
      std::string name;
      for (const al::Token &token : source->value) { name += token.text; }
      const auto found = objects.tables.find(LowerKey(name));
      if (found != objects.tables.end() && !found->second.header.empty()) {
        headers.insert(found->second.header);
      }
    }
  }
  for (const al::VarDecl &declared : unit.variables) { reach(declared); }
  for (const al::VarDecl &declared : unit.variables) { both(declared); }
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { both(declared); }
    for (const al::VarDecl &declared : procedure.variables) { both(declared); }
    both(procedure.returned);
  }
  std::string out = std::string(kDoorMarker);
  if (NamesAbsent(unit, objects)) { out += "#include \"absent/Types.h\"\n"; }
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }

  if (!forward.empty()) { out += "\n"; }
  for (const auto &[space, named] : forward) {
    out += "namespace agiru::app::" + space + " {\n";
    const ObjectKind kind = KindOfNamespace(space);
    for (const std::string &one : named) {
      out += "class " + ClassName(one, kind) + ";\n" + ClassAlias(one, kind);
    }
    out += "} // namespace agiru::app::" + space + "\n";
  }
  out += "\n";
  return out;
}

using DotNetNames = std::map<std::string, std::string>;

void NoteDotNet(const al::VarDecl &declared, DotNetNames &named, DotNetUse &use) {
  if (TypeName(declared.type) == "DotNet" && !declared.subtype.empty()) {
    const std::string bare = Identifier(declared.subtype);
    named.insert_or_assign(LowerKey(declared.name), bare);
    use.try_emplace(bare);
  }
}

void GatherFieldArguments(const std::vector<al::Token> &tokens,
                          std::size_t method,
                          std::set<std::string> &into) {
  const std::size_t count = FieldArguments(tokens[method].text);
  if (count == 0) { return; }
  std::size_t at = method + 1;
  if (at >= tokens.size() || tokens[at].text != "(") { return; }
  ++at;
  int depth = 0;
  bool first = true;
  for (; at < tokens.size(); ++at) {
    const al::Token &token = tokens[at];
    if (token.text == "(") { ++depth; }
    if (token.text == ")") {
      if (depth == 0) { return; }
      --depth;
    }
    if (depth != 0) { continue; }
    if (token.text == ",") {
      if (count != kEveryArgument) { return; }
      first = true;
      continue;
    }
    if (first && (token.kind == al::TokenKind::Identifier ||
                  token.kind == al::TokenKind::QuotedIdentifier)) {
      into.insert(Identifier(token.text));
    }
    first = false;
  }
}

void GatherCalls(const al::ProcedureDecl &procedure, const DotNetNames &named, DotNetUse &use) {
  for (std::size_t i = 0; i + 2 < procedure.tokens.size(); ++i) {
    if (procedure.tokens[i].kind != al::TokenKind::Identifier &&
        procedure.tokens[i].kind != al::TokenKind::QuotedIdentifier) {
      continue;
    }
    if (procedure.tokens[i + 1].text != ".") { continue; }
    if (procedure.tokens[i + 2].kind != al::TokenKind::Identifier &&
        procedure.tokens[i + 2].kind != al::TokenKind::QuotedIdentifier) {
      continue;
    }
    const auto found = named.find(LowerKey(procedure.tokens[i].text));
    if (found == named.end()) { continue; }
    use[found->second].insert(Identifier(procedure.tokens[i + 2].text));
    GatherFieldArguments(procedure.tokens, i + 2, use[found->second]);
  }
}

void NoteAbsent(const al::VarDecl &declared,
                const Objects &objects,
                DotNetNames &named,
                DotNetUse &use) {
  if (NamesAbsentType(declared)) {
    const std::string bare = Identifier(declared.type);
    named.insert_or_assign(LowerKey(declared.name), bare);
    use.try_emplace(bare);
    return;
  }
  if (!NamesAnObject(declared) || Reach(declared, objects) != nullptr) { return; }
  const std::string subtype = Identifier(declared.subtype);
  named.insert_or_assign(LowerKey(declared.name), subtype);
  use.try_emplace(subtype);
}

void GatherDotNet(const al::CodeunitObject &unit,
                  const Objects &objects,
                  DotNetUse &use,
                  DotNetUse &absent) {
  DotNetNames named;
  DotNetNames missing;
  for (const al::VarDecl &declared : unit.variables) {
    NoteDotNet(declared, named, use);
    NoteAbsent(declared, objects, missing, absent);
  }

  for (const al::ProcedureDecl &procedure : unit.procedures) {
    DotNetNames inner = named;
    DotNetNames innerMissing = missing;
    for (const al::VarDecl &declared : procedure.parameters) {
      NoteDotNet(declared, inner, use);
      NoteAbsent(declared, objects, innerMissing, absent);
    }
    for (const al::VarDecl &declared : procedure.variables) {
      NoteDotNet(declared, inner, use);
      NoteAbsent(declared, objects, innerMissing, absent);
    }
    GatherCalls(procedure, inner, use);
    GatherCalls(procedure, innerMissing, absent);
  }
}

std::vector<std::string> Unresolved(const al::CodeunitObject &unit, const Objects &objects) {
  std::vector<std::string> missing;
  const auto note = [&](const al::VarDecl &declared) {
    if (!NamesAnObject(declared) || Reach(declared, objects) != nullptr) { return; }
    if (std::ranges::find(missing, declared.subtype) == missing.end()) {
      missing.push_back(declared.subtype);
    }
  };
  for (const al::VarDecl &declared : unit.variables) { note(declared); }
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) { note(declared); }
  }
  return missing;
}

class CodeunitNames : public Names {
public:
  CodeunitNames(const al::CodeunitObject &unit,
                const al::ProcedureDecl &procedure,
                const Objects &objects)
      : unit_(unit), procedure_(procedure), objects_(objects) {}

  [[nodiscard]] std::string ExitValue() const override {
    if (!procedure_.returnName.empty()) { return " " + Identifier(procedure_.returnName); }
    return procedure_.returnType.empty() ? std::string{} : std::string(" {}");
  }

  [[nodiscard]] bool IsRecord(std::string_view variable) const override {
    if (LowerKey(std::string(variable)) == "rec") { return !TableNoOf(unit_).empty(); }
    const al::VarDecl *declared = Declaration(variable);
    return declared != nullptr && TypeName(declared->type) == "Record";
  }

  [[nodiscard]] bool MembersAreCalls(std::string_view variable) const override {
    const al::VarDecl *declared = Declaration(variable);
    if (declared == nullptr) { return false; }
    const std::string type = TypeName(declared->type);
    return type == "RecordRef" || type == "FieldRef" || type == "KeyRef" || type == "Variant" ||
           type == "RecordId" || type == "ModuleInfo" || type == "Version";
  }

  [[nodiscard]] std::string FieldEnumeration(const OfVariable &field) const override {
    const std::string subtype = LowerKey(std::string(field.variable)) == "rec"
                                    ? TableNoOf(unit_)
                                    : SubtypeOfRecord(field.variable);
    if (subtype.empty()) { return {}; }
    const auto table = objects_.fieldEnums.find(LowerKey(subtype));
    if (table == objects_.fieldEnums.end()) { return {}; }
    const auto found = table->second.find(LowerKey(std::string(field.field)));
    return found == table->second.end() ? std::string{} : found->second;
  }

  [[nodiscard]] std::string SubtypeOfRecord(std::string_view variable) const {
    const al::VarDecl *declared = Declaration(variable);
    if (declared == nullptr || TypeName(declared->type) != "Record") { return {}; }
    return declared->subtype;
  }

  [[nodiscard]] const al::VarDecl *Declaration(std::string_view name) const {
    const auto same = [&name](const al::VarDecl &declared) {
      return LowerKey(declared.name) == LowerKey(std::string(name));
    };
    for (const al::VarDecl &declared : procedure_.variables) {
      if (same(declared)) { return &declared; }
    }
    for (const al::VarDecl &declared : procedure_.parameters) {
      if (same(declared)) { return &declared; }
    }
    for (const al::VarDecl &declared : unit_.variables) {
      if (same(declared)) { return &declared; }
    }
    return nullptr;
  }

  [[nodiscard]] bool IsHandle(std::string_view name) const override {
    const auto same = [&name](const al::VarDecl &declared) {
      return LowerKey(declared.name) == LowerKey(std::string(name));
    };
    if (std::ranges::any_of(procedure_.variables, same)) { return false; }
    if (std::ranges::any_of(procedure_.parameters, same)) { return false; }
    for (const al::VarDecl &declared : unit_.variables) {
      if (same(declared)) { return NamesAnObject(declared); }
    }
    return false;
  }

  [[nodiscard]] std::string Resolve(std::string_view name) const override {
    for (const al::VarDecl &declared : procedure_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) {
        return Identifier(declared.name);
      }
    }
    for (const al::VarDecl &declared : procedure_.parameters) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) {
        return Identifier(declared.name);
      }
    }
    if (!procedure_.returnName.empty() &&
        LowerKey(procedure_.returnName) == LowerKey(std::string(name))) {
      return Identifier(procedure_.returnName);
    }
    for (const al::VarDecl &declared : unit_.variables) {
      if (LowerKey(declared.name) == LowerKey(std::string(name))) {
        return Identifier(declared.name);
      }
    }
    for (const al::LabelDecl &label : unit_.labels) {
      if (LowerKey(label.name) == LowerKey(std::string(name))) { return Identifier(label.name); }
    }
    for (const al::ProcedureDecl &other : unit_.procedures) {
      if (LowerKey(other.name) == LowerKey(std::string(name))) { return Identifier(other.name); }
    }
    if (LowerKey(std::string(name)) == "rec" && al::Find(unit_.properties, "TableNo") != nullptr) {
      return "Rec";
    }
    return {};
  }

  [[nodiscard]] std::string Enumeration(std::string_view name) const override {
    const al::VarDecl *declared = Declaration(name);
    if (declared == nullptr) { return {}; }
    const std::string type = TypeName(declared->type);
    if (type == "Enum" && !declared->subtype.empty()) {
      const auto found = objects_.enums.find(LowerKey(declared->subtype));
      if (found != objects_.enums.end()) { return "enums::" + found->second.identifier; }
      return {};
    }
    if (type != "Option" || declared->members.empty()) { return {}; }
    return OptionNameOf(unit_.name, procedure_.name, *declared, unit_.procedures);
  }

private:
  const al::CodeunitObject &unit_;
  const al::ProcedureDecl &procedure_;
  const Objects &objects_;
};

std::string FallsOff(const al::ProcedureDecl &procedure, const Names &names) {
  if (procedure.returnType.empty() && procedure.returned.type.empty()) { return {}; }
  if (!procedure.body.empty() && procedure.body.back().kind == al::StmtKind::Exit) { return {}; }
  return "  return" + names.ExitValue() + ";\n";
}

std::string WithoutLiterals(const std::string &body) {
  std::string out = body;
  bool inside = false;
  for (std::size_t at = 0; at < out.size(); ++at) {
    if (out[at] == '\\' && inside) {
      ++at;
      continue;
    }
    if (out[at] == '"') {
      inside = !inside;
      continue;
    }
    if (inside) { out[at] = ' '; }
  }
  return out;
}

bool Mentions(const std::string &body, const std::string &name) {
  for (std::size_t at = body.find(name); at != std::string::npos; at = body.find(name, at + 1)) {
    const bool before = at > 0 && (std::isalnum(static_cast<unsigned char>(body[at - 1])) != 0 ||
                                   body[at - 1] == '_');
    const std::size_t after = at + name.size();
    const bool behind =
        after < body.size() &&
        (std::isalnum(static_cast<unsigned char>(body[after])) != 0 || body[after] == '_');
    if (!before && !behind) { return true; }
  }
  return false;
}

std::string Locals(const al::ProcedureDecl &procedure,
                   const Objects &objects,
                   const std::string &unit,
                   const std::vector<al::ProcedureDecl> &all = {},
                   const std::string &body = {}) {
  std::string out;
  const std::string code = WithoutLiterals(body);
  const auto unused = [&body, &code](const std::string &name) {
    return body.empty() || Mentions(code, name) ? std::string{} : std::string("[[maybe_unused]] ");
  };
  if (!procedure.returnName.empty()) {
    out += "  " + Returns(procedure, objects) + " " + Identifier(procedure.returnName) + "{};\n";
  }
  std::set<std::string> names;
  for (const al::VarDecl &declared : procedure.variables) {
    names.insert(Identifier(declared.name));
  }
  if (!procedure.returnName.empty()) { names.insert(Identifier(procedure.returnName)); }
  for (const al::VarDecl &declared : procedure.variables) {
    std::string type = TypeOf(declared, objects, OptionNameOf(unit, procedure.name, declared, all));
    if (Hidden(type, names)) { type = Qualified(type, names); }
    out +=
        "  " + unused(Identifier(declared.name)) + type + " " + Identifier(declared.name) + "{};\n";
  }
  return out;
}

} // namespace

std::string WriteCodeunitSource(const al::CodeunitObject &unit,
                                const std::string &sourcePath,
                                const Objects &objects) {
  const std::string identifier = Identifier(unit.name);
  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#include \"" + identifier + ".h\"\n\n";
  out += kDoorMarker;
  out += SourceIncludes(unit, objects);
  const std::string catalogue = TestCatalogueOf(unit, identifier);
  if (!catalogue.empty()) { out += "\n#include <array>\n"; }
  out += "\nnamespace agiru::app::codeunits {\n\n";

  for (const al::ProcedureDecl &procedure : unit.procedures) {
    const bool publisher = IsPublisher(procedure);
    out += Returns(procedure, objects) + " " + identifier + "::" + Identifier(procedure.name) +
           "(" + Parameters(procedure, objects, !publisher, unit.name, {}, unit.procedures) + ") {";
    const std::string body =
        publisher ? std::string{}
                  : WriteStatements(CodeunitNames(unit, procedure, objects), procedure.body, 2) +
                        FallsOff(procedure, CodeunitNames(unit, procedure, objects));
    const std::string locals =
        publisher ? std::string{} : Locals(procedure, objects, unit.name, unit.procedures, body);
    if (locals.empty() && body.empty()) {
      out += "}\n\n";
      continue;
    }
    out += "\n";
    out += locals;
    if (!locals.empty() && !body.empty()) { out += "\n"; }
    out += body;
    out += "}\n\n";
  }

  out += catalogue;
  out += "} // namespace agiru::app::codeunits\n";
  return WithDoor(out, ObjectKind::Codeunit);
}

std::string ProcedureSignature(const al::ProcedureDecl &procedure,
                               const Objects &objects,
                               const std::string &owner,
                               const std::string &qualifier,
                               bool named,
                               const std::set<std::string> &shadowed,
                               const std::vector<al::ProcedureDecl> &all,
                               const std::string &spelled) {
  std::set<std::string> hiding = shadowed;
  hiding.insert(Identifier(procedure.name));
  const std::string name = spelled.empty() ? Identifier(procedure.name) : spelled;
  return Returns(procedure, objects, hiding) + " " + qualifier + "::" + name + "(" +
         Parameters(procedure, objects, named, owner, hiding, all) + ")";
}

std::string FallsOffEnd(const al::ProcedureDecl &procedure, const Names &names) {
  return FallsOff(procedure, names);
}

std::string MemberDeclarations(const std::string &owner,
                               const std::vector<al::VarDecl> &variables,
                               const std::vector<al::LabelDecl> &labels,
                               const std::vector<al::ProcedureDecl> &procedures,
                               const Objects &objects) {
  const std::set<std::string> shadowed = Shadowing(variables, procedures, labels);
  std::string out;
  for (const al::VarDecl &declared : variables) {
    std::string type = TypeOf(declared, objects, OptionName(owner, {}, declared.name));
    if (Hidden(type, shadowed)) { type = Qualified(type, shadowed); }
    const bool handle = HandleMember(declared);
    out +=
        "  " + (handle ? "Instance<" + type + ">" : type) + " " + Identifier(declared.name) + ";\n";
  }
  if (!labels.empty() && !out.empty()) { out += "\n"; }
  for (const al::LabelDecl &label : labels) {
    out += "  static constexpr std::string_view " + Identifier(label.name) + "{" +
           Literal(label.text) + "};\n";
  }
  return out;
}

std::string ProcedureLocals(const al::ProcedureDecl &procedure,
                            const Objects &objects,
                            const std::string &owner,
                            const std::vector<al::ProcedureDecl> &all) {
  return Locals(procedure, objects, owner, all);
}

std::string SourceIncludesOf(const std::vector<al::VarDecl> &variables,
                             const std::vector<al::ProcedureDecl> &procedures,
                             const Objects &objects) {
  al::CodeunitObject unit;
  unit.variables = variables;
  unit.procedures = procedures;
  return SourceIncludes(unit, objects);
}

std::string ProcedureDeclaration(const al::ProcedureDecl &procedure,
                                 const Objects &objects,
                                 const std::string &owner,
                                 const std::set<std::string> &shadowed,
                                 const std::vector<al::ProcedureDecl> &all,
                                 const std::string &spelled) {
  std::string line = Declaration(procedure, objects, owner, shadowed, all);
  if (spelled.empty() || spelled == Identifier(procedure.name)) { return line; }
  const std::string wanted = " " + Identifier(procedure.name) + "(";
  const std::size_t at = line.find(wanted);
  if (at == std::string::npos) { return line; }
  return line.substr(0, at) + " " + spelled + "(" + line.substr(at + wanted.size());
}

std::string QualifiedType(const std::string &type, const std::set<std::string> &names) {
  return Hidden(type, names) ? Qualified(type, names) : type;
}

std::string DeclaredType(const al::VarDecl &declared, const Objects &objects) {
  return TypeOf(declared, objects);
}

void GatherAbsentIn(const std::vector<al::VarDecl> &variables,
                    const std::vector<al::ProcedureDecl> &procedures,
                    const Objects &objects,
                    DotNetUse &dotnet,
                    DotNetUse &absent) {
  al::CodeunitObject unit;
  unit.variables = variables;
  unit.procedures = procedures;
  GatherDotNet(unit, objects, dotnet, absent);
}

bool NamesAbsentIn(const std::vector<al::VarDecl> &variables,
                   const std::vector<al::ProcedureDecl> &procedures,
                   const Objects &objects) {
  al::CodeunitObject unit;
  unit.variables = variables;
  unit.procedures = procedures;
  return NamesAbsent(unit, objects);
}

bool DeclaresAnObject(const al::VarDecl &declared) {
  return NamesAnObject(declared);
}

const TableRef *ReachObject(const al::VarDecl &declared, const Objects &objects) {
  return NamesAnObject(declared) ? Reach(declared, objects) : nullptr;
}

std::string InlineOptionsOf(const std::string &owner,
                            const std::string &space,
                            const std::vector<al::VarDecl> &variables,
                            const std::vector<al::ProcedureDecl> &procedures) {
  return InlineOptionsIn(owner, space, variables, procedures);
}

std::string CodeunitHeaderPath(const al::CodeunitObject &unit) {
  return OutputDirectory(unit.nameSpace, ObjectKind::Codeunit) + "/" + Identifier(unit.name) + ".h";
}

TableIndex PlatformTables() {
  TableIndex tables;
  const auto add = [&tables](std::string_view name, std::string_view number) {
    const TableRef ref{.identifier = "platform::" + Identifier(name), .header = {}};
    tables.insert_or_assign(LowerKey(std::string(name)), ref);
    tables.insert_or_assign(std::string(number), ref);
  };
  add("Field", "2000000041");
  add("Integer", "2000000026");
  add("Date", "2000000007");
  add("User", "2000000120");
  return tables;
}

namespace {

void FaceReach(const al::VarDecl &declared,
               const Objects &objects,
               std::set<std::string> &headers,
               std::map<std::string, std::set<std::string>> &forward) {
  for (const al::VarDecl &argument : declared.arguments) {
    FaceReach(argument, objects, headers, forward);
  }
  if (TypeName(declared.type) == "Interface") {
    const auto found = objects.interfaces.find(LowerKey(declared.subtype));
    if (found != objects.interfaces.end()) {
      forward["interfaces"].insert(found->second.identifier.substr(std::size("interfaces::") - 1));
    }
    return;
  }
  if (NamesAnObject(declared)) {
    const TableRef *ref = Reach(declared, objects);
    if (ref != nullptr && !ref->header.empty()) { headers.insert(ref->header); }
  }
  if (TypeName(declared.type) == "Enum" && !declared.subtype.empty()) {
    const auto found = objects.enums.find(LowerKey(declared.subtype));
    if (found != objects.enums.end() && !found->second.header.empty()) {
      headers.insert(found->second.header);
    }
  }
}

std::string FaceDeclarations(const al::InterfaceObject &object, const Objects &objects) {
  std::string out;
  std::set<std::string> headers;
  std::map<std::string, std::set<std::string>> forward;
  for (const al::ProcedureDecl &procedure : object.procedures) {
    for (const al::VarDecl &declared : procedure.parameters) {
      FaceReach(declared, objects, headers, forward);
    }
    FaceReach(procedure.returned, objects, headers, forward);
  }
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }
  if (NamesAbsentIn({}, object.procedures, objects)) { out += "#include \"absent/Types.h\"\n"; }
  for (const auto &[space, named] : forward) {
    out += "\nnamespace agiru::app::" + space + " {\n";
    for (const std::string &one : named) {
      out += "class " + ClassName(one, ObjectKind::Interface) + ";\n" +
             ClassAlias(one, ObjectKind::Interface);
    }
    out += "} // namespace agiru::app::" + space + "\n";
  }
  return out;
}

} // namespace

InterfaceHeader WriteInterface(const al::InterfaceObject &object,
                               const std::string &sourcePath,
                               const Objects &objects) {
  const std::string identifier = Identifier(object.name);
  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n#pragma once\n\n";
  out += kDoorMarker;
  out += FaceDeclarations(object, objects);
  out += "\nnamespace agiru::app::interfaces {\n\n";
  const std::string faceClass = ClassName(identifier, ObjectKind::Interface);
  out += "class " + faceClass + ";\n" + ClassAlias(identifier, ObjectKind::Interface) + "\n";
  out += "class " + faceClass + " {\n";
  out += "public:\n";
  out += "  virtual ~" + faceClass + "() = default;\n\n";
  for (const al::ProcedureDecl &procedure : object.procedures) {
    out += "  virtual " + Returns(procedure, objects) + " " + Identifier(procedure.name) + "(" +
           Parameters(procedure, objects, true, object.name) + ") = 0;\n";
  }
  out += "};\n\n} // namespace agiru::app::interfaces\n";
  DotNetUse missing;
  DotNetUse dotnet;
  GatherAbsentIn({}, object.procedures, objects, dotnet, missing);
  return InterfaceHeader{.text = WithDoor(out, ObjectKind::Interface),
                         .absent = std::move(missing),
                         .dotnet = std::move(dotnet)};
}

CodeunitHeader WriteCodeunit(const al::CodeunitObject &unit,
                             const std::string &sourcePath,
                             const Objects &objects) {
  const std::string identifier = Identifier(unit.name);
  const std::set<std::string> shadowed = Shadowing(unit.variables, unit.procedures, unit.labels);

  std::string out;
  out += "// Generated from " + sourcePath + ". Do not edit.\n";
  out += "\n";
  out += "#pragma once\n\n";
  out += Includes(unit, objects);
  out += "#include <array>\n#include <cstdint>\n#include <string_view>\n\n";
  out += InlineOptions(unit);

  out += "namespace agiru::app::codeunits {\n\n";
  const std::string unitClass = ClassName(identifier, ObjectKind::Codeunit);
  out += "class " + unitClass + ";\n" + ClassAlias(identifier, ObjectKind::Codeunit) + "\n";
  out += "class " + unitClass + " : public Codeunit<" + unitClass + "> {\npublic:\n";

  const std::string source = SourceTableOf(unit, objects);
  if (!source.empty()) { out += "  " + source + " Rec;\n\n"; }

  bool previousWasTrigger = false;
  bool first = true;
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    if (procedure.isLocal) { continue; }
    if (!first && previousWasTrigger != procedure.isTrigger) { out += "\n"; }
    out += Declaration(procedure, objects, unit.name, shadowed, unit.procedures);
    previousWasTrigger = procedure.isTrigger;
    first = false;
  }

  std::string hidden;
  for (const al::VarDecl &declared : unit.variables) {
    std::string type = TypeOf(declared, objects, OptionName(unit.name, {}, declared.name));
    if (Hidden(type, shadowed)) { type = Qualified(type, shadowed); }
    const bool handle = HandleMember(declared);
    hidden +=
        "  " + (handle ? "Instance<" + type + ">" : type) + " " + Identifier(declared.name) + ";\n";
  }
  if (!unit.labels.empty() && !hidden.empty()) { hidden += "\n"; }
  for (const al::LabelDecl &label : unit.labels) {
    hidden += "  static constexpr std::string_view " + Identifier(label.name) + "{" +
              Literal(label.text) + "};\n";
  }
  std::string locals;
  for (const al::ProcedureDecl &procedure : unit.procedures) {
    if (!procedure.isLocal) { continue; }
    locals += Declaration(procedure, objects, unit.name, shadowed, unit.procedures);
  }
  if (!hidden.empty() || !locals.empty()) {
    out += "\nprivate:\n";
    out += hidden;
    if (!hidden.empty() && !locals.empty()) { out += "\n"; }
    out += locals;
  }
  out += "};\n\n";
  out += "} // namespace agiru::app::codeunits\n\n";

  out += "template <> struct agiru::CodeunitTraits<agiru::app::codeunits::" + identifier + "> {\n";
  out += "  static constexpr CodeunitId kId{" + std::to_string(unit.id) + "};\n";
  out += "  static constexpr std::string_view kName{" + Literal(unit.name) + "};\n";
  out += "  static constexpr Subtype kSubtype{Subtype::" + SubtypeOf(unit) + "};\n";
  out += "};\n";
  DotNetUse dotnet;
  DotNetUse absent;
  GatherDotNet(unit, objects, dotnet, absent);
  return CodeunitHeader{.text = WithDoor(out, ObjectKind::Codeunit),
                        .unresolvedTables = Unresolved(unit, objects),
                        .dotnet = std::move(dotnet),
                        .absent = std::move(absent)};
}

} // namespace agiru::gen

#include "Ast.h"
#include "EnumWriter.h"
#include "Names.h"
#include "ObjectKind.h"
#include "Parser.h"
#include "Scope.h"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {

void Emit(const char *path) {
  std::ifstream stream(path);
  if (!stream) { throw std::runtime_error("missing original enum source"); }
  const std::string source{std::istreambuf_iterator<char>(stream),
                           std::istreambuf_iterator<char>()};
  auto object = agiru::al::ParseEnum(source);
  const auto type =
      "::agiru::" + agiru::gen::NamespaceSuffix(object.nameSpace) +
      agiru::gen::ClassName(agiru::gen::Identifier(object.name), agiru::gen::ObjectKind::Enum);
  std::cout << "#include \"" << agiru::gen::EnumHeaderPath(object) << "\"\n"
            << "using Traits = agiru::EnumTraits<" << type << ">;\n"
            << "static_assert(Traits::kObjectID == " << object.id << ");\n"
            << "static_assert(Traits::kName == " << agiru::gen::Literal(object.name) << ");\n"
            << "static_assert(Traits::kValues.size() == " << object.values.size() << ");\n";
  std::cout << "static_assert(Traits::kDisplayOrdinals.size() == " << object.values.size()
            << ");\n";
  for (std::size_t at = 0; at < object.values.size(); ++at) {
    std::cout << "static_assert(Traits::kDisplayOrdinals[" << at
              << "] == " << object.values[at].ordinal << ");\n";
  }
  const auto *caption = agiru::al::Find(object.properties, "Caption");
  const auto *scope = agiru::al::Find(object.properties, "Scope");
  const auto *extensible = agiru::al::Find(object.properties, "Extensible");
  const auto *implementation = agiru::al::Find(object.properties, "DefaultImplementation");
  std::cout << "static_assert(Traits::kCaption == "
            << agiru::gen::Literal(caption == nullptr ? object.name : caption->text) << ");\n"
            << "static_assert(Traits::kScope == "
            << agiru::gen::Literal(scope == nullptr ? "" : scope->text) << ");\n"
            << "static_assert(Traits::kDefaultImplementation == "
            << agiru::gen::Literal(implementation == nullptr ? "" : implementation->text) << ");\n"
            << "static_assert(Traits::kExtensible == "
            << (extensible != nullptr && agiru::gen::LowerKey(extensible->text) == "true" ? "true"
                                                                                          : "false")
            << ");\n"
            << "static_assert(Traits::kInterfaces.size() == " << object.implements.size() << ");\n";
  for (std::size_t at = 0; at < object.implements.size(); ++at) {
    std::cout << "static_assert(Traits::kInterfaces[" << at
              << "] == " << agiru::gen::Literal(object.implements[at]) << ");\n";
  }
  std::ranges::sort(object.values, {}, &agiru::al::EnumValueDecl::ordinal);
  for (std::size_t at = 0; at < object.values.size(); ++at) {
    const auto &value = object.values[at];
    const auto *valueCaption = agiru::al::Find(value.properties, "Caption");
    const auto *valueImplementation = agiru::al::Find(value.properties, "Implementation");
    std::cout << "static_assert(static_cast<std::int32_t>(" << type
              << "::" << agiru::gen::EnumeratorName(value.name) << ") == " << value.ordinal
              << ");\n"
              << "static_assert(Traits::kValues[" << at << "].ordinal == " << value.ordinal
              << ");\n"
              << "static_assert(Traits::kValues[" << at
              << "].name == " << agiru::gen::Literal(value.name) << ");\n"
              << "static_assert(Traits::kValueImplementations[" << at << "] == "
              << agiru::gen::Literal(valueImplementation == nullptr ? ""
                                                                    : valueImplementation->text)
              << ");\n"
              << "static_assert(Traits::kValues[" << at << "].caption == "
              << agiru::gen::Literal(valueCaption == nullptr ? value.name : valueCaption->text)
              << ");\n";
  }
}

}

int main(int argc, char **argv) {
  try {
    if (argc != 2) { throw std::runtime_error("usage: native-enum-contract <original-source>"); }
    Emit(argv[1]);
    return 0;
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 2;
  }
}

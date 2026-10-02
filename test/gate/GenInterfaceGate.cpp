#include "../interface-defaults/Fixture.h"
#include "Ast.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "EnumWriter.h"
#include "Names.h"
#include "Parser.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {

using agiru::al::ParseInterface;
using agiru::gen::WriteInterface;

constexpr std::size_t kContractMethods = 9;
constexpr std::size_t kNamedMethod = 5;
constexpr std::size_t kLabelMethod = 6;
constexpr std::size_t kZeroMethod = 7;

agiru::gen::Objects Objects() {
  agiru::gen::Objects objects;
  for (const auto source : {interface_fixture::kContract, interface_fixture::kChild}) {
    const auto object = ParseInterface(source);
    agiru::gen::TableRef ref;
    ref.identifier = "::agiru::" + agiru::gen::Identifier(object.name) + "_Interface";
    ref.header = agiru::gen::Identifier(object.name) + ".h";
    ref.procedureDeclarations = object.procedures;
    ref.interfaceBases = object.extends;
    for (const auto &procedure : object.procedures) {
      ref.procedures.emplace(agiru::gen::LowerKey(procedure.name),
                             agiru::gen::Identifier(procedure.name));
    }
    objects.interfaces.emplace(agiru::gen::LowerKey(object.name), std::move(ref));
  }
  return objects;
}

void ParserKeepsBodyIdentity() {
  const auto object = ParseInterface(interface_fixture::kContract);
  CHECK_TRUE("every signature survives", object.procedures.size() == kContractMethods);
  CHECK_TRUE("a declaration remains required", !object.procedures.front().hasBody);
  CHECK_TRUE("the delegating overload has a body", object.procedures[1].hasBody);
  CHECK_TRUE("the overload retains both statements", (object.procedures[1].body.size()) == (2));
  CHECK_TRUE("an explicitly empty body is optional", object.procedures[2].hasBody);
  CHECK_TRUE("empty is distinct from missing", object.procedures[2].body.empty());
  CHECK_TRUE("default locals survive", object.procedures[kNamedMethod].variables.size() == 1);
  CHECK_TRUE("default labels survive", object.procedures[kLabelMethod].labels.size() == 1);
  CHECK_TEXT("named returns survive", object.procedures[kNamedMethod].returnName, "Result");
  CHECK_TRUE("empty value-returning bodies survive", object.procedures[kZeroMethod].hasBody);
  CHECK_TRUE("the empty named return has no statements",
             object.procedures[kZeroMethod].body.empty());
  const auto annotated = ParseInterface(R"(interface Marked {
    [RequiredPending('Implement later', '29.0')]
    procedure Optional() begin end;
    procedure Required();
  })");
  CHECK_TRUE("an attribute belongs to its method", annotated.procedures.front().hasBody);
  CHECK_TRUE("the attribute is retained", (annotated.procedures.front().attributes.size()) == (1));
  CHECK_TRUE("attributes do not leak", annotated.procedures.back().attributes.empty());
}

void DefinitionsStayOutOfHeaders() {
  const auto written = WriteInterface(
      ParseInterface(interface_fixture::kContract), "DefaultContract.Interface.al", Objects());
  CHECK_TRUE("required methods remain pure virtual",
             written.text.find("virtual void Add(Integer &Value) = 0;") != std::string::npos);
  CHECK_TRUE("default methods are virtual declarations",
             written.text.find("virtual Integer DefaultFee();") != std::string::npos);
  CHECK_TRUE("bodies do not widen the header", written.text.find("return 13") == std::string::npos);
  CHECK_TRUE("default methods have out-of-line bodies",
             written.source.find("DefaultContract_Interface::DefaultFee()") != std::string::npos);
  CHECK_TRUE("the AL body is translated", written.source.find("return 13;") != std::string::npos);
  CHECK_TRUE("this remains a virtual receiver",
             written.source.find("(*this).DefaultFee()") != std::string::npos);
  CHECK_TRUE("the empty hook has a definition",
             written.source.find("DefaultContract_Interface::Empty(") != std::string::npos);
  const auto required =
      WriteInterface(ParseInterface("interface OnlyRequired { procedure Run(); }"),
                     "OnlyRequired.Interface.al",
                     Objects());
  CHECK_TRUE("signature-only interfaces need no source", required.source.empty());
}

void InvalidBodiesRefuse() {
  bool refused = false;
  try {
    static_cast<void>(ParseInterface("interface Broken { procedure Run() begin exit(1); }"));
  } catch (const agiru::al::ParseError &) { refused = true; }
  CHECK_TRUE("an unterminated default never becomes a required signature", refused);
}

void SourcesNameOnlyTheirBodyDependencies() {
  auto objects = Objects();
  for (const std::string name : {"Required", "Used"}) {
    agiru::gen::TableRef ref;
    ref.identifier = "::agiru::" + name + "_Table";
    ref.header = name + ".h";
    objects.tables.emplace(agiru::gen::LowerKey(name), std::move(ref));
  }
  const auto written = WriteInterface(ParseInterface(R"(interface Dependencies {
        procedure Required(var Value: Record Required);
        procedure Optional(var Value: Record Used) begin end;
      })"),
                                      "Dependencies.Interface.al",
                                      objects);
  CHECK_TRUE("a default definition includes the record type it names",
             written.source.find("#include \"Used.h\"") != std::string::npos);
  CHECK_TRUE("signature-only records do not widen the source dependency list",
             written.source.find("#include \"Required.h\"") == std::string::npos);
}

void Write(const std::filesystem::path &path, std::string_view source) {
  std::ofstream file(path);
  file << source;
  file.close();
  if (!file) { throw std::runtime_error("cannot write fixture " + path.string()); }
}

void Emit(const std::filesystem::path &root) {
  std::filesystem::create_directories(root / "Fixture");
  for (const auto &[name, source] : interface_fixture::kSources) {
    Write(root / "Fixture" / name, source);
  }
  Write(root / "apps.json", R"({"apps":[{"name":"fixture","source":"Fixture","depends":[]}]})");
}

}

int main(int argc, char **argv) {
  return gate::Run("GenInterface", [argc, argv] {
    if (argc == 2) {
      Emit(argv[1]);
      return;
    }
    if (argc != 1) { throw std::runtime_error("expected at most one fixture directory"); }
    ParserKeepsBodyIdentity();
    DefinitionsStayOutOfHeaders();
    InvalidBodiesRefuse();
    SourcesNameOnlyTheirBodyDependencies();
  });
}

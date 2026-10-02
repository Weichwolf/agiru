#include "Check.h"
#include "CodeunitWriter.h"
#include "PageWriter.h"
#include "Parser.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

int main(int argc, char **argv) {
  if (argc != 3) { return 2; }
  std::ifstream input(argv[1]);
  if (!input) { throw std::runtime_error("original OData Edm Type source unavailable"); }
  const std::string original{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
  const auto table = agiru::al::ParseTable(original);
  const std::array declarations{table};
  agiru::gen::Objects objects;
  objects.tables = agiru::gen::PlatformTables(declarations);
  if (!objects.tables.contains("odata edm type")) { throw std::runtime_error("pre-existing native binding missing"); }
  const auto global = agiru::al::ParseCodeunit(R"(codeunit 50270 NativeGlobal {
    var Row: Record "OData Edm Type";
  })");
  const auto parameter = agiru::al::ParseCodeunit(R"(codeunit 50271 NativeParameter {
    procedure Read(var Row: Record "OData Edm Type"): Code[50] begin exit(Row."Key"); end;
  })");
  const auto page = agiru::al::ParsePage(R"(page 50272 NativePage {
    procedure Read(var Row: Record "OData Edm Type"): Code[50] begin exit(Row."Key"); end;
  })");
  const std::array headers{
      std::pair{"NativeGlobal.h", agiru::gen::WriteCodeunit(global, "NativeGlobal.Codeunit.al", objects).text},
      std::pair{"NativeParameter.h", agiru::gen::WriteCodeunit(parameter, "NativeParameter.Codeunit.al", objects).text},
      std::pair{"NativePage.h", agiru::gen::WritePage(page, "NativePage.Page.al", objects).text}};
  const std::filesystem::path output(argv[2]);
  std::filesystem::create_directories(output);
  return gate::Run("NativeHeaderDependencies", [&] {
    for (const auto &[name, text] : headers) {
      std::ofstream file(output / name);
      file << text;
      if (!file) { throw std::runtime_error("cannot retain generated header"); }
      CHECK_TRUE(name, text.contains("#include \"platform/ODataEdmType.h\""));
    }
  });
}

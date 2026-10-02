#include "CodeunitWriter.h"
#include "Names.h"
#include "Parser.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

int main(int argc, char **argv) {
  if (argc != 3) { return 2; }
  try {
    std::vector<agiru::al::TableObject> tables;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(argv[1])) {
      if (!entry.is_regular_file() || entry.path().extension() != ".al") { continue; }
      std::ifstream input(entry.path());
      std::ostringstream source;
      source << input.rdbuf();
      if (!agiru::gen::DeclarationOf(source.str(), agiru::gen::ObjectKind::Table).found) { continue; }
      tables.push_back(agiru::al::ParseTable(source.str()));
    }
    const auto bindings = agiru::gen::PlatformTables(tables);
    std::filesystem::create_directories(argv[2]);
    std::size_t bound = 0;
    for (const auto &table : tables) {
      const auto binding = bindings.find(std::to_string(table.id));
      if (binding == bindings.end()) { continue; }
      const auto path = std::filesystem::path(argv[2]) / (agiru::gen::Identifier(table.name) + ".cpp");
      std::ofstream output(path);
      output << binding->second.declarationAssertions;
      if (!output) { throw std::runtime_error("cannot write " + path.string()); }
      ++bound;
    }
    std::cout << "tables " << tables.size() << ", binding candidates " << bound << '\n';
    return tables.size() == 223 && bound == 14 ? 0 : 1;
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 2;
  }
}

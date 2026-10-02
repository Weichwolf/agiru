#include "Ast.h"
#include "CodeunitWriter.h"
#include "Parser.h"

#include <array>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

int main(int argc, char **argv) {
  if (argc != 2) { return 2; }
  try {
    std::ifstream file(argv[1]);
    const std::string text{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    const std::array sources{agiru::al::ParseTable(text)};
    const auto bindings = agiru::gen::PlatformTables(sources);
    if (bindings.empty()) { return 3; }
    std::cout << bindings.at(std::to_string(sources.front().id)).declarationAssertions;
    return 0;
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

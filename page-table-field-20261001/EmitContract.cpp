#include "CodeunitWriter.h"
#include "Parser.h"
#include "TableWriter.h"

#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

int main(int count, char **args) {
  if (count != 2) { return 2; }
  const std::ifstream stream(args[1]);
  if (!stream) { return 2; }
  std::ifstream input(args[1]);
  const std::string source{std::istreambuf_iterator<char>(input), {}};
  const auto table = agiru::al::ParseTable(source);
  const std::array declarations{table};
  const auto bindings = agiru::gen::PlatformTables(declarations);
  const auto found = bindings.find("2000000171");
  if (found == bindings.end()) { return 1; }
  std::cout << found->second.declarationAssertions;
}

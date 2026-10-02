#include "CodeunitWriter.h"
#include "Parser.h"
#include "TableWriter.h"

#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char **argv) {
  if (argc != 2) { return 2; }
  std::ifstream input(argv[1]);
  std::ostringstream text;
  text << input.rdbuf();
  const auto table = agiru::al::ParseTable(text.str());
  for (const auto &field : table.fields) {
    std::cout << field.number << ' ' << field.name << ' '
              << agiru::gen::FieldIdentifier(table, field.name) << '\n';
  }
  std::cout << agiru::gen::NativeTableAssertions(
      table, agiru::gen::BindTable(table, "::agiru::platform::ObjectOptions", "platform/ObjectOptions.h"));
}

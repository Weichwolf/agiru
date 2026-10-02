#include "Apps.h"

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>

std::string Lower(std::string_view text) {
  std::string value(text);
  for (char &c : value) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
  return value;
}

int main() {
  std::size_t ties = 0;
  for (const auto *path : {"scope.json", "src/gen/scope.json"}) {
    const auto rules = agiru::gen::ReadScope(std::filesystem::path(path));
    std::size_t found = 0;
    for (const auto &include : rules.include) {
      for (const auto &exclude : rules.exclude) {
        if (Lower(include) == Lower(exclude)) { ++found; }
      }
    }
    std::printf("%s: %zu equal include/exclude prefixes\n", path, found);
    ties += found;
  }
  return ties == 0 ? 0 : 1;
}

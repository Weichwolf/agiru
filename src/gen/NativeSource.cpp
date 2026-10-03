#include "NativeSource.h"

#include "EnumWriter.h"
#include "Lexer.h"
#include "Parser.h"
#include "TableKeys.h"
#include "Token.h"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::gen {

namespace {

bool DeclaresTable(std::string_view source) {
  const auto tokens = al::Tokenize(source);
  for (std::size_t at = 0; at + 1 < tokens.size(); ++at) {
    if (al::IsPunctuation(tokens[at], "{")) { return false; }
    if (al::IsKeyword(tokens[at], "table") && tokens[at + 1].kind == al::TokenKind::Integer) {
      return true;
    }
  }
  return false;
}

std::vector<std::filesystem::path> Sources(const std::filesystem::path &package) {
  const auto root = package / "src";
  if (!std::filesystem::is_directory(root)) {
    throw std::runtime_error("System source directory is missing: " + root.string());
  }
  std::vector<std::filesystem::path> paths;
  for (const auto &entry : std::filesystem::recursive_directory_iterator(root)) {
    if (entry.is_symlink()) {
      throw std::runtime_error("System source contains a symlink: " + entry.path().string());
    }
    if (entry.is_regular_file() && LowerKey(entry.path().extension().string()) == ".al") {
      paths.push_back(entry.path());
    }
  }
  std::ranges::sort(paths);
  if (paths.empty()) { throw std::runtime_error("System source contains no AL files"); }
  return paths;
}

void ReadOne(const std::filesystem::path &package,
             const std::filesystem::path &path,
             NativeTableSources &into) {
  const auto relative = std::filesystem::relative(path, package).generic_string();
  try {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) { throw std::runtime_error("cannot read System source"); }
    const std::string source{std::istreambuf_iterator<char>(stream),
                             std::istreambuf_iterator<char>()};
    if (stream.bad()) { throw std::runtime_error("cannot finish reading System source"); }
    if (!DeclaresTable(source)) {
      into.otherSources.push_back(relative);
      return;
    }
    auto table = al::ParseTable(source);
    CompletePrimaryKey(table);
    into.tables.push_back(std::move(table));
    into.paths.push_back(relative);
  } catch (const std::exception &error) {
    into.issues.push_back({.source = relative, .reason = error.what()});
  }
}

}

NativeTableSources ReadNativeTables(const std::filesystem::path &package) {
  if (std::filesystem::is_symlink(package) || std::filesystem::is_symlink(package / "src")) {
    throw std::runtime_error("System source root is a symlink");
  }
  NativeTableSources into;
  for (const auto &path : Sources(package)) { ReadOne(package, path, into); }
  return into;
}

}

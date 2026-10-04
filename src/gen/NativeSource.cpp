#include "NativeSource.h"

#include "Apps.h"
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
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::gen {

namespace {

std::string_view DeclaredKind(std::string_view source) {
  const auto tokens = al::Tokenize(source);
  for (std::size_t at = 0; at + 1 < tokens.size(); ++at) {
    if (al::IsPunctuation(tokens[at], "{")) { return {}; }
    if (al::IsKeyword(tokens[at], "interface") &&
        (tokens[at + 1].kind == al::TokenKind::Identifier ||
         tokens[at + 1].kind == al::TokenKind::QuotedIdentifier ||
         tokens[at + 1].kind == al::TokenKind::Integer)) {
      return "interface";
    }
    if (tokens[at + 1].kind != al::TokenKind::Integer) { continue; }
    for (const auto *const kind : {"table", "report", "enum"}) {
      if (al::IsKeyword(tokens[at], kind)) { return kind; }
    }
  }
  return {};
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
             NativeSources &into) {
  const auto relative = std::filesystem::relative(path, package).generic_string();
  try {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) { throw std::runtime_error("cannot read System source"); }
    const std::string source{std::istreambuf_iterator<char>(stream),
                             std::istreambuf_iterator<char>()};
    if (stream.bad()) { throw std::runtime_error("cannot finish reading System source"); }
    const auto kind = DeclaredKind(source);
    if (kind.empty()) {
      into.otherSources.push_back(relative);
      return;
    }
    if (kind == "table") {
      auto table = al::ParseTable(source);
      CompletePrimaryKey(table);
      into.tables.push_back(std::move(table));
      into.paths.push_back(relative);
    } else if (kind == "report") {
      into.reports.push_back(al::ParseReport(source));
      into.reportPaths.push_back(relative);
    } else if (kind == "enum") {
      into.enums.push_back(al::ParseEnum(source));
      into.enumPaths.push_back(relative);
    } else {
      into.interfaces.push_back(al::ParseInterface(source));
      into.interfacePaths.push_back(relative);
    }
  } catch (const std::exception &error) {
    into.issues.push_back({.source = relative, .reason = error.what()});
  }
}

bool NeedsNativeIdentity(const NativeSources &sources, const std::filesystem::path &package) {
  if (!sources.reports.empty() || !sources.enums.empty() || !sources.interfaces.empty()) {
    return true;
  }
  if (sources.tables.empty()) { return false; }
  const auto manifest = package / "NavxManifest.xml";
  return std::filesystem::exists(manifest) || std::filesystem::is_symlink(manifest);
}

template <typename Object>
void SelectFamily(std::vector<Object> &objects,
                  std::vector<std::string> &paths,
                  std::string_view kind,
                  NativeSources &sources,
                  const TranspileScope &scope) {
  if (objects.size() != paths.size()) {
    throw std::logic_error("Native source identities and paths do not match");
  }
  std::size_t kept = 0;
  for (std::size_t i = 0; i < objects.size(); ++i) {
    const auto reason = ProductExclusion(scope, paths[i], SourceDomain::SystemSymbols);
    if (reason) {
      const auto &object = objects[i];
      int id = 0;
      if constexpr (requires { object.id; }) { id = object.id; }
      sources.excluded.push_back({.source = paths[i],
                                  .reason = std::string(*reason),
                                  .kind = std::string(kind),
                                  .id = id,
                                  .name = object.name,
                                  .nameSpace = object.nameSpace});
    } else {
      if (kept != i) {
        objects[kept] = std::move(objects[i]);
        paths[kept] = std::move(paths[i]);
      }
      ++kept;
    }
  }
  objects.resize(kept);
  paths.resize(kept);
}

}

NativeSources ReadNativeSources(const std::filesystem::path &package) {
  if (std::filesystem::is_symlink(package) || std::filesystem::is_symlink(package / "src")) {
    throw std::runtime_error("System source root is a symlink");
  }
  NativeSources into;
  for (const auto &path : Sources(package)) { ReadOne(package, path, into); }
  std::set<int> reportIds;
  std::set<std::string> reportNames;
  for (const auto &report : into.reports) {
    if (!reportIds.insert(report.id).second ||
        !reportNames.insert(LowerKey(report.nameSpace + "." + report.name)).second) {
      throw std::runtime_error("System source duplicates report identity: " + report.name);
    }
  }
  std::set<int> enumIds;
  std::set<std::string> enumNames;
  std::set<std::string> enumBareNames;
  for (const auto &object : into.enums) {
    if (!enumIds.insert(object.id).second ||
        !enumNames.insert(LowerKey(object.nameSpace + "." + object.name)).second) {
      throw std::runtime_error("System source duplicates enum identity: " + object.name);
    }
    if (!enumBareNames.insert(LowerKey(object.name)).second) {
      throw std::runtime_error("System source has ambiguous unqualified enum name: " + object.name);
    }
  }
  std::set<std::string> interfaceNames;
  std::set<std::string> interfaceBareNames;
  for (const auto &object : into.interfaces) {
    if (!interfaceNames.insert(LowerKey(object.nameSpace + "." + object.name)).second) {
      throw std::runtime_error("System source duplicates interface identity: " + object.name);
    }
    if (!interfaceBareNames.insert(LowerKey(object.name)).second) {
      throw std::runtime_error("System source has ambiguous unqualified interface name: " +
                               object.name);
    }
  }
  if (NeedsNativeIdentity(into, package)) { into.app = ReadNativeIdentity(package); }
  return into;
}

void SelectNativeSources(NativeSources &sources, const TranspileScope &scope) {
  SelectFamily(sources.tables, sources.paths, "table", sources, scope);
  SelectFamily(sources.reports, sources.reportPaths, "report", sources, scope);
  SelectFamily(sources.enums, sources.enumPaths, "enum", sources, scope);
  SelectFamily(sources.interfaces, sources.interfacePaths, "interface", sources, scope);
}

}

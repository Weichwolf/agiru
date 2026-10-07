#include "meta/SystemFields.h"

#include "Ast.h"
#include "BodyWriter.h"
#include "CodeunitWriter.h"
#include "EnumWriter.h"
#include "PageWriter.h"
#include "Parser.h"

#include <array>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

constexpr int kPageArgumentCount = 5;
constexpr int kCodeunitArgumentCount = 4;
constexpr int kStoredSource = 5;

std::string Read(const char *path) {
  std::ifstream stream(path);
  if (!stream) { throw std::runtime_error("missing native source: " + std::string(path)); }
  return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

int Emit(const char *path) {
  const auto table = agiru::al::ParseTable(Read(path));
  const std::array declarations{table};
  const auto bindings =
      agiru::gen::PlatformTables(declarations, agiru::SystemFieldProfile::Runtime18);
  const auto binding = bindings.find(agiru::gen::LowerKey(table.name));
  if (binding == bindings.end()) {
    std::cerr << "unbound native table: " << table.id << ' ' << table.name << '\n';
    return 3;
  }
  if (!binding->second.native) {
    std::cerr << "stored System source requires native-storage SQL qualification: " << table.name
              << '\n';
    return kStoredSource;
  }
  static_cast<void>(agiru::gen::PlatformFieldEnums(declarations, bindings));
  std::cout << binding->second.declarationAssertions;
  return 0;
}

void Write(const std::filesystem::path &path, std::string_view text) {
  std::filesystem::create_directories(path.parent_path());
  std::ofstream stream(path);
  stream.exceptions(std::ios::failbit | std::ios::badbit);
  stream << text;
}

int EmitCodeunit(const char *source, const char *output) {
  const auto unit = agiru::al::ParseCodeunit(Read(source));
  const agiru::gen::Objects objects;
  auto path = std::filesystem::path(output) / agiru::gen::CodeunitHeaderPath(unit);
  Write(path, agiru::gen::WriteCodeunit(unit, source, objects).text);
  path.replace_extension(".cpp");
  Write(path, agiru::gen::WriteCodeunitSource(unit, source, objects));
  return 0;
}

int EmitPage(const char *tableSource, const char *pageSource, const char *output) {
  const auto table = agiru::al::ParseTable(Read(tableSource));
  const std::array declarations{table};
  agiru::gen::Objects objects;
  objects.hostProfile = agiru::SystemFieldProfile::Runtime18;
  objects.tables = agiru::gen::PlatformTables(declarations, objects.hostProfile);
  objects.fieldEnums = agiru::gen::PlatformFieldEnums(declarations, objects.tables);
  if (objects.tables.empty()) { throw std::runtime_error("native page source table is unbound"); }
  const auto page = agiru::al::ParsePage(Read(pageSource));
  const auto *source = agiru::al::Find(page.properties, "SourceTable");
  if (source == nullptr || agiru::gen::LowerKey(source->text) != agiru::gen::LowerKey(table.name)) {
    throw std::runtime_error("page source does not name the supplied native declaration");
  }
  std::filesystem::path path = std::filesystem::path(output) / agiru::gen::PageHeaderPath(page);
  Write(path, agiru::gen::WritePage(page, pageSource, objects).text);
  path.replace_extension(".cpp");
  Write(path, agiru::gen::WriteSource(page, pageSource, objects, &table));
  path.replace_extension(".def.cpp");
  Write(path, agiru::gen::WriteDefinitions(page, pageSource, objects, &table));
  return 0;
}

}

int main(int argc, char **argv) {
  try {
    if (argc == 2) { return Emit(argv[1]); }
    if (argc == kCodeunitArgumentCount && std::string_view(argv[1]) == "--codeunit") {
      return EmitCodeunit(argv[2], argv[3]);
    }
    if (argc == kPageArgumentCount && std::string_view(argv[1]) == "--page") {
      return EmitPage(argv[2], argv[3], argv[4]);
    }
    std::cerr << "usage: native-binding-emit <original-table-source>\n"
                 "       native-binding-emit --codeunit <codeunit-source> <output>\n"
                 "       native-binding-emit --page <table-source> <page-source> <output>\n";
    return 2;
  } catch (const agiru::al::ParseError &error) {
    std::cerr << error.what() << '\n';
    return 4;
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 2;
  }
}

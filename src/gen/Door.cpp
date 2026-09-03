#include "Door.h"

#include "Scope.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::gen {

namespace {

constexpr std::array kAlsoAMember{
    std::string_view{"Field"},
    std::string_view{"RecordId"},
    std::string_view{"TestField"},
    std::string_view{"TestAction"},
};

std::vector<std::string> &DoorTypes() {
  static const std::vector<std::string> types = [] {
    const std::filesystem::path door = std::filesystem::path(AGIRU_SOURCE_DIR) / "include" / "type";
    if (!std::filesystem::is_directory(door)) {
      throw std::runtime_error("the door has no type/ directory at " + door.string());
    }
    std::vector<std::string> found;
    for (const auto &entry : std::filesystem::directory_iterator(door)) {
      if (entry.path().extension() != ".h") { continue; }
      const std::string name = entry.path().stem().string();
      if (std::ranges::contains(kAlsoAMember, name)) { continue; }
      found.push_back(name);
    }
    if (found.empty()) { throw std::runtime_error("the door declares no types"); }
    return found;
  }();
  return const_cast<std::vector<std::string> &>(types);
}

std::map<std::string, std::string> &DoorSpellings() {
  static const std::map<std::string, std::string> spellings = [] {
    std::map<std::string, std::string> found;
    const std::filesystem::path root = std::filesystem::path(AGIRU_SOURCE_DIR) / "include";
    if (!std::filesystem::is_directory(root)) {
      throw std::runtime_error("the door has no include/ directory at " + root.string());
    }
    const std::regex declared(R"(\b([A-Z][A-Za-z0-9]*)\s*\()");
    for (const auto &entry : std::filesystem::recursive_directory_iterator(root)) {
      if (entry.path().extension() != ".h") { continue; }
      std::ifstream file(entry.path());
      std::string line;
      while (std::getline(file, line)) {
        if (line.starts_with("///") || line.starts_with("//")) { continue; }
        for (std::sregex_iterator it(line.begin(), line.end(), declared), end; it != end; ++it) {
          const std::string name = (*it)[1].str();
          std::string key;
          for (const char c : name) {
            key += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
          }
          const auto standing = found.find(key);
          if (standing == found.end()) {
            found.emplace(key, name);
          } else if (standing->second != name) {
            standing->second.clear();
          }
        }
      }
    }
    if (found.empty()) { throw std::runtime_error("the door declares no names"); }
    return found;
  }();
  return const_cast<std::map<std::string, std::string> &>(spellings);
}

constexpr std::array<std::pair<std::string_view, std::string_view>, 32> kElsewhere{{
    {"Temporary", "runtime/Table.h"},
    {"StateHandle", "runtime/RecordState.h"},
    {"TestPage", "runtime/test/TestPage.h"},
    {"TempStore", "runtime/Table.h"},
    {"Instance", "runtime/Codeunit.h"},
    {"CodeunitTraits", "runtime/Codeunit.h"},
    {"TableTraits", "runtime/Table.h"},
    {"PageTraits", "runtime/Page.h"},
    {"Commit", "runtime/Transaction.h"},
    {"RecordRef", "runtime/RecordRef.h"},
    {"FieldRef", "runtime/RecordRef.h"},
    {"TestCatalogue", "runtime/TestRunner.h"},
    {"InvokeTest", "runtime/TestRunner.h"},
    {"TestMethod", "runtime/TestRunner.h"},
    {"RegisterTable", "runtime/Catalogue.h"},
    {"SelectLatestVersion", "runtime/Database.h"},
    {"GetLastErrorText", "runtime/Error.h"},
    {"AssertError", "runtime/Error.h"},
    {"platform::Date", "platform/Date.h"},
    {"platform::Field", "platform/Field.h"},
    {"platform::Integer", "platform/Integer.h"},
    {"platform::Tenant", "platform/Tenant.h"},
    {"platform::User", "platform/User.h"},
    {"absent::", "dotnet/Refused.h"},
    {"DotNetGeneric", "dotnet/Generic.h"},
    {"ALConfigSettings", "dotnet/ALConfigSettings.h"},
    {"NavTenantSettingsHelper", "dotnet/NavTenantSettingsHelper.h"},
    {"UserInfo", "dotnet/UserInfo.h"},
    {"StrSubstNo", "runtime/Record.h"},
    {"Format", "runtime/Record.h"},
    {"AsText", "runtime/Record.h"},
    {"FieldNo", "meta/Ids.h"},
}};

bool Mentions(std::string_view text, std::string_view name) {
  for (std::size_t at = text.find(name); at != std::string_view::npos;
       at = text.find(name, at + 1)) {
    const bool before = at > 0 && (std::isalnum(static_cast<unsigned char>(text[at - 1])) != 0 ||
                                   text[at - 1] == '_');
    const std::size_t after = at + name.size();
    const bool behind =
        after < text.size() &&
        (std::isalnum(static_cast<unsigned char>(text[after])) != 0 || text[after] == '_');
    if (!before && (!behind || name.ends_with("::"))) { return true; }
  }
  return false;
}

}

std::string DoorIncludes(std::string_view text, ObjectKind kind) {
  std::set<std::string> headers;
  headers.insert("meta/Ids.h");
  headers.insert("runtime/Error.h");
  switch (kind) {
    case ObjectKind::Table:
      headers.insert("meta/Declare.h");
      headers.insert("meta/TableDef.h");
      headers.insert("runtime/Table.h");
      break;
    case ObjectKind::Codeunit: headers.insert("runtime/Codeunit.h"); break;
    case ObjectKind::Page: headers.insert("runtime/Page.h"); break;
    case ObjectKind::Enum: headers.insert("meta/EnumDef.h"); break;
    default: break;
  }
  for (const std::string &type : DoorTypes()) {
    if (Mentions(text, type)) { headers.insert("type/" + type + ".h"); }
  }
  for (const auto &[name, header] : kElsewhere) {
    if (Mentions(text, name)) { headers.insert(std::string(header)); }
  }
  if (text.find(") {\n") != std::string_view::npos) { headers.insert("Builtins.h"); }
  std::string out;
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }
  return out;
}

std::string WithoutEmptyNamespaces(std::string text) {
  for (std::size_t at = text.find("namespace agiru::app::"); at != std::string::npos;
       at = text.find("namespace agiru::app::", at + 1)) {
    const std::size_t open = text.find(" {\n", at);
    if (open == std::string::npos) { break; }
    if (text.compare(open + 3, std::string_view("} // namespace").size(), "} // namespace") != 0) {
      continue;
    }
    const std::size_t shut = text.find('\n', open + 3);
    if (shut == std::string::npos) { break; }
    text.erase(at, shut + 1 - at);
    at = at > 0 ? at - 1 : 0;
  }
  return text;
}

std::string WithDoor(std::string text, ObjectKind kind) {
  const std::size_t at = text.find(kDoorMarker);
  if (at == std::string::npos) { return text; }
  std::string without = text;
  without.erase(at, kDoorMarker.size());
  const std::string whole =
      without.substr(0, at) + DoorIncludes(without, kind) + "\n" + without.substr(at);
  return WithoutEmptyNamespaces(whole);
}

void KnowDoorTypes(const std::filesystem::path &include) {
  static constexpr std::array kAlsoAMember{
      std::string_view{"Field"},
      std::string_view{"RecordId"},
      std::string_view{"TestField"},
      std::string_view{"TestAction"},
  };
  DoorTypes().clear();
  if (!std::filesystem::is_directory(include / "type")) {
    throw std::runtime_error("the door has no type/ directory at " + include.string());
  }
  for (const auto &entry : std::filesystem::directory_iterator(include / "type")) {
    if (entry.path().extension() != ".h") { continue; }
    const std::string name = entry.path().stem().string();
    if (std::ranges::contains(kAlsoAMember, name)) { continue; }
    DoorTypes().push_back(name);
  }
  if (DoorTypes().empty()) { throw std::runtime_error("the door declares no types"); }
}

bool DoorDeclares(std::string_view name) {
  std::string key;
  for (const char c : name) {
    key += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return DoorSpellings().contains(key);
}

std::string AsTheDoorSpellsIt(std::string_view name) {
  std::string key;
  for (const char c : name) {
    key += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  const auto found = DoorSpellings().find(key);
  if (found == DoorSpellings().end() || found->second.empty()) { return std::string(name); }
  return found->second;
}

}

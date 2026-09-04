#include "Door.h"

#include "EnumWriter.h"
#include "Names.h"
#include "Scope.h"

#include <array>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::gen {

namespace {

std::string TextOf(const std::filesystem::path &file) {
  const std::ifstream in(file);
  std::stringstream held;
  held << in.rdbuf();
  return held.str();
}

const std::set<std::string> &BaseMembers() {
  static const std::set<std::string> members = [] {
    const std::filesystem::path bases =
        std::filesystem::path(AGIRU_SOURCE_DIR) / "include" / "runtime";
    if (!std::filesystem::is_directory(bases)) {
      throw std::runtime_error("the door has no runtime/ directory at " + bases.string());
    }
    std::string whole;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(bases)) {
      if (entry.path().extension() == ".h") { whole += TextOf(entry.path()); }
    }
    std::set<std::string> found;
    static const std::regex declared(R"([\w>&*:\s]\s([A-Z][A-Za-z0-9]*)\s*\()");
    for (std::sregex_iterator at(whole.begin(), whole.end(), declared), end; at != end; ++at) {
      found.insert((*at)[1].str());
    }
    if (found.empty()) { throw std::runtime_error("the object bases declare no members"); }
    return found;
  }();
  return members;
}

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
      if (BaseMembers().contains(name)) { continue; }
      found.push_back(name);
    }
    if (found.empty()) { throw std::runtime_error("the door declares no types"); }
    return found;
  }();
  return const_cast<std::vector<std::string> &>(types);
}

std::set<std::string> &Callables() {
  static std::set<std::string> callable;
  return callable;
}

void NoteEnumerators(const std::string &line,
                     bool &inside,
                     std::map<std::string, std::string> &found) {
  static const std::regex member(R"(^\s*([A-Z][A-Za-z0-9]*)\s*[,=])");
  if (!inside) {
    inside = line.find("enum class") != std::string::npos && line.find(';') == std::string::npos;
    return;
  }
  if (line.find("};") != std::string::npos) {
    inside = false;
    return;
  }
  std::smatch matched;
  if (!std::regex_search(line, matched, member)) { return; }
  const std::string name = matched[1].str();
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

void NoteSpellings(const std::string &line, std::map<std::string, std::string> &found) {
  static const std::regex declared(R"(\b([A-Z][A-Za-z0-9]*)\s*[({;])");
  const std::size_t first = line.find_first_not_of(" \t");
  if (first != std::string::npos && line.compare(first, 2, "//") == 0) { return; }
  for (std::sregex_iterator it(line.begin(), line.end(), declared), end; it != end; ++it) {
    const std::string name = (*it)[1].str();
    if ((*it)[0].str().back() == '(') { Callables().insert(name); }
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

std::map<std::string, std::string> ReadSpellings() {
  std::map<std::string, std::string> found;
  const std::filesystem::path root = std::filesystem::path(AGIRU_SOURCE_DIR) / "include";
  if (!std::filesystem::is_directory(root)) {
    throw std::runtime_error("the door has no include/ directory at " + root.string());
  }
  for (const auto &entry : std::filesystem::recursive_directory_iterator(root)) {
    if (entry.path().extension() != ".h") { continue; }
    std::ifstream file(entry.path());
    std::string line;
    bool inEnum = false;
    while (std::getline(file, line)) {
      NoteSpellings(line, found);
      NoteEnumerators(line, inEnum, found);
    }
  }
  if (found.empty()) { throw std::runtime_error("the door declares no names"); }
  return found;
}

const std::map<std::string, std::string> &DoorSpellings() {
  static const std::map<std::string, std::string> spellings = ReadSpellings();
  return spellings;
}

constexpr std::array<std::pair<std::string_view, std::string_view>, 34> kElsewhere{{
    {"Implementation", "runtime/Implementation.h"},
    {"CurrFieldNo", "runtime/Table.h"},
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
  if (text.find(") {\n") != std::string_view::npos) {
    headers.insert("Builtins.h");
    headers.insert("BuiltinsWritten.h");
  }
  std::string out;
  for (const std::string &header : headers) { out += "#include \"" + header + "\"\n"; }
  return out;
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

bool DoorDeclares(std::string_view name) {
  std::string key;
  for (const char c : name) {
    key += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return DoorSpellings().contains(key);
}

const std::map<std::string, std::string> &PlatformMembers(std::string_view table) {
  static std::map<std::string, std::map<std::string, std::string>> members;
  const std::string key = LowerKey(std::string(table));
  const auto held = members.find(key);
  if (held != members.end()) { return held->second; }
  const std::filesystem::path page =
      std::filesystem::path(AGIRU_SOURCE_DIR) / "include" / "platform" / (Identifier(table) + ".h");
  std::map<std::string, std::string> declared;
  if (std::filesystem::is_regular_file(page)) {
    static const std::regex member(R"(^\s*[\w:<>,&*\s]+?\s([A-Z][A-Za-z0-9]*_?)\s*(\{\})?;\s*$)");
    std::ifstream file(page);
    std::string line;
    while (std::getline(file, line)) {
      std::smatch matched;
      if (!std::regex_match(line, matched, member)) { continue; }
      std::string name = matched[1].str();
      std::string bare = name;
      if (bare.ends_with("_")) { bare.pop_back(); }
      declared.insert_or_assign(LowerKey(bare), name);
    }
  }
  return members.emplace(key, std::move(declared)).first->second;
}

bool PlatformFieldNamed(const PlatformField &wanted) {
  return PlatformMembers(wanted.table).contains(LowerKey(std::string(wanted.field)));
}

std::string PlatformFieldSpelling(const PlatformField &wanted) {
  const auto &declared = PlatformMembers(wanted.table);
  const auto found = declared.find(LowerKey(std::string(wanted.field)));
  return found == declared.end() ? std::string{} : found->second;
}

bool HiddenByABaseMember(std::string_view name) {
  return BaseMembers().contains(std::string(name));
}

bool DoorCalls(std::string_view name) {
  const std::string spelled = AsTheDoorSpellsIt(name);
  return Callables().contains(spelled);
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

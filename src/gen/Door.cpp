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
#include <span>
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

const std::map<std::string, std::string> &TestDoorHeaders() {
  static const std::map<std::string, std::string> found = [] {
    const std::filesystem::path door =
        std::filesystem::path(AGIRU_SOURCE_DIR) / "include" / "runtime" / "test";
    if (!std::filesystem::is_directory(door)) {
      throw std::runtime_error("the door has no runtime/test/ directory at " + door.string());
    }
    std::map<std::string, std::string> named;
    for (const auto &entry : std::filesystem::directory_iterator(door)) {
      if (entry.path().extension() != ".h") { continue; }
      const std::string stem = entry.path().stem().string();
      named.emplace(stem, "runtime/test/" + stem + ".h");
    }
    if (named.empty()) { throw std::runtime_error("the door declares no test type"); }
    return named;
  }();
  return found;
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
      found.push_back(BaseMembers().contains(name) ? "agiru::" + name : name);
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

std::set<std::string> &StaticCallables() {
  static std::set<std::string> callable;
  return callable;
}

std::string Folded(std::string_view name) {
  std::string key;
  for (const char c : name) {
    key += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return key;
}

void NoteStaticCalls(const std::string &line, std::string_view type) {
  static const std::regex declared(R"(\b([A-Z][A-Za-z0-9]*)\s*\()");
  const std::size_t first = line.find_first_not_of(" \t");
  if (first == std::string::npos || line.compare(first, 2, "//") == 0) { return; }
  if (line.find("static ") == std::string::npos) { return; }
  for (std::sregex_iterator it(line.begin(), line.end(), declared), end; it != end; ++it) {
    StaticCallables().insert(Folded(type) + "::" + Folded((*it)[1].str()));
  }
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

constexpr std::array<std::pair<std::string_view, std::string_view>, 1> kSpelledApart{{
    {"time", "CurrentTime"},
}};

std::map<std::string, std::string> ReadSpellings() {
  std::map<std::string, std::string> found;
  const std::filesystem::path root = std::filesystem::path(AGIRU_SOURCE_DIR) / "include";
  if (!std::filesystem::is_directory(root)) {
    throw std::runtime_error("the door has no include/ directory at " + root.string());
  }
  for (const auto &entry : std::filesystem::recursive_directory_iterator(root)) {
    if (entry.path().extension() != ".h") { continue; }
    const bool platformTable = entry.path().parent_path().filename() == "platform";
    if (entry.path().filename() == "Refused.h") { continue; }
    std::ifstream file(entry.path());
    std::string line;
    bool inEnum = false;
    const std::string type = entry.path().stem().string();
    while (std::getline(file, line)) {
      if (!platformTable) { NoteSpellings(line, found); }
      NoteEnumerators(line, inEnum, found);
      NoteStaticCalls(line, type);
    }
  }
  if (found.empty()) { throw std::runtime_error("the door declares no names"); }
  return found;
}

const std::map<std::string, std::string> &DoorSpellings() {
  static const std::map<std::string, std::string> spellings = ReadSpellings();
  return spellings;
}

constexpr std::array kJson{std::string_view{"JsonToken"},
                           std::string_view{"JsonValue"},
                           std::string_view{"JsonObject"},
                           std::string_view{"JsonArray"}};
constexpr std::array kHttp{std::string_view{"HttpClient"},
                           std::string_view{"HttpContent"},
                           std::string_view{"HttpHeaders"},
                           std::string_view{"HttpRequestMessage"},
                           std::string_view{"HttpResponseMessage"}};
constexpr std::array kXml{std::string_view{"XmlDocument"},
                          std::string_view{"XmlElement"},
                          std::string_view{"XmlNode"},
                          std::string_view{"XmlNodeList"},
                          std::string_view{"XmlAttribute"},
                          std::string_view{"XmlAttributeCollection"},
                          std::string_view{"XmlText"},
                          std::string_view{"XmlNameTable"},
                          std::string_view{"XmlNamespaceManager"}};

std::span<const std::string_view> DoorFamily(char which) {
  switch (which) {
    case 'j': return kJson;
    case 'h': return kHttp;
    default: return kXml;
  }
}

constexpr std::array<std::pair<std::string_view, char>, 18> kFamilies{{
    {"JsonToken", 'j'},
    {"JsonValue", 'j'},
    {"JsonObject", 'j'},
    {"JsonArray", 'j'},
    {"HttpClient", 'h'},
    {"HttpContent", 'h'},
    {"HttpHeaders", 'h'},
    {"HttpRequestMessage", 'h'},
    {"HttpResponseMessage", 'h'},
    {"XmlDocument", 'x'},
    {"XmlElement", 'x'},
    {"XmlNode", 'x'},
    {"XmlNodeList", 'x'},
    {"XmlAttribute", 'x'},
    {"XmlAttributeCollection", 'x'},
    {"XmlText", 'x'},
    {"XmlNameTable", 'x'},
    {"XmlNamespaceManager", 'x'},
}};

constexpr std::array<std::pair<std::string_view, std::string_view>, 89> kElsewhere{{
    {"Implementation", "runtime/Implementation.h"},
    {"CurrFieldNo", "runtime/Table.h"},
    {"Temporary", "runtime/Table.h"},
    {"StateHandle", "runtime/RecordState.h"},
    {"TestPage", "runtime/test/TestPage.h"},
    {"TestRequestPage", "runtime/test/TestRequestPage.h"},
    {"TempStore", "runtime/Table.h"},
    {"Instance", "runtime/Codeunit.h"},
    {"CodeunitTraits", "runtime/Codeunit.h"},
    {"TableTraits", "runtime/Table.h"},
    {"PageTraits", "runtime/Page.h"},
    {"Commit", "runtime/Transaction.h"},
    {"RecordRef", "runtime/RecordRef.h"},
    {"FieldRef", "runtime/RecordRef.h"},
    {"Page", "runtime/Page.h"},
    {"CommitScope", "runtime/Scopes.h"},
    {"ErrorScope", "runtime/Scopes.h"},
    {"InStream", "type/Stream.h"},
    {"OutStream", "type/Stream.h"},
    {"Report", "runtime/Report.h"},
    {"XmlPort", "runtime/Report.h"},
    {"Query", "runtime/Query.h"},
    {"QueryTraits", "runtime/Query.h"},
    {"QueryHandle", "runtime/Query.h"},
    {"QueryDef", "meta/QueryDef.h"},
    {"GenericList1", "dotnet/Generic.h"},
    {"GenericDictionary2", "dotnet/Generic.h"},
    {"Materialised", "runtime/Events.h"},
    {"Subscription", "runtime/Events.h"},
    {"SubscriptionCatalogue", "runtime/Events.h"},
    {"InvokeSubscriber", "runtime/Events.h"},
    {"RaiseEvent", "runtime/Events.h"},
    {"EventObject", "runtime/Events.h"},
    {"TestCatalogue", "runtime/TestRunner.h"},
    {"InvokeTest", "runtime/TestRunner.h"},
    {"TestMethod", "runtime/TestRunner.h"},
    {"RegisterTable", "runtime/Catalogue.h"},
    {"SelectLatestVersion", "runtime/Database.h"},
    {"GetLastErrorText", "runtime/Error.h"},
    {"AssertError", "runtime/Error.h"},
    {"platform::AllObj", "platform/AllObj.h"},
    {"platform::AllObjWithCaption", "platform/AllObjWithCaption.h"},
    {"platform::AllObjType", "platform/AllObjType.h"},
    {"platform::AllProfile", "platform/AllProfile.h"},
    {"platform::FeatureKey", "platform/FeatureKey.h"},
    {"platform::FeatureKeyEnabled", "platform/FeatureKey.h"},
    {"platform::Company", "platform/Company.h"},
    {"platform::Date", "platform/Date.h"},
    {"platform::Field", "platform/Field.h"},
    {"platform::Integer", "platform/Integer.h"},
    {"platform::PrivacyNotice", "platform/PrivacyNotice.h"},
    {"platform::PrivacyNoticeApproval", "platform/PrivacyNoticeApproval.h"},
    {"platform::RecordLink", "platform/RecordLink.h"},
    {"platform::RecordLinkType", "platform/RecordLink.h"},
    {"platform::TenantLicenseState", "platform/TenantLicenseState.h"},
    {"platform::TenantLicenseStateState", "platform/TenantLicenseState.h"},
    {"platform::Tenant", "platform/Tenant.h"},
    {"platform::User", "platform/User.h"},
    {"platform::UserPersonalization", "platform/UserPersonalization.h"},
    {"absent::", "dotnet/Refused.h"},
    {"DotNetGeneric", "dotnet/Generic.h"},
    {"ALConfigSettings", "dotnet/ALConfigSettings.h"},
    {"NavTenantSettingsHelper", "dotnet/NavTenantSettingsHelper.h"},
    {"UserInfo", "dotnet/UserInfo.h"},
    {"dotnet::Path", "dotnet/Path.h"},
    {"dotnet::Math", "dotnet/Math.h"},
    {"dotnet::NavTestExecution", "dotnet/NavTestExecution.h"},
    {"dotnet::BinaryReader", "dotnet/BinaryReader.h"},
    {"dotnet::BinaryWriter", "dotnet/BinaryWriter.h"},
    {"dotnet::XmlDocument", "dotnet/XmlDocument.h"},
    {"dotnet::XmlElement", "dotnet/XmlDocument.h"},
    {"dotnet::XmlAttribute", "dotnet/XmlDocument.h"},
    {"dotnet::XmlAttributeCollection", "dotnet/XmlDocument.h"},
    {"dotnet::XmlNamespaceManager", "dotnet/XmlDocument.h"},
    {"dotnet::XmlDeclaration", "dotnet/XmlDocument.h"},
    {"dotnet::XmlDocumentType", "dotnet/XmlDocument.h"},
    {"dotnet::XmlProcessingInstruction", "dotnet/XmlDocument.h"},
    {"dotnet::XmlComment", "dotnet/XmlDocument.h"},
    {"dotnet::XmlText", "dotnet/XmlDocument.h"},
    {"dotnet::XmlCDataSection", "dotnet/XmlDocument.h"},
    {"dotnet::XmlNode", "dotnet/XmlNode.h"},
    {"dotnet::XmlNodeList", "dotnet/XmlNode.h"},
    {"DateTimeOffset", "dotnet/DateTimeOffset.h"},
    {"DateTime", "dotnet/DateTime.h"},
    {"StrSubstNo", "runtime/Record.h"},
    {"Format", "runtime/Record.h"},
    {"AsText", "runtime/Record.h"},
    {"FieldNo", "meta/Ids.h"},
    {"At", "type/AlArray.h"},
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
  for (std::size_t at = text.find("namespace agiru::"); at != std::string::npos;
       at = text.find("namespace agiru::", at + 1)) {
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
    case ObjectKind::Codeunit:
      headers.insert("meta/CodeunitDef.h");
      headers.insert("runtime/Codeunit.h");
      break;
    case ObjectKind::Page:
      headers.insert("meta/PageDef.h");
      headers.insert("runtime/Page.h");
      break;
    case ObjectKind::Enum: headers.insert("meta/EnumDef.h"); break;
    default: break;
  }
  for (const std::string &type : DoorTypes()) {
    if (!Mentions(text, type)) { continue; }
    const std::size_t bare = type.starts_with("agiru::") ? std::string_view{"agiru::"}.size() : 0;
    headers.insert("type/" + type.substr(bare) + ".h");
  }
  for (const auto &[member, family] : kFamilies) {
    if (!headers.contains("type/" + std::string(member) + ".h")) { continue; }
    for (const std::string_view &beside : DoorFamily(family)) {
      headers.insert("type/" + std::string(beside) + ".h");
    }
  }
  for (const auto &[name, header] : kElsewhere) {
    if (Mentions(text, name)) { headers.insert(std::string(header)); }
  }
  for (const auto &[name, header] : TestDoorHeaders()) {
    if (Mentions(text, name)) { headers.insert(header); }
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

namespace {

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
}

bool PlatformFieldNamed(const PlatformField &wanted) {
  return PlatformMembers(wanted.table).contains(LowerKey(Identifier(wanted.field)));
}

std::string PlatformFieldSpelling(const PlatformField &wanted) {
  const auto &declared = PlatformMembers(wanted.table);
  const auto found = declared.find(LowerKey(Identifier(wanted.field)));
  return found == declared.end() ? std::string{} : found->second;
}

const std::set<std::string> &TableMembers() {
  static const std::set<std::string> members = [] {
    const std::filesystem::path table =
        std::filesystem::path(AGIRU_SOURCE_DIR) / "include" / "runtime" / "Table.h";
    const std::string whole = TextOf(table);
    std::set<std::string> found;
    static const std::regex declared(R"([\w>&*:\s]\s([A-Z][A-Za-z0-9]*)\s*\()");
    for (std::sregex_iterator at(whole.begin(), whole.end(), declared), end; at != end; ++at) {
      found.insert((*at)[1].str());
    }
    if (found.empty()) { throw std::runtime_error("the door's Table.h declares no member at all"); }
    return found;
  }();
  return members;
}

bool HiddenByABaseMember(std::string_view name) {
  return BaseMembers().contains(std::string(name));
}

bool DoorCalls(std::string_view name) {
  const std::string spelled = AsTheDoorSpellsIt(name);
  return Callables().contains(spelled);
}

bool DoorStaticCalls(const StaticMember &wanted) {
  DoorSpellings();
  return StaticCallables().contains(Folded(wanted.type) + "::" + Folded(wanted.member));
}

std::string BuiltinSpelling(std::string_view name) {
  std::string key;
  for (const char c : name) {
    key += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  for (const auto &[al, door] : kSpelledApart) {
    if (al == key) { return std::string(door); }
  }
  return AsTheDoorSpellsIt(name);
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

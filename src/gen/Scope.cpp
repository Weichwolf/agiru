#include "Scope.h"

#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::gen {

namespace {

std::string Lower(std::string_view text) {
  std::string out;
  out.reserve(text.size());
  for (const char c : text) {
    out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return out;
}

class JsonArrays {
public:
  explicit JsonArrays(std::string json) : json_(std::move(json)) {}

  [[nodiscard]] std::vector<std::string> Read(std::string_view key) const;

private:
  std::string json_;
};

std::vector<std::string> JsonArrays::Read(std::string_view key) const {
  const std::string_view json = json_;
  const std::string needle = "\"" + std::string(key) + "\"";
  const std::size_t at = json.find(needle);
  if (at == std::string_view::npos) { return {}; }
  const std::size_t open = json.find('[', at);
  const std::size_t close = json.find(']', open);
  if (open == std::string_view::npos || close == std::string_view::npos) { return {}; }

  std::vector<std::string> values;
  std::size_t cursor = open;
  while (true) {
    const std::size_t quote = json.find('"', cursor);
    if (quote == std::string_view::npos || quote > close) { break; }
    const std::size_t end = json.find('"', quote + 1);
    if (end == std::string_view::npos || end > close) { break; }
    values.emplace_back(json.substr(quote + 1, end - quote - 1));
    cursor = end + 1;
  }
  return values;
}

bool Matches(std::string_view nameSpace, std::string_view prefix) {
  return nameSpace == prefix || (nameSpace.size() > prefix.size() &&
                                 nameSpace.starts_with(prefix) && nameSpace[prefix.size()] == '.');
}

std::size_t LongestMatch(const std::vector<std::string> &prefixes, std::string_view nameSpace) {
  std::size_t best = 0;
  for (const std::string &prefix : prefixes) {
    if (Matches(nameSpace, prefix) && prefix.size() > best) { best = prefix.size(); }
  }
  return best;
}

std::string SnakeCase(std::string_view segment) {
  std::string out;
  for (std::size_t i = 0; i < segment.size(); ++i) {
    const char c = segment[i];
    const bool boundary =
        i != 0 && (std::isupper(static_cast<unsigned char>(c)) != 0) &&
        (std::islower(static_cast<unsigned char>(segment[i - 1])) != 0 ||
         (i + 1 < segment.size() && std::islower(static_cast<unsigned char>(segment[i + 1])) != 0));
    if (boundary) { out += '_'; }
    out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return out;
}

} // namespace

Scope Scope::FromFile(const std::filesystem::path &path) {
  const std::ifstream file(path);
  if (!file) { throw std::runtime_error("scope: cannot read " + path.string()); }
  std::ostringstream text;
  text << file.rdbuf();
  const std::string json = text.str();

  const JsonArrays arrays(json);
  Scope scope;
  for (const std::string &value : arrays.Read("include")) {
    scope.include_.push_back(Lower(value));
  }
  for (const std::string &value : arrays.Read("exclude")) {
    scope.exclude_.push_back(Lower(value));
  }
  return scope;
}

bool Scope::Contains(std::string_view nameSpace) const {
  if (nameSpace.empty()) { return true; }
  const std::string lowered = Lower(nameSpace);
  const std::size_t included = LongestMatch(include_, lowered);
  const std::size_t excluded = LongestMatch(exclude_, lowered);
  return included > excluded;
}

std::string_view DirectoryOf(ObjectKind kind) {
  switch (kind) {
    case ObjectKind::Table: return "table";
    case ObjectKind::Codeunit: return "codeunit";
    case ObjectKind::Page: return "page";
    case ObjectKind::Report: return "report";
    case ObjectKind::Query: return "query";
    case ObjectKind::XmlPort: return "xmlport";
    case ObjectKind::Enum: return "enum";
    case ObjectKind::Interface: return "interface";
    case ObjectKind::PermissionSet: return "permissionset";
  }
  return "core";
}

std::string OutputDirectory(std::string_view nameSpace, ObjectKind kind) {
  const std::string_view directory = DirectoryOf(kind);
  std::string_view rest = nameSpace;
  constexpr std::string_view kRoot = "Microsoft";
  if (rest == kRoot) { return "core/" + std::string(directory); }
  if (rest.starts_with(std::string(kRoot) + ".")) { rest.remove_prefix(kRoot.size() + 1); }
  if (rest.empty()) { return "core/" + std::string(directory); }

  std::string out;
  std::string segment;
  for (const char c : rest) {
    if (c == '.') {
      out += SnakeCase(segment) + "/";
      segment.clear();
      continue;
    }
    segment += c;
  }
  out += SnakeCase(segment) + "/" + std::string(directory);
  return out;
}

} // namespace agiru::gen

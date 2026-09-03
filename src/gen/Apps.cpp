#include "Apps.h"

#include <algorithm>
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

class Reader {
public:
  explicit Reader(std::string text) : text_(std::move(text)) {}

  [[nodiscard]] std::vector<std::string> At(std::string_view key) {
    at_ = 0;
    const std::size_t found = text_.find(key);
    if (found == std::string::npos) { return {}; }
    at_ = text_.find(':', found) + 1;
    SkipSpace();
    return Strings();
  }

  [[nodiscard]] std::vector<App> Apps() {
    Seek("\"apps\"");
    Expect('[');
    std::vector<App> apps;
    while (SkipSpace() && Peek() != ']') {
      apps.push_back(Object());
      if (Peek() == ',') { ++at_; }
    }
    return apps;
  }

private:
  [[nodiscard]] char Peek() const { return at_ < text_.size() ? text_[at_] : '\0'; }

  bool SkipSpace() {
    while (at_ < text_.size() && (std::isspace(static_cast<unsigned char>(text_[at_])) != 0)) {
      ++at_;
    }
    return at_ < text_.size();
  }

  void Expect(char c) {
    if (!SkipSpace() || text_[at_] != c) {
      throw std::runtime_error(std::string("apps.json: expected '") + c + "'");
    }
    ++at_;
  }

  void Seek(std::string_view key) {
    const std::size_t found = text_.find(key);
    if (found == std::string::npos) { throw std::runtime_error("apps.json: no \"apps\" array"); }
    at_ = text_.find(':', found) + 1;
  }

  [[nodiscard]] std::string String() {
    Expect('"');
    std::string out;
    while (at_ < text_.size() && text_[at_] != '"') {
      out += text_[at_];
      ++at_;
    }
    ++at_;
    return out;
  }

  [[nodiscard]] std::vector<std::string> Strings() {
    Expect('[');
    std::vector<std::string> out;
    while (SkipSpace() && Peek() != ']') {
      out.push_back(String());
      if (SkipSpace() && Peek() == ',') { ++at_; }
    }
    ++at_;
    return out;
  }

  [[nodiscard]] App Object() {
    Expect('{');
    App app;
    while (SkipSpace() && Peek() != '}') {
      const std::string key = String();
      Expect(':');
      SkipSpace();
      if (key == "depends") {
        app.depends = Strings();
      } else if (key == "name") {
        app.name = String();
      } else if (key == "source") {
        app.source = String();
      } else {
        (void)String();
      }
      if (SkipSpace() && Peek() == ',') { ++at_; }
    }
    ++at_;
    if (app.name.empty() || app.source.empty()) {
      throw std::runtime_error("apps.json: an app declares no name or no source");
    }
    return app;
  }

  std::string text_;
  std::size_t at_ = 0;
};

} // namespace

std::vector<App> ReadApps(const std::filesystem::path &path) {
  const std::ifstream file(path);
  if (!file) { throw std::runtime_error("apps.json: cannot read " + path.string()); }
  std::ostringstream text;
  text << file.rdbuf();
  std::vector<App> apps = Reader(text.str()).Apps();
  if (apps.empty()) { throw std::runtime_error("apps.json: declares no apps"); }
  return apps;
}

namespace {

std::string Lowered(std::string_view text) {
  std::string out(text);
  for (char &c : out) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
  return out;
}

std::size_t LongestPrefix(std::string_view lowered, const std::vector<std::string> &table) {
  std::size_t best = 0;
  for (const std::string &entry : table) {
    const std::string prefix = Lowered(entry);
    const bool matches =
        lowered == prefix || (lowered.starts_with(prefix) && lowered.size() > prefix.size() &&
                              lowered[prefix.size()] == '.');
    if (matches && prefix.size() > best) { best = prefix.size(); }
  }
  return best;
}

} // namespace

bool Holds(const TranspileScope &scope, std::string_view nameSpace) {
  if (nameSpace.empty()) { return true; }
  const std::string lowered = Lowered(nameSpace);
  const std::size_t included = LongestPrefix(lowered, scope.include);
  if (included == 0) { return false; }
  return LongestPrefix(lowered, scope.exclude) <= included;
}

bool HoldsArea(const TranspileScope &scope, std::string_view area) {
  const std::string lowered = Lowered(area);
  const auto named = [&lowered](const std::string &entry) { return lowered == Lowered(entry); };
  const auto ends = [&lowered](const std::string &s) { return lowered.ends_with(Lowered(s)); };
  return !std::ranges::any_of(scope.areaExclude, named) &&
         !std::ranges::any_of(scope.areaExcludeSuffix, ends);
}

TranspileScope ReadScope(const std::filesystem::path &path) {
  const std::ifstream file(path);
  if (!file) { throw std::runtime_error("scope.json: cannot read " + path.string()); }
  std::ostringstream text;
  text << file.rdbuf();
  Reader reader(text.str());
  TranspileScope scope;
  scope.include = reader.At("\"include\"");
  scope.exclude = reader.At("\"exclude\"");
  scope.areaExclude = reader.At("\"area_exclude\"");
  scope.areaExcludeSuffix = reader.At("\"area_exclude_suffix\"");
  if (scope.include.empty()) {
    throw std::runtime_error("scope.json: the include list is empty, so nothing is in scope");
  }
  return scope;
}

} // namespace agiru::gen

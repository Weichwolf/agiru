#include "Apps.h"

#include "Scope.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <optional>
#include <set>
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

}

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

}

bool Holds(const TranspileScope &scope, std::string_view nameSpace) {
  return NamespaceInScope(nameSpace, scope.include, scope.exclude);
}

bool HoldsArea(const TranspileScope &scope, std::string_view area) {
  const std::string lowered = Lowered(area);
  const auto named = [&lowered](const std::string &entry) { return lowered == Lowered(entry); };
  const auto ends = [&lowered](const std::string &s) { return lowered.ends_with(Lowered(s)); };
  return !std::ranges::any_of(scope.areaExclude, named) &&
         !std::ranges::any_of(scope.areaExcludeSuffix, ends);
}

namespace {

void RequireRelativeSource(std::string_view name) {
  const std::filesystem::path source(name);
  if (name.empty() || source.is_absolute() || name.find('\\') != std::string_view::npos ||
      name.find("//") != std::string_view::npos) {
    throw std::runtime_error("scope.json: product exclusions require relative source paths");
  }
  for (const auto &part : source) {
    if (part == "." || part == "..") {
      throw std::runtime_error("scope.json: product exclusions cannot traverse source roots");
    }
  }
}

}

std::optional<std::string_view> ProductExclusion(const TranspileScope &scope,
                                                 const std::filesystem::path &relativeSource) {
  const std::string source = relativeSource.generic_string();
  RequireRelativeSource(source);
  for (const SourceExclusion &rule : scope.productExclude) {
    if (source == rule.source || (rule.source.ends_with('/') && source.starts_with(rule.source))) {
      return rule.reason;
    }
  }
  return std::nullopt;
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
  std::set<std::string> selected;
  for (const std::string &entry : reader.At("\"product_exclude\"")) {
    const std::size_t colon = entry.find(':');
    if (colon == std::string::npos) {
      throw std::runtime_error("scope.json: product exclusions need a reason and source");
    }
    const std::string reason = entry.substr(0, colon);
    const std::string source = entry.substr(colon + 1);
    if (reason != "bc-licensing" && reason != "microsoft-cloud" &&
        reason != "licensing-and-microsoft-cloud") {
      throw std::runtime_error("scope.json: product exclusion reason is not approved");
    }
    RequireRelativeSource(source);
    if (!selected.insert(source).second) {
      throw std::runtime_error("scope.json: duplicate product exclusion source");
    }
    scope.productExclude.push_back(SourceExclusion{.reason = reason, .source = source});
  }
  if (scope.include.empty()) {
    throw std::runtime_error("scope.json: the include list is empty, so nothing is in scope");
  }
  return scope;
}

}

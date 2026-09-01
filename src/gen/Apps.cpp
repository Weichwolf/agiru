#include "Apps.h"

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

// A READER FOR THIS FILE AND NOT A JSON LIBRARY. apps.json is written by hand, has four keys and is
// read once per run; a dependency that parses every JSON in existence would be carried by every
// build of this tree for that. The shape it accepts is the shape apps.json has, and anything else
// is a refusal with a reason rather than a quiet default.
class Reader {
public:
  explicit Reader(std::string text) : text_(std::move(text)) {}

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

} // namespace agiru::gen

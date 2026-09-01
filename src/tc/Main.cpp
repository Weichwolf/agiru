#include "Ast.h"
#include "Parser.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <map>
#include <print>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Failure {
  std::string reason;
  std::string path;
  std::string detail;
};

std::string Read(const std::filesystem::path &path) {
  const std::ifstream file(path);
  std::ostringstream text;
  text << file.rdbuf();
  return text.str();
}

std::string Normalised(std::string_view reason) {
  const std::size_t onLine = reason.find(" on line");
  return std::string(onLine == std::string_view::npos ? reason : reason.substr(0, onLine));
}

int Scan(const std::filesystem::path &root) {
  std::vector<std::filesystem::path> sources;
  for (const auto &entry : std::filesystem::recursive_directory_iterator(root)) {
    if (entry.is_regular_file() && entry.path().string().ends_with(".Table.al")) {
      sources.push_back(entry.path());
    }
  }
  std::ranges::sort(sources);

  std::size_t parsed = 0;
  std::size_t fields = 0;
  std::vector<Failure> failures;
  for (const std::filesystem::path &path : sources) {
    try {
      const agiru::al::TableObject table = agiru::al::ParseTable(Read(path));
      ++parsed;
      fields += table.fields.size();
    } catch (const std::exception &e) {
      failures.push_back(Failure{.reason = Normalised(e.what()),
                                 .path = std::filesystem::relative(path, root).string(),
                                 .detail = e.what()});
    }
  }

  std::println("tables    {} of {} parsed ({} fields)", parsed, sources.size(), fields);
  if (failures.empty()) { return 0; }

  std::map<std::string, std::vector<std::string>> clusters;
  for (const Failure &failure : failures) {
    clusters[failure.reason].push_back(failure.path + "  (" + failure.detail + ")");
  }
  std::println("failures  {} in {} cluster(s)", failures.size(), clusters.size());
  for (const auto &[reason, paths] : clusters) {
    std::println("  {:5}  {}", paths.size(), reason);
    for (std::size_t i = 0; i < paths.size() && i < 3; ++i) {
      std::println("         {}", paths[i]);
    }
  }
  return 0;
}

} // namespace

int main(int argc, char **argv) {
  const std::span<char *> arguments(argv, static_cast<std::size_t>(argc));
  if (arguments.size() < 2) {
    std::fputs("agirutc <al-source-root>\n", stderr);
    return 2;
  }
  try {
    return Scan(std::filesystem::path(arguments[1]));
  } catch (const std::exception &e) {
    std::fputs("agirutc: ", stderr);
    std::fputs(e.what(), stderr);
    std::fputs("\n", stderr);
    return 1;
  } catch (...) {
    std::fputs("agirutc: an unknown exception left the transpiler\n", stderr);
    return 1;
  }
}

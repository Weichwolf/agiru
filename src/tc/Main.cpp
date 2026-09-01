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

bool Declares(std::string_view reason) {
  return reason.find("declares no ") != std::string_view::npos;
}

struct Counts {
  std::size_t files = 0;
  std::size_t parsed = 0;
  std::size_t members = 0;
  std::size_t tests = 0;
  std::size_t unitFiles = 0;
  std::size_t unitTests = 0;
};

bool IsUnitTest(std::string_view name) {
  return name.size() >= 2 && (name.ends_with("UT") || name.ends_with("Ut") || name.ends_with("ut"));
}

void Report(std::string_view what, const Counts &counts) {
  if (what == "codeunits") {
    std::println("{:<10}{} of {} parsed ({} procedures, {} [Test] methods)",
                 what,
                 counts.parsed,
                 counts.files,
                 counts.members,
                 counts.tests);
    if (counts.unitTests != 0) {
      std::println("{:<10}{} codeunits, {} [Test] methods -- the milestone's population",
                   "UT",
                   counts.unitFiles,
                   counts.unitTests);
    }
    return;
  }
  std::println(
      "{:<10}{} of {} parsed ({} fields)", what, counts.parsed, counts.files, counts.members);
}

void Cluster(const std::vector<Failure> &failures) {
  if (failures.empty()) { return; }
  std::map<std::string, std::vector<std::string>> clusters;
  for (const Failure &failure : failures) {
    clusters[failure.reason].push_back(failure.path + "  (" + failure.detail + ")");
  }
  std::println("failures  {} in {} cluster(s)", failures.size(), clusters.size());
  for (const auto &[reason, paths] : clusters) {
    std::println("  {:5}  {}", paths.size(), reason);
    for (std::size_t i = 0; i < paths.size() && i < 2; ++i) {
      std::println("         {}", paths[i]);
    }
  }
}

std::vector<std::filesystem::path> SourcesEndingIn(const std::filesystem::path &root,
                                                   std::string_view suffix) {
  std::vector<std::filesystem::path> sources;
  for (const auto &entry : std::filesystem::recursive_directory_iterator(root)) {
    if (entry.is_regular_file() && entry.path().string().ends_with(suffix)) {
      sources.push_back(entry.path());
    }
  }
  std::ranges::sort(sources);
  return sources;
}

int Scan(const std::filesystem::path &root) {
  std::vector<Failure> failures;

  Counts tables;
  for (const std::filesystem::path &path : SourcesEndingIn(root, ".Table.al")) {
    ++tables.files;
    try {
      const agiru::al::TableObject table = agiru::al::ParseTable(Read(path));
      ++tables.parsed;
      tables.members += table.fields.size();
    } catch (const std::exception &e) {
      if (Declares(e.what())) {
        --tables.files;
        continue;
      }
      failures.push_back(Failure{.reason = Normalised(e.what()),
                                 .path = std::filesystem::relative(path, root).string(),
                                 .detail = e.what()});
    }
  }

  Counts codeunits;
  for (const std::filesystem::path &path : SourcesEndingIn(root, ".Codeunit.al")) {
    ++codeunits.files;
    try {
      const agiru::al::CodeunitObject unit = agiru::al::ParseCodeunit(Read(path));
      ++codeunits.parsed;
      codeunits.members += unit.procedures.size();
      std::size_t tests = 0;
      for (const agiru::al::ProcedureDecl &procedure : unit.procedures) {
        if (agiru::al::HasAttribute(procedure, "Test")) { ++tests; }
      }
      codeunits.tests += tests;
      if (tests != 0 && IsUnitTest(unit.name)) {
        ++codeunits.unitFiles;
        codeunits.unitTests += tests;
      }
    } catch (const std::exception &e) {
      if (Declares(e.what())) {
        --codeunits.files;
        continue;
      }
      failures.push_back(Failure{.reason = Normalised(e.what()),
                                 .path = std::filesystem::relative(path, root).string(),
                                 .detail = e.what()});
    }
  }

  Report("tables", tables);
  Report("codeunits", codeunits);
  Cluster(failures);
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

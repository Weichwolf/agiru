#include "Ast.h"
#include "BodyWriter.h"
#include "Names.h"
#include "Parser.h"
#include "Scope.h"
#include "TableWriter.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
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

struct Output {
  std::filesystem::path directory;
  std::string relative;
};

void Write(const Output &where, const std::string &text) {
  const std::filesystem::path path = where.directory / where.relative;
  std::filesystem::create_directories(path.parent_path());
  std::ofstream file(path, std::ios::binary);
  file << text;
}

struct Job {
  std::filesystem::path source;
  std::filesystem::path output;
};

int Scan(const Job &job) {
  const std::filesystem::path &root = job.source;
  const std::filesystem::path &out = job.output;
  // The generator OWNS its output directory. An aborted run must not leave half a tree behind for
  // the next build to read as if it were whole -- the predecessor records exactly that failure, a
  // clean step that wiped the tree and an abort that left a partial one the loader took happily.
  if (!out.empty()) {
    std::filesystem::remove_all(out);
    std::filesystem::create_directories(out);
    std::ofstream reaches(out / "reaches", std::ios::binary);
    reaches << "# GENERATED. Never by hand.\n#\n"
            << "# An app sees ONLY the door under include/agiru/ and the apps it declares a "
               "dependency on in\n"
            << "# apps.json. It does not see the runtime's internals: with `rt` here, every change "
               "to an\n"
            << "# internal runtime header would throw away every generated translation unit in "
               "every app.\n";
  }

  std::vector<Failure> failures;
  std::size_t written = 0;

  Counts tables;
  for (const std::filesystem::path &path : SourcesEndingIn(root, ".Table.al")) {
    ++tables.files;
    try {
      const agiru::al::TableObject table = agiru::al::ParseTable(Read(path));
      ++tables.parsed;
      tables.members += table.fields.size();
      if (!out.empty()) {
        const std::string relative = std::filesystem::relative(path, root).string();
        const std::string directory =
            agiru::gen::OutputDirectory(table.nameSpace, agiru::gen::ObjectKind::Table);
        const std::string name = agiru::gen::Identifier(table.name);
        std::string stem = directory;
        stem += "/";
        stem += name;
        Write(Output{.directory = out, .relative = stem + ".h"},
              agiru::gen::WriteHeader(table, relative));
        Write(Output{.directory = out, .relative = stem + ".cpp"},
              agiru::gen::WriteSource(table, relative));
        ++written;
      }
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

  if (!out.empty()) { std::println("written   {} table objects into {}", written, out.string()); }
  Report("tables", tables);
  Report("codeunits", codeunits);
  Cluster(failures);
  return 0;
}

} // namespace

int main(int argc, char **argv) {
  const std::span<char *> arguments(argv, static_cast<std::size_t>(argc));
  if (arguments.size() < 2) {
    std::fputs("agirutc <al-source-root> [<output-directory>]\n", stderr);
    return 2;
  }
  try {
    return Scan(Job{.source = std::filesystem::path(arguments[1]),
                    .output = arguments.size() > 2 ? std::filesystem::path(arguments[2])
                                                   : std::filesystem::path{}});
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

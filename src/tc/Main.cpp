#include "Ast.h"
#include "BodyWriter.h"
#include "EnumWriter.h"
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
#include <utility>
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
  std::println("{:<10}{} of {} parsed ({} {})",
               what,
               counts.parsed,
               counts.files,
               counts.members,
               what == "enums" ? "values" : "fields");
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

/// What every pass shares: where the source is, where the output goes, and what went wrong.
struct Run {
  std::filesystem::path root;
  std::filesystem::path output;
  std::vector<Failure> failures;
  std::size_t written = 0;
};

/// Records one parse failure, or reports the file as carrying no object of this kind at all.
/// \return True when the file held no such object and must not count towards the population.
bool Note(Run &run, const std::filesystem::path &path, const std::exception &e) {
  if (Declares(e.what())) { return true; }
  run.failures.push_back(Failure{.reason = Normalised(e.what()),
                                 .path = std::filesystem::relative(path, run.root).string(),
                                 .detail = e.what()});
  return false;
}

// THE ENUMS COME FIRST AND THAT IS NOT AN ORDERING PREFERENCE. A table field spelled
// `Enum "Item Type"` names an object declared in another file, so neither the type it translates to
// nor the header that declares it is knowable from the table alone. The index is built over the
// whole source before the first table is written.
agiru::gen::EnumIndex ScanEnums(Run &run, Counts &counts) {
  std::vector<agiru::al::EnumObject> objects;
  std::vector<std::string> paths;
  for (const std::filesystem::path &path : SourcesEndingIn(run.root, ".Enum.al")) {
    ++counts.files;
    try {
      agiru::al::EnumObject object = agiru::al::ParseEnum(Read(path));
      ++counts.parsed;
      counts.members += object.values.size();
      paths.push_back(std::filesystem::relative(path, run.root).string());
      objects.push_back(std::move(object));
    } catch (const std::exception &e) {
      if (Note(run, path, e)) { --counts.files; }
    }
  }

  agiru::gen::EnumIndex index;
  for (const agiru::al::EnumObject &object : objects) {
    index.insert_or_assign(agiru::gen::LowerKey(object.name),
                           agiru::gen::EnumRef{.identifier = agiru::gen::Identifier(object.name),
                                               .header = agiru::gen::EnumHeaderPath(object)});
  }
  if (run.output.empty()) { return index; }
  for (std::size_t i = 0; i < objects.size(); ++i) {
    Write(Output{.directory = run.output, .relative = agiru::gen::EnumHeaderPath(objects[i])},
          agiru::gen::WriteEnum(objects[i], paths[i]));
    ++run.written;
  }
  return index;
}

void WriteTable(Run &run,
                const agiru::al::TableObject &table,
                const std::string &relative,
                const agiru::gen::EnumIndex &index,
                std::map<std::string, std::size_t> &unresolved) {
  const std::string stem =
      agiru::gen::OutputDirectory(table.nameSpace, agiru::gen::ObjectKind::Table) + "/" +
      agiru::gen::Identifier(table.name);
  const agiru::gen::TableHeader header = agiru::gen::WriteHeader(table, relative, index);
  for (const std::string &missing : header.unresolvedEnums) { ++unresolved[missing]; }
  Write(Output{.directory = run.output, .relative = stem + ".h"}, header.text);
  Write(Output{.directory = run.output, .relative = stem + ".cpp"},
        agiru::gen::WriteSource(table, relative));
  ++run.written;
}

void ScanTables(Run &run,
                Counts &counts,
                const agiru::gen::EnumIndex &index,
                std::map<std::string, std::size_t> &unresolved) {
  for (const std::filesystem::path &path : SourcesEndingIn(run.root, ".Table.al")) {
    ++counts.files;
    try {
      const agiru::al::TableObject table = agiru::al::ParseTable(Read(path));
      ++counts.parsed;
      counts.members += table.fields.size();
      if (!run.output.empty()) {
        WriteTable(
            run, table, std::filesystem::relative(path, run.root).string(), index, unresolved);
      }
    } catch (const std::exception &e) {
      if (Note(run, path, e)) { --counts.files; }
    }
  }
}

void ScanCodeunits(Run &run, Counts &counts) {
  for (const std::filesystem::path &path : SourcesEndingIn(run.root, ".Codeunit.al")) {
    ++counts.files;
    try {
      const agiru::al::CodeunitObject unit = agiru::al::ParseCodeunit(Read(path));
      ++counts.parsed;
      counts.members += unit.procedures.size();
      std::size_t tests = 0;
      for (const agiru::al::ProcedureDecl &procedure : unit.procedures) {
        if (agiru::al::HasAttribute(procedure, "Test")) { ++tests; }
      }
      counts.tests += tests;
      if (tests != 0 && IsUnitTest(unit.name)) {
        ++counts.unitFiles;
        counts.unitTests += tests;
      }
    } catch (const std::exception &e) {
      if (Note(run, path, e)) { --counts.files; }
    }
  }
}

// AN UNRESOLVED ENUM IS REPORTED, NOT SWALLOWED. It is not a defect in the table that names it: the
// declaration lives in a layer this run was not given, and the count is what says how much the next
// layer is worth.
void ReportUnresolved(const std::map<std::string, std::size_t> &unresolved) {
  if (unresolved.empty()) { return; }
  std::size_t fields = 0;
  for (const auto &[name, count] : unresolved) { fields += count; }
  std::println("unknown   {} enum(s) named by {} field(s) are declared outside this source root",
               unresolved.size(),
               fields);
}

// The generator OWNS its output directory. An aborted run must not leave half a tree behind for the
// next build to read as if it were whole -- the predecessor records exactly that failure, a clean
// step that wiped the tree and an abort that left a partial one the loader took happily.
void ClaimOutput(const std::filesystem::path &out) {
  if (out.empty()) { return; }
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

int Scan(const Job &job) {
  Run run{.root = job.source, .output = job.output, .failures = {}, .written = 0};
  ClaimOutput(run.output);

  std::map<std::string, std::size_t> unresolved;
  Counts enums;
  Counts tables;
  Counts codeunits;
  const agiru::gen::EnumIndex index = ScanEnums(run, enums);
  ScanTables(run, tables, index, unresolved);
  ScanCodeunits(run, codeunits);

  if (!run.output.empty()) {
    std::println("written   {} objects into {}", run.written, run.output.string());
  }
  Report("enums", enums);
  Report("tables", tables);
  Report("codeunits", codeunits);
  ReportUnresolved(unresolved);
  Cluster(run.failures);
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

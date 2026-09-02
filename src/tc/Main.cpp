#include "Apps.h"
#include "Ast.h"
#include "BodyWriter.h"
#include "CodeunitWriter.h"
#include "EnumWriter.h"
#include "Names.h"
#include "Parser.h"
#include "Scope.h"
#include "TableWriter.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <map>
#include <memory>
#include <print>
#include <ranges>
#include <set>
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
  std::size_t unitFiles = 0;  ///< UT codeunits in the POPULATION, counted from the text.
  std::size_t unitTests = 0;  ///< Their [Test] methods, likewise.
  std::size_t unitParsed = 0; ///< How many of those methods came through the parser.
  std::size_t emitted = 0;    ///< Objects the generator could WRITE, which is the narrower count.
  std::size_t unitLost = 0;   ///< UT codeunits the parser could not read at all.
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
                   counts.unitFiles + counts.unitLost,
                   counts.unitTests);
      std::println("{:<10}{} of them reach the parser; {} codeunit(s) do not parse at all",
                   "",
                   counts.unitParsed,
                   counts.unitLost);
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
    // THE SUFFIX IS MATCHED WITHOUT CASE, because BCApps does not hold to its own convention:
    // 34 enums are `.enum.al`, 156 tables `.table.al`, 127 codeunits `.codeunit.al`. Three of the
    // five enums this run could not resolve were sitting in a scanned root under a lower-case
    // extension -- not missing, just not looked at.
    const std::string path = entry.path().string();
    if (entry.is_regular_file() && path.size() >= suffix.size() &&
        std::ranges::equal(path.substr(path.size() - suffix.size()), suffix, [](char a, char b) {
          return std::tolower(static_cast<unsigned char>(a)) ==
                 std::tolower(static_cast<unsigned char>(b));
        })) {
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

// A FILE THAT DID NOT CHANGE IS NOT WRITTEN, and that is what makes verifying a change affordable.
// A generator change usually rewrites a few hundred of the 6 398 objects; rewriting all of them
// gives every one a new timestamp, and the build then recompiles the whole ERP to find out that
// 6 000 of them are byte for byte what they were. Compared before writing, the rest keep their
// timestamps and the build skips them.
bool WriteFile(const Output &where, const std::string &text) {
  const std::filesystem::path path = where.directory / where.relative;
  if (std::filesystem::exists(path)) {
    const std::ifstream existing(path, std::ios::binary);
    std::ostringstream held;
    held << existing.rdbuf();
    if (held.str() == text) { return false; }
  }
  std::filesystem::create_directories(path.parent_path());
  std::ofstream file(path, std::ios::binary);
  file << text;
  return true;
}

struct Job {
  std::filesystem::path source;
  std::filesystem::path output;
  std::filesystem::path apps;
};

/// What every pass shares: where the source is, where the output goes, and what went wrong.
struct Run {
  std::filesystem::path root;
  std::filesystem::path output;
  std::vector<Failure> failures;        ///< What could not be READ.
  std::vector<Failure> refusals;        ///< What could be read and not WRITTEN.
  std::size_t written = 0;              ///< Objects emitted.
  std::size_t changed = 0;              ///< Files whose bytes actually differ from what was there.
  std::set<std::filesystem::path> kept; ///< Every path this run stands behind.
};

void Keep(Run &run, const Output &where, const std::string &text) {
  if (WriteFile(where, text)) { ++run.changed; }
  run.kept.insert(where.directory / where.relative);
}

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
// AN INTERFACE IS READ BEFORE THE CODEUNITS THAT IMPLEMENT IT, for the reason the enums are: a
// codeunit's variable of interface type resolves through this index, and an index filled after the
// fact resolves nothing (board:0027).
/// What a run gathers beside its objects: the members of every type it does not have.
struct Gathered {
  agiru::gen::DotNetUse dotnet; ///< Per .NET type, the members the corpus calls.
  agiru::gen::DotNetUse absent; ///< Per AL object no source root declares, the same.
};

void Absorb(agiru::gen::DotNetUse &into, const agiru::gen::DotNetUse &from) {
  for (const auto &[type, members] : from) { into[type].insert(members.begin(), members.end()); }
}

void ScanInterfaces(Run &run, Counts &counts, Gathered &gathered, agiru::gen::Objects &objects) {
  for (const std::filesystem::path &path : SourcesEndingIn(run.root, ".Interface.al")) {
    ++counts.files;
    try {
      const agiru::al::InterfaceObject object = agiru::al::ParseInterface(Read(path));
      ++counts.parsed;
      counts.members += object.procedures.size();
      const std::string identifier = agiru::gen::Identifier(object.name);
      const std::string header =
          agiru::gen::OutputDirectory(object.nameSpace, agiru::gen::ObjectKind::Interface) + "/" +
          identifier + ".h";
      objects.interfaces.insert_or_assign(
          agiru::gen::LowerKey(object.name),
          agiru::gen::TableRef{.identifier = "interfaces::" + identifier, .header = header});
      if (run.output.empty()) { continue; }
      const agiru::gen::InterfaceHeader written = agiru::gen::WriteInterface(
          object, std::filesystem::relative(path, run.root).string(), objects);
      Absorb(gathered.absent, written.absent);
      Keep(run, Output{.directory = run.output, .relative = header}, written.text);
      ++run.written;
    } catch (const std::exception &e) {
      if (Note(run, path, e)) { --counts.files; }
    }
  }
}

void ScanEnums(Run &run, Counts &counts, agiru::gen::EnumIndex &index) {
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

  for (const agiru::al::EnumObject &object : objects) {
    index.insert_or_assign(agiru::gen::LowerKey(object.name),
                           agiru::gen::EnumRef{.identifier = agiru::gen::Identifier(object.name),
                                               .header = agiru::gen::EnumHeaderPath(object)});
  }
  if (run.output.empty()) { return; }
  for (std::size_t i = 0; i < objects.size(); ++i) {
    Keep(run,
         Output{.directory = run.output, .relative = agiru::gen::EnumHeaderPath(objects[i])},
         agiru::gen::WriteEnum(objects[i], paths[i]));
    ++run.written;
  }
}

std::string TableHeaderPath(const agiru::al::TableObject &table) {
  return agiru::gen::OutputDirectory(table.nameSpace, agiru::gen::ObjectKind::Table) + "/" +
         agiru::gen::Identifier(table.name) + ".h";
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
  Keep(run, Output{.directory = run.output, .relative = stem + ".h"}, header.text);
  Keep(run,
       Output{.directory = run.output, .relative = stem + ".cpp"},
       agiru::gen::WriteSource(table, relative));
  ++run.written;
}

void ScanTables(Run &run,
                Counts &counts,
                const agiru::gen::EnumIndex &index,
                agiru::gen::Objects &objects,
                std::map<std::string, std::size_t> &unresolvedEnums) {
  for (const std::filesystem::path &path : SourcesEndingIn(run.root, ".Table.al")) {
    ++counts.files;
    try {
      const agiru::al::TableObject table = agiru::al::ParseTable(Read(path));
      ++counts.parsed;
      counts.members += table.fields.size();
      // BY NAME AND BY NUMBER BOTH, because AL names an object either way and test code uses the
      // number freely: `var GLEntry: Record 17`.
      // THE KIND IS PART OF THE NAME. 51 objects in the read roots are a table AND a codeunit at
      // once -- `Language`, `Default Dimension`, `Currency` -- because AL tells them apart by the
      // keyword and C++ has no keyword to tell them apart by. `enums::` already did this; the other
      // two kinds follow it rather than inventing a second answer.
      const agiru::gen::TableRef ref{.identifier = "tables::" + agiru::gen::Identifier(table.name),
                                     .header = TableHeaderPath(table)};
      objects.tables.insert_or_assign(agiru::gen::LowerKey(table.name), ref);
      objects.tables.insert_or_assign(std::to_string(table.id), ref);
      if (!run.output.empty()) {
        WriteTable(
            run, table, std::filesystem::relative(path, run.root).string(), index, unresolvedEnums);
      }
    } catch (const std::exception &e) {
      if (Note(run, path, e)) { --counts.files; }
    }
  }
}

// THE UT POPULATION IS COUNTED FROM THE TEXT AND NOT FROM THE PARSE, and that is the whole point.
// A codeunit that fails to parse contributes nothing to `unitTests` through the parser, so the
// milestone's DENOMINATOR would shrink every time the parser lost a file -- the baseline that falls
// by accident, which CLAUDE.md lists as a trap and which cost exactly 3 codeunits and 67 methods
// here before it was found. The name and the [Test] count are both recoverable lexically, so they
// are recovered lexically and the denominator is the population.
struct UnitTestPopulation {
  std::size_t files = 0;
  std::size_t tests = 0;
};

UnitTestPopulation UnitTestsIn(const std::string &source) {
  const std::size_t at = source.find("codeunit ");
  if (at == std::string::npos) { return {}; }
  const std::size_t eol = source.find('\n', at);
  std::string header = source.substr(at, eol == std::string::npos ? eol : eol - at);
  while (!header.empty() && (header.back() == '\r' || header.back() == '"' ||
                             std::isspace(static_cast<unsigned char>(header.back())) != 0)) {
    header.pop_back();
  }
  if (!IsUnitTest(header)) { return {}; }
  UnitTestPopulation found;
  found.files = 1;
  for (std::size_t cursor = source.find("[Test]"); cursor != std::string::npos;
       cursor = source.find("[Test]", cursor + 1)) {
    ++found.tests;
  }
  return found.tests != 0 ? found : UnitTestPopulation{};
}

// THE NAMES ARE INDEXED BEFORE ANYTHING IS EMITTED, and lexically rather than by parsing. A
// codeunit may hold a variable of a codeunit declared later in the same app, so a single pass in
// file order would not have it -- and parsing 3 914 objects twice to find that out costs minutes
// for two lines of header. The object's kind, name and namespace are all on its declaration line.
void IndexCodeunits(const Run &run, agiru::gen::Objects &objects) {
  for (const std::filesystem::path &path : SourcesEndingIn(run.root, ".Codeunit.al")) {
    const agiru::gen::ObjectDeclaration declared =
        agiru::gen::DeclarationOf(Read(path), agiru::gen::ObjectKind::Codeunit);
    if (!declared.found) { continue; }
    const std::string &name = declared.name;
    const std::string &nameSpace = declared.nameSpace;

    const std::string identifier = agiru::gen::Identifier(name);
    objects.codeunits.insert_or_assign(
        agiru::gen::LowerKey(name),
        agiru::gen::TableRef{
            .identifier = "codeunits::" + identifier,
            .header = agiru::gen::OutputDirectory(nameSpace, agiru::gen::ObjectKind::Codeunit) +
                      "/" + identifier + ".h"});
  }
}

void ScanCodeunits(Run &run,
                   Counts &counts,
                   Gathered &gathered,
                   agiru::gen::Objects &objects,
                   std::map<std::string, std::size_t> &unresolvedTables) {
  for (const std::filesystem::path &path : SourcesEndingIn(run.root, ".Codeunit.al")) {
    ++counts.files;
    const std::string source = Read(path);
    const UnitTestPopulation population = UnitTestsIn(source);
    counts.unitFiles += population.files;
    counts.unitTests += population.tests;
    // PARSING AND EMITTING ARE COUNTED APART, and mixing them cost a whole run's numbers: an
    // emitter that refused a body threw out of the same try, so the codeunit went down as a PARSE
    // failure and its [Test] methods were never counted. 41 568 became 4 624 in one step. What an
    // object can be READ is one question and what it can be WRITTEN is another, and the second is
    // where the runtime's gaps show up.
    std::unique_ptr<agiru::al::CodeunitObject> unit;
    try {
      unit = std::make_unique<agiru::al::CodeunitObject>(agiru::al::ParseCodeunit(source));
    } catch (const std::exception &e) {
      if (Note(run, path, e)) {
        --counts.files;
        counts.unitFiles -= population.files;
        counts.unitTests -= population.tests;
        if (population.files != 0) { ++counts.unitLost; }
      }
      continue;
    }
    ++counts.parsed;
    counts.members += unit->procedures.size();
    std::size_t tests = 0;
    for (const agiru::al::ProcedureDecl &procedure : unit->procedures) {
      if (agiru::al::HasAttribute(procedure, "Test")) { ++tests; }
    }
    counts.tests += tests;
    if (population.files != 0) { counts.unitParsed += tests; }
    if (run.output.empty()) { continue; }
    try {
      const std::string relative = std::filesystem::relative(path, run.root).string();
      const agiru::gen::CodeunitHeader header = agiru::gen::WriteCodeunit(*unit, relative, objects);
      for (const std::string &missing : header.unresolvedTables) { ++unresolvedTables[missing]; }
      Absorb(gathered.dotnet, header.dotnet);
      Absorb(gathered.absent, header.absent);
      const std::string stem = agiru::gen::CodeunitHeaderPath(*unit);
      const std::string body = agiru::gen::WriteCodeunitSource(*unit, relative, objects);
      Keep(run, Output{.directory = run.output, .relative = stem}, header.text);
      Keep(run,
           Output{.directory = run.output, .relative = stem.substr(0, stem.size() - 1) + "cpp"},
           body);
      ++counts.emitted;
      ++run.written;
    } catch (const std::exception &e) {
      run.refusals.push_back(Failure{.reason = Normalised(e.what()),
                                     .path = std::filesystem::relative(path, run.root).string(),
                                     .detail = e.what()});
    }
  }
}

// AN UNRESOLVED ENUM IS REPORTED, NOT SWALLOWED. It is not a defect in the table that names it: the
// declaration lives in a layer this run was not given, and the count is what says how much the next
// layer is worth.
constexpr std::size_t kUnresolvedShown = 10;

// THE .NET TYPES THIS TREE HAS REBUILT BY HAND. They live behind the door with real behaviour, so
// the generated stubs must not declare them a second time. The list is the platform's own
// vocabulary in the same sense `TypeName()`'s list of AL types is (board:0035).
const std::set<std::string> &Rebuilt() {
  static const std::set<std::string> kRebuilt{"ALConfigSettings",
                                              "GenericDictionary2",
                                              "GenericList1",
                                              "NavTenantSettingsHelper",
                                              "UserInfo"};
  return kRebuilt;
}

// ONE FILE FOR EVERY .NET TYPE THE CORPUS NAMES, and its members are exactly the ones the corpus
// asks for. .NET's own API is thousands of members that nobody here needs; this is the set that is
// actually reached, which makes it a worklist with a denominator rather than a port.
struct Counted {
  std::size_t types = 0;
  std::size_t members = 0;
};

Counted Stubs(std::string &text, const agiru::gen::DotNetUse &use, bool skipRebuilt) {
  Counted counted;
  for (const auto &[type, named] : use) {
    if (skipRebuilt && Rebuilt().contains(type)) { continue; }
    ++counted.types;
    text += "\nstruct ";
    text += type;
    text += " {\n";
    for (const std::string &member : named) {
      ++counted.members;
      text += "  ::agiru::dotnet::Refused ";
      text += member;
      text += "{{.type = \"";
      text += type;
      text += "\", .member = \"";
      text += member;
      text += "\"}};\n";
    }
    text += "};\n";
  }
  return counted;
}

// ONE FILE FOR EVERY TYPE THE CORPUS NAMES AND THIS RUN DOES NOT HAVE, and its members are exactly
// the ones the corpus asks for. .NET's own API is thousands of members nobody here needs, and a
// platform table's field list is the platform's; this is the set actually reached, which makes it a
// worklist with a denominator rather than a port (board:0035).
void WriteAbsent(const std::filesystem::path &out,
                 const agiru::gen::DotNetUse &dotnet,
                 const agiru::gen::DotNetUse &absent) {
  if (out.empty()) { return; }
  std::string text = "// Generated from every AL body that names a type this run does not have.\n";
  text += "// Do not edit.\n\n#pragma once\n\n#include \"agiru.h\"\n";
  text += "\nnamespace agiru::dotnet {\n";
  const Counted net = Stubs(text, dotnet, true);
  // A NAMESPACE OF THEIR OWN, because an absent AL object may carry a name the AL TYPE system
  // already uses: the virtual table `Integer` is one, and declaring `agiru::app::Integer` shadowed
  // `agiru::Integer` for every generated table that has an Integer field. `absent::` also says at
  // the use site what the type is -- something this run does not have.
  text += "\n} // namespace agiru::dotnet\n\nnamespace agiru::app::absent {\n";
  const Counted objects = Stubs(text, absent, false);
  text += "\n} // namespace agiru::app::absent\n";

  const std::filesystem::path path = out / "absent" / "absent" / "Types.h";
  std::filesystem::create_directories(path.parent_path());
  std::ofstream file(path, std::ios::binary);
  file << text;
  std::println("absent    {} .NET type(s) with {} member(s), {} AL object(s) with {}",
               net.types,
               net.members,
               objects.types,
               objects.members);
}

void ReportUnresolved(std::string_view what,
                      std::string_view by,
                      const std::map<std::string, std::size_t> &unresolved) {
  if (unresolved.empty()) { return; }
  std::size_t uses = 0;
  for (const auto &[name, count] : unresolved) { uses += count; }
  std::println("unknown   {} {} named by {} {} are declared outside this source root",
               unresolved.size(),
               what,
               uses,
               by);
  // AND WHICH ONES, because a bare count cannot be ranked and a list that cannot be ranked cannot
  // decide the next piece of work. The ones named most are the platform's own objects -- the
  // virtual `Field` table and its neighbours -- and they are what the count is really about.
  std::vector<std::pair<std::string, std::size_t>> ranked(unresolved.begin(), unresolved.end());
  std::ranges::sort(ranked, [](const auto &a, const auto &b) { return a.second > b.second; });
  const std::size_t shown = std::min<std::size_t>(ranked.size(), kUnresolvedShown);
  for (std::size_t i = 0; i < shown; ++i) {
    std::println("          {:>5} x {}", ranked[i].second, ranked[i].first);
  }
}

// THE GENERATOR STILL OWNS ITS OUTPUT DIRECTORY, and it owns it by SWEEPING rather than by wiping.
// An aborted run must not leave half a tree behind for the next build to read as if it were whole
// -- the predecessor records exactly that failure. Wiping up front achieved that and cost every
// unchanged file its timestamp; sweeping at the end achieves the same and costs nothing, because a
// run that aborts sweeps nothing and leaves the previous tree intact rather than a fragment.
void ClaimOutput(const std::filesystem::path &out) {
  if (out.empty()) { return; }
  std::filesystem::create_directories(out);
}

/// Removes every generated file the run did not write or keep.
std::size_t Sweep(const std::filesystem::path &out, const std::set<std::filesystem::path> &kept) {
  if (out.empty() || !std::filesystem::is_directory(out)) { return 0; }
  std::vector<std::filesystem::path> stale;
  for (const auto &entry : std::filesystem::recursive_directory_iterator(out)) {
    if (!entry.is_regular_file()) { continue; }
    if (entry.path().filename() == "reaches") { continue; }
    if (!kept.contains(entry.path())) { stale.push_back(entry.path()); }
  }
  for (const std::filesystem::path &path : stale) { std::filesystem::remove(path); }
  return stale.size();
}

void ClaimApp(const std::filesystem::path &out) {
  if (out.empty()) { return; }
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

void Add(Counts &into, const Counts &one) {
  into.files += one.files;
  into.parsed += one.parsed;
  into.members += one.members;
  into.tests += one.tests;
  into.unitFiles += one.unitFiles;
  into.unitTests += one.unitTests;
  into.unitParsed += one.unitParsed;
  into.unitLost += one.unitLost;
  into.emitted += one.emitted;
}

// ONE ENUM INDEX ACROSS THE APPS, FILLED IN DECLARATION ORDER, AND THAT ORDER IS THE ENFORCEMENT.
// apps.json lists the apps in dependency order, so a table resolves an enum against its own app and
// everything read before it -- never against an app that comes later. The direction AL declares
// therefore holds because of the shape of the loop, not because of a second check that could drift.
int Scan(const Job &job) {
  const std::vector<agiru::gen::App> apps = agiru::gen::ReadApps(job.apps);
  ClaimOutput(job.output);

  std::map<std::string, std::size_t> unresolvedEnums;
  std::map<std::string, std::size_t> unresolvedTables;
  Gathered gathered;
  agiru::gen::EnumIndex index;
  agiru::gen::Objects objects;
  Counts allEnums;
  Counts allTables;
  Counts allCodeunits;
  std::vector<Failure> failures;
  std::vector<Failure> refusals;
  std::size_t written = 0;
  std::size_t changed = 0;
  std::set<std::filesystem::path> kept;

  // THE COLUMN IS AS WIDE AS THE WIDEST APP NAME. A fixed width was 11, which fits `foundation`
  // and truncates nothing but runs `library_variable_storage` straight into its own count.
  std::size_t column = 0;
  for (const agiru::gen::App &app : apps) { column = std::max(column, app.name.size() + 1); }

  // THE PLATFORM'S OWN TABLES ARE THERE BEFORE THE FIRST APP IS READ, so `Record Field` resolves
  // where it lives. An app that declares a table of the same name overwrites the entry, which is
  // the precedence AL itself has.
  objects.tables = agiru::gen::PlatformTables();

  for (const agiru::gen::App &app : apps) {
    const std::filesystem::path source = job.source / app.source;
    if (!std::filesystem::is_directory(source)) {
      std::println("{:<10}{} -- no such tree, skipped", app.name, source.string());
      continue;
    }
    Run run{.root = source,
            .output = job.output.empty() ? std::filesystem::path{} : job.output / app.name,
            .failures = {},
            .refusals = {},
            .written = 0,
            .changed = 0,
            .kept = {}};
    Counts enums;
    Counts interfaces;
    Counts tables;
    Counts codeunits;
    ClaimApp(run.output);
    IndexCodeunits(run, objects);
    ScanEnums(run, enums, index);
    // AFTER the enums are read, not before: a codeunit variable of enum type resolves through the
    // same index a table field does, and the index is filled by the pass above.
    objects.enums = index;
    ScanTables(run, tables, index, objects, unresolvedEnums);
    // AFTER the tables and BEFORE the codeunits: an interface's SIGNATURE names records and enums,
    // and a codeunit's variable names an interface. Read in any other order, one of the two
    // resolves nothing.
    ScanInterfaces(run, interfaces, gathered, objects);
    ScanCodeunits(run, codeunits, gathered, objects, unresolvedTables);

    std::println("{:<{}}{} table(s), {} codeunit(s), {} enum(s), {} [Test] method(s){}",
                 app.name,
                 column,
                 tables.parsed,
                 codeunits.parsed,
                 enums.parsed,
                 codeunits.tests,
                 run.written != 0 ? std::format(" -- {} written", run.written) : std::string{});
    Add(allEnums, enums);
    Add(allTables, tables);
    Add(allCodeunits, codeunits);
    written += run.written;
    changed += run.changed;
    kept.merge(run.kept);
    failures.insert(failures.end(), run.failures.begin(), run.failures.end());
    refusals.insert(refusals.end(), run.refusals.begin(), run.refusals.end());
  }

  if (!job.output.empty()) {
    const std::size_t swept = Sweep(job.output, kept);
    std::println("written   {} objects into {}; {} changed, {} swept",
                 written,
                 job.output.string(),
                 changed,
                 swept);
  }
  Report("enums", allEnums);
  Report("tables", allTables);
  Report("codeunits", allCodeunits);
  if (allCodeunits.emitted != 0) {
    std::println("emitted   {} of {} codeunits; the rest name what the runtime cannot do yet",
                 allCodeunits.emitted,
                 allCodeunits.parsed);
  }
  ReportUnresolved("enum(s)", "field(s)", unresolvedEnums);
  WriteAbsent(job.output, gathered.dotnet, gathered.absent);
  ReportUnresolved("table(s)", "declaration(s)", unresolvedTables);
  Cluster(failures);
  if (!refusals.empty()) {
    std::println("");
    std::println("refused   what parses and cannot be written yet");
    Cluster(refusals);
  }
  return 0;
}

} // namespace

int main(int argc, char **argv) {
  const std::span<char *> arguments(argv, static_cast<std::size_t>(argc));
  if (arguments.size() < 3) {
    std::fputs("agirutc <bcapps-src-root> <apps.json> [<output-root>]\n", stderr);
    return 2;
  }
  try {
    return Scan(Job{.source = std::filesystem::path(arguments[1]),
                    .output = arguments.size() > 3 ? std::filesystem::path(arguments[3])
                                                   : std::filesystem::path{},
                    .apps = std::filesystem::path(arguments[2])});
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

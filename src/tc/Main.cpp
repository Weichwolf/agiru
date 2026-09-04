#include "meta/Declare.h"

#include "Apps.h"
#include "Ast.h"
#include "BodyWriter.h"
#include "CodeunitWriter.h"
#include "EnumWriter.h"
#include "Names.h"
#include "PageWriter.h"
#include "Parser.h"
#include "Scope.h"
#include "TableWriter.h"

#include <algorithm>
#include <array>
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
  std::size_t unitFiles = 0;
  std::size_t unitTests = 0;
  std::size_t unitParsed = 0;
  std::size_t emitted = 0;
  std::size_t unitLost = 0;
};

bool IsUnitTest(std::string_view name) {
  if (name.size() < 3) { return false; }
  const std::string_view tail = name.substr(name.size() - 2);
  const bool suffix = (tail[0] == 'U' || tail[0] == 'u') && (tail[1] == 'T' || tail[1] == 't');
  const char before = name[name.size() - 3];
  return suffix && (before == ' ' || before == '-' || before == '.');
}

void Report(std::string_view what, const Counts &counts) {
  if (what == "codeunits") {
    std::println("{:<10} {} of {} parsed ({} procedures, {} [Test] methods)",
                 what,
                 counts.parsed,
                 counts.files,
                 counts.members,
                 counts.tests);
    if (counts.unitTests != 0) {
      std::println("{:<10} {} codeunits, {} [Test] methods -- the milestone's population",
                   "UT",
                   counts.unitFiles + counts.unitLost,
                   counts.unitTests);
      std::println("{:<10} {} of them reach the parser; {} codeunit(s) do not parse at all",
                   "",
                   counts.unitParsed,
                   counts.unitLost);
    }
    return;
  }
  std::println("{:<10} {} of {} parsed ({} {})",
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

struct Run {
  const agiru::gen::TranspileScope *scope = nullptr;
  std::filesystem::path root;
  std::filesystem::path output;
  std::vector<Failure> failures;
  std::vector<Failure> refusals;
  std::size_t written = 0;
  std::size_t changed = 0;
  std::set<std::filesystem::path> kept;
};

std::string DeclaredNamespace(const std::filesystem::path &path) {
  std::ifstream file(path);
  if (!file) { return {}; }
  std::string line;
  while (std::getline(file, line)) {
    const std::size_t at = line.find("namespace ");
    if (at == std::string::npos) { continue; }
    if (line.find_first_not_of(" \t") != at) { continue; }
    std::string name = line.substr(at + std::string_view("namespace ").size());
    const std::size_t end = name.find(';');
    if (end != std::string::npos) { name.resize(end); }
    while (!name.empty() && (std::isspace(static_cast<unsigned char>(name.back())) != 0)) {
      name.pop_back();
    }
    return name;
  }
  return {};
}

bool InScope(const Run &run, const std::filesystem::path &path) {
  if (run.scope == nullptr) { return true; }
  const std::string nameSpace = DeclaredNamespace(path);
  if (!nameSpace.empty()) { return agiru::gen::Holds(*run.scope, nameSpace); }
  const std::filesystem::path relative = path.lexically_relative(run.root);
  if (relative.empty() || relative.begin() == relative.end()) { return true; }
  return agiru::gen::HoldsArea(*run.scope, relative.begin()->string());
}

std::vector<std::filesystem::path> SourcesEndingIn(const Run &run, std::string_view suffix) {
  const std::filesystem::path &root = run.root;
  std::vector<std::filesystem::path> sources;
  for (const auto &entry : std::filesystem::recursive_directory_iterator(root)) {
    const std::string path = entry.path().string();
    if (entry.is_regular_file() && path.size() >= suffix.size() &&
        std::ranges::equal(path.substr(path.size() - suffix.size()), suffix, [](char a, char b) {
          return std::tolower(static_cast<unsigned char>(a)) ==
                 std::tolower(static_cast<unsigned char>(b));
        })) {
      if (InScope(run, entry.path())) { sources.push_back(entry.path()); }
    }
  }
  std::ranges::sort(sources);
  return sources;
}

std::map<std::string, std::size_t> UntranslatedKinds(const Run &run) {
  static constexpr std::array kWithoutAGenerator{
      std::string_view{"report"},
      std::string_view{"query"},
      std::string_view{"xmlport"},
      std::string_view{"permissionset"},
      std::string_view{"permissionsetext"},
      std::string_view{"profile"},
      std::string_view{"controladdin"},
      std::string_view{"entitlement"},
      std::string_view{"pagecustomization"},
      std::string_view{"reportext"},
      std::string_view{"dotnet"},
  };
  std::map<std::string, std::size_t> counted;
  for (const std::string_view kind : kWithoutAGenerator) {
    const std::string suffix = "." + std::string(kind) + ".al";
    const std::size_t found = SourcesEndingIn(run, suffix).size();
    if (found != 0) { counted[std::string(kind)] += found; }
  }
  return counted;
}

struct Output {
  std::filesystem::path directory;
  std::string relative;
};

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

void Keep(Run &run, const Output &where, const std::string &text) {
  if (WriteFile(where, text)) { ++run.changed; }
  run.kept.insert(where.directory / where.relative);
}

bool Note(Run &run, const std::filesystem::path &path, const std::exception &e) {
  if (Declares(e.what())) { return true; }
  run.failures.push_back(Failure{.reason = Normalised(e.what()),
                                 .path = std::filesystem::relative(path, run.root).string(),
                                 .detail = e.what()});
  return false;
}

struct Gathered {
  agiru::gen::DotNetUse dotnet;
  agiru::gen::DotNetUse absent;
};

void Absorb(agiru::gen::DotNetUse &into, const agiru::gen::DotNetUse &from) {
  for (const auto &[type, members] : from) { into[type].insert(members.begin(), members.end()); }
}

struct Interfaces {
  std::vector<agiru::al::InterfaceObject> objects;
  std::vector<std::string> paths;
};

Interfaces IndexInterfaces(Run &run, Counts &counts, agiru::gen::Objects &objects) {
  Interfaces kept;
  for (const std::filesystem::path &path : SourcesEndingIn(run, ".Interface.al")) {
    ++counts.files;
    try {
      agiru::al::InterfaceObject object = agiru::al::ParseInterface(Read(path));
      ++counts.parsed;
      counts.members += object.procedures.size();
      const std::string identifier = agiru::gen::Identifier(object.name);
      objects.interfaces.insert_or_assign(
          agiru::gen::LowerKey(object.name),
          agiru::gen::TableRef{.identifier = "interfaces::" + identifier,
                               .header = agiru::gen::OutputDirectory(
                                             object.nameSpace, agiru::gen::ObjectKind::Interface) +
                                         "/" + identifier + ".h",
                               .fields = {}});
      kept.paths.push_back(std::filesystem::relative(path, run.root).string());
      kept.objects.push_back(std::move(object));
    } catch (const std::exception &e) {
      if (Note(run, path, e)) { --counts.files; }
    }
  }
  return kept;
}

void WriteInterfaces(Run &run,
                     const Interfaces &kept,
                     Gathered &gathered,
                     const agiru::gen::Objects &objects) {
  if (run.output.empty()) { return; }
  for (std::size_t i = 0; i < kept.objects.size(); ++i) {
    const agiru::gen::InterfaceHeader written =
        agiru::gen::WriteInterface(kept.objects[i], kept.paths[i], objects);
    Absorb(gathered.dotnet, written.dotnet);
    Absorb(gathered.absent, written.absent);
    const std::string identifier = agiru::gen::Identifier(kept.objects[i].name);
    Keep(run,
         Output{.directory = run.output,
                .relative = agiru::gen::OutputDirectory(kept.objects[i].nameSpace,
                                                        agiru::gen::ObjectKind::Interface) +
                            "/" + identifier + ".h"},
         written.text);
    ++run.written;
  }
}

struct Extensions {
  std::map<std::string, std::vector<agiru::al::TableExtensionObject>> tables;
  std::map<std::string, std::vector<agiru::al::EnumExtensionObject>> enums;
  std::map<std::string, std::vector<agiru::al::PageExtensionObject>> pages;
  mutable std::map<std::string, std::size_t> held;
  mutable std::map<std::string, std::size_t> consumed;
};

std::string Overload(const agiru::al::ProcedureDecl &procedure) {
  std::string key = agiru::gen::LowerKey(procedure.name);
  for (const agiru::al::VarDecl &parameter : procedure.parameters) {
    key +=
        "|" + agiru::gen::LowerKey(parameter.type) + " " + agiru::gen::LowerKey(parameter.subtype);
  }
  return key;
}

void TakeFields(std::vector<agiru::al::FieldDecl> &into,
                const std::vector<agiru::al::FieldDecl> &from) {
  std::set<std::string> names;
  std::set<int> numbers;
  for (const agiru::al::FieldDecl &field : into) {
    names.insert(agiru::gen::LowerKey(field.name));
    numbers.insert(field.number);
  }
  for (const agiru::al::FieldDecl &field : from) {
    if (!names.insert(agiru::gen::LowerKey(field.name)).second) { continue; }
    if (!numbers.insert(field.number).second) { continue; }
    into.push_back(field);
  }
}

void TakeVariables(std::vector<agiru::al::VarDecl> &into,
                   const std::vector<agiru::al::VarDecl> &from) {
  std::set<std::string> declared;
  for (const agiru::al::VarDecl &variable : into) {
    declared.insert(agiru::gen::LowerKey(variable.name));
  }
  for (const agiru::al::VarDecl &variable : from) {
    if (declared.insert(agiru::gen::LowerKey(variable.name)).second) { into.push_back(variable); }
  }
}

void TakeProcedures(std::vector<agiru::al::ProcedureDecl> &into,
                    const std::vector<agiru::al::ProcedureDecl> &from) {
  std::set<std::string> declared;
  for (const agiru::al::ProcedureDecl &procedure : into) { declared.insert(Overload(procedure)); }
  for (const agiru::al::ProcedureDecl &procedure : from) {
    if (declared.insert(Overload(procedure)).second) { into.push_back(procedure); }
  }
}

void NoteTargets(const Extensions &store) {
  for (const auto &[name, list] : store.tables) { store.held["table " + name] += list.size(); }
  for (const auto &[name, list] : store.enums) { store.held["enum " + name] += list.size(); }
  for (const auto &[name, list] : store.pages) { store.held["page " + name] += list.size(); }
}

Extensions ReadExtensions(Run &run,
                          Counts &counts,
                          const std::vector<agiru::gen::App> &apps,
                          const std::filesystem::path &source) {
  Extensions store;
  for (const agiru::gen::App &app : apps) {
    run.root = source / app.source;
    if (!std::filesystem::is_directory(run.root)) { continue; }
    const auto read = [&](std::string_view suffix, auto parse, auto &into) {
      for (const std::filesystem::path &path : SourcesEndingIn(run, suffix)) {
        ++counts.files;
        try {
          auto extension = parse(Read(path));
          ++counts.parsed;
          into[agiru::gen::LowerKey(extension.extends)].push_back(std::move(extension));
        } catch (const std::exception &e) {
          if (Note(run, path, e)) { --counts.files; }
        }
      }
    };
    read(".TableExt.al", agiru::al::ParseTableExtension, store.tables);
    read(".EnumExt.al", agiru::al::ParseEnumExtension, store.enums);
    read(".PageExt.al", agiru::al::ParsePageExtension, store.pages);
  }
  return store;
}

struct Pages {
  std::vector<agiru::al::PageObject> objects;
  std::vector<std::string> paths;
};

Pages IndexPages(Run &run, Counts &counts, agiru::gen::Objects &objects) {
  Pages pages;
  for (const std::filesystem::path &path : SourcesEndingIn(run, ".Page.al")) {
    ++counts.files;
    try {
      agiru::al::PageObject object = agiru::al::ParsePage(Read(path));
      ++counts.parsed;
      counts.members += object.procedures.size();
      std::map<std::string, std::string> controlNames = agiru::gen::ControlIdentifiers(object);
      objects.pages.insert_or_assign(
          agiru::gen::LowerKey(object.name),
          agiru::gen::TableRef{.identifier = "pages::" + agiru::gen::Identifier(object.name),
                               .header = agiru::gen::PageHeaderPath(object),
                               .fields = std::move(controlNames)});
      pages.paths.push_back(std::filesystem::relative(path, run.root).string());
      pages.objects.push_back(std::move(object));
    } catch (const std::exception &e) {
      if (Note(run, path, e)) { --counts.files; }
    }
  }
  return pages;
}

std::size_t MergePageExtensions(const Extensions &store, Pages &pages) {
  std::size_t merged = 0;
  const auto take = [](auto &into, auto &from) {
    into.insert(into.end(), from.begin(), from.end());
  };
  for (agiru::al::PageObject &page : pages.objects) {
    const auto found = store.pages.find(agiru::gen::LowerKey(page.name));
    if (found == store.pages.end()) { continue; }
    for (const agiru::al::PageExtensionObject &extension : found->second) {
      take(page.layout, extension.layout);
      take(page.actions, extension.actions);
      TakeProcedures(page.procedures, extension.procedures);
      TakeVariables(page.variables, extension.variables);
      take(page.labels, extension.labels);
      ++merged;
      ++store.consumed["page " + found->first];
    }
  }
  return merged;
}

using TableByName = std::map<std::string, const agiru::al::TableObject *>;

const agiru::al::TableObject *SourceOf(const agiru::al::PageObject &page,
                                       const TableByName &tables) {
  const agiru::al::Property *source = agiru::al::Find(page.properties, "SourceTable");
  if (source == nullptr) { return nullptr; }
  const auto found = tables.find(agiru::gen::LowerKey(source->text));
  return found == tables.end() ? nullptr : found->second;
}

void WritePages(Run &run,
                const Pages &pages,
                const agiru::gen::Objects &objects,
                Gathered &gathered,
                const TableByName &tables) {
  if (run.output.empty()) { return; }
  for (std::size_t i = 0; i < pages.objects.size(); ++i) {
    const agiru::gen::PageHeader written =
        agiru::gen::WritePage(pages.objects[i], pages.paths[i], objects);
    Absorb(gathered.dotnet, written.dotnet);
    Absorb(gathered.absent, written.absent);
    const std::filesystem::path header = agiru::gen::PageHeaderPath(pages.objects[i]);
    Keep(run, Output{.directory = run.output, .relative = header}, written.text);
    ++run.written;
    std::filesystem::path body = header;
    body.replace_extension(".cpp");
    Keep(run,
         Output{.directory = run.output, .relative = body},
         agiru::gen::WriteSource(
             pages.objects[i], pages.paths[i], objects, SourceOf(pages.objects[i], tables)));
    ++run.written;
  }
}

struct Enums {
  std::vector<agiru::al::EnumObject> objects;
  std::vector<std::string> paths;
};

void ScanEnums(
    Run &run, Counts &counts, const Extensions &store, agiru::gen::EnumIndex &index, Enums &held) {
  std::vector<agiru::al::EnumObject> objects;
  std::vector<std::string> paths;
  for (const std::filesystem::path &path : SourcesEndingIn(run, ".Enum.al")) {
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

  for (agiru::al::EnumObject &object : objects) {
    const auto found = store.enums.find(agiru::gen::LowerKey(object.name));
    if (found == store.enums.end()) { continue; }
    for (const agiru::al::EnumExtensionObject &extension : found->second) {
      object.values.insert(object.values.end(), extension.values.begin(), extension.values.end());
      ++counts.emitted;
      ++store.consumed["enum " + found->first];
    }
  }

  for (const agiru::al::EnumObject &object : objects) {
    std::map<std::string, int> ordinals;
    for (const agiru::al::EnumValueDecl &value : object.values) {
      ordinals.insert_or_assign(agiru::gen::LowerKey(value.name), value.ordinal);
    }
    index.insert_or_assign(agiru::gen::LowerKey(object.name),
                           agiru::gen::EnumRef{.identifier = agiru::gen::Identifier(object.name),
                                               .header = agiru::gen::EnumHeaderPath(object),
                                               .ordinals = std::move(ordinals)});
  }
  if (run.output.empty()) { return; }
  held.objects = std::move(objects);
  held.paths = std::move(paths);
}

void WriteEnums(Run &run, const Enums &held, const agiru::gen::Objects &objects) {
  if (run.output.empty()) { return; }
  for (std::size_t i = 0; i < held.objects.size(); ++i) {
    Keep(run,
         Output{.directory = run.output, .relative = agiru::gen::EnumHeaderPath(held.objects[i])},
         agiru::gen::WriteEnum(held.objects[i], held.paths[i], objects));
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
                const agiru::gen::Objects &objects,
                Gathered &gathered,
                std::map<std::string, std::size_t> &unresolved) {
  const std::string stem =
      agiru::gen::OutputDirectory(table.nameSpace, agiru::gen::ObjectKind::Table) + "/" +
      agiru::gen::Identifier(table.name);
  const agiru::gen::TableHeader header = agiru::gen::WriteHeader(table, relative, index, objects);
  Absorb(gathered.dotnet, header.dotnet);
  Absorb(gathered.absent, header.absent);
  for (const std::string &missing : header.unresolvedEnums) { ++unresolved[missing]; }
  Keep(run, Output{.directory = run.output, .relative = stem + ".h"}, header.text);
  Keep(run,
       Output{.directory = run.output, .relative = stem + ".cpp"},
       agiru::gen::WriteSource(table, relative, objects));
  ++run.written;
}

struct Tables {
  std::vector<agiru::al::TableObject> objects;
  std::vector<std::string> paths;
};

void NoteFieldEnums(const agiru::al::TableObject &table, agiru::gen::FieldEnums &into) {
  auto &fields = into[agiru::gen::LowerKey(table.name)];
  for (const agiru::al::FieldDecl &field : table.fields) {
    if (agiru::gen::TypeName(field.type) == "Enum" && !field.subtype.empty()) {
      fields.insert_or_assign(agiru::gen::LowerKey(field.name),
                              "enums::" + agiru::gen::Identifier(field.subtype));
      continue;
    }
    if (agiru::al::Find(field.properties, "OptionMembers") != nullptr) {
      fields.insert_or_assign(agiru::gen::LowerKey(field.name),
                              "tables::" + agiru::gen::OptionEnumName(table.name, field.name));
    }
  }
}

Tables IndexTables(Run &run, Counts &counts, agiru::gen::Objects &objects) {
  Tables kept;
  for (const std::filesystem::path &path : SourcesEndingIn(run, ".Table.al")) {
    ++counts.files;
    try {
      agiru::al::TableObject table = agiru::al::ParseTable(Read(path));
      ++counts.parsed;
      counts.members += table.fields.size();
      std::map<std::string, std::string> fieldNames;
      for (const agiru::al::FieldDecl &field : table.fields) {
        fieldNames.emplace(agiru::gen::LowerKey(field.name),
                           agiru::gen::FieldIdentifier(table, field.name));
      }
      for (const agiru::SystemFieldDecl &field : agiru::kSystemFields) {
        fieldNames.emplace(agiru::gen::LowerKey(std::string(field.name)), std::string(field.name));
      }
      const agiru::gen::TableRef ref{.identifier = "tables::" + agiru::gen::Identifier(table.name),
                                     .header = TableHeaderPath(table),
                                     .fields = std::move(fieldNames)};
      objects.tables.insert_or_assign(agiru::gen::LowerKey(table.name), ref);
      objects.tables.insert_or_assign(std::to_string(table.id), ref);
      NoteFieldEnums(table, objects.fieldEnums);
      kept.paths.push_back(std::filesystem::relative(path, run.root).string());
      kept.objects.push_back(std::move(table));
    } catch (const std::exception &e) {
      if (Note(run, path, e)) { --counts.files; }
    }
  }
  return kept;
}

std::size_t MergeExtensions(const Extensions &store, Tables &tables) {
  std::size_t merged = 0;
  const auto take = [](auto &into, const auto &from) {
    into.insert(into.end(), from.begin(), from.end());
  };
  for (agiru::al::TableObject &table : tables.objects) {
    const auto found = store.tables.find(agiru::gen::LowerKey(table.name));
    if (found == store.tables.end()) { continue; }
    for (const agiru::al::TableExtensionObject &extension : found->second) {
      for (const agiru::al::FieldDecl &change : extension.modified) {
        for (agiru::al::FieldDecl &field : table.fields) {
          if (agiru::gen::LowerKey(field.name) != agiru::gen::LowerKey(change.name)) { continue; }
          take(field.properties, change.properties);
          take(field.triggers, change.triggers);
          break;
        }
      }
      TakeFields(table.fields, extension.fields);
      take(table.keys, extension.keys);
      take(table.labels, extension.labels);
      TakeVariables(table.variables, extension.variables);
      TakeProcedures(table.procedures, extension.procedures);
      ++merged;
      ++store.consumed["table " + found->first];
    }
  }
  return merged;
}

void WriteTables(Run &run,
                 const Tables &kept,
                 const agiru::gen::EnumIndex &index,
                 const agiru::gen::Objects &objects,
                 Gathered &gathered,
                 std::map<std::string, std::size_t> &unresolvedEnums) {
  if (run.output.empty()) { return; }
  for (std::size_t i = 0; i < kept.objects.size(); ++i) {
    WriteTable(run, kept.objects[i], kept.paths[i], index, objects, gathered, unresolvedEnums);
  }
}

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

void IndexCodeunits(const Run &run, agiru::gen::Objects &objects) {
  for (const std::filesystem::path &path : SourcesEndingIn(run, ".Codeunit.al")) {
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
                      "/" + identifier + ".h",
            .fields = {}});
  }
}

void IndexReports(const Run &run, agiru::gen::Objects &objects) {
  for (const std::filesystem::path &path : SourcesEndingIn(run, ".Report.al")) {
    const agiru::gen::ObjectDeclaration declared =
        agiru::gen::DeclarationOf(Read(path), agiru::gen::ObjectKind::Report);
    if (!declared.found || declared.id == 0) { continue; }
    const std::string identifier = agiru::gen::Identifier(declared.name);
    objects.reports.insert_or_assign(
        agiru::gen::LowerKey(declared.name),
        agiru::gen::TableRef{
            .identifier = "reports::" + identifier,
            .header =
                agiru::gen::OutputDirectory(declared.nameSpace, agiru::gen::ObjectKind::Report) +
                "/" + identifier + ".h",
            .fields = {{"id", std::to_string(declared.id)}, {"name", declared.name}}});
  }
}

void WriteReports(Run &run, const agiru::gen::Objects &objects) {
  if (run.output.empty()) { return; }
  for (const auto &[key, ref] : objects.reports) {
    const std::string identifier = ref.identifier.substr(std::string("reports::").size());
    const auto number = ref.fields.find("id");
    std::string out = "// Generated from the report's declaration. Do not edit.\n\n";
    out += "#pragma once\n\n";
    out += "#include \"meta/Ids.h\"\n";
    out += "#include \"runtime/Error.h\"\n\n";
    out += "#include <string_view>\n\n";
    out += "namespace agiru::app::reports {\n\n";
    out += "class " + identifier + " {\npublic:\n";
    out += "  static constexpr ReportId kId{" + number->second + "};\n";
    out += "  static constexpr std::string_view kName{" +
           agiru::gen::Literal(ref.fields.at("name")) + "};\n\n";
    out += "  static constexpr ReportId Id() { return kId; }\n\n";
    for (const std::string_view member :
         {"Run",          "RunModal",       "RunRequestPage",       "Print",       "Execute",
          "SaveAs",       "SaveAsPdf",      "SaveAsExcel",          "SaveAsWord",  "SaveAsHtml",
          "SaveAsXml",    "RdlcLayout",     "WordLayout",           "ExcelLayout", "DefaultLayout",
          "SetTableView", "UseRequestPage", "TargetFormat",         "ObjectId",    "Language",
          "FormatRegion", "Preview",        "GetSubstituteReportId"}) {
      out += "  template <typename... Arguments> std::string ";
      out += member;
      out += "(Arguments &&...arguments) const {\n";
      out += "    (static_cast<void>(arguments), ...);\n";
      out += "    throw ::agiru::Error(\"Report.";
      out += member;
      out += " is declared and not implemented yet (board:0034)\");\n";
      out += "  }\n\n";
    }
    out += "};\n\n";
    out += "} // namespace agiru::app::reports\n";
    Keep(run, Output{.directory = run.output, .relative = ref.header}, out);
  }
}

void ScanCodeunits(Run &run,
                   Counts &counts,
                   Gathered &gathered,
                   agiru::gen::Objects &objects,
                   std::map<std::string, std::size_t> &unresolvedTables) {
  for (const std::filesystem::path &path : SourcesEndingIn(run, ".Codeunit.al")) {
    ++counts.files;
    const std::string source = Read(path);
    const UnitTestPopulation population = UnitTestsIn(source);
    counts.unitFiles += population.files;
    counts.unitTests += population.tests;
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

constexpr std::size_t kUnresolvedShown = 10;

const std::set<std::string> &Rebuilt() {
  static const std::set<std::string> kRebuilt{"ALConfigSettings",
                                              "GenericDictionary2",
                                              "GenericList1",
                                              "NavTenantSettingsHelper",
                                              "UserInfo"};
  return kRebuilt;
}

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

void WriteAbsent(const std::filesystem::path &out,
                 const agiru::gen::DotNetUse &dotnet,
                 const agiru::gen::DotNetUse &absent) {
  if (out.empty()) { return; }
  std::string text = "// Generated from every AL body that names a type this run does not have.\n";
  text += "// Do not edit.\n\n#pragma once\n\n#include \"dotnet/Refused.h\"\n";
  text += "\nnamespace agiru::dotnet {\n";
  const Counted net = Stubs(text, dotnet, true);
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
  std::vector<std::pair<std::string, std::size_t>> ranked(unresolved.begin(), unresolved.end());
  std::ranges::sort(ranked, [](const auto &a, const auto &b) { return a.second > b.second; });
  const std::size_t shown = std::min<std::size_t>(ranked.size(), kUnresolvedShown);
  for (std::size_t i = 0; i < shown; ++i) {
    std::println("          {:>5} x {}", ranked[i].second, ranked[i].first);
  }
}

void ClaimOutput(const std::filesystem::path &out) {
  if (out.empty()) { return; }
  std::filesystem::create_directories(out);
}

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

int Scan(const Job &job) {
  const std::vector<agiru::gen::App> apps = agiru::gen::ReadApps(job.apps);
  const agiru::gen::TranspileScope scope =
      agiru::gen::ReadScope(job.apps.parent_path() / "scope.json");
  ClaimOutput(job.output);

  Counts allExtensionsRead;
  Run reader{.scope = &scope,
             .root = job.source,
             .output = {},
             .failures = {},
             .refusals = {},
             .written = 0,
             .changed = 0,
             .kept = {}};
  const Extensions store = ReadExtensions(reader, allExtensionsRead, apps, job.source);
  NoteTargets(store);

  std::map<std::string, std::size_t> unresolvedEnums;
  std::map<std::string, std::size_t> unresolvedTables;
  Gathered gathered;
  agiru::gen::EnumIndex index;
  agiru::gen::Objects objects;
  Counts allEnums;
  Counts allTables;
  Counts allCodeunits;
  Counts allPages;
  Counts allExtensions;
  std::vector<Failure> failures;
  std::vector<Failure> refusals;
  std::size_t written = 0;
  std::size_t changed = 0;
  std::set<std::filesystem::path> kept;
  std::vector<Tables> held;
  TableByName everyTable;
  std::map<std::string, std::size_t> untranslated;

  std::size_t column = 0;
  for (const agiru::gen::App &app : apps) { column = std::max(column, app.name.size() + 1); }

  objects.tables = agiru::gen::PlatformTables();
  objects.fieldEnums = agiru::gen::PlatformFieldEnums();

  for (const agiru::gen::App &app : apps) {
    const std::filesystem::path source = job.source / app.source;
    if (!std::filesystem::is_directory(source)) {
      std::println("{:<10} {} -- no such tree, skipped", app.name, source.string());
      continue;
    }
    Run run{.scope = &scope,
            .root = source,
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
    Counts pages;
    Counts extensions;
    ClaimApp(run.output);
    IndexCodeunits(run, objects);
    IndexReports(run, objects);
    WriteReports(run, objects);
    Enums heldEnums;
    ScanEnums(run, enums, store, index, heldEnums);
    objects.enums = index;
    Tables &parsedTables = held.emplace_back(IndexTables(run, tables, objects));
    extensions.emitted += MergeExtensions(store, parsedTables);
    const Interfaces parsedInterfaces = IndexInterfaces(run, interfaces, objects);
    for (const agiru::al::TableObject &table : parsedTables.objects) {
      everyTable.insert_or_assign(agiru::gen::LowerKey(table.name), &table);
    }
    Pages parsed = IndexPages(run, pages, objects);
    extensions.emitted += MergePageExtensions(store, parsed);
    ScanCodeunits(run, codeunits, gathered, objects, unresolvedTables);
    WriteEnums(run, heldEnums, objects);
    WriteInterfaces(run, parsedInterfaces, gathered, objects);
    WriteTables(run, parsedTables, index, objects, gathered, unresolvedEnums);
    WritePages(run, parsed, objects, gathered, everyTable);

    std::println("{:<{}}{} table(s), {} codeunit(s), {} page(s), {} enum(s), {} [Test] method(s){}",
                 app.name,
                 column,
                 tables.parsed,
                 codeunits.parsed,
                 pages.parsed,
                 enums.parsed,
                 codeunits.tests,
                 run.written != 0 ? std::format(" -- {} written", run.written) : std::string{});
    for (const auto &[kind, found] : UntranslatedKinds(run)) { untranslated[kind] += found; }
    Add(allEnums, enums);
    Add(allTables, tables);
    Add(allCodeunits, codeunits);
    Add(allPages, pages);
    Add(allExtensions, extensions);
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
  Report("pages", allPages);
  Report("extensions", allExtensionsRead);
  std::println("merged     {} extension(s) into the objects they extend", allExtensions.emitted);
  std::map<std::string, std::size_t> orphans;
  for (const auto &[name, total] : store.held) {
    const auto taken = store.consumed.find(name);
    if (taken == store.consumed.end()) { orphans.insert_or_assign(name, total); }
  }
  if (!untranslated.empty()) {
    std::size_t total = 0;
    for (const auto &[kind, found] : untranslated) { total += found; }
    std::println("untranslated {} object(s) in scope whose kind has no generator (board:0034)",
                 total);
    std::vector<std::pair<std::string, std::size_t>> ranked(untranslated.begin(),
                                                            untranslated.end());
    std::ranges::sort(ranked, [](const auto &a, const auto &b) { return a.second > b.second; });
    for (const auto &[kind, found] : ranked) { std::println("          {:>5} x {}", found, kind); }
  }
  ReportUnresolved("extension(s)", "object(s) no app declares", orphans);
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

}

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

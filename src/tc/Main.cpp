#include "meta/Declare.h"

#include "Apps.h"
#include "Ast.h"
#include "BodyWriter.h"
#include "CodeunitWriter.h"
#include "EnumWriter.h"
#include "Names.h"
#include "PageWriter.h"
#include "Parser.h"
#include "Refused.h"
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
  std::size_t moved = 0;
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

std::map<std::string, std::size_t> DeclaredOnlyKinds(const Run &run) {
  static constexpr std::array kDeclarationOnly{
      std::string_view{"report"},
      std::string_view{"query"},
      std::string_view{"xmlport"},
  };
  std::map<std::string, std::size_t> counted;
  for (const std::string_view kind : kDeclarationOnly) {
    const std::size_t found = SourcesEndingIn(run, "." + std::string(kind) + ".al").size();
    if (found != 0) { counted[std::string(kind)] += found; }
  }
  return counted;
}

std::map<std::string, std::size_t> UntranslatedKinds(const Run &run) {
  static constexpr std::array kWithoutAGenerator{
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

bool IsMoved(const std::vector<agiru::al::Property> &properties) {
  const agiru::al::Property *state = agiru::al::Find(properties, "ObsoleteState");
  return state != nullptr && agiru::gen::LowerKey(state->text) == "moved";
}

struct Gathered {
  agiru::gen::DotNetUse dotnet;
  agiru::gen::DotNetUse absent;
  std::vector<agiru::gen::RefusedProperty> refused;
  std::map<std::string, std::size_t> attributes;
  std::map<std::string, std::vector<std::string>> options;
  std::map<std::string, std::size_t> deprecatedScopes;
  std::map<std::string, std::size_t> properties;
  std::map<std::string, std::size_t> contradictions;
};

constexpr std::array kAcknowledgedAttributes{
    std::pair{std::string_view{"scope"},
              std::string_view{"reach from an extension; no run-time behaviour"}},
    std::pair{std::string_view{"normal"}, std::string_view{"not a [Test]; the default already"}},
    std::pair{std::string_view{"obsolete"},
              std::string_view{"a compile-time warning in AL; the body stands"}},
    std::pair{std::string_view{"nondebuggable"},
              std::string_view{"debugger visibility; no debugger here"}},
    std::pair{std::string_view{"inherentpermissions"},
              std::string_view{"it ELEVATES one method's access, and no permission is checked "
                               "before a read yet (board:0062, board:0202)"}},
    std::pair{std::string_view{"serviceenabled"},
              std::string_view{"it publishes one method as a web-service action, and there is no "
                               "web-service surface (board:0030)"}},
};

constexpr std::string_view kTranslatedProperties[] = {
    std::string_view{"codeunit.eventsubscriberinstance"},
    std::string_view{"codeunit.subtype"},
    std::string_view{"codeunit.tableno"},
    std::string_view{"codeunit.testpermissions"},
    std::string_view{"field.access"},
    std::string_view{"field.allowincustomizations"},
    std::string_view{"field.autoformatexpression"},
    std::string_view{"field.autoformattype"},
    std::string_view{"field.autoincrement"},
    std::string_view{"field.blanknumbers"},
    std::string_view{"field.blankzero"},
    std::string_view{"field.calcformula"},
    std::string_view{"field.caption"},
    std::string_view{"field.captionclass"},
    std::string_view{"field.charallowed"},
    std::string_view{"field.closingdates"},
    std::string_view{"field.compressed"},
    std::string_view{"field.decimalplaces"},
    std::string_view{"field.drilldownpageid"},
    std::string_view{"field.editable"},
    std::string_view{"field.extendeddatatype"},
    std::string_view{"field.fieldclass"},
    std::string_view{"field.initvalue"},
    std::string_view{"field.lookuppageid"},
    std::string_view{"field.masktype"},
    std::string_view{"field.maxvalue"},
    std::string_view{"field.minvalue"},
    std::string_view{"field.notblank"},
    std::string_view{"field.numeric"},
    std::string_view{"field.obsoletereason"},
    std::string_view{"field.obsoletestate"},
    std::string_view{"field.obsoletetag"},
    std::string_view{"field.optimizefortextsearch"},
    std::string_view{"field.optioncaption"},
    std::string_view{"field.optionmembers"},
    std::string_view{"field.validatetablerelation"},
    std::string_view{"field.valuesallowed"},
    std::string_view{"field.width"},
    std::string_view{"key.clustered"},
    std::string_view{"key.enabled"},
    std::string_view{"key.includedfields"},
    std::string_view{"key.maintainsiftindex"},
    std::string_view{"key.maintainsqlindex"},
    std::string_view{"key.sumindexfields"},
    std::string_view{"key.unique"},
    std::string_view{"page.sourcetable"},
    std::string_view{"action.allowedfileextensions"},
    std::string_view{"action.allowmultiplefiles"},
    std::string_view{"action.closingdates"},
    std::string_view{"action.columnspan"},
    std::string_view{"action.cuegrouplayout"},
    std::string_view{"action.description"},
    std::string_view{"action.entityname"},
    std::string_view{"action.entitysetname"},
    std::string_view{"action.numeric"},
    std::string_view{"action.rowspan"},
    std::string_view{"action.treeinitialstate"},
    std::string_view{"action.valuesallowed"},
    std::string_view{"codeunit.description"},
    std::string_view{"codeunit.testisolation"},
    std::string_view{"codeunit.testtype"},
    std::string_view{"control.allowedfileextensions"},
    std::string_view{"control.allowmultiplefiles"},
    std::string_view{"control.closingdates"},
    std::string_view{"control.columnspan"},
    std::string_view{"control.cuegrouplayout"},
    std::string_view{"control.description"},
    std::string_view{"control.entityname"},
    std::string_view{"control.entitysetname"},
    std::string_view{"control.numeric"},
    std::string_view{"control.rowspan"},
    std::string_view{"control.treeinitialstate"},
    std::string_view{"control.valuesallowed"},
    std::string_view{"field.enabled"},
    std::string_view{"field.movedfrom"},
    std::string_view{"field.subtype"},
    std::string_view{"key.description"},
    std::string_view{"key.obsoletestate"},
    std::string_view{"page.apigroup"},
    std::string_view{"page.apipublisher"},
    std::string_view{"page.apiversion"},
    std::string_view{"page.changetrackingallowed"},
    std::string_view{"page.dataaccessintent"},
    std::string_view{"page.description"},
    std::string_view{"page.entitycaption"},
    std::string_view{"page.entityname"},
    std::string_view{"page.entitysetcaption"},
    std::string_view{"page.entitysetname"},
    std::string_view{"page.helplink"},
    std::string_view{"page.ispreview"},
    std::string_view{"table.description"},
    std::string_view{"table.pasteisvalid"},
    std::string_view{"action.blanknumbers"},
    std::string_view{"action.flowtemplatecategoryname"},
    std::string_view{"action.gesture"},
    std::string_view{"action.gridlayout"},
    std::string_view{"action.isheader"},
    std::string_view{"action.masktype"},
    std::string_view{"action.maxvalue"},
    std::string_view{"action.minvalue"},
    std::string_view{"action.notblank"},
    std::string_view{"action.provider"},
    std::string_view{"action.showastree"},
    std::string_view{"control.blanknumbers"},
    std::string_view{"control.flowtemplatecategoryname"},
    std::string_view{"control.gesture"},
    std::string_view{"control.gridlayout"},
    std::string_view{"control.isheader"},
    std::string_view{"control.masktype"},
    std::string_view{"control.maxvalue"},
    std::string_view{"control.minvalue"},
    std::string_view{"control.notblank"},
    std::string_view{"control.provider"},
    std::string_view{"control.showastree"},
    std::string_view{"page.accessbypermission"},
    std::string_view{"page.contextsensitivehelppage"},
    std::string_view{"page.inherententitlements"},
    std::string_view{"page.inherentpermissions"},
    std::string_view{"page.odatakeyfields"},
    std::string_view{"page.populateallfields"},
    std::string_view{"page.querycategory"},
    std::string_view{"table.extensible"},
    std::string_view{"codeunit.permissions"},
    std::string_view{"codeunit.inherentpermissions"},
    std::string_view{"codeunit.inherententitlements"},
    std::string_view{"codeunit.access"},
    std::string_view{"codeunit.singleinstance"},
    std::string_view{"codeunit.obsoletestate"},
    std::string_view{"action.abouttext"},
    std::string_view{"action.abouttitle"},
    std::string_view{"action.drilldownpageid"},
    std::string_view{"action.extendeddatatype"},
    std::string_view{"action.indentationcolumn"},
    std::string_view{"action.indentationcontrols"},
    std::string_view{"action.infooterbar"},
    std::string_view{"action.instructionaltext"},
    std::string_view{"action.lookuppageid"},
    std::string_view{"action.optioncaption"},
    std::string_view{"action.runpageonrec"},
    std::string_view{"action.showas"},
    std::string_view{"action.showfilter"},
    std::string_view{"action.updatepropagation"},
    std::string_view{"control.abouttext"},
    std::string_view{"control.abouttitle"},
    std::string_view{"control.drilldownpageid"},
    std::string_view{"control.extendeddatatype"},
    std::string_view{"control.indentationcolumn"},
    std::string_view{"control.indentationcontrols"},
    std::string_view{"control.infooterbar"},
    std::string_view{"control.instructionaltext"},
    std::string_view{"control.lookuppageid"},
    std::string_view{"control.optioncaption"},
    std::string_view{"control.runpageonrec"},
    std::string_view{"control.showas"},
    std::string_view{"control.showfilter"},
    std::string_view{"control.updatepropagation"},
    std::string_view{"field.description"},
    std::string_view{"field.movedto"},
    std::string_view{"page.abouttext"},
    std::string_view{"page.abouttitle"},
    std::string_view{"page.autosplitkey"},
    std::string_view{"page.extensible"},
    std::string_view{"page.sourcetabletemporary"},
    std::string_view{"table.access"},
    std::string_view{"table.inherententitlements"},
    std::string_view{"table.inherentpermissions"},
    std::string_view{"table.permissions"},
    std::string_view{"field.tooltip"},
    std::string_view{"field.accessbypermission"},
    std::string_view{"table.lookuppageid"},
    std::string_view{"table.drilldownpageid"},
    std::string_view{"control.autoformattype"},
    std::string_view{"control.autoformatexpression"},
    std::string_view{"control.captionclass"},
    std::string_view{"control.decimalplaces"},
    std::string_view{"control.tablerelation"},
    std::string_view{"control.blankzero"},
    std::string_view{"action.autoformattype"},
    std::string_view{"action.autoformatexpression"},
    std::string_view{"action.captionclass"},
    std::string_view{"action.decimalplaces"},
    std::string_view{"action.tablerelation"},
    std::string_view{"action.blankzero"},
    std::string_view{"action.accessbypermission"},
    std::string_view{"action.applicationarea"},
    std::string_view{"action.assistedit"},
    std::string_view{"action.caption"},
    std::string_view{"action.drilldown"},
    std::string_view{"action.editable"},
    std::string_view{"action.ellipsis"},
    std::string_view{"action.enabled"},
    std::string_view{"action.freezecolumn"},
    std::string_view{"action.hidevalue"},
    std::string_view{"action.image"},
    std::string_view{"action.importance"},
    std::string_view{"action.lookup"},
    std::string_view{"action.multiline"},
    std::string_view{"action.obsoletestate"},
    std::string_view{"action.quickentry"},
    std::string_view{"action.runobject"},
    std::string_view{"action.runpagelink"},
    std::string_view{"action.runpagemode"},
    std::string_view{"action.runpageview"},
    std::string_view{"action.scope"},
    std::string_view{"action.shortcutkey"},
    std::string_view{"action.showcaption"},
    std::string_view{"action.showmandatory"},
    std::string_view{"action.style"},
    std::string_view{"action.styleexpr"},
    std::string_view{"action.subpagelink"},
    std::string_view{"action.subpageview"},
    std::string_view{"action.tooltip"},
    std::string_view{"action.visible"},
    std::string_view{"action.width"},
    std::string_view{"control.accessbypermission"},
    std::string_view{"control.applicationarea"},
    std::string_view{"control.assistedit"},
    std::string_view{"control.caption"},
    std::string_view{"control.drilldown"},
    std::string_view{"control.editable"},
    std::string_view{"control.ellipsis"},
    std::string_view{"control.enabled"},
    std::string_view{"control.freezecolumn"},
    std::string_view{"control.hidevalue"},
    std::string_view{"control.image"},
    std::string_view{"control.importance"},
    std::string_view{"control.lookup"},
    std::string_view{"control.multiline"},
    std::string_view{"control.obsoletestate"},
    std::string_view{"control.quickentry"},
    std::string_view{"control.runobject"},
    std::string_view{"control.runpagelink"},
    std::string_view{"control.runpagemode"},
    std::string_view{"control.runpageview"},
    std::string_view{"control.scope"},
    std::string_view{"control.shortcutkey"},
    std::string_view{"control.showcaption"},
    std::string_view{"control.showmandatory"},
    std::string_view{"control.style"},
    std::string_view{"control.styleexpr"},
    std::string_view{"control.subpagelink"},
    std::string_view{"control.subpageview"},
    std::string_view{"control.tooltip"},
    std::string_view{"control.visible"},
    std::string_view{"control.width"},
    std::string_view{"page.access"},
    std::string_view{"page.additionalsearchterms"},
    std::string_view{"page.analysismodeenabled"},
    std::string_view{"page.applicationarea"},
    std::string_view{"page.caption"},
    std::string_view{"page.cardpageid"},
    std::string_view{"page.datacaptionexpression"},
    std::string_view{"page.datacaptionfields"},
    std::string_view{"page.delayedinsert"},
    std::string_view{"page.deleteallowed"},
    std::string_view{"page.editable"},
    std::string_view{"page.insertallowed"},
    std::string_view{"page.instructionaltext"},
    std::string_view{"page.linksallowed"},
    std::string_view{"page.modifyallowed"},
    std::string_view{"page.obsoletestate"},
    std::string_view{"page.pagetype"},
    std::string_view{"page.permissions"},
    std::string_view{"page.promotedactioncategories"},
    std::string_view{"page.refreshonactivate"},
    std::string_view{"page.savevalues"},
    std::string_view{"page.showfilter"},
    std::string_view{"page.sourcetableview"},
    std::string_view{"page.usagecategory"},
    std::string_view{"table.allowincustomizations"},
    std::string_view{"table.caption"},
    std::string_view{"table.compressiontype"},
    std::string_view{"table.datacaptionfields"},
    std::string_view{"table.dataaccessintent"},
    std::string_view{"table.datapercompany"},
    std::string_view{"table.movedfrom"},
    std::string_view{"table.movedto"},
    std::string_view{"table.obsoletestate"},
    std::string_view{"table.replicatedata"},
    std::string_view{"table.tabletype"},
};

constexpr std::array kPartlyTranslatedProperties{
    std::pair{std::string_view{"field.tablerelation"},
              std::string_view{"the bare Table[.Field] form reaches the metadata and the "
                               "conditional grammar does not (board:0043)"}},
};

constexpr std::array kDroppedProperties{
    std::pair{std::string_view{"promoted"}, std::string_view{"the UI's action bar (board:0030)"}},
    std::pair{std::string_view{"promotedcategory"},
              std::string_view{"the UI's action bar (board:0030)"}},
    std::pair{std::string_view{"promotedisbig"},
              std::string_view{"the UI's action bar (board:0030)"}},
    std::pair{std::string_view{"promotedonly"},
              std::string_view{"the UI's action bar (board:0030)"}},
    std::pair{std::string_view{"multiplenewlines"},
              std::string_view{"the UI's list behaviour (board:0030)"}},
    std::pair{std::string_view{"dataclassification"},
              std::string_view{"telemetry classification, no run-time behaviour"}},
    std::pair{std::string_view{"obsoletereason"},
              std::string_view{"a diagnostic's text (board:0069)"}},
    std::pair{std::string_view{"obsoletetag"},
              std::string_view{"a diagnostic's text (board:0069)"}},
};

constexpr std::array kActedOnAttributes{
    std::string_view{"businessevent"},
    std::string_view{"commitbehavior"},
    std::string_view{"confirmhandler"},
    std::string_view{"errorbehavior"},
    std::string_view{"eventsubscriber"},
    std::string_view{"filterpagehandler"},
    std::string_view{"handlerfunctions"},
    std::string_view{"hyperlinkhandler"},
    std::string_view{"integrationevent"},
    std::string_view{"internalevent"},
    std::string_view{"messagehandler"},
    std::string_view{"modalpagehandler"},
    std::string_view{"pagehandler"},
    std::string_view{"recallnotificationhandler"},
    std::string_view{"reporthandler"},
    std::string_view{"requestpagehandler"},
    std::string_view{"securityfiltering"},
    std::string_view{"sendnotificationhandler"},
    std::string_view{"sessionsettingshandler"},
    std::string_view{"strmenuhandler"},
    std::string_view{"test"},
    std::string_view{"testpermissions"},
    std::string_view{"transactionmodel"},
    std::string_view{"tryfunction"},
};

constexpr std::array kDeprecatedScopes{
    std::pair{std::string_view{"internal"}, std::string_view{"OnPrem"}},
    std::pair{std::string_view{"solution"}, std::string_view{"OnPrem"}},
    std::pair{std::string_view{"personalization"}, std::string_view{"Cloud"}},
    std::pair{std::string_view{"extension"}, std::string_view{"Cloud"}},
};

void NoteProperties(const std::vector<agiru::al::Property> &properties,
                    std::string_view owner,
                    std::map<std::string, std::size_t> &into) {
  for (const agiru::al::Property &property : properties) {
    ++into[std::string(owner) + "." + agiru::gen::LowerKey(property.name)];
  }
}

std::string_view PropertyName(std::string_view qualified) {
  const std::size_t dot = qualified.find('.');
  return dot == std::string_view::npos ? qualified : qualified.substr(dot + 1);
}

void NoteFieldClasses(const agiru::al::TableObject &table,
                      std::map<std::string, std::size_t> &into) {
  for (const agiru::al::FieldDecl &field : table.fields) {
    const agiru::al::Property *kind = agiru::al::Find(field.properties, "FieldClass");
    const bool formula = agiru::al::Find(field.properties, "CalcFormula") != nullptr;
    const std::string named = kind == nullptr ? std::string{} : agiru::gen::LowerKey(kind->text);
    if (named == "flowfield" && !formula) { ++into["a FlowField with no CalcFormula"]; }
    if ((named.empty() || named == "normal") && formula) {
      ++into["a CalcFormula on a Normal field"];
    }
  }
}

void NotePropertiesOf(const agiru::al::TableObject &table,
                      std::map<std::string, std::size_t> &into) {
  NoteProperties(table.properties, "table", into);
  for (const agiru::al::FieldDecl &field : table.fields) {
    NoteProperties(field.properties, "field", into);
  }
  for (const agiru::al::KeyDecl &key : table.keys) { NoteProperties(key.properties, "key", into); }
}

void NotePropertiesOf(const agiru::al::PageObject &page, std::map<std::string, std::size_t> &into) {
  NoteProperties(page.properties, "page", into);
  const auto walk = [&into](auto &&self,
                            std::string_view owner,
                            const std::vector<agiru::al::PageControl> &controls) -> void {
    for (const agiru::al::PageControl &control : controls) {
      NoteProperties(control.properties, owner, into);
      self(self, owner, control.children);
    }
  };
  walk(walk, "control", page.layout);
  walk(walk, "action", page.actions);
}

void NotePropertiesOf(const agiru::al::CodeunitObject &unit,
                      std::map<std::string, std::size_t> &into) {
  NoteProperties(unit.properties, "codeunit", into);
}

template <typename Object>
void CheckNormal(const Object &unit,
                 std::string_view kind,
                 bool isTestCodeunit,
                 std::vector<agiru::gen::RefusedProperty> &into) {
  for (const agiru::al::ProcedureDecl &procedure : unit.procedures) {
    const bool normal = agiru::al::HasAttribute(procedure, "Normal");
    if (normal && kind != "codeunit") {
      into.push_back({.property = "[Normal] on " + procedure.name,
                      .where = std::string(kind) + " \"" + unit.name +
                               "\" -- the attribute is only legal inside a codeunit "
                               "(attributes/devenv-normal-attribute.md)"});
    }
    if (isTestCodeunit && !normal && agiru::al::HasAttribute(procedure, "TryFunction")) {
      into.push_back({.property = "[TryFunction] on " + procedure.name,
                      .where = "test codeunit \"" + unit.name +
                               "\" -- in a test codeunit [TryFunction] applies to [Normal] "
                               "methods only (attributes/devenv-tryfunction-attribute.md)"});
    }
  }
}

template <typename Object>
void CountAttributes(const Object &unit,
                     std::map<std::string, std::size_t> &into,
                     std::map<std::string, std::size_t> &deprecated) {
  for (const agiru::al::ProcedureDecl &procedure : unit.procedures) {
    for (const std::string &attribute : procedure.attributes) {
      const std::string lowered = agiru::gen::LowerKey(attribute);
      if (lowered.starts_with("scope")) {
        for (const auto &[dead, replacement] : kDeprecatedScopes) {
          if (lowered.find(dead) != std::string::npos) {
            ++deprecated[std::string(dead) + " -> " + std::string(replacement)];
          }
        }
      }
      ++into[agiru::gen::LowerKey(attribute.substr(0, attribute.find('(')))];
    }
  }
}

void Absorb(std::vector<agiru::gen::RefusedProperty> &into,
            const std::vector<agiru::gen::RefusedProperty> &from) {
  into.insert(into.end(), from.begin(), from.end());
}

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
          agiru::gen::TableRef{
              .identifier = "::agiru::" + agiru::gen::NamespaceSuffix(object.nameSpace) +
                            agiru::gen::ClassName(identifier, agiru::gen::ObjectKind::Interface),
              .header =
                  agiru::gen::OutputDirectory(object.nameSpace, agiru::gen::ObjectKind::Interface) +
                  "/" + identifier + ".h",
              .fields = {},
              .procedures = {}});
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
  mutable std::size_t unplaced = 0;
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
      std::map<std::string, std::string> controlNames =
          agiru::gen::ControlIdentifiers(object, objects);
      objects.pages.insert_or_assign(
          agiru::gen::LowerKey(object.name),
          agiru::gen::TableRef{.identifier =
                                   "::agiru::" + agiru::gen::NamespaceSuffix(object.nameSpace) +
                                   agiru::gen::ClassName(agiru::gen::Identifier(object.name),
                                                         agiru::gen::ObjectKind::Page),
                               .header = agiru::gen::PageHeaderPath(object),
                               .id = object.id,
                               .fields = std::move(controlNames),
                               .procedures = {}});
      pages.paths.push_back(std::filesystem::relative(path, run.root).string());
      pages.objects.push_back(std::move(object));
    } catch (const std::exception &e) {
      if (Note(run, path, e)) { --counts.files; }
    }
  }
  return pages;
}

agiru::al::PageControl *NamedControl(std::vector<agiru::al::PageControl> &controls,
                                     const std::string &name,
                                     std::vector<agiru::al::PageControl> **holder) {
  for (agiru::al::PageControl &control : controls) {
    if (agiru::gen::LowerKey(control.name) == name) {
      *holder = &controls;
      return &control;
    }
    if (agiru::al::PageControl *found = NamedControl(control.children, name, holder);
        found != nullptr) {
      return found;
    }
  }
  return nullptr;
}

bool Splice(std::vector<agiru::al::PageControl> &into, const agiru::al::PageControl &operation) {
  const std::string where = agiru::gen::LowerKey(operation.kind);
  const std::string anchor = agiru::gen::LowerKey(operation.name);
  std::vector<agiru::al::PageControl> *holder = nullptr;
  agiru::al::PageControl *at = NamedControl(into, anchor, &holder);
  if (at == nullptr || holder == nullptr) { return false; }
  if (where == "modify") {
    for (const agiru::al::Property &property : operation.properties) {
      const auto standing =
          std::ranges::find_if(at->properties, [&property](const agiru::al::Property &one) {
            return agiru::gen::LowerKey(one.name) == agiru::gen::LowerKey(property.name);
          });
      if (standing == at->properties.end()) {
        at->properties.push_back(property);
      } else {
        *standing = property;
      }
    }
    return true;
  }
  if (where == "addfirst") {
    at->children.insert(at->children.begin(), operation.children.begin(), operation.children.end());
    return true;
  }
  if (where == "addlast") {
    at->children.insert(at->children.end(), operation.children.begin(), operation.children.end());
    return true;
  }
  const auto seat =
      std::ranges::find_if(*holder, [at](const agiru::al::PageControl &one) { return &one == at; });
  if (seat == holder->end()) { return false; }
  const auto put = where == "addbefore" ? seat : seat + 1;
  holder->insert(put, operation.children.begin(), operation.children.end());
  return true;
}

bool IsAnOperation(const std::string &kind) {
  const std::string lowered = agiru::gen::LowerKey(kind);
  return lowered == "addfirst" || lowered == "addlast" || lowered == "addafter" ||
         lowered == "addbefore" || lowered == "modify";
}

std::size_t TakeControls(std::vector<agiru::al::PageControl> &into,
                         const std::vector<agiru::al::PageControl> &from,
                         std::size_t &unplaced) {
  std::size_t placed = 0;
  for (const agiru::al::PageControl &control : from) {
    if (!IsAnOperation(control.kind)) {
      into.push_back(control);
      continue;
    }
    if (Splice(into, control)) {
      ++placed;
    } else {
      ++unplaced;
      into.push_back(control);
    }
  }
  return placed;
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
      std::size_t ignored = 0;
      TakeControls(page.layout, extension.layout, ignored);
      TakeControls(page.actions, extension.actions, ignored);
      TakeProcedures(page.procedures, extension.procedures);
      TakeVariables(page.variables, extension.variables);
      take(page.labels, extension.labels);
      ++merged;
      ++store.consumed["page " + found->first];
    }
    for (std::vector<agiru::al::PageControl> *section : {&page.layout, &page.actions}) {
      std::vector<agiru::al::PageControl> waiting;
      for (const agiru::al::PageControl &control : *section) {
        if (IsAnOperation(control.kind)) { waiting.push_back(control); }
      }
      if (waiting.empty()) { continue; }
      std::erase_if(*section, [](const agiru::al::PageControl &control) {
        return IsAnOperation(control.kind);
      });
      for (bool moved = true; moved && !waiting.empty();) {
        moved = false;
        std::vector<agiru::al::PageControl> again;
        for (const agiru::al::PageControl &operation : waiting) {
          if (Splice(*section, operation)) {
            moved = true;
          } else {
            again.push_back(operation);
          }
        }
        waiting = std::move(again);
      }
      store.unplaced += waiting.size();
      section->insert(section->end(), waiting.begin(), waiting.end());
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
    Absorb(gathered.refused, agiru::gen::Refused(pages.objects[i]));
    CountAttributes(pages.objects[i], gathered.attributes, gathered.deprecatedScopes);
    NotePropertiesOf(pages.objects[i], gathered.properties);
    CheckNormal(pages.objects[i], "page", false, gathered.refused);
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
    std::map<std::string, std::string> members;
    for (const agiru::al::EnumValueDecl &value : object.values) {
      ordinals.insert_or_assign(agiru::gen::LowerKey(value.name), value.ordinal);
      members.insert_or_assign(agiru::gen::LowerKey(value.name),
                               agiru::gen::EnumeratorName(value.name));
    }
    index.insert_or_assign(
        agiru::gen::LowerKey(object.name),
        agiru::gen::EnumRef{.identifier =
                                "::agiru::" + agiru::gen::NamespaceSuffix(object.nameSpace) +
                                agiru::gen::ClassName(agiru::gen::Identifier(object.name),
                                                      agiru::gen::ObjectKind::Enum),
                            .header = agiru::gen::EnumHeaderPath(object),
                            .ordinals = std::move(ordinals),
                            .members = std::move(members)});
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
    const std::string source = agiru::gen::WriteEnumSource(held.objects[i], held.paths[i], objects);
    if (source.empty()) { continue; }
    Keep(run,
         Output{.directory = run.output, .relative = agiru::gen::EnumSourcePath(held.objects[i])},
         source);
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
  Absorb(gathered.refused, agiru::gen::Refused(table));
  CountAttributes(table, gathered.attributes, gathered.deprecatedScopes);
  NotePropertiesOf(table, gathered.properties);
  NoteFieldClasses(table, gathered.contradictions);
  CheckNormal(table, "table", false, gathered.refused);
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

using OptionsInScope = std::map<std::string, std::vector<std::string>>;

void NoteOption(const agiru::al::VarDecl &declared, OptionsInScope &into) {
  if (agiru::gen::TypeName(declared.type) != "Option" || declared.members.empty()) { return; }
  into.insert_or_assign(agiru::gen::OptionContentName(declared.members), declared.members);
}

void NoteOptions(const std::vector<agiru::al::VarDecl> &variables,
                 const std::vector<agiru::al::ProcedureDecl> &procedures,
                 OptionsInScope &into) {
  for (const agiru::al::VarDecl &declared : variables) { NoteOption(declared, into); }
  for (const agiru::al::ProcedureDecl &procedure : procedures) {
    for (const agiru::al::VarDecl &declared : procedure.parameters) { NoteOption(declared, into); }
    for (const agiru::al::VarDecl &declared : procedure.variables) { NoteOption(declared, into); }
    NoteOption(procedure.returned, into);
  }
}

void NoteFieldEnums(const agiru::al::TableObject &table,
                    const agiru::gen::EnumIndex &enums,
                    agiru::gen::FieldEnums &into) {
  auto &fields = into[agiru::gen::LowerKey(table.name)];
  for (const agiru::al::FieldDecl &field : table.fields) {
    if (agiru::gen::TypeName(field.type) == "Enum" && !field.subtype.empty()) {
      const auto known = enums.find(agiru::gen::LowerKey(field.subtype));
      if (known != enums.end()) {
        fields.insert_or_assign(agiru::gen::LowerKey(field.name), known->second.identifier);
      }
      continue;
    }
    if (const agiru::al::Property *members = agiru::al::Find(field.properties, "OptionMembers");
        members != nullptr) {
      const std::string named =
          agiru::gen::OptionEnumName(table.name, field.name, agiru::al::ListValue(*members));
      fields.insert_or_assign(agiru::gen::LowerKey(field.name),
                              named.find("::") == std::string::npos ? "tables::" + named : named);
    }
  }
}

Tables IndexTables(Run &run, Counts &counts, agiru::gen::Objects &objects) {
  Tables kept;
  for (const std::filesystem::path &path : SourcesEndingIn(run, ".Table.al")) {
    ++counts.files;
    try {
      agiru::al::TableObject table = agiru::al::ParseTable(Read(path));
      if (IsMoved(table.properties)) {
        ++counts.moved;
        continue;
      }
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
      std::map<std::string, std::string> procedureNames;
      for (const agiru::al::ProcedureDecl &procedure : table.procedures) {
        procedureNames.emplace(agiru::gen::LowerKey(procedure.name),
                               agiru::gen::ProcedureIdentifier(table, procedure.name));
      }
      const agiru::gen::TableRef ref{
          .identifier = "::agiru::" + agiru::gen::NamespaceSuffix(table.nameSpace) +
                        agiru::gen::ClassName(agiru::gen::Identifier(table.name),
                                              agiru::gen::ObjectKind::Table),
          .header = TableHeaderPath(table),
          .fields = std::move(fieldNames),
          .procedures = std::move(procedureNames)};
      objects.tables.insert_or_assign(agiru::gen::LowerKey(table.name), ref);
      objects.tables.insert_or_assign(std::to_string(table.id), ref);
      NoteFieldEnums(table, objects.enums, objects.fieldEnums);
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

std::map<std::string, std::string> DeclaredProcedures(std::string_view source) {
  std::map<std::string, std::string> procedures;
  static constexpr std::string_view kKeyword = "procedure";
  for (std::size_t at = source.find(kKeyword); at != std::string_view::npos;
       at = source.find(kKeyword, at + kKeyword.size())) {
    const bool wordStart = at == 0 || std::isalnum(static_cast<unsigned char>(source[at - 1])) == 0;
    std::size_t cursor = at + kKeyword.size();
    if (!wordStart || cursor >= source.size() ||
        std::isspace(static_cast<unsigned char>(source[cursor])) == 0) {
      continue;
    }
    while (cursor < source.size() &&
           std::isspace(static_cast<unsigned char>(source[cursor])) != 0) {
      ++cursor;
    }
    std::string named;
    if (cursor < source.size() && source[cursor] == '"') {
      const std::size_t close = source.find('"', cursor + 1);
      if (close == std::string_view::npos) { continue; }
      named = std::string(source.substr(cursor + 1, close - cursor - 1));
    } else {
      std::size_t end = cursor;
      while (end < source.size() &&
             (std::isalnum(static_cast<unsigned char>(source[end])) != 0 || source[end] == '_')) {
        ++end;
      }
      named = std::string(source.substr(cursor, end - cursor));
    }
    if (!named.empty()) {
      procedures.emplace(agiru::gen::LowerKey(named), agiru::gen::Identifier(named));
    }
  }
  return procedures;
}

void IndexCodeunits(const Run &run, agiru::gen::Objects &objects) {
  for (const std::filesystem::path &path : SourcesEndingIn(run, ".Codeunit.al")) {
    const std::string source = Read(path);
    const agiru::gen::ObjectDeclaration declared =
        agiru::gen::DeclarationOf(source, agiru::gen::ObjectKind::Codeunit);
    if (!declared.found) { continue; }
    const std::string &name = declared.name;
    const std::string &nameSpace = declared.nameSpace;

    const std::string identifier = agiru::gen::Identifier(name);
    const std::map<std::string, std::string> procedures = DeclaredProcedures(source);
    objects.codeunits.insert_or_assign(
        agiru::gen::LowerKey(name),
        agiru::gen::TableRef{
            .identifier = "::agiru::" + agiru::gen::NamespaceSuffix(nameSpace) +
                          agiru::gen::ClassName(identifier, agiru::gen::ObjectKind::Codeunit),
            .header = agiru::gen::OutputDirectory(nameSpace, agiru::gen::ObjectKind::Codeunit) +
                      "/" + identifier + ".h",
            .fields = {},
            .procedures = std::move(procedures)});
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
            .identifier = "::agiru::" + agiru::gen::NamespaceSuffix(declared.nameSpace) +
                          agiru::gen::ClassName(identifier, agiru::gen::ObjectKind::Report),
            .header =
                agiru::gen::OutputDirectory(declared.nameSpace, agiru::gen::ObjectKind::Report) +
                "/" + identifier + ".h",
            .fields = {{"id", std::to_string(declared.id)}, {"name", declared.name}},
            .procedures = {}});
  }
}

void WriteReports(Run &run, const agiru::gen::Objects &objects) {
  if (run.output.empty()) { return; }
  for (const auto &[key, ref] : objects.reports) {
    const std::string reachable = agiru::gen::Unprefixed(ref.identifier);
    const std::size_t colons = reachable.rfind("::");
    const std::string space =
        colons == std::string::npos ? "agiru" : "agiru::" + reachable.substr(0, colons);
    const std::string identifier =
        colons == std::string::npos ? reachable : reachable.substr(colons + 2);
    const auto number = ref.fields.find("id");
    std::string out = "// Generated from the report\'s declaration. Do not edit.\n\n";
    out += "#pragma once\n\n";
    out += "#include \"meta/Ids.h\"\n";
    out += "#include \"runtime/Report.h\"\n\n";
    out += "#include <string_view>\n\n";
    out += "namespace " + space + " {\n\n";
    out += "class " + identifier + " : public ::agiru::Report<" + identifier + "> {\npublic:\n";
    out += "  static constexpr ReportId kId{" + number->second + "};\n";
    out += "  static constexpr std::string_view kName{" +
           agiru::gen::Literal(ref.fields.at("name")) + "};\n";
    out += "};\n\n";
    out += "} // namespace " + space + "\n\n";
    out += "template <> struct agiru::ReportTraits<" + space + "::" + identifier + "> {\n";
    out += "  static constexpr ReportId kId{" + number->second + "};\n";
    out += "  static constexpr std::string_view kName{" +
           agiru::gen::Literal(ref.fields.at("name")) + "};\n};\n";
    Keep(run, Output{.directory = run.output, .relative = ref.header}, out);
  }
}

void IndexXmlPorts(const Run &run, agiru::gen::Objects &objects) {
  for (const std::filesystem::path &path : SourcesEndingIn(run, ".XmlPort.al")) {
    const agiru::gen::ObjectDeclaration declared =
        agiru::gen::DeclarationOf(Read(path), agiru::gen::ObjectKind::XmlPort);
    if (!declared.found || declared.id == 0) { continue; }
    const std::string identifier = agiru::gen::Identifier(declared.name);
    objects.xmlports.insert_or_assign(
        agiru::gen::LowerKey(declared.name),
        agiru::gen::TableRef{
            .identifier = "::agiru::" + agiru::gen::NamespaceSuffix(declared.nameSpace) +
                          agiru::gen::ClassName(identifier, agiru::gen::ObjectKind::XmlPort),
            .header =
                agiru::gen::OutputDirectory(declared.nameSpace, agiru::gen::ObjectKind::XmlPort) +
                "/" + identifier + ".h",
            .fields = {{"id", std::to_string(declared.id)}, {"name", declared.name}},
            .procedures = {}});
  }
}

void WriteXmlPorts(Run &run, const agiru::gen::Objects &objects) {
  if (run.output.empty()) { return; }
  for (const auto &[key, ref] : objects.xmlports) {
    const std::string reachable = agiru::gen::Unprefixed(ref.identifier);
    const std::size_t colons = reachable.rfind("::");
    const std::string space =
        colons == std::string::npos ? "agiru" : "agiru::" + reachable.substr(0, colons);
    const std::string identifier =
        colons == std::string::npos ? reachable : reachable.substr(colons + 2);
    const auto number = ref.fields.find("id");
    std::string out = "// Generated from the xmlport\'s declaration. Do not edit.\n\n";
    out += "#pragma once\n\n";
    out += "#include \"meta/Ids.h\"\n";
    out += "#include \"runtime/Report.h\"\n\n";
    out += "#include <string_view>\n\n";
    out += "namespace " + space + " {\n\n";
    out += "class " + identifier + " : public ::agiru::XmlPort<" + identifier + "> {\npublic:\n";
    out += "  static constexpr XmlPortId kId{" + number->second + "};\n";
    out += "  static constexpr std::string_view kName{" +
           agiru::gen::Literal(ref.fields.at("name")) + "};\n";
    out += "};\n\n";
    out += "} // namespace " + space + "\n\n";
    out += "template <> struct agiru::XmlPortTraits<" + space + "::" + identifier + "> {\n";
    out += "  static constexpr XmlPortId kId{" + number->second + "};\n";
    out += "  static constexpr std::string_view kName{" +
           agiru::gen::Literal(ref.fields.at("name")) + "};\n};\n";
    Keep(run, Output{.directory = run.output, .relative = ref.header}, out);
  }
}

void IndexQueries(const Run &run, agiru::gen::Objects &objects) {
  for (const std::filesystem::path &path : SourcesEndingIn(run, ".Query.al")) {
    const agiru::gen::ObjectDeclaration declared =
        agiru::gen::DeclarationOf(Read(path), agiru::gen::ObjectKind::Query);
    if (!declared.found || declared.id == 0) { continue; }
    const std::string identifier = agiru::gen::Identifier(declared.name);
    objects.queries.insert_or_assign(
        agiru::gen::LowerKey(declared.name),
        agiru::gen::TableRef{
            .identifier = "::agiru::" + agiru::gen::NamespaceSuffix(declared.nameSpace) +
                          agiru::gen::ClassName(identifier, agiru::gen::ObjectKind::Query),
            .header =
                agiru::gen::OutputDirectory(declared.nameSpace, agiru::gen::ObjectKind::Query) +
                "/" + identifier + ".h",
            .fields = {{"id", std::to_string(declared.id)}, {"name", declared.name}},
            .procedures = {}});
  }
}

void WriteQueries(Run &run, const agiru::gen::Objects &objects) {
  if (run.output.empty()) { return; }
  for (const auto &[key, ref] : objects.queries) {
    const std::string reachable = agiru::gen::Unprefixed(ref.identifier);
    const std::size_t colons = reachable.rfind("::");
    const std::string space =
        colons == std::string::npos ? "agiru" : "agiru::" + reachable.substr(0, colons);
    const std::string identifier =
        colons == std::string::npos ? reachable : reachable.substr(colons + 2);
    const auto number = ref.fields.find("id");
    std::string out = "// Generated from the query\'s declaration. Do not edit.\n\n";
    out += "#pragma once\n\n";
    out += "#include \"meta/Ids.h\"\n";
    out += "#include \"runtime/Report.h\"\n\n";
    out += "#include <string_view>\n\n";
    out += "namespace " + space + " {\n\n";
    out += "class " + identifier + " : public ::agiru::Query<" + identifier + "> {\npublic:\n";
    out += "  static constexpr QueryId kId{" + number->second + "};\n";
    out += "  static constexpr std::string_view kName{" +
           agiru::gen::Literal(ref.fields.at("name")) + "};\n";
    out += "};\n\n";
    out += "} // namespace " + space + "\n\n";
    out += "template <> struct agiru::QueryTraits<" + space + "::" + identifier + "> {\n";
    out += "  static constexpr QueryId kId{" + number->second + "};\n";
    out += "  static constexpr std::string_view kName{" +
           agiru::gen::Literal(ref.fields.at("name")) + "};\n};\n";
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
    CountAttributes(*unit, gathered.attributes, gathered.deprecatedScopes);
    NotePropertiesOf(*unit, gathered.properties);
    CheckNormal(*unit, "codeunit", agiru::gen::IsTestCodeunit(*unit), gathered.refused);
    NoteOptions(unit->variables, unit->procedures, gathered.options);
    if (run.output.empty()) { continue; }
    try {
      const std::string relative = std::filesystem::relative(path, run.root).string();
      const agiru::gen::CodeunitHeader header = agiru::gen::WriteCodeunit(*unit, relative, objects);
      Absorb(gathered.refused, agiru::gen::Refused(*unit));
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

Counted Stubs(std::string &text,
              const agiru::gen::DotNetUse &use,
              bool skipRebuilt,
              bool alObjects = false) {
  Counted counted;
  for (const auto &[type, named] : use) {
    if (skipRebuilt && Rebuilt().contains(type)) { continue; }
    ++counted.types;
    text += "\nstruct ";
    text += type;
    text +=
        alObjects ? " : ::agiru::dotnet::AbsentObject {\n" : " : ::agiru::dotnet::AbsentType {\n";
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
    text += "  [[nodiscard]] const ::agiru::dotnet::RefusedResult *begin() const {\n";
    text += "    return ::agiru::dotnet::Refused{{.type = \"" + type +
            "\", .member = \"GetEnumerator\"}}();\n  }\n";
    text += "  [[nodiscard]] const ::agiru::dotnet::RefusedResult *end() const { return begin(); "
            "}\n";
    text += "  template <typename T> auto &operator=(const T &) {\n";
    text += "    ::agiru::dotnet::Refused{{.type = \"" + type + "\", .member = \"=\"}}();\n";
    text += "    return *this;\n  }\n";

    text += "};\n";
  }
  return counted;
}

void WriteOptions(const std::filesystem::path &out, const OptionsInScope &options) {
  if (out.empty()) { return; }
  std::string text = "// Generated from every option AL declares by its members. Do not edit.\n";
  text += "// Do not edit.\n\n#pragma once\n\n#include \"meta/EnumDef.h\"\n";
  text += "#include \"type/Option.h\"\n\n#include <array>\n#include <cstdint>\n\n";
  for (const auto &[qualified, members] : options) {
    const std::string name = qualified.substr(qualified.rfind(':') + 1);
    const std::vector<std::string> names = agiru::gen::EnumeratorNames(members);
    text += "namespace agiru::options {\n\nenum class " + name + " : std::int32_t {\n";
    for (std::size_t i = 0; i < names.size(); ++i) {
      text += "  " + names[i] + " = " + std::to_string(i) + ",\n";
    }
    text += "};\n\n} // namespace agiru::options\n\n";
    text += "template <> struct agiru::OptionTraits<agiru::options::" + name + "> {\n";
    text += "  static constexpr std::array<EnumValueDef, " + std::to_string(members.size()) +
            "> kValues{{\n";
    for (std::size_t i = 0; i < members.size(); ++i) {
      text += "      EnumValueDef{.ordinal = " + std::to_string(i) +
              ", .name = " + agiru::gen::Literal(names[i]) +
              ", .caption = " + agiru::gen::Literal(members[i]) + "},\n";
    }
    text += "  }};\n};\n\n";
  }
  const std::filesystem::path path = out / "shared" / "options" / "Types.h";
  std::filesystem::create_directories(path.parent_path());
  std::ofstream file(path, std::ios::binary);
  file << text;
  std::println("options   {} enumeration(s) named by their members", options.size());
}

void WriteAbsent(const std::filesystem::path &out,
                 const agiru::gen::DotNetUse &dotnet,
                 const agiru::gen::DotNetUse &absent) {
  if (out.empty()) { return; }
  std::string text = "// Generated from every AL body that names a type this run does not have.\n";
  text += "// Do not edit.\n\n#pragma once\n\n#include \"dotnet/Refused.h\"\n";
  text += "\nnamespace agiru::dotnet {\n";
  const Counted net = Stubs(text, dotnet, true);
  text += "\n} // namespace agiru::dotnet\n\nnamespace agiru::absent {\n";
  const Counted objects = Stubs(text, absent, false, true);
  text += "\n} // namespace agiru::absent\n";

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
  into.moved += one.moved;
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
  std::map<std::string, std::size_t> declaredOnly;

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
    IndexXmlPorts(run, objects);
    IndexQueries(run, objects);
    WriteReports(run, objects);
    WriteXmlPorts(run, objects);
    WriteQueries(run, objects);
    Enums heldEnums;
    ScanEnums(run, enums, store, index, heldEnums);
    objects.enums = index;
    Tables &parsedTables = held.emplace_back(IndexTables(run, tables, objects));
    extensions.emitted += MergeExtensions(store, parsedTables);
    for (const agiru::al::TableObject &table : parsedTables.objects) {
      NoteFieldEnums(table, objects.enums, objects.fieldEnums);
    }
    const Interfaces parsedInterfaces = IndexInterfaces(run, interfaces, objects);
    for (const agiru::al::TableObject &table : parsedTables.objects) {
      everyTable.insert_or_assign(agiru::gen::LowerKey(table.name), &table);
    }
    Pages parsed = IndexPages(run, pages, objects);
    extensions.emitted += MergePageExtensions(store, parsed);
    for (const agiru::al::PageObject &page : parsed.objects) {
      const auto found = objects.pages.find(agiru::gen::LowerKey(page.name));
      if (found != objects.pages.end()) {
        found->second.fields = agiru::gen::ControlIdentifiers(page, objects);
      }
    }
    agiru::gen::NoteObjectNames(objects);
    ScanCodeunits(run, codeunits, gathered, objects, unresolvedTables);
    for (const agiru::al::TableObject &table : parsedTables.objects) {
      NoteOptions(table.variables, table.procedures, gathered.options);
      for (const agiru::al::FieldDecl &field : table.fields) {
        const agiru::al::Property *members = agiru::al::Find(field.properties, "OptionMembers");
        if (members == nullptr) { continue; }
        const std::vector<std::string> listed = agiru::al::ListValue(*members);
        gathered.options.insert_or_assign(agiru::gen::OptionContentName(listed), listed);
      }
    }
    for (const agiru::al::PageObject &page : parsed.objects) {
      NoteOptions(page.variables, page.procedures, gathered.options);
    }
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
    for (const auto &[kind, found] : DeclaredOnlyKinds(run)) { declaredOnly[kind] += found; }
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
  if (store.unplaced != 0) {
    std::println("unplaced   {} page-extension operation(s) name a control the base page does not "
                 "declare, and stay where they were written (board:0033)",
                 store.unplaced);
  }
  std::map<std::string, std::size_t> orphans;
  for (const auto &[name, total] : store.held) {
    const auto taken = store.consumed.find(name);
    if (taken == store.consumed.end()) { orphans.insert_or_assign(name, total); }
  }
  std::size_t silentProperties = 0;
  std::size_t silentAttributes = 0;
  {
    std::size_t counted = 0;
    std::size_t decided = 0;
    std::vector<std::pair<std::string, std::size_t>> silent;
    std::vector<std::pair<std::string, std::size_t>> partial;
    for (const auto &[name, found] : gathered.properties) {
      counted += found;
      if (std::ranges::find(kTranslatedProperties, name) !=
          std::ranges::end(kTranslatedProperties)) {
        continue;
      }
      const std::string_view bare = PropertyName(name);
      if (std::ranges::find_if(kPartlyTranslatedProperties, [&name](const auto &known) {
            return known.first == name;
          }) != kPartlyTranslatedProperties.end()) {
        partial.emplace_back(name, found);
        continue;
      }
      if (agiru::gen::RefusedByName(bare)) { continue; }
      if (std::ranges::find_if(kDroppedProperties, [bare](const auto &known) {
            return known.first == bare;
          }) != kDroppedProperties.end()) {
        decided += found;
        continue;
      }
      silent.emplace_back(name, found);
    }
    std::ranges::sort(silent, [](const auto &a, const auto &b) { return a.second > b.second; });
    std::size_t dropped = 0;
    for (const auto &[name, found] : silent) { dropped += found; }
    std::println("properties {} declaration(s) of {} kind(s); {} dropped by decision, {} of {} "
                 "kind(s) read and dropped in silence (board:0067)",
                 counted,
                 gathered.properties.size(),
                 decided,
                 dropped,
                 silent.size());
    for (std::size_t i = 0; i < silent.size() && i < 60; ++i) {
      std::println("          {:>7} x {}", silent[i].second, silent[i].first);
    }
    if (dropped != 0) { silentProperties = dropped; }
    for (const auto &[name, found] : partial) {
      const auto known = std::ranges::find_if(
          kPartlyTranslatedProperties, [&name](const auto &one) { return one.first == name; });
      std::println("partly    {:>7} x {} -- {}", found, name, known->second);
    }
  }
  for (const auto &[what, found] : gathered.contradictions) {
    std::println("declared  {:>5} x {} -- carried as declared, and it cannot mean what it says "
                 "(board:0339)",
                 found,
                 what);
  }
  if (!declaredOnly.empty()) {
    std::size_t total = 0;
    for (const auto &[kind, found] : declaredOnly) { total += found; }
    std::println("declared  {} object(s) whose kind carries its NUMBER, NAME and a refusing "
                 "surface, and no body (board:0034)",
                 total);
    std::vector<std::pair<std::string, std::size_t>> ranked(declaredOnly.begin(),
                                                            declaredOnly.end());
    std::ranges::sort(ranked, [](const auto &a, const auto &b) { return a.second > b.second; });
    for (const auto &[kind, found] : ranked) { std::println("          {:>5} x {}", found, kind); }
  }
  if (!untranslated.empty()) {
    std::size_t total = 0;
    for (const auto &[kind, found] : untranslated) { total += found; }
    std::println(
        "untranslated {} object(s) in scope whose kind has no generator at all (board:0034)",
        total);
    std::vector<std::pair<std::string, std::size_t>> ranked(untranslated.begin(),
                                                            untranslated.end());
    std::ranges::sort(ranked, [](const auto &a, const auto &b) { return a.second > b.second; });
    for (const auto &[kind, found] : ranked) { std::println("          {:>5} x {}", found, kind); }
  }
  {
    std::map<std::string, std::size_t> refused;
    for (const agiru::gen::RefusedProperty &found : gathered.refused) { ++refused[found.property]; }
    std::println("refused   {} property declaration(s) the transpiler will not act on",
                 gathered.refused.size());
    std::vector<std::pair<std::string, std::size_t>> ranked(refused.begin(), refused.end());
    std::ranges::sort(ranked, [](const auto &a, const auto &b) { return a.second > b.second; });
    for (const auto &[name, count] : ranked) { std::println("          {:>5} x {}", count, name); }
    for (const agiru::gen::RefusedProperty &found : gathered.refused) {
      std::println("          {} in {}", found.property, found.where);
    }
  }
  {
    std::size_t dropped = 0;
    std::vector<std::pair<std::string, std::size_t>> ranked;
    std::size_t acknowledged = 0;
    for (const auto &[name, count] : gathered.attributes) {
      if (std::ranges::find(kActedOnAttributes, name) != kActedOnAttributes.end()) { continue; }
      if (std::ranges::find_if(kAcknowledgedAttributes, [&](const auto &known) {
            return known.first == name;
          }) != kAcknowledgedAttributes.end()) {
        acknowledged += count;
        continue;
      }
      dropped += count;
      ranked.emplace_back(name, count);
    }
    std::ranges::sort(ranked, [](const auto &a, const auto &b) { return a.second > b.second; });
    if (dropped != 0) { silentAttributes = dropped; }
    std::println("attributes acted on {} of {} kind(s) declared, {} declaration(s) acknowledged as "
                 "no-ops; {} declaration(s) of {} kind(s) are read and dropped (board:0190)",
                 gathered.attributes.size() - ranked.size() - kAcknowledgedAttributes.size(),
                 gathered.attributes.size(),
                 acknowledged,
                 dropped,
                 ranked.size());
    for (const auto &[dead, found] : gathered.deprecatedScopes) {
      std::println(
          "          {:>5} x [Scope] {} -- deprecated in runtime 4.0 (board:0216)", found, dead);
    }
    for (const auto &[name, count] : ranked) { std::println("          {:>5} x {}", count, name); }
  }
  ReportUnresolved("extension(s)", "object(s) no app declares", orphans);
  if (allCodeunits.emitted != 0) {
    std::println("moved     {} table(s) and extension(s) declare ObsoleteState = Moved and are "
                 "left to the app their MovedTo names",
                 allTables.moved);
    std::println("emitted   {} of {} codeunits; the rest name what the runtime cannot do yet",
                 allCodeunits.emitted,
                 allCodeunits.parsed);
  }
  ReportUnresolved("enum(s)", "field(s)", unresolvedEnums);
  WriteAbsent(job.output, gathered.dotnet, gathered.absent);
  WriteOptions(job.output, gathered.options);
  ReportUnresolved("table(s)", "declaration(s)", unresolvedTables);
  Cluster(failures);
  if (!refusals.empty()) {
    std::println("");
    std::println("refused   what parses and cannot be written yet");
    Cluster(refusals);
  }
  if (silentAttributes != 0) {
    std::println("");
    std::println("ABORT     {} attribute declaration(s) are read and dropped, and the count is 0 "
                 "(board:0190)",
                 silentAttributes);
    std::println("          Every one belongs in `kActedOnAttributes` with a generator behind it "
                 "or in");
    std::println("          `kAcknowledgedAttributes` with the reason it is a no-op here.");
    return 1;
  }
  if (silentProperties != 0) {
    std::println("");
    std::println("ABORT     {} property declaration(s) are read and dropped in silence, and the "
                 "count is 0 (board:0067)",
                 silentProperties);
    std::println("          Every one belongs in `kTranslatedProperties` with a member behind it, "
                 "in");
    std::println(
        "          `kDroppedProperties` with a reason, or in `kPartlyTranslatedProperties` "
        "with what");
    std::println("          reaches the metadata and what does not.");
    return 1;
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

#include "Apps.h"
#include "Check.h"
#include "Scope.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

using agiru::gen::ObjectKind;
using agiru::gen::OutputDirectory;
using agiru::gen::Scope;

namespace {

Scope Load() {
  return Scope::FromFile(std::filesystem::path(AGIRU_SOURCE_DIR) / "src/gen/scope.json");
}

void TheWhitelistIsRead(const Scope &scope) {
  CHECK_TRUE("the include list is read", scope.IncludeCount() > 20);
  CHECK_TRUE("and the exclude list", scope.ExcludeCount() > 20);
}

void ANamespaceIsInOrOutByItsLongestMatch(const Scope &scope) {
  // Microsoft is whitelisted whole: this is a full clone, not a subset.
  CHECK_TRUE("the BaseApp root is in", scope.Contains("Microsoft"));
  CHECK_TRUE("and everything under it", scope.Contains("Microsoft.Projects.Resources.Pricing"));
  // A carve-out is more specific than the root, so it wins.
  CHECK_TRUE("a glue carve-out is out", !scope.Contains("Microsoft.Integration.Dataverse"));
  // And a whitelisted sub of a carved-out branch wins again, because it is longer still.
  CHECK_TRUE("a re-included sub of a carve-out is in",
             scope.Contains("Microsoft.Integration.Entity"));
  // A namespace no rule covers is OUT: the default of a whitelist is exclusion.
  CHECK_TRUE("an unknown root is out", scope.Contains("Contoso.Something") == false);
  // The match is on a DOT BOUNDARY, so a namespace that merely starts with the same letters is out.
  CHECK_TRUE("a prefix that is not a boundary does not match",
             scope.Contains("MicrosoftSomethingElse") == false);
  // Case does not decide anything: AL identifiers are case-insensitive.
  CHECK_TRUE("the match ignores case", scope.Contains("MICROSOFT.Sales.Document"));
  // An object with no namespace lands in core and is always needed.
  CHECK_TRUE("no namespace is in scope", scope.Contains(""));
}

void TheNamespaceDecidesTheDirectory() {
  CHECK_TEXT("the root prefix is dropped and the rest is a path",
             OutputDirectory("Microsoft.Projects.Resources.Pricing", ObjectKind::Table),
             "projects/resources/pricing/table");
  CHECK_TEXT("PascalCase becomes snake_case",
             OutputDirectory("Microsoft.Finance.GeneralLedger.Account", ObjectKind::Table),
             "finance/general_ledger/account/table");
  CHECK_TEXT(
      "the bare root is core", OutputDirectory("Microsoft", ObjectKind::Codeunit), "core/codeunit");
  CHECK_TEXT("and so is no namespace at all", OutputDirectory("", ObjectKind::Table), "core/table");
  // A platform namespace keeps its own root, because System is not Microsoft.
  CHECK_TEXT("a System namespace keeps its root",
             OutputDirectory("System.Utilities", ObjectKind::Codeunit),
             "system/utilities/codeunit");
}

void NamespaceSelectionUsesTheSameTieRule() {
  agiru::gen::TranspileScope rules;
  rules.include = {"Contoso", "Contoso.Hidden.Exposed"};
  rules.exclude = {"CONTOSO", "Contoso.Hidden"};
  CHECK_TRUE("an equal include/exclude match is excluded", !agiru::gen::Holds(rules, "Contoso"));
  CHECK_TRUE("an equal include/exclude match excludes descendants",
             !agiru::gen::Holds(rules, "Contoso.Public"));
  CHECK_TRUE("an equal include/exclude match ignores namespace case",
             !agiru::gen::Holds(rules, "cOnToSo.PUBLIC"));
  CHECK_TRUE("a longer include overrides a shorter exclusion",
             agiru::gen::Holds(rules, "CONTOSO.hidden.Exposed.Child"));
  CHECK_TRUE("the longer include requires a dot boundary",
             !agiru::gen::Holds(rules, "Contoso.Hidden.ExposedThing"));
  CHECK_TRUE("a prefix alone does not select an unrelated root",
             !agiru::gen::Holds(rules, "ContosoOther"));
  CHECK_TRUE("namespace-less objects keep their existing area-selection contract",
             agiru::gen::Holds(rules, ""));
}

void ConfiguredMatchersAgree(const Scope &scope) {
  const auto rules =
      agiru::gen::ReadScope(std::filesystem::path(AGIRU_SOURCE_DIR) / "src/gen/scope.json");
  std::vector<std::string> names = rules.include;
  names.insert(names.end(), rules.exclude.begin(), rules.exclude.end());
  for (const std::string &name : names) {
    for (const std::string_view tail : {"", ".Child", "Other"}) {
      const std::string tested = name + std::string(tail);
      CHECK_TRUE("both configured matchers select the same namespace",
                 scope.Contains(tested) == agiru::gen::Holds(rules, tested));
    }
  }
}

} // namespace

int main() {
  return gate::Run("GenScope", [] {
    const Scope scope = Load();
    TheWhitelistIsRead(scope);
    ANamespaceIsInOrOutByItsLongestMatch(scope);
    TheNamespaceDecidesTheDirectory();
    NamespaceSelectionUsesTheSameTieRule();
    ConfiguredMatchersAgree(scope);
  });
}

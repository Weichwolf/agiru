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
  return Scope::FromFile(std::filesystem::path(AGIRU_SOURCE_DIR) / "scope.json");
}

void TheWhitelistIsRead(const Scope &scope) {
  CHECK_TRUE("the include list is read", scope.IncludeCount() > 20);
  CHECK_TRUE("and the exclude list", scope.ExcludeCount() > 20);
}

void ANamespaceIsInOrOutByItsLongestMatch(const Scope &scope) {
  CHECK_TRUE("the BaseApp root is in", scope.Contains("Microsoft"));
  CHECK_TRUE("and everything under it", scope.Contains("Microsoft.Projects.Resources.Pricing"));
  // A carve-out is more specific than the root, so it wins.
  CHECK_TRUE("an unmatched integration branch is out",
             !scope.Contains("Microsoft.Integration.Unclassified"));
  CHECK_TRUE("the legacy Dataverse dependency remains visible",
             scope.Contains("Microsoft.Integration.Dataverse"));
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

void ProductBoundariesPreserveCore(const Scope &scope) {
  const auto rules = agiru::gen::ReadScope(std::filesystem::path(AGIRU_SOURCE_DIR) / "scope.json");
  for (const std::string_view name : {"System.Integration.Graph",
                                      "System.Integration.Graph.Child",
                                      "system.integration.graph",
                                      "System.Azure.Unclassified",
                                      "Microsoft.Booking",
                                      "System.Azure",
                                      "System.Integration.PowerBI",
                                      "System.Integration.Graph"}) {
    CHECK_TRUE("Microsoft cloud integration is excluded", !scope.Contains(name));
    CHECK_TRUE("transpiler excludes the same cloud integration", !agiru::gen::Holds(rules, name));
  }
  for (const std::string_view name : {"Microsoft.Finance.GeneralLedger",
                                      "Microsoft.Sales.Document",
                                      "System.Security.AccessControl",
                                      "System.RestClient",
                                      "System.Email"}) {
    CHECK_TRUE("core ERP, permissions and generic protocols remain", scope.Contains(name));
    CHECK_TRUE("transpiler retains the same core contract", agiru::gen::Holds(rules, name));
  }
  for (const std::string_view name : {"Microsoft.Integration.Graph",
                                      "Microsoft.Integration.Graph.Child",
                                      "microsoft.integration.graph",
                                      "Microsoft.API",
                                      "Microsoft.CRM.Outlook"}) {
    CHECK_TRUE("local API data helpers and legacy core dependencies remain", scope.Contains(name));
    CHECK_TRUE("the transpiler uses the same dependency selection", agiru::gen::Holds(rules, name));
  }
  CHECK_TRUE(
      "the stale duplicate scope file is absent",
      !std::filesystem::exists(std::filesystem::path(AGIRU_SOURCE_DIR) / "src/gen/scope.json"));
  const auto reason = agiru::gen::ProductExclusion(
      rules, "Layers/W1/Tests/SMB/O365RoleCenterNotifications.Codeunit.al");
  CHECK_TRUE("the explicit license/SaaS test exclusion has a reason", reason.has_value());
  if (reason) {
    CHECK_TEXT("the approved reason is retained", *reason, "licensing-and-microsoft-cloud");
  }
  CHECK_TRUE(
      "O365 names alone never exclude ERP tests",
      !agiru::gen::ProductExclusion(rules, "Layers/W1/Tests/SMB/O365TrialBalance.Codeunit.al"));
  for (const std::string_view path :
       {"System Application/App/Tenant License State/src/TenantLicenseStateImpl.Codeunit.al",
        "System Application/App/Azure AD Tenant/src/AzureADTenantImpl.Codeunit.al"}) {
    CHECK_TRUE("pure commercial/service implementations are explicitly excluded",
               agiru::gen::ProductExclusion(rules, path).has_value());
  }
  for (const std::string_view path :
       {"System Application/App/Tenant License State/src/TenantLicenseState.Codeunit.al",
        "System Application/App/Azure AD Tenant/src/AzureADTenant.Codeunit.al",
        "System Application/App/User Settings/src/UserSettingsImpl.Codeunit.al",
        "Layers/W1/BaseApp/System/Notifications/MyPlatformNotifications.Codeunit.al"}) {
    CHECK_TRUE("mixed callers remain required rather than being silently retired",
               !agiru::gen::ProductExclusion(rules, path));
  }
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
  const auto rules = agiru::gen::ReadScope(std::filesystem::path(AGIRU_SOURCE_DIR) / "scope.json");
  std::vector<std::string> names = rules.include;
  names.insert(names.end(), rules.exclude.begin(), rules.exclude.end());
  names.emplace_back("Microsoft.Integration.Graph");
  for (const std::string_view historical :
       {"Microsoft.CRM.Outlook", "Microsoft.API", "System.Privacy", "System.Telemetry"}) {
    names.emplace_back(historical);
  }
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
    ProductBoundariesPreserveCore(scope);
    TheNamespaceDecidesTheDirectory();
    NamespaceSelectionUsesTheSameTieRule();
    ConfiguredMatchersAgree(scope);
  });
}

#include "Apps.h"
#include "Ast.h"
#include "BodyWriter.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "PageSelection.h"
#include "PageWriter.h"
#include "Parser.h"

#include <filesystem>
#include <stdexcept>

namespace {

agiru::al::PageObject Host() {
  return agiru::al::ParsePage(R"(namespace Fixture;
page 50300 "Feature Host"
{
    layout { area(content) {
        part("Feature Part Ω"; "Remote Panel") { }
        part(Required; "Business Chart") { }
        part(Missing; "Unselected ERP") { }
        usercontrol(Chart; BusinessChart) { }
    } }
    var Value: Integer;
    procedure OpenFeatures()
    begin
        Value := 1;
        CurrPage."Feature Part Ω".Page.Publish(Bump(2), Bump(3));
        CurrPage.Required.Page.Touch();
        if Value > 0 then
            CurrPage."Feature Part Ω".Page.Publish(Bump(4), Bump(5));
        CurrPage.Missing.Page.Touch();
    end;
    procedure Consume(): Integer
    begin
        exit(CurrPage."Feature Part Ω".Page.ReadValue(Bump(6)));
    end;
    procedure Conditional()
    begin
        if CurrPage."Feature Part Ω".Page.ReadValue(Bump(7)) = 1 then Value := 2;
    end;
    procedure Bare()
    begin
        "Feature Part Ω".Page.Publish(Bump(8), Bump(9));
    end;
    local procedure Bump(Number: Integer): Integer
    begin Value := Value * 10 + Number; exit(Value); end;
})");
}

agiru::gen::TranspileScope Policy() {
  agiru::gen::TranspileScope scope;
  scope.include = {"Fixture"};
  scope.exclude = {"Fixture.Cloud"};
  scope.productExclude.push_back({.reason = "microsoft-cloud", .source = "cloud/"});
  return scope;
}

void Index(agiru::gen::Objects &objects, const agiru::gen::TranspileScope &scope) {
  const auto target =
      agiru::al::ParsePage(R"(namespace Fixture.Cloud; page 50301 "Remote Panel" { })");
  agiru::gen::IndexProductPage(scope, "cloud/Remote.Page.al", target, objects);
  objects.pages["business chart"].identifier = "::agiru::Fixture::BusinessChart_Page";
  objects.pages["business chart"].header = "fixture/page/BusinessChart.h";
}

void NamespaceOmissionsCannotAuthorizePruning() {
  auto scope = Policy();
  scope.productExclude.clear();
  agiru::gen::Objects objects;
  Index(objects, scope);
  auto page = Host();
  const auto original = agiru::gen::WriteSource(page, "fixture/Host.Page.al", objects, nullptr);
  CHECK_TRUE("an omitted cloud namespace alone cannot authorize product-part removal",
             agiru::gen::SelectProductPageParts(page, objects).empty());
  CHECK_TRUE("an omitted cloud part retains its exact explicit refusal",
             original.contains("AbsentControl(\"Feature Part Ω.Page.Publish\")"));
  CHECK_TRUE("unselected ERP parts remain explicitly refused",
             original.contains("AbsentControl(\"Missing.Page.Touch\")"));
}

void ApprovedPartsLeaveCoreAndValuesVisible() {
  agiru::gen::Objects objects;
  Index(objects, Policy());
  auto page = Host();
  const auto raw = page;
  const auto rows = agiru::gen::SelectProductPageParts(page, objects);
  CHECK_TRUE("product selection counts one part and two discarded direct calls", rows.size() == 3);
  CHECK_TRUE("product selection preserves raw procedure identities and bodies",
             page.procedures.size() == raw.procedures.size() &&
                 page.procedures.front().body.size() == raw.procedures.front().body.size());
  CHECK_TRUE("the excluded part is removed but charts and unavailable ERP remain",
             page.layout.front().children.size() == 3 &&
                 page.layout.front().children.front().name == "Required");
  CHECK_TRUE("part receipt retains original Unicode alias source target and approved reason",
             rows.front().control == "Feature Part Ω" &&
                 rows.front().target == "Page 50301 Remote Panel" &&
                 rows.front().targetSource == "cloud/Remote.Page.al" &&
                 rows.front().reason == "microsoft-cloud");
  CHECK_TRUE("nested discarded calls retain raw statement locations and members",
             rows.back().location == "page/OpenFeatures/3/body/0" &&
                 rows.back().member == "Publish");
  objects.pages["feature host"].fields = agiru::gen::ControlIdentifiers(page, objects);
  objects.pages["feature host"].parts = agiru::gen::PartPages(page);
  const auto header = agiru::gen::WritePage(page, "fixture/Host.Page.al", objects);
  CHECK_TRUE("excluded parts have no public member or page-control metadata",
             !header.text.contains("Feature Part Ω"));
  CHECK_TRUE("generic chart parts and add-ins are not product-excluded",
             header.text.contains("BusinessChart_Page") && header.text.contains("BusinessChart"));
  const auto body = agiru::gen::WriteSource(page, "fixture/Host.Page.al", objects, nullptr);
  CHECK_TRUE("excluded direct calls preserve argument evaluation in source order",
             body.contains(
                 "static_cast<void>(Bump(2)), static_cast<void>(Bump(3)), static_cast<void>(0)"));
  CHECK_TRUE("a required page call still executes normally",
             body.contains("Required.Page().Touch()"));
  CHECK_TRUE("missing ERP and consumed cloud calls still refuse, never fabricate values",
             body.contains("AbsentControl(\"Missing.Page.Touch\")") &&
                 body.contains("AbsentControl(\"Feature Part Ω.Page.ReadValue\")"));
  CHECK_TRUE("unqualified receivers are not assumed to be CurrPage",
             body.contains("AbsentControl(\"Feature Part Ω.Page.Publish\")"));
}

void ApprovedTargetsAreBoundedAndUnambiguous() {
  agiru::gen::Objects objects;
  const auto target = agiru::al::ParsePage(R"(page 50301 "Remote Panel" { })");
  agiru::gen::IndexProductPage(Policy(), "cloud-like/Remote.Page.al", target, objects);
  CHECK_TRUE("a module-prefix lookalike does not authorize a page exclusion",
             objects.productExcludedPages.empty());
  agiru::gen::IndexProductPage(
      Policy(), "cloud/Remote.Page.al", target, objects, agiru::gen::SourceDomain::SystemSymbols);
  CHECK_TRUE("a BCApps product rule does not authorize a System-domain exclusion",
             objects.productExcludedPages.empty());
  Index(objects, Policy());
  CHECK_TRUE("approved targets resolve names namespace names and exact page IDs",
             objects.productExcludedPages.contains("remote panel") &&
                 objects.productExcludedPages.contains("fixture.cloud.remote panel") &&
                 objects.productExcludedPages.contains("50301"));
  bool rejected = false;
  try {
    agiru::gen::IndexProductPage(Policy(), "cloud/Other.Page.al", target, objects);
  } catch (const std::runtime_error &) { rejected = true; }
  CHECK_TRUE("duplicate target identities from another source refuse", rejected);
  objects.pages["remote panel"].id = target.id;
  auto page = Host();
  rejected = false;
  try {
    static_cast<void>(agiru::gen::SelectProductPageParts(page, objects));
  } catch (const std::runtime_error &) { rejected = true; }
  CHECK_TRUE("a selected/excluded page collision refuses rather than pruning core", rejected);
}

void RootPolicyPreservesGenericCharts() {
  const auto scope = agiru::gen::ReadScope(std::filesystem::path(AGIRU_SOURCE_DIR) / "scope.json");
  CHECK_TRUE(
      "Power BI cloud sources have a bounded explicit exclusion",
      agiru::gen::ProductExclusion(
          scope,
          "Layers/W1/BaseApp/Modules/System/PowerBI/Embedding/PowerBIEmbeddedReportPart.Page.al") ==
          "microsoft-cloud");
  CHECK_TRUE("a similarly named module is not product-excluded",
             !agiru::gen::ProductExclusion(
                 scope, "Layers/W1/BaseApp/Modules/System/PowerBIReports/Core.Page.al"));
  CHECK_TRUE("generic visualization remains in scope",
             agiru::gen::Holds(scope, "System.Visualization"));
}

}

int main() {
  return gate::Run("GenPageSelection", [] {
    NamespaceOmissionsCannotAuthorizePruning();
    ApprovedPartsLeaveCoreAndValuesVisible();
    ApprovedTargetsAreBoundedAndUnambiguous();
    RootPolicyPreservesGenericCharts();
  });
}

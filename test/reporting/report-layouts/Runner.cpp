#include "meta/Ids.h"
#include "meta/ReportLayoutDef.h"

#include "Check.h"
#include "test/reporting/report/LayoutContract.h"

#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace {

using Traits = agiru::ReportTraits<agiru::Test::Reporting::LayoutContract_Report>;
constexpr auto kLayouts = Traits::kLayouts;
constexpr agiru::ReportId kReport{50080};
constexpr agiru::ReportExtensionId kExtension{50081};
constexpr std::string_view kAppId = "12345678-1234-5678-9012-123456789012";
constexpr std::string_view kAddonId = "98765432-1234-5678-9012-123456789012";
constexpr std::size_t kCaptionTokenCount = 5;

static_assert(kLayouts.size() == 3);
static_assert(
    std::is_same_v<decltype(Traits::kLayouts), const std::span<const agiru::ReportLayoutDef>>);
static_assert(!std::is_convertible_v<agiru::ReportId, agiru::ReportExtensionId>);

const agiru::ReportLayoutPropertyDef &Property(const agiru::ReportLayoutDef &layout,
                                               std::string_view name) {
  const auto found =
      std::ranges::find(layout.properties, name, &agiru::ReportLayoutPropertyDef::name);
  if (found == layout.properties.end()) { throw std::runtime_error("missing layout property"); }
  return *found;
}

void BaseDeclarationSurvivesEmission() {
  const auto &layout = kLayouts.front();
  CHECK_TEXT("base layout remains first", layout.name, "Original Layout");
  CHECK_TRUE("base report owns its layout", layout.origin.report == kReport);
  CHECK_TRUE("base layout is not extension-owned", layout.origin.extension.Value() == 0);
  CHECK_TEXT("declaring name is emitted", layout.origin.name, "Layout Contract");
  CHECK_TEXT("AL namespace is not its shortened C++ namespace",
             layout.origin.nameSpace,
             "Microsoft.Test.Reporting");
  CHECK_TEXT("app identity is emitted", layout.origin.appId, kAppId);
  CHECK_TEXT("source provenance is root-relative",
             layout.origin.source,
             "Fixture/LayoutContract.Report.al");
  CHECK_TEXT("declared type reaches metadata", Property(layout, "Type").text, "Word");
  CHECK_TEXT(
      "asset path is preserved", Property(layout, "LayoutFile").text, "Layouts\\Original.docx");
  CHECK_TEXT(
      "summary is not dropped", Property(layout, "Summary").text, "A layout, not a dataset dump");
  CHECK_TEXT(
      "declared obsoletion state is retained", Property(layout, "ObsoleteState").text, "Pending");
  CHECK_TEXT("obsoletion reason is retained",
             Property(layout, "ObsoleteReason").text,
             "Use the replacement layout");
  CHECK_TEXT(
      "free-form obsoletion tag is not converted", Property(layout, "ObsoleteTag").text, "27.0");
  const auto tokens = Property(layout, "Caption").value;
  CHECK_TRUE("all localization tokens are emitted", tokens.size() == kCaptionTokenCount);
  if (tokens.size() != kCaptionTokenCount) { throw std::runtime_error("caption tokens were lost"); }
  CHECK_TRUE("caption keeps string lexical kind",
             tokens[0].kind == agiru::ReportLayoutTokenKind::String);
  CHECK_TEXT("escaped apostrophe survives", tokens[0].text, "Writer's original");
  CHECK_TRUE("comma keeps punctuation kind",
             tokens[1].kind == agiru::ReportLayoutTokenKind::Punctuation);
  CHECK_TEXT("comma survives", tokens[1].text, ",");
  CHECK_TRUE("localization key keeps identifier kind",
             tokens[2].kind == agiru::ReportLayoutTokenKind::Identifier);
  CHECK_TEXT("localization key survives", tokens[2].text, "Comment");
  CHECK_TEXT("localization assignment survives", tokens[3].text, "=");
  CHECK_TEXT("localization comment survives", tokens[4].text, "Keep localization metadata");
}

void ExtensionOwnershipSurvivesEmission() {
  const auto &theme = kLayouts[1];
  const auto &sheet = kLayouts[2];
  CHECK_TEXT("theme is appended", theme.name, "Theme Part");
  CHECK_TEXT("spreadsheet is appended", sheet.name, "Spreadsheet");
  for (const auto &layout : kLayouts.subspan(1)) {
    CHECK_TRUE("target report does not take extension ownership",
               layout.origin.report.Value() == 0);
    CHECK_TRUE("extension identity reaches metadata", layout.origin.extension == kExtension);
    CHECK_TEXT("extension name reaches metadata", layout.origin.name, "Extra Layouts");
    CHECK_TEXT("extension app owns the asset", layout.origin.appId, kAddonId);
    CHECK_TEXT("extension provenance reaches metadata",
               layout.origin.source,
               "Addon/ExtraLayouts.ReportExt.al");
  }
  CHECK_TEXT("Word theme subtype is preserved", Property(theme, "Subtype").text, "Theme");
  CHECK_TEXT(
      "template path is preserved", Property(theme, "LayoutFile").text, "Layouts\\Theme.dotx");
  CHECK_TEXT("Excel remains a separate layout format", Property(sheet, "Type").text, "Excel");
  CHECK_TEXT("layout-level worksheet override survives",
             Property(sheet, "ExcelLayoutMultipleDataSheets").text,
             "true");
  CHECK_TEXT("portable forward separators survive",
             Property(sheet, "LayoutFile").text,
             "Layouts/Spreadsheet.xlsx");
}

}

int main() {
  return gate::Run("Generated Report Layouts", [] {
    BaseDeclarationSurvivesEmission();
    ExtensionOwnershipSurvivesEmission();
  });
}

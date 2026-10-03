#include "meta/ReportLayoutDef.h"

#include "../reporting/report-layouts/Fixture.h"
#include "Ast.h"
#include "Check.h"
#include "Parser.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace {

constexpr std::size_t kSourceLayouts = 14;

static_assert(std::is_trivially_copyable_v<agiru::ReportLayoutDef>);
static_assert(std::is_trivially_copyable_v<agiru::ReportLayoutPropertyDef>);
static_assert(std::is_trivially_copyable_v<agiru::ReportLayoutTokenDef>);

std::string PropertyText(const agiru::al::ReportLayoutDecl &layout, std::string_view name) {
  const auto *property = agiru::al::Find(layout.properties, name);
  if (property == nullptr) {
    throw std::runtime_error("missing layout property " + std::string(name));
  }
  return property->text;
}

void ReportDeclarationSurvives() {
  const auto report = agiru::al::ParseReport(report_layout_fixture::kReport);
  CHECK_TRUE("one report layout survives", report.rendering.size() == 1);
  if (report.rendering.empty()) { throw std::runtime_error("report rendering was discarded"); }
  const auto &layout = report.rendering.front();
  CHECK_TEXT("quoted layout name is preserved", layout.name, "Original Layout");
  CHECK_TEXT("type survives", PropertyText(layout, "Type"), "Word");
  CHECK_TEXT("AL asset path is not rewritten",
             PropertyText(layout, "LayoutFile"),
             "Layouts\\Original.docx");
  CHECK_TEXT("unknown properties remain visible",
             PropertyText(layout, "FutureProperty"),
             "Keep unknown declarations visible");
  const auto *caption = agiru::al::Find(layout.properties, "Caption");
  CHECK_TRUE("caption keeps localization tokens", caption != nullptr && caption->value.size() > 1);
  if (caption == nullptr) { throw std::runtime_error("caption was discarded"); }
  CHECK_TEXT("escaped caption text survives", caption->value.front().text, "Writer's original");
  CHECK_TRUE("base identity is separate from extension identity",
             layout.owner.id == report.id && !layout.owner.extension);
  CHECK_TEXT("owner AL name survives", layout.owner.name, report.name);
  CHECK_TEXT("owner namespace survives", layout.owner.nameSpace, report.nameSpace);
  CHECK_TEXT("the selected default remains the report property",
             agiru::al::Find(report.properties, "DefaultRenderingLayout")->text,
             "Original Layout");
}

void ExtensionDeclarationSurvives() {
  const auto extension = agiru::al::ParseReportExtension(report_layout_fixture::kExtension);
  CHECK_TRUE("both extension layouts survive", extension.rendering.size() == 2);
  if (extension.rendering.size() != 2) {
    throw std::runtime_error("extension rendering was discarded");
  }
  CHECK_TEXT("native/ordinary target name is not fabricated", extension.extends, "Layout Contract");
  CHECK_TEXT(
      "Word subtype survives", PropertyText(extension.rendering.front(), "Subtype"), "Theme");
  CHECK_TRUE("owner is the extension, not the target report",
             extension.rendering.front().owner.id == extension.id &&
                 extension.rendering.front().owner.extension);
  CHECK_TEXT("extension namespace survives",
             extension.rendering.front().owner.nameSpace,
             extension.nameSpace);
  CHECK_TEXT(
      "Excel declaration survives", PropertyText(extension.rendering.back(), "Type"), "Excel");
  const auto unresolved = agiru::al::ParseReportExtension(report_layout_fixture::kUnresolved);
  CHECK_TRUE("unresolved target does not erase a layout", unresolved.rendering.size() == 1);
  if (unresolved.rendering.empty()) {
    throw std::runtime_error("unresolved rendering was discarded");
  }
  CHECK_TEXT("custom MIME type survives",
             PropertyText(unresolved.rendering.front(), "MimeType"),
             "application/x-agiru-layout-test");
}

void MalformedRenderingRefuses() {
  for (const char *const body : {"layout(Broken) { Type = Word; }",
                                 "layout(Broken) { LayoutFile = 'file.docx'; }",
                                 "layout(Broken) { Type = Unknown; LayoutFile = 'file'; }",
                                 "layout(Broken) { Type = Word; LayoutFile = ''; }",
                                 "layout(Broken) { Type = Word; LayoutFile = Variable; }",
                                 "layout(Broken) { Type = Word; LayoutFile = 'file'; ",
                                 "unknown(Broken) { Type = Word; LayoutFile = 'file'; }",
                                 R"(layout(Name) { Type = RDLC; LayoutFile = 'a'; }
                             layout(nAME) { Type = RDLC; LayoutFile = 'b'; })"}) {
    bool refused = false;
    try {
      static_cast<void>(
          agiru::al::ParseReport("report 50080 R { rendering { " + std::string(body) + " } }"));
    } catch (const agiru::al::ParseError &) { refused = true; }
    CHECK_TRUE("malformed or ambiguous rendering refuses rather than disappearing", refused);
  }
}

std::string ReadFile(const std::filesystem::path &path) {
  std::ifstream file(path);
  if (!file) { throw std::runtime_error("cannot read " + path.string()); }
  std::string text;
  for (std::string line; std::getline(file, line);) { text += line + '\n'; }
  if (!file.eof()) { throw std::runtime_error("incomplete read " + path.string()); }
  return text;
}

void ActualCompositeSourceSurvives(const std::filesystem::path &root) {
  const auto extension = agiru::al::ParseReportExtension(
      ReadFile(root / "Foundation/Reporting/CompositeLayout.ReportExt.al"));
  CHECK_TRUE("all fourteen raw Composite declarations survive",
             extension.rendering.size() == kSourceLayouts);
  CHECK_TRUE("native target is retained, not synthesized",
             extension.extends == "Tenant Report Defaults");
  std::size_t themes = 0;
  std::size_t headers = 0;
  for (const auto &layout : extension.rendering) {
    CHECK_TEXT("real layout remains Word", PropertyText(layout, "Type"), "Word");
    const auto subtype = PropertyText(layout, "Subtype");
    themes += subtype == "Theme" ? 1 : 0;
    headers += subtype == "HeaderFooter" ? 1 : 0;
    std::string file = PropertyText(layout, "LayoutFile");
    std::ranges::replace(file, '\\', '/');
    CHECK_TRUE("every declared real asset exists", std::filesystem::is_regular_file(root / file));
    CHECK_TRUE("real identity remains the extension",
               layout.owner.id == extension.id && layout.owner.extension);
  }
  CHECK_TRUE("three themes survive", themes == 3);
  CHECK_TRUE("eleven header/footer parts survive", headers == 11);
}

void WriteFixture(const std::filesystem::path &root, bool unknown) {
  std::filesystem::create_directories(root / "Fixture");
  std::filesystem::create_directories(root / "Addon");
  const auto write = [&root](std::string_view name, std::string_view text) {
    const auto path = root / name;
    std::ofstream file(path);
    file << text;
    file.close();
    if (!file) { throw std::runtime_error("cannot write " + path.string()); }
  };
  std::string report{report_layout_fixture::kReport};
  if (!unknown) {
    constexpr std::string_view line =
        "            FutureProperty = 'Keep unknown declarations visible';\n";
    const auto at = report.find(line);
    if (at == std::string::npos || report.find(line, at + line.size()) != std::string::npos) {
      throw std::runtime_error("unknown-property fixture marker is missing or ambiguous");
    }
    report.erase(at, line.size());
  }
  write("Fixture/LayoutContract.Report.al", report);
  write("Addon/ExtraLayouts.ReportExt.al", report_layout_fixture::kExtension);
  write("Addon/UnresolvedLayout.ReportExt.al", report_layout_fixture::kUnresolved);
  write(
      "Fixture/app.json",
      R"({"id":"12345678-1234-5678-9012-123456789012","name":"Layout fixture","publisher":"agiru","version":"1.0.0.0"})");
  write(
      "Addon/app.json",
      R"({"id":"98765432-1234-5678-9012-123456789012","name":"Layout addon","publisher":"agiru","version":"1.0.0.0"})");
  write("apps.json",
        R"({"apps":[{"name":"fixture","source":"Fixture"},{"name":"addon","source":"Addon"}]})");
}

}

int main(int argc, char **argv) {
  return gate::Run("ReportLayouts", [argc, argv] {
    ReportDeclarationSurvives();
    ExtensionDeclarationSurvives();
    MalformedRenderingRefuses();
    if (argc > 1) { ActualCompositeSourceSurvives(argv[1]); }
    if (argc > 2) { WriteFixture(argv[2], false); }
    if (argc > 3) { WriteFixture(argv[3], true); }
  });
}

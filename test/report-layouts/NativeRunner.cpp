#include "meta/Ids.h"
#include "meta/ReportLayoutDef.h"
#include "runtime/Report.h"

#include "Ast.h"
#include "Check.h"
#include "Parser.h"
#include "Token.h"
#include "system/administration/reports/report/TenantReportDefaults.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using Traits =
    agiru::ReportTraits<agiru::System::Administration::Reports::TenantReportDefaults_Report>;
constexpr auto kLayouts = Traits::kLayouts;
constexpr std::size_t kNativeLayouts = 2;
constexpr std::size_t kExtensionLayouts = 14;
constexpr int kArgumentCount = 5;
constexpr int kArgumentCountWithSource = 6;

static_assert(kLayouts.size() == kNativeLayouts + kExtensionLayouts);

std::string Read(const std::filesystem::path &path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) { throw std::runtime_error("cannot read " + path.string()); }
  const std::string text{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
  if (stream.bad()) { throw std::runtime_error("incomplete read " + path.string()); }
  return text;
}

agiru::ReportLayoutTokenKind Kind(agiru::al::TokenKind kind) {
  switch (kind) {
    case agiru::al::TokenKind::Identifier: return agiru::ReportLayoutTokenKind::Identifier;
    case agiru::al::TokenKind::QuotedIdentifier:
      return agiru::ReportLayoutTokenKind::QuotedIdentifier;
    case agiru::al::TokenKind::String: return agiru::ReportLayoutTokenKind::String;
    case agiru::al::TokenKind::Integer: return agiru::ReportLayoutTokenKind::Integer;
    case agiru::al::TokenKind::Decimal: return agiru::ReportLayoutTokenKind::Decimal;
    case agiru::al::TokenKind::DateTime: return agiru::ReportLayoutTokenKind::DateTime;
    case agiru::al::TokenKind::Punctuation: return agiru::ReportLayoutTokenKind::Punctuation;
    case agiru::al::TokenKind::Directive: return agiru::ReportLayoutTokenKind::Directive;
    case agiru::al::TokenKind::EndOfFile: break;
  }
  throw std::runtime_error("unexpected end-of-file in property value");
}

void Properties(const agiru::ReportLayoutDef &emitted, const agiru::al::ReportLayoutDecl &source) {
  CHECK_TRUE("complete original property population survives",
             emitted.properties.size() == source.properties.size());
  if (emitted.properties.size() != source.properties.size()) { return; }
  for (std::size_t i = 0; i < source.properties.size(); ++i) {
    const auto &original = source.properties[i];
    const auto &property = emitted.properties[i];
    CHECK_TEXT("original property spelling survives", property.name, original.name);
    CHECK_TEXT("original property value survives", property.text, original.text);
    CHECK_TRUE("all original property tokens survive",
               property.value.size() == original.value.size());
    if (property.value.size() != original.value.size()) { continue; }
    for (std::size_t j = 0; j < original.value.size(); ++j) {
      CHECK_TRUE("original token kind survives",
                 property.value[j].kind == Kind(original.value[j].kind));
      CHECK_TEXT(
          "original token spelling survives", property.value[j].text, original.value[j].text);
    }
  }
}

void Layouts(std::span<const agiru::ReportLayoutDef> emitted,
             std::span<const agiru::al::ReportLayoutDecl> source,
             std::string_view appId,
             std::string_view sourcePath,
             const std::filesystem::path &assets) {
  CHECK_TRUE("complete original layout population survives", emitted.size() == source.size());
  if (emitted.size() != source.size()) { return; }
  for (std::size_t i = 0; i < source.size(); ++i) {
    const auto &original = source[i];
    const auto &layout = emitted[i];
    CHECK_TEXT("original layout name and order survive", layout.name, original.name);
    CHECK_TRUE("original declaring report identity survives",
               layout.origin.report.Value() == (original.owner.extension ? 0 : original.owner.id));
    CHECK_TRUE("original declaring extension identity survives",
               layout.origin.extension.Value() ==
                   (original.owner.extension ? original.owner.id : 0));
    CHECK_TEXT("original declaring name survives", layout.origin.name, original.owner.name);
    CHECK_TEXT(
        "original declaring namespace survives", layout.origin.nameSpace, original.owner.nameSpace);
    CHECK_TEXT("original declaring app identity survives", layout.origin.appId, appId);
    CHECK_TEXT("original declaring source identity survives", layout.origin.source, sourcePath);
    Properties(layout, original);
    const auto *file = agiru::al::Find(original.properties, "LayoutFile");
    if (file == nullptr) { throw std::runtime_error("missing original layout file"); }
    std::string relative = file->text;
    std::ranges::replace(relative, '\\', '/');
    CHECK_TRUE("original owned asset exists", std::filesystem::is_regular_file(assets / relative));
    CHECK_TRUE("original owned asset is not empty", !Read(assets / relative).empty());
  }
}

void OriginalContract(const std::filesystem::path &package,
                      const std::filesystem::path &base,
                      std::string_view nativeAppId,
                      std::string_view baseAppId,
                      std::string_view nativeSource) {
  const auto report =
      agiru::al::ParseReport(Read(package / "src/Reports/TenantReportDefaults.Report.al"));
  const auto extension = agiru::al::ParseReportExtension(
      Read(base / "Foundation/Reporting/CompositeLayout.ReportExt.al"));
  CHECK_TRUE("original native report population", report.rendering.size() == kNativeLayouts);
  CHECK_TRUE("all fourteen actual extension declarations are present",
             extension.rendering.size() == kExtensionLayouts);
  CHECK_TEXT("extension targets the original native report", extension.extends, report.name);
  CHECK_TRUE("original native report ID reaches its typed traits",
             Traits::kId.Value() == report.id);
  CHECK_TEXT("original native report name reaches its typed traits", Traits::kName, report.name);
  const auto *entry = agiru::FindReport(agiru::ReportId{report.id});
  CHECK_TRUE("the linked original report is registered", entry != nullptr);
  if (entry != nullptr) {
    CHECK_TEXT("the registered report name is source-backed", entry->name, report.name);
    CHECK_TRUE("the registered report has a generated entrypoint", entry->run != nullptr);
  }
  CHECK_TRUE("the original native report explicitly has no dataitems", report.dataset.empty());
  const auto *selected = agiru::al::Find(report.properties, "DefaultRenderingLayout");
  if (selected == nullptr) { throw std::runtime_error("missing original selected layout"); }
  CHECK_TEXT("native declarations stay before extension declarations",
             kLayouts.front().name,
             selected->text);
  Layouts(kLayouts.first(kNativeLayouts),
          report.rendering,
          nativeAppId,
          nativeSource,
          package / "layout");
  Layouts(kLayouts.subspan(kNativeLayouts),
          extension.rendering,
          baseAppId,
          "base/Foundation/Reporting/CompositeLayout.ReportExt.al",
          base);
}

}

int main(int argc, char **argv) {
  return gate::Run("Native Report Layouts", [argc, argv] {
    if (argc != kArgumentCount && argc != kArgumentCountWithSource) {
      throw std::invalid_argument("expected package, BaseApp and both app identities");
    }
    OriginalContract(argv[1],
                     argv[2],
                     argv[3],
                     argv[4],
                     argc == kArgumentCountWithSource
                         ? argv[kArgumentCount]
                         : "native/src/Reports/TenantReportDefaults.Report.al");
  });
}

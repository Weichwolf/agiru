#include "Ast.h"
#include "Check.h"
#include "Parser.h"
#include "ReportAssets.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>

namespace {

void BoundAndUnresolvedLayoutsRemain() {
  auto report = agiru::al::ParseReport(R"(namespace Microsoft.Test.Assets;
report 50100 "Asset Report" {
  dataset {} rendering {
    layout("Word layout") { Type = Word; LayoutFile = 'Layouts\Original.docx'; }
  }
})");
  report.rendering.front().owner.appId = "12345678-1234-5678-9012-123456789012";
  report.rendering.front().owner.source = "Fixture/Original.Report.al";
  agiru::gen::ReportAssetRequests requests;
  agiru::gen::AddReportAssets(requests, report);
  auto unresolved = requests.layouts.front();
  unresolved.report = 0;
  unresolved.target = "Absent report";
  unresolved.layout.name = "Unresolved layout";
  requests.layouts.push_back(unresolved);
  const auto manifest = agiru::gen::ReportAssetsManifest(requests);
  CHECK_TRUE("bound and unresolved declarations remain counted",
             manifest.contains("\"summary\":{\"declared\":2,\"bound\":1,\"unresolved\":1}"));
  CHECK_TRUE("original AL separator survives JSON encoding",
             manifest.contains("\"file\":\"Layouts\\\\Original.docx\""));
  CHECK_TRUE("unresolved target identity survives",
             manifest.contains("\"report\":0,\"target\":\"Absent report\""));
  CHECK_TRUE("declaring identity is not replaced by target identity",
             manifest.contains("\"appId\":\"12345678-1234-5678-9012-123456789012\""));
}

void JsonStringsPreserveBytes() {
  agiru::gen::ReportAssetRequests requests;
  agiru::gen::ReportAssetRequest request;
  request.layout.name = "quoted\"path\\Καλημέρα";
  constexpr std::size_t kJsonControlBytes = 0x20;
  for (std::size_t byte = 0; byte < kJsonControlBytes; ++byte) {
    request.layout.name += static_cast<char>(byte);
  }
  requests.layouts.push_back(request);
  const auto manifest = agiru::gen::ReportAssetsManifest(requests);
  CHECK_TRUE("quotes and backslashes remain escaped without losing UTF-8",
             manifest.contains("quoted\\\"path\\\\Καλημέρα"));
  CHECK_TRUE("NUL remains an explicit JSON escape", manifest.contains("\\u0000"));
  CHECK_TRUE("final control byte remains an explicit JSON escape", manifest.contains("\\u001f"));
  const auto body = std::string_view(manifest).substr(0, manifest.size() - 1);
  CHECK_TRUE("no control byte is emitted literally in JSON",
             std::ranges::none_of(body, [](char value) {
               return static_cast<unsigned char>(value) < kJsonControlBytes;
             }));
}

}

int main() {
  return gate::Run("ReportAssets", [] {
    BoundAndUnresolvedLayoutsRemain();
    JsonStringsPreserveBytes();
  });
}

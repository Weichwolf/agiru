#include "ReportAssets.h"

#include "Ast.h"
#include "NativeSource.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace agiru::gen {
namespace {

std::string Quote(std::string_view value) {
  constexpr std::string_view kHex = "0123456789abcdef";
  constexpr unsigned char kJsonControlLimit = 0x20;
  constexpr unsigned char kLowNibbleMask = 0x0f;
  std::string out = "\"";
  for (const char character : value) {
    const auto byte = static_cast<unsigned char>(character);
    if (character == '"' || character == '\\') {
      out += '\\';
      out += character;
    } else if (byte < kJsonControlLimit) {
      out += "\\u00";
      out += kHex[byte >> 4];
      out += kHex[byte & kLowNibbleMask];
    } else {
      out += character;
    }
  }
  return out + '"';
}

std::string Property(const al::ReportLayoutDecl &layout, std::string_view name) {
  const auto *found = al::Find(layout.properties, name);
  return found == nullptr ? std::string{} : found->text;
}

std::string App(const ReportAssetApp &app) {
  return "{\"id\":" + Quote(app.identity.id) + ",\"name\":" + Quote(app.identity.name) +
         ",\"publisher\":" + Quote(app.identity.publisher) +
         ",\"version\":" + Quote(app.identity.version) + ",\"source\":" + Quote(app.source) +
         ",\"platform\":" + (app.platform ? "true" : "false") + "}";
}

std::string Layout(const ReportAssetRequest &request) {
  const auto &layout = request.layout;
  const auto &owner = layout.owner;
  return "{\"report\":" + std::to_string(request.report) + ",\"target\":" + Quote(request.target) +
         ",\"name\":" + Quote(layout.name) + ",\"file\":" + Quote(Property(layout, "LayoutFile")) +
         ",\"type\":" + Quote(Property(layout, "Type")) +
         ",\"subtype\":" + Quote(Property(layout, "Subtype")) +
         ",\"mimeType\":" + Quote(Property(layout, "MimeType")) + R"(,"owner":{"appId":)" +
         Quote(owner.appId) + ",\"id\":" + std::to_string(owner.id) +
         ",\"extension\":" + (owner.extension ? "true" : "false") +
         ",\"name\":" + Quote(owner.name) + ",\"namespace\":" + Quote(owner.nameSpace) +
         ",\"source\":" + Quote(owner.source) + "}}";
}

}

void AddReportAssets(ReportAssetRequests &requests, const al::PageObject &report) {
  for (const auto &layout : report.rendering) {
    requests.layouts.push_back({.report = report.id, .target = report.name, .layout = layout});
  }
}

std::string ReportAssetsManifest(const ReportAssetRequests &requests) {
  std::string out = R"({"schema":1,"population":"named-rendering-layouts","apps":[)";
  bool first = true;
  for (const auto &app : requests.apps) {
    if (!first) { out += ','; }
    first = false;
    out += App(app);
  }
  out += "],\"layouts\":[";
  first = true;
  std::size_t bound = 0;
  for (const auto &request : requests.layouts) {
    if (!first) { out += ','; }
    first = false;
    out += Layout(request);
    if (request.report != 0) { ++bound; }
  }
  out += R"(],"summary":{"declared":)" + std::to_string(requests.layouts.size()) +
         ",\"bound\":" + std::to_string(bound) +
         ",\"unresolved\":" + std::to_string(requests.layouts.size() - bound) + "}}\n";
  return out;
}

}

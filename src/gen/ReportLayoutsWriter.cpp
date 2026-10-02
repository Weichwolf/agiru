#include "ReportLayoutsWriter.h"

#include "Ast.h"
#include "Names.h"
#include "Scope.h"
#include "Token.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

namespace agiru::gen {
namespace {

std::string ReportLayoutsName(const al::PageObject &report) {
  return "k" + Identifier(report.name) + "ReportLayouts";
}

std::string_view TokenKindName(al::TokenKind kind) {
  switch (kind) {
    case al::TokenKind::Identifier: return "Identifier";
    case al::TokenKind::QuotedIdentifier: return "QuotedIdentifier";
    case al::TokenKind::String: return "String";
    case al::TokenKind::Integer: return "Integer";
    case al::TokenKind::Decimal: return "Decimal";
    case al::TokenKind::DateTime: return "DateTime";
    case al::TokenKind::Punctuation: return "Punctuation";
    case al::TokenKind::Directive: return "Directive";
    case al::TokenKind::EndOfFile: break;
  }
  throw std::runtime_error("report layout contains an invalid value token");
}

void WritePropertyTokens(const al::ReportLayoutDecl &layout,
                         const std::string &prefix,
                         std::string &out) {
  for (std::size_t index = 0; index < layout.properties.size(); ++index) {
    const auto &property = layout.properties[index];
    if (property.value.empty()) { continue; }
    out += "constexpr ::agiru::ReportLayoutTokenDef " + prefix + "Tokens" + std::to_string(index) +
           "[]{\n";
    for (const auto &token : property.value) {
      out +=
          "  {.kind = ::agiru::ReportLayoutTokenKind::" + std::string(TokenKindName(token.kind)) +
          ", .text = " + Literal(token.text) + "},\n";
    }
    out += "};\n";
  }
}

void WriteProperties(const al::ReportLayoutDecl &layout,
                     const std::string &prefix,
                     std::string &out) {
  WritePropertyTokens(layout, prefix, out);
  if (layout.properties.empty()) { return; }
  out += "constexpr ::agiru::ReportLayoutPropertyDef " + prefix + "Properties[]{\n";
  for (std::size_t index = 0; index < layout.properties.size(); ++index) {
    const auto &property = layout.properties[index];
    out += "  {.name = " + Literal(property.name) + ", .text = " + Literal(property.text);
    if (!property.value.empty()) {
      out += ", .value = " + prefix + "Tokens" + std::to_string(index);
    }
    out += "},\n";
  }
  out += "};\n";
}

std::string LayoutOrigin(const al::ReportLayoutOwner &owner) {
  std::string out = "{.report = ::agiru::ReportId{";
  out += std::to_string(owner.extension ? 0 : owner.id);
  out += "}, .extension = ::agiru::ReportExtensionId{";
  out += std::to_string(owner.extension ? owner.id : 0);
  out += "}, .name = " + Literal(owner.name) + ", .nameSpace = " + Literal(owner.nameSpace);
  out += ", .appId = " + Literal(owner.appId) + ", .source = " + Literal(owner.source) + "}";
  return out;
}

}

std::string ReportLayoutsIncludes(const al::PageObject &report) {
  if (!report.report || report.rendering.empty()) { return {}; }
  return "#include \"meta/ReportLayoutDef.h\"\n#include <span>\n";
}

std::string ReportLayoutsDeclaration(const al::PageObject &report) {
  if (!report.report || report.rendering.empty()) { return {}; }
  return "extern const ::agiru::ReportLayoutDef " + ReportLayoutsName(report) + "[" +
         std::to_string(report.rendering.size()) + "];\n";
}

std::string ReportLayoutsTrait(const al::PageObject &report) {
  if (!report.report || report.rendering.empty()) { return {}; }
  return "  static constexpr std::span<const ::agiru::ReportLayoutDef> kLayouts{" +
         NamespaceOf(report.nameSpace) + "::" + ReportLayoutsName(report) + "};\n";
}

std::string ReportLayoutDefinitions(const al::PageObject &report) {
  if (!report.report || report.rendering.empty()) { return {}; }
  const std::string name = ReportLayoutsName(report);
  std::string out;
  for (std::size_t index = 0; index < report.rendering.size(); ++index) {
    WriteProperties(report.rendering[index], name + std::to_string(index), out);
  }
  out += "constexpr ::agiru::ReportLayoutDef " + name + "[]{\n";
  for (std::size_t index = 0; index < report.rendering.size(); ++index) {
    const auto &layout = report.rendering[index];
    out += "  {.name = " + Literal(layout.name);
    if (!layout.properties.empty()) {
      out += ", .properties = " + name + std::to_string(index) + "Properties";
    }
    out += ", .origin = " + LayoutOrigin(layout.owner) + "},\n";
  }
  out += "};\n";
  for (std::size_t index = 0; index < report.rendering.size(); ++index) {
    const std::string origin = name + "[" + std::to_string(index) + "].origin";
    out += "static_assert((";
    out += origin;
    out += ".report.Value() != 0) != (";
    out += origin;
    out += ".extension.Value() != 0));\n";
  }
  out += "\n";
  return out;
}

}

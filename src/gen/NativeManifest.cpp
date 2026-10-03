#include "NativeSource.h"

#include <cctype>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>

namespace agiru::gen {

namespace {

bool Named(const xmlNode &node, const char *name) {
  static constexpr std::string_view kNamespace = "http://schemas.microsoft.com/navx/2015/manifest";
  return node.type == XML_ELEMENT_NODE &&
         xmlStrEqual(node.name, reinterpret_cast<const xmlChar *>(name)) != 0 &&
         node.ns != nullptr && node.ns->href != nullptr &&
         kNamespace == reinterpret_cast<const char *>(node.ns->href);
}

std::string Attribute(const xmlNode &node, const char *name) {
  const std::unique_ptr<xmlChar, decltype(xmlFree)> value{
      xmlGetProp(&node, reinterpret_cast<const xmlChar *>(name)), xmlFree};
  if (value == nullptr || *value == '\0') {
    throw std::runtime_error("System manifest lacks App attribute " + std::string(name));
  }
  return reinterpret_cast<const char *>(value.get());
}

void RequireGuid(std::string_view value) {
  static constexpr std::size_t kGuidLength = 36;
  if (value.size() != kGuidLength) {
    throw std::runtime_error("System manifest has invalid App Id");
  }
  for (std::size_t at = 0; at < value.size(); ++at) {
    const bool dash = at == 8 || at == 13 || at == 18 || at == 23;
    if ((dash && value[at] != '-') ||
        (!dash && std::isxdigit(static_cast<unsigned char>(value[at])) == 0)) {
      throw std::runtime_error("System manifest has invalid App Id");
    }
  }
}

const xmlNode &AppNode(const xmlDoc &document) {
  const auto *root = xmlDocGetRootElement(&document);
  if (root == nullptr || !Named(*root, "Package")) {
    throw std::runtime_error("System manifest lacks namespaced Package");
  }
  const xmlNode *app = nullptr;
  for (const auto *child = root->children; child != nullptr; child = child->next) {
    if (!Named(*child, "App")) { continue; }
    if (app != nullptr) {
      throw std::runtime_error("System manifest has duplicate App declarations");
    }
    app = child;
  }
  if (app == nullptr) { throw std::runtime_error("System manifest lacks namespaced App"); }
  return *app;
}

}

NativeAppIdentity ReadNativeIdentity(const std::filesystem::path &package) {
  const auto manifest = package / "NavxManifest.xml";
  if (std::filesystem::is_symlink(manifest)) {
    throw std::runtime_error("System manifest is a symlink");
  }
  const std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> document{
      xmlReadFile(
          manifest.c_str(), nullptr, XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING),
      &xmlFreeDoc};
  if (document == nullptr) { throw std::runtime_error("cannot read System NavxManifest.xml"); }
  if (document->intSubset != nullptr || document->extSubset != nullptr) {
    throw std::runtime_error("System manifest must not declare a DTD");
  }
  const auto &app = AppNode(*document);
  NativeAppIdentity identity{.id = Attribute(app, "Id"),
                             .name = Attribute(app, "Name"),
                             .publisher = Attribute(app, "Publisher"),
                             .version = Attribute(app, "Version")};
  RequireGuid(identity.id);
  return identity;
}

}

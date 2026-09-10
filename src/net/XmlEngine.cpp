#include "XmlEngine.h"

#include "type/XmlHandle.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xmlerror.h>
#include <libxml/xpathInternals.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::detail {

namespace {

const xmlChar *Bytes(std::string_view text) {
  return reinterpret_cast<const xmlChar *>(text.data());
}

std::string Owned(xmlChar *text) {
  const std::string out = Text(text);
  xmlFree(text);
  return out;
}

}

XmlTree *NewTree(xmlDocPtr doc) {
  auto *tree = new XmlTree{};
  tree->doc = doc;
  return tree;
}

void XmlRetain(XmlTree *tree) noexcept {
  if (tree != nullptr) { ++tree->count; }
}

void XmlRelease(XmlTree *tree) noexcept {
  if (tree == nullptr) { return; }
  if (--tree->count > 0) { return; }
  for (XmlTree *kept : tree->kept) { XmlRelease(kept); }
  if (tree->doc != nullptr) { xmlFreeDoc(tree->doc); }
  delete tree;
}

XmlHandle EmptyDocument() {
  xmlDocPtr doc = xmlNewDoc(Bytes("1.0"));
  return XmlHandle(NewTree(doc), doc);
}

XmlHandle HandleOf(XmlTree *tree, void *node) {
  return XmlHandle(tree, node);
}

XmlHandle Detached(xmlNodePtr node) {
  xmlDocPtr doc = xmlNewDoc(Bytes("1.0"));
  xmlSetTreeDoc(node, doc);
  return XmlHandle(NewTree(doc), node);
}

xmlNodePtr NodeOf(const XmlHandle &handle) {
  return static_cast<xmlNodePtr>(handle.node);
}

xmlDocPtr DocOf(const XmlHandle &handle) {
  return handle.tree == nullptr ? nullptr : handle.tree->doc;
}

void KeepAlive(XmlTree *keeper, XmlTree *kept) {
  if (keeper == nullptr || kept == nullptr || keeper == kept) { return; }
  if (std::ranges::find(keeper->kept, kept) != keeper->kept.end()) { return; }
  XmlRetain(kept);
  keeper->kept.push_back(kept);
}

std::string Text(const xmlChar *text) {
  return text == nullptr ? std::string{} : std::string(reinterpret_cast<const char *>(text));
}

std::string Utf8(std::string_view text) {
  return std::string(text);
}

std::string Dump(const XmlHandle &handle) {
  xmlNodePtr node = NodeOf(handle);
  if (node == nullptr) { return {}; }
  if (node->type == XML_DOCUMENT_NODE) {
    xmlChar *out = nullptr;
    int size = 0;
    xmlDocDumpMemoryEnc(reinterpret_cast<xmlDocPtr>(node), &out, &size, "UTF-8");
    return Owned(out);
  }
  if (node->type == XML_ATTRIBUTE_NODE) {
    auto *attribute = reinterpret_cast<xmlAttrPtr>(node);
    return QualifiedName(node) + "=\"" + Owned(xmlNodeGetContent(node)) + "\"" +
           (attribute->ns == nullptr ? "" : "");
  }
  xmlBufferPtr buffer = xmlBufferCreate();
  xmlNodeDump(buffer, node->doc, node, 0, 0);
  const std::string out = Text(xmlBufferContent(buffer));
  xmlBufferFree(buffer);
  return out;
}

std::string DumpChildren(const XmlHandle &handle) {
  xmlNodePtr node = NodeOf(handle);
  if (node == nullptr) { return {}; }
  std::string out;
  for (xmlNodePtr child = node->children; child != nullptr; child = child->next) {
    xmlBufferPtr buffer = xmlBufferCreate();
    xmlNodeDump(buffer, child->doc, child, 0, 0);
    out += Text(xmlBufferContent(buffer));
    xmlBufferFree(buffer);
  }
  return out;
}

void Quiet(void *, xmlErrorPtr) {}

void Silence() {
  static const bool once = [] {
    xmlSetStructuredErrorFunc(nullptr, Quiet);
    return true;
  }();
  static_cast<void>(once);
}

bool Parse(std::string_view text, bool preserveWhitespace, XmlHandle &into) {
  Silence();
  int options = XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING;
  if (!preserveWhitespace) { options |= XML_PARSE_NOBLANKS; }
  xmlDocPtr doc =
      xmlReadMemory(text.data(), static_cast<int>(text.size()), "agiru.xml", nullptr, options);
  if (doc == nullptr) { return false; }
  into = XmlHandle(NewTree(doc), doc);
  return true;
}

std::vector<XmlHandle> XPath(const XmlHandle &from,
                             std::string_view expression,
                             const std::vector<std::pair<std::string, std::string>> &namespaces) {
  std::vector<XmlHandle> found;
  xmlNodePtr node = NodeOf(from);
  xmlDocPtr doc = DocOf(from);
  if (node == nullptr || doc == nullptr) { return found; }
  Silence();
  xmlXPathContextPtr context = xmlXPathNewContext(doc);
  if (context == nullptr) { return found; }
  context->node = node->type == XML_DOCUMENT_NODE ? reinterpret_cast<xmlNodePtr>(doc) : node;
  for (const auto &[prefix, uri] : namespaces) {
    if (!prefix.empty()) { xmlXPathRegisterNs(context, Bytes(prefix), Bytes(uri)); }
  }
  const std::string held(expression);
  xmlXPathObjectPtr result = xmlXPathEvalExpression(Bytes(held), context);
  if (result != nullptr) {
    if (result->type == XPATH_NODESET && result->nodesetval != nullptr) {
      for (int i = 0; i < result->nodesetval->nodeNr; ++i) {
        found.emplace_back(from.tree, result->nodesetval->nodeTab[i]);
      }
    }
    xmlXPathFreeObject(result);
  }
  xmlXPathFreeContext(context);
  return found;
}

std::vector<XmlHandle> Children(const XmlHandle &of, bool elementsOnly) {
  std::vector<XmlHandle> out;
  xmlNodePtr node = NodeOf(of);
  if (node == nullptr) { return out; }
  for (xmlNodePtr child = node->children; child != nullptr; child = child->next) {
    if (elementsOnly && child->type != XML_ELEMENT_NODE) { continue; }
    out.emplace_back(of.tree, child);
  }
  return out;
}

namespace {

void Walk(XmlTree *tree, xmlNodePtr node, bool elementsOnly, std::vector<XmlHandle> &out) {
  for (xmlNodePtr child = node->children; child != nullptr; child = child->next) {
    if (!elementsOnly || child->type == XML_ELEMENT_NODE) { out.emplace_back(tree, child); }
    Walk(tree, child, elementsOnly, out);
  }
}

}

std::vector<XmlHandle> Descendants(const XmlHandle &of, bool elementsOnly) {
  std::vector<XmlHandle> out;
  if (NodeOf(of) != nullptr) { Walk(of.tree, NodeOf(of), elementsOnly, out); }
  return out;
}

std::vector<XmlHandle> Attributes(const XmlHandle &of) {
  std::vector<XmlHandle> out;
  xmlNodePtr node = NodeOf(of);
  if (node == nullptr || node->type != XML_ELEMENT_NODE) { return out; }
  for (xmlAttrPtr attribute = node->properties; attribute != nullptr; attribute = attribute->next) {
    out.emplace_back(of.tree, attribute);
  }
  return out;
}

std::string QualifiedName(xmlNodePtr node) {
  if (node == nullptr) { return {}; }
  const std::string local = Text(node->name);
  if (node->ns != nullptr && node->ns->prefix != nullptr) {
    return Text(node->ns->prefix) + ":" + local;
  }
  return local;
}

bool SameName(xmlNodePtr node, std::string_view qualified) {
  return QualifiedName(node) == qualified;
}

bool SameLocalName(xmlNodePtr node, std::string_view local, std::string_view uri) {
  if (node == nullptr || Text(node->name) != local) { return false; }
  const std::string held = node->ns == nullptr ? std::string{} : Text(node->ns->href);
  return held == uri;
}

void Adopt(const XmlHandle &parent, const XmlHandle &child) {
  xmlNodePtr target = NodeOf(parent);
  xmlNodePtr node = NodeOf(child);
  if (target == nullptr || node == nullptr) { return; }
  xmlUnlinkNode(node);
  if (target->type == XML_DOCUMENT_NODE) {
    xmlDocPtr doc = reinterpret_cast<xmlDocPtr>(target);
    if (node->type == XML_ELEMENT_NODE && xmlDocGetRootElement(doc) == nullptr) {
      xmlDocSetRootElement(doc, node);
    } else {
      xmlAddChild(target, node);
    }
  } else {
    xmlAddChild(target, node);
  }
  if (parent.tree != child.tree) {
    xmlSetTreeDoc(node, DocOf(parent));
    KeepAlive(parent.tree, child.tree);
    KeepAlive(child.tree, parent.tree);
  }
}

}

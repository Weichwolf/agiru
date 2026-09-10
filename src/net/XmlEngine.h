#pragma once

#include "type/XmlHandle.h"

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <string>
#include <string_view>
#include <vector>

namespace agiru::detail {

struct XmlTree {
  xmlDocPtr doc = nullptr;
  int count = 0;
  std::vector<XmlTree *> kept;
};

XmlTree *NewTree(xmlDocPtr doc);
XmlHandle HandleOf(XmlTree *tree, void *node);
XmlHandle Detached(xmlNodePtr node);
xmlNodePtr NodeOf(const XmlHandle &handle);
xmlDocPtr DocOf(const XmlHandle &handle);
void KeepAlive(XmlTree *keeper, XmlTree *kept);
std::string Text(const xmlChar *text);
std::string Utf8(std::string_view text);
std::string Dump(const XmlHandle &handle);
std::string DumpChildren(const XmlHandle &handle);
bool Parse(std::string_view text, bool preserveWhitespace, XmlHandle &into);
std::vector<XmlHandle> XPath(const XmlHandle &from,
                             std::string_view expression,
                             const std::vector<std::pair<std::string, std::string>> &namespaces);
std::vector<XmlHandle> Children(const XmlHandle &of, bool elementsOnly);
std::vector<XmlHandle> Descendants(const XmlHandle &of, bool elementsOnly);
std::vector<XmlHandle> Attributes(const XmlHandle &of);
bool SameName(xmlNodePtr node, std::string_view qualified);
bool SameLocalName(xmlNodePtr node, std::string_view local, std::string_view uri);
void Adopt(const XmlHandle &parent, const XmlHandle &child);
std::string QualifiedName(xmlNodePtr node);

}

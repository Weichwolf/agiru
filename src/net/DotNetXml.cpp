#include "dotnet/XmlDocument.h"
#include "dotnet/XmlNode.h"
#include "dotnet/XmlReader.h"
#include "runtime/ErrorValue.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/StringValue.h"
#include "type/XmlHandle.h"

#include "XmlEngine.h"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <ios>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/valid.h>
#include <libxml/xmlstring.h>

namespace agiru::dotnet {

using ::agiru::detail::DocOf;
using ::agiru::detail::NodeOf;
using ::agiru::detail::XmlHandle;

namespace {

const xmlChar *Bytes(std::string_view text) {
  return reinterpret_cast<const xmlChar *>(text.data());
}

XmlHandle Sibling(const XmlHandle &of, xmlNodePtr node) {
  return node == nullptr ? XmlHandle{} : XmlHandle(of.tree, node);
}

std::string ContentOf(const XmlHandle &handle) {
  xmlNodePtr node = NodeOf(handle);
  if (node == nullptr) { return {}; }
  xmlChar *text = xmlNodeGetContent(node);
  const std::string out = ::agiru::detail::Text(text);
  xmlFree(text);
  return out;
}

void Relink(const XmlHandle &into, const XmlHandle &node) {
  if (into.tree != node.tree) {
    xmlSetTreeDoc(NodeOf(node), DocOf(into));
    ::agiru::detail::KeepAlive(into.tree, node.tree);
    ::agiru::detail::KeepAlive(node.tree, into.tree);
  }
}

XmlHandle NewInDocument(const XmlHandle &document, xmlNodePtr node) {
  if (document.tree == nullptr) { return ::agiru::detail::Detached(node); }
  xmlSetTreeDoc(node, DocOf(document));
  return {document.tree, node};
}

std::vector<std::pair<std::string, std::string>> NoNamespaces() {
  return {};
}

std::vector<XmlHandle> ByTagName(const XmlHandle &from, std::string_view name) {
  std::vector<XmlHandle> out;
  for (XmlHandle &node : ::agiru::detail::Descendants(from, true)) {
    if (name == "*" || ::agiru::detail::SameName(NodeOf(node), name)) {
      out.push_back(std::move(node));
    }
  }
  return out;
}

struct QualifiedNameParts {
  std::string_view prefix;
  std::string_view local;
  std::string_view uri;
};

XmlHandle MakeElement(const XmlHandle &document, const QualifiedNameParts &parts) {
  xmlNodePtr node = xmlNewNode(nullptr, Bytes(std::string(parts.local)));
  const XmlHandle made = NewInDocument(document, node);
  if (!parts.uri.empty()) {
    const std::string prefix(parts.prefix);
    xmlNsPtr ns =
        xmlNewNs(node, Bytes(std::string(parts.uri)), prefix.empty() ? nullptr : Bytes(prefix));
    xmlSetNs(node, ns);
  }
  return made;
}

XmlHandle MakeAttribute(const QualifiedNameParts &parts) {
  xmlNodePtr holder = xmlNewNode(nullptr, Bytes("attribute"));
  const XmlHandle tree = ::agiru::detail::Detached(holder);
  xmlAttrPtr attribute = nullptr;
  if (parts.uri.empty()) {
    attribute = xmlNewProp(holder, Bytes(std::string(parts.local)), Bytes(""));
  } else {
    const std::string prefix(parts.prefix);
    xmlNsPtr ns =
        xmlNewNs(holder, Bytes(std::string(parts.uri)), prefix.empty() ? nullptr : Bytes(prefix));
    attribute = xmlNewNsProp(holder, ns, Bytes(std::string(parts.local)), Bytes(""));
  }
  return {tree.tree, attribute};
}

XmlHandle MakeDeclaration(const XmlHandle &document,
                          const ::agiru::detail::XmlDeclarationParts &parts) {
  xmlDocPtr doc = DocOf(document);
  if (doc == nullptr) {
    throw Error("XmlDocument.CreateXmlDeclaration: the document was never made");
  }
  xmlFree(const_cast<xmlChar *>(doc->version));
  doc->version = xmlStrdup(Bytes(std::string(parts.version)));
  xmlFree(const_cast<xmlChar *>(doc->encoding));
  doc->encoding = parts.encoding.empty() ? nullptr : xmlStrdup(Bytes(std::string(parts.encoding)));
  doc->standalone = parts.standalone == "yes" ? 1 : parts.standalone == "no" ? 0 : -1;
  return {document.tree, doc};
}

XmlHandle MakeDocumentType(const XmlHandle &document,
                           const ::agiru::detail::XmlDocumentTypeParts &parts) {
  if (DocOf(document) == nullptr) {
    throw Error("XmlDocument.CreateDocumentType: the document was never made");
  }
  static_cast<void>(parts.subset);
  const std::string name(parts.name);
  const std::string publicId(parts.publicId);
  const std::string systemId(parts.systemId);
  xmlDtdPtr dtd = xmlNewDtd(nullptr,
                            Bytes(name),
                            publicId.empty() ? nullptr : Bytes(publicId),
                            systemId.empty() ? nullptr : Bytes(systemId));
  return NewInDocument(document, reinterpret_cast<xmlNodePtr>(dtd));
}

}

XmlNode XmlNode::AppendChild(const XmlNode &node) {
  if (handle_.Empty() || node.Handle().Empty()) {
    throw Error("XmlNode.AppendChild: a null reference was used");
  }
  if (NodeOf(node.Handle())->type == XML_DOCUMENT_NODE) { return node; }
  if (!::agiru::detail::Attachable(NodeOf(handle_), NodeOf(node.Handle()))) {
    throw Error("XmlNode.AppendChild: the node to append is this node or one of its ancestors");
  }
  ::agiru::detail::Adopt(handle_, node.Handle());
  return XmlNode(XmlHandle(handle_.tree, node.Handle().node));
}

XmlAttributeCollection XmlNode::Attributes() const {
  if (NodeOf(handle_) == nullptr || NodeOf(handle_)->type != XML_ELEMENT_NODE) { return {}; }
  return XmlAttributeCollection(handle_);
}

XmlNodeList XmlNode::ChildNodes() const {
  return XmlNodeList(::agiru::detail::Children(handle_, false));
}

XmlNode XmlNode::FirstChild() const {
  return XmlNode(NodeOf(handle_) == nullptr ? XmlHandle{}
                                            : Sibling(handle_, NodeOf(handle_)->children));
}

XmlNode XmlNode::LastChild() const {
  return XmlNode(NodeOf(handle_) == nullptr ? XmlHandle{}
                                            : Sibling(handle_, NodeOf(handle_)->last));
}

XmlNode XmlNode::NextSibling() const {
  return XmlNode(NodeOf(handle_) == nullptr ? XmlHandle{}
                                            : Sibling(handle_, NodeOf(handle_)->next));
}

XmlNode XmlNode::ParentNode() const {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr || node->parent == nullptr) { return {}; }
  return XmlNode(XmlHandle(handle_.tree, node->parent));
}

XmlDocument XmlNode::OwnerDocument() const {
  if (DocOf(handle_) == nullptr) { return {}; }
  return XmlDocument(XmlHandle(handle_.tree, DocOf(handle_)));
}

::agiru::Boolean XmlNode::HasChildNodes() const {
  return NodeOf(handle_) != nullptr && NodeOf(handle_)->children != nullptr;
}

::agiru::Text<0> XmlNode::InnerText() const {
  return ContentOf(handle_);
}

::agiru::Text<0> XmlNode::InnerText(std::string_view text) {
  xmlNodePtr node = NodeOf(handle_);
  if (node != nullptr) {
    while (node->children != nullptr) {
      xmlNodePtr child = node->children;
      xmlUnlinkNode(child);
      xmlFreeNode(child);
    }
    const std::string held(text);
    if (xmlNodePtr made = xmlNewText(Bytes(held)); made != nullptr) { xmlAddChild(node, made); }
  }
  return std::string(text);
}

::agiru::Text<0> XmlNode::InnerXml() const {
  return ::agiru::detail::DumpChildren(handle_);
}

::agiru::Text<0> XmlNode::InnerXml(std::string_view markup) {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr) { return std::string(markup); }
  while (node->children != nullptr) {
    xmlNodePtr child = node->children;
    xmlUnlinkNode(child);
    xmlFreeNode(child);
  }
  const std::string held(markup);
  xmlNodePtr parsed = nullptr;
  if (xmlParseInNodeContext(node,
                            held.data(),
                            static_cast<int>(held.size()),
                            XML_PARSE_NOERROR | XML_PARSE_NOWARNING,
                            &parsed) == XML_ERR_OK &&
      parsed != nullptr) {
    xmlAddChildList(node, parsed);
  } else {
    throw Error("XmlNode.InnerXml: the markup is not well-formed XML");
  }
  return held;
}

::agiru::Text<0> XmlNode::OuterXml() const {
  return ::agiru::detail::Dump(handle_);
}

::agiru::Text<0> XmlNode::LocalName() const {
  return NodeOf(handle_) == nullptr ? std::string{} : ::agiru::detail::Text(NodeOf(handle_)->name);
}

::agiru::Text<0> XmlNode::Name() const {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr) { return {}; }
  if (node->type == XML_DOCUMENT_NODE) { return "#document"; }
  if (node->type == XML_TEXT_NODE) { return "#text"; }
  if (node->type == XML_CDATA_SECTION_NODE) { return "#cdata-section"; }
  if (node->type == XML_COMMENT_NODE) { return "#comment"; }
  return ::agiru::detail::QualifiedName(node);
}

::agiru::Text<0> XmlNode::NamespaceURI() const {
  xmlNodePtr node = NodeOf(handle_);
  return node == nullptr || node->ns == nullptr ? std::string{}
                                                : ::agiru::detail::Text(node->ns->href);
}

::agiru::Text<0> XmlNode::Prefix() const {
  xmlNodePtr node = NodeOf(handle_);
  return node == nullptr || node->ns == nullptr ? std::string{}
                                                : ::agiru::detail::Text(node->ns->prefix);
}

::agiru::Text<0> XmlNode::Value() const {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr || node->type == XML_ELEMENT_NODE || node->type == XML_DOCUMENT_NODE) {
    return {};
  }
  return ContentOf(handle_);
}

::agiru::Text<0> XmlNode::Value(std::string_view text) {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr) { return std::string(text); }
  const std::string held(text);
  if (node->type == XML_ATTRIBUTE_NODE) {
    auto *attribute = reinterpret_cast<xmlAttrPtr>(node);
    xmlSetNsProp(attribute->parent, attribute->ns, attribute->name, Bytes(held));
  } else {
    xmlNodeSetContent(node, Bytes(held));
  }
  return held;
}

XmlNode XmlNode::RemoveChild(const XmlNode &node) {
  xmlNodePtr child = NodeOf(node.Handle());
  if (child == nullptr || child->parent != NodeOf(handle_)) {
    throw Error("XmlNode.RemoveChild: the node to remove is not a child of this node");
  }
  xmlUnlinkNode(child);
  return node;
}

XmlNode XmlNode::ReplaceChild(const XmlNode &made, const XmlNode &old) {
  xmlNodePtr oldNode = NodeOf(old.Handle());
  xmlNodePtr newNode = NodeOf(made.Handle());
  if (oldNode == nullptr || newNode == nullptr || oldNode->parent != NodeOf(handle_)) {
    throw Error("XmlNode.ReplaceChild: the node to replace is not a child of this node");
  }
  if (newNode->type == XML_DOCUMENT_NODE) { return old; }
  if (!::agiru::detail::Attachable(NodeOf(handle_), newNode)) {
    throw Error("XmlNode.ReplaceChild: the new node is this node or one of its ancestors");
  }
  xmlUnlinkNode(newNode);
  xmlReplaceNode(oldNode, newNode);
  Relink(handle_, made.Handle());
  return old;
}

XmlNode XmlNode::InsertBefore(const XmlNode &made, const XmlNode &before) {
  xmlNodePtr newNode = NodeOf(made.Handle());
  xmlNodePtr reference = NodeOf(before.Handle());
  if (newNode == nullptr) { throw Error("XmlNode.InsertBefore: a null reference was used"); }
  if (newNode->type == XML_DOCUMENT_NODE) { return made; }
  if (reference == nullptr) { return AppendChild(made); }
  if (!::agiru::detail::Attachable(NodeOf(handle_), newNode)) {
    throw Error("XmlNode.InsertBefore: the node to insert is this node or one of its ancestors");
  }
  xmlUnlinkNode(newNode);
  xmlAddPrevSibling(reference, newNode);
  Relink(handle_, made.Handle());
  return XmlNode(XmlHandle(handle_.tree, newNode));
}

XmlNodeList XmlNode::SelectNodes(std::string_view xpath) const {
  return XmlNodeList(::agiru::detail::XPath(handle_, xpath, NoNamespaces()));
}

XmlNodeList XmlNode::SelectNodes(std::string_view xpath, const XmlNamespaceManager &manager) const {
  return XmlNodeList(::agiru::detail::XPath(handle_, xpath, manager.Declared()));
}

XmlNode XmlNode::SelectSingleNode(std::string_view xpath) const {
  const std::vector<XmlHandle> found = ::agiru::detail::XPath(handle_, xpath, NoNamespaces());
  return found.empty() ? XmlNode{} : XmlNode(found.front());
}

XmlNode XmlNode::SelectSingleNode(std::string_view xpath,
                                  const XmlNamespaceManager &manager) const {
  const std::vector<XmlHandle> found = ::agiru::detail::XPath(handle_, xpath, manager.Declared());
  return found.empty() ? XmlNode{} : XmlNode(found.front());
}

::agiru::Integer XmlNodeList::Count() const {
  return static_cast<Integer>(items_.size());
}

XmlNode XmlNodeList::Item(::agiru::Integer index) const {
  if (index < 0 || static_cast<std::size_t>(index) >= items_.size()) { return {}; }
  return XmlNode(items_[static_cast<std::size_t>(index)]);
}

std::vector<XmlNode> XmlNodeList::Nodes() const {
  std::vector<XmlNode> out;
  out.reserve(items_.size());
  for (const XmlHandle &item : items_) { out.emplace_back(item); }
  return out;
}

std::vector<XmlNode>::const_iterator XmlNodeList::begin() const {
  walked_ = Nodes();
  return walked_.begin();
}

std::vector<XmlNode>::const_iterator XmlNodeList::end() const {
  return walked_.end();
}

void XmlElement::SetAttribute(std::string_view name, std::string_view value) {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr) { throw Error("XmlElement.SetAttribute: a null reference was used"); }
  xmlSetProp(node, Bytes(std::string(name)), Bytes(std::string(value)));
}

::agiru::Text<0> XmlElement::GetAttribute(std::string_view name) const {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr) { return {}; }
  xmlChar *value = xmlGetProp(node, Bytes(std::string(name)));
  const std::string out = ::agiru::detail::Text(value);
  xmlFree(value);
  return out;
}

::agiru::Boolean XmlElement::HasAttribute(std::string_view name) const {
  xmlNodePtr node = NodeOf(handle_);
  return node != nullptr && xmlHasProp(node, Bytes(std::string(name))) != nullptr;
}

void XmlElement::RemoveAttribute(std::string_view name) {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr) { return; }
  xmlAttrPtr attribute = xmlHasProp(node, Bytes(std::string(name)));
  if (attribute != nullptr) { xmlRemoveProp(attribute); }
}

XmlNodeList XmlElement::GetElementsByTagName(std::string_view name) const {
  return XmlNodeList(ByTagName(handle_, name));
}

::agiru::Boolean XmlElement::IsEmpty() const {
  return NodeOf(handle_) == nullptr || NodeOf(handle_)->children == nullptr;
}

XmlElement XmlAttribute::OwnerElement() const {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr || node->parent == nullptr) { return {}; }
  return XmlElement(XmlHandle(handle_.tree, node->parent));
}

::agiru::Integer XmlAttributeCollection::Count() const {
  return static_cast<Integer>(::agiru::detail::Attributes(owner_).size());
}

XmlAttribute XmlAttributeCollection::Item(::agiru::Integer index) const {
  const std::vector<XmlHandle> items = ::agiru::detail::Attributes(owner_);
  if (index < 0 || static_cast<std::size_t>(index) >= items.size()) { return {}; }
  return XmlAttribute(items[static_cast<std::size_t>(index)]);
}

XmlAttribute XmlAttributeCollection::GetNamedItem(std::string_view name) const {
  for (const XmlHandle &item : ::agiru::detail::Attributes(owner_)) {
    if (::agiru::detail::SameName(NodeOf(item), name)) { return XmlAttribute(item); }
  }
  return {};
}

XmlAttribute XmlAttributeCollection::SetNamedItem(const XmlNode &attribute) {
  xmlNodePtr owner = NodeOf(owner_);
  xmlNodePtr node = NodeOf(attribute.Handle());
  if (owner == nullptr || node == nullptr) {
    throw Error("XmlAttributeCollection.SetNamedItem: a null reference was used");
  }
  auto *source = reinterpret_cast<xmlAttrPtr>(node);
  xmlChar *value = xmlNodeGetContent(node);
  xmlAttrPtr set = source->ns == nullptr ? xmlSetProp(owner, source->name, value)
                                         : xmlSetNsProp(owner, source->ns, source->name, value);
  xmlFree(value);
  return XmlAttribute(XmlHandle(owner_.tree, set));
}

XmlAttribute XmlAttributeCollection::RemoveNamedItem(std::string_view name) {
  xmlNodePtr owner = NodeOf(owner_);
  if (owner == nullptr) { return {}; }
  xmlAttrPtr attribute = xmlHasProp(owner, Bytes(std::string(name)));
  if (attribute == nullptr) { return {}; }
  xmlAttrPtr copy = xmlCopyProp(nullptr, attribute);
  xmlRemoveProp(attribute);
  return XmlAttribute(::agiru::detail::Detached(reinterpret_cast<xmlNodePtr>(copy)));
}

std::vector<XmlAttribute>::const_iterator XmlAttributeCollection::begin() const {
  walked_.clear();
  for (const XmlHandle &item : ::agiru::detail::Attributes(owner_)) { walked_.emplace_back(item); }
  return walked_.begin();
}

std::vector<XmlAttribute>::const_iterator XmlAttributeCollection::end() const {
  return walked_.end();
}

void XmlNamespaces::AddNamespace(std::string_view prefix, std::string_view uri) {
  for (auto &[held, value] : declared_) {
    if (held == prefix) {
      value = std::string(uri);
      return;
    }
  }
  declared_.emplace_back(std::string(prefix), std::string(uri));
}

::agiru::Text<0> XmlNamespaces::LookupNamespace(std::string_view prefix) const {
  for (const auto &[held, value] : declared_) {
    if (held == prefix) { return value; }
  }
  return {};
}

::agiru::Text<0> XmlNamespaces::LookupPrefix(std::string_view uri) const {
  for (const auto &[held, value] : declared_) {
    if (value == uri) { return held; }
  }
  return {};
}

::agiru::Boolean XmlNamespaces::HasNamespace(std::string_view prefix) const {
  return std::ranges::any_of(declared_,
                             [prefix](const auto &entry) { return entry.first == prefix; });
}

void XmlTreeDocument::Load(const ::agiru::InStream &stream) {
  auto &input = const_cast<::agiru::InStream &>(stream);
  LoadXml(input.ReadBytes(input.Length()));
}

void XmlTreeDocument::Load(std::string_view filename) {
  auto result = ::agiru::detail::ReadXmlLocation(filename, preserveWhitespace_);
  if (!result.has_value()) { throw Error(result.error()); }
  handle_ = std::move(*result);
}

void XmlTreeDocument::Load(const XmlReader &reader) {
  reader.LoadDocument(handle_, preserveWhitespace_);
}

void XmlTreeDocument::LoadXml(std::string_view text) {
  auto result = ::agiru::detail::ReadXml(text, preserveWhitespace_);
  if (!result.has_value()) { throw Error(result.error()); }
  handle_ = std::move(*result);
}

void XmlTreeDocument::Save(const ::agiru::OutStream &stream) {
  if (handle_.Empty()) { throw Error("XmlDocument.Save: the document was never loaded"); }
  const_cast<::agiru::OutStream &>(stream).WriteBytes(::agiru::detail::Dump(handle_));
}

void XmlTreeDocument::Save(std::string_view filename) {
  if (handle_.Empty()) { throw Error("XmlDocument.Save: the document was never loaded"); }
  const std::string bytes = ::agiru::detail::Dump(handle_);
  std::ofstream file{std::string(filename), std::ios::binary | std::ios::trunc};
  if (!file) { throw Error("XmlDocument.Save: the file could not be opened for writing"); }
  file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

XmlElement XmlTreeDocument::DocumentElement() const {
  xmlDocPtr doc = DocOf(handle_);
  xmlNodePtr root = doc == nullptr ? nullptr : xmlDocGetRootElement(doc);
  return root == nullptr ? XmlElement{} : XmlElement(XmlHandle(handle_.tree, root));
}

XmlNode XmlTreeDocument::DocumentType() const {
  xmlDocPtr doc = DocOf(handle_);
  if (doc == nullptr || doc->intSubset == nullptr) { return {}; }
  return XmlNode(XmlHandle(handle_.tree, doc->intSubset));
}

XmlElement XmlTreeDocument::CreateElement(std::string_view name) const {
  return XmlElement(NewInDocument(handle_, xmlNewNode(nullptr, Bytes(std::string(name)))));
}

XmlElement XmlTreeDocument::CreateElement(std::string_view name, std::string_view uri) const {
  const std::string held(name);
  const std::size_t colon = held.find(':');
  if (colon == std::string::npos) { return CreateElement("", held, uri); }
  return CreateElement(held.substr(0, colon), held.substr(colon + 1), uri);
}

XmlElement XmlTreeDocument::CreateElement(std::string_view prefix,
                                          std::string_view local,
                                          std::string_view uri) const {
  return XmlElement(MakeElement(handle_, {.prefix = prefix, .local = local, .uri = uri}));
}

XmlAttribute XmlTreeDocument::CreateAttribute(std::string_view name) const {
  return CreateAttribute("", name, "");
}

XmlAttribute XmlTreeDocument::CreateAttribute(std::string_view name, std::string_view uri) const {
  const std::string held(name);
  const std::size_t colon = held.find(':');
  if (colon == std::string::npos) { return CreateAttribute("", held, uri); }
  return CreateAttribute(held.substr(0, colon), held.substr(colon + 1), uri);
}

XmlAttribute XmlTreeDocument::CreateAttribute(std::string_view prefix,
                                              std::string_view local,
                                              std::string_view uri) const {
  return XmlAttribute(MakeAttribute({.prefix = prefix, .local = local, .uri = uri}));
}

XmlNode XmlTreeDocument::CreateNode(std::string_view type,
                                    std::string_view name,
                                    std::string_view uri) const {
  if (type == "element") { return CreateElement(name, uri); }
  if (type == "attribute") { return CreateAttribute("", name, uri); }
  if (type == "text") { return CreateTextNode(""); }
  throw Error("XmlDocument.CreateNode: the node type '" + std::string(type) +
              "' is not one this runtime makes");
}

XmlNode XmlTreeDocument::CreateTextNode(std::string_view text) const {
  return XmlNode(NewInDocument(handle_, xmlNewText(Bytes(std::string(text)))));
}

XmlNode XmlTreeDocument::CreateCDataSection(std::string_view text) const {
  const std::string held(text);
  return XmlNode(NewInDocument(
      handle_, xmlNewCDataBlock(DocOf(handle_), Bytes(held), static_cast<int>(held.size()))));
}

XmlNode XmlTreeDocument::CreateComment(std::string_view text) const {
  return XmlNode(NewInDocument(handle_, xmlNewComment(Bytes(std::string(text)))));
}

XmlNode XmlTreeDocument::CreateProcessingInstruction(std::string_view target,
                                                     std::string_view data) const {
  return XmlNode(
      NewInDocument(handle_, xmlNewPI(Bytes(std::string(target)), Bytes(std::string(data)))));
}

XmlNode XmlTreeDocument::CreateXmlDeclaration(std::string_view version,
                                              std::string_view encoding,
                                              std::string_view standalone) {
  return XmlNode(MakeDeclaration(
      handle_, {.version = version, .encoding = encoding, .standalone = standalone}));
}

XmlNode XmlTreeDocument::CreateDocumentType(std::string_view name,
                                            std::string_view publicId,
                                            std::string_view systemId,
                                            std::string_view subset) {
  return XmlNode(MakeDocumentType(
      handle_, {.name = name, .publicId = publicId, .systemId = systemId, .subset = subset}));
}

XmlNodeList XmlTreeDocument::GetElementsByTagName(std::string_view name) const {
  return XmlNodeList(ByTagName(handle_, name));
}

::agiru::Text<0> XmlDocumentType::PublicId() const {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr || node->type != XML_DTD_NODE) { return {}; }
  return ::agiru::detail::Text(reinterpret_cast<xmlDtdPtr>(node)->ExternalID);
}

::agiru::Text<0> XmlDeclaration::Version() const {
  xmlDocPtr doc = DocOf(handle_);
  if (doc == nullptr || NodeOf(handle_) != reinterpret_cast<xmlNodePtr>(doc)) {
    throw Error("XmlDeclaration.Version: this node is not an XML declaration");
  }
  return ::agiru::detail::Text(doc->version);
}

::agiru::Text<0> XmlDeclaration::Encoding() const {
  xmlDocPtr doc = DocOf(handle_);
  if (doc == nullptr || NodeOf(handle_) != reinterpret_cast<xmlNodePtr>(doc)) {
    throw Error("XmlDeclaration.Encoding: this node is not an XML declaration");
  }
  return ::agiru::detail::Text(doc->encoding);
}

::agiru::Text<0> XmlDeclaration::Standalone() const {
  xmlDocPtr doc = DocOf(handle_);
  if (doc == nullptr || NodeOf(handle_) != reinterpret_cast<xmlNodePtr>(doc)) {
    throw Error("XmlDeclaration.Standalone: this node is not an XML declaration");
  }
  if (doc->standalone < 0) { return {}; }
  return doc->standalone == 0 ? "no" : "yes";
}

::agiru::Text<0> XmlDocumentType::SystemId() const {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr || node->type != XML_DTD_NODE) { return {}; }
  return ::agiru::detail::Text(reinterpret_cast<xmlDtdPtr>(node)->SystemID);
}

}

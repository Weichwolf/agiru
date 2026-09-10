#include "dotnet/XmlDocument.h"
#include "dotnet/XmlNode.h"
#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/XmlHandle.h"

#include "XmlEngine.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <libxml/tree.h>

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
  return XmlHandle(document.tree, node);
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

std::string XmlNode::InnerText() const {
  return ContentOf(handle_);
}

std::string XmlNode::InnerText(std::string_view text) {
  if (NodeOf(handle_) != nullptr) {
    const std::string held(text);
    xmlNodeSetContent(NodeOf(handle_), Bytes(held));
  }
  return std::string(text);
}

std::string XmlNode::InnerXml() const {
  return ::agiru::detail::DumpChildren(handle_);
}

std::string XmlNode::InnerXml(std::string_view markup) {
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

std::string XmlNode::OuterXml() const {
  return ::agiru::detail::Dump(handle_);
}

std::string XmlNode::LocalName() const {
  return NodeOf(handle_) == nullptr ? std::string{} : ::agiru::detail::Text(NodeOf(handle_)->name);
}

std::string XmlNode::Name() const {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr) { return {}; }
  if (node->type == XML_DOCUMENT_NODE) { return "#document"; }
  if (node->type == XML_TEXT_NODE) { return "#text"; }
  if (node->type == XML_CDATA_SECTION_NODE) { return "#cdata-section"; }
  if (node->type == XML_COMMENT_NODE) { return "#comment"; }
  return ::agiru::detail::QualifiedName(node);
}

std::string XmlNode::NamespaceURI() const {
  xmlNodePtr node = NodeOf(handle_);
  return node == nullptr || node->ns == nullptr ? std::string{}
                                                : ::agiru::detail::Text(node->ns->href);
}

std::string XmlNode::Prefix() const {
  xmlNodePtr node = NodeOf(handle_);
  return node == nullptr || node->ns == nullptr ? std::string{}
                                                : ::agiru::detail::Text(node->ns->prefix);
}

std::string XmlNode::Value() const {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr || node->type == XML_ELEMENT_NODE || node->type == XML_DOCUMENT_NODE) {
    return {};
  }
  return ContentOf(handle_);
}

std::string XmlNode::Value(std::string_view text) {
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

std::string XmlElement::GetAttribute(std::string_view name) const {
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

void XmlNamespaceManager::AddNamespace(std::string_view prefix, std::string_view uri) {
  for (auto &[held, value] : declared_) {
    if (held == prefix) {
      value = std::string(uri);
      return;
    }
  }
  declared_.emplace_back(std::string(prefix), std::string(uri));
}

std::string XmlNamespaceManager::LookupNamespace(std::string_view prefix) const {
  for (const auto &[held, value] : declared_) {
    if (held == prefix) { return value; }
  }
  return {};
}

std::string XmlNamespaceManager::LookupPrefix(std::string_view uri) const {
  for (const auto &[held, value] : declared_) {
    if (value == uri) { return held; }
  }
  return {};
}

::agiru::Boolean XmlNamespaceManager::HasNamespace(std::string_view prefix) const {
  for (const auto &[held, value] : declared_) {
    if (held == prefix) { return true; }
  }
  return false;
}

void XmlDocument::Load(const ::agiru::InStream &stream) {
  auto &input = const_cast<::agiru::InStream &>(stream);
  LoadXml(input.ReadBytes(input.Length()));
}

void XmlDocument::LoadXml(std::string_view text) {
  XmlHandle read;
  if (!::agiru::detail::Parse(text, preserveWhitespace_, read)) {
    throw Error("XmlDocument.LoadXml: the data at the root level is invalid, or the document is "
                "not well-formed XML");
  }
  handle_ = read;
}

void XmlDocument::Save(const ::agiru::OutStream &stream) {
  if (handle_.Empty()) { throw Error("XmlDocument.Save: the document was never loaded"); }
  const_cast<::agiru::OutStream &>(stream).WriteBytes(::agiru::detail::Dump(handle_));
}

XmlElement XmlDocument::DocumentElement() const {
  xmlDocPtr doc = DocOf(handle_);
  xmlNodePtr root = doc == nullptr ? nullptr : xmlDocGetRootElement(doc);
  return root == nullptr ? XmlElement{} : XmlElement(XmlHandle(handle_.tree, root));
}

XmlNode XmlDocument::DocumentType() const {
  xmlDocPtr doc = DocOf(handle_);
  if (doc == nullptr || doc->intSubset == nullptr) { return {}; }
  return XmlNode(XmlHandle(handle_.tree, doc->intSubset));
}

XmlElement XmlDocument::CreateElement(std::string_view name) const {
  return XmlElement(NewInDocument(handle_, xmlNewNode(nullptr, Bytes(std::string(name)))));
}

XmlElement XmlDocument::CreateElement(std::string_view name, std::string_view uri) const {
  const std::string held(name);
  const std::size_t colon = held.find(':');
  if (colon == std::string::npos) { return CreateElement("", held, uri); }
  return CreateElement(held.substr(0, colon), held.substr(colon + 1), uri);
}

XmlElement XmlDocument::CreateElement(std::string_view prefix,
                                      std::string_view local,
                                      std::string_view uri) const {
  xmlNodePtr node = xmlNewNode(nullptr, Bytes(std::string(local)));
  const XmlHandle made = NewInDocument(handle_, node);
  if (!uri.empty()) {
    const std::string heldPrefix(prefix);
    xmlNsPtr ns =
        xmlNewNs(node, Bytes(std::string(uri)), heldPrefix.empty() ? nullptr : Bytes(heldPrefix));
    xmlSetNs(node, ns);
  }
  return XmlElement(made);
}

XmlAttribute XmlDocument::CreateAttribute(std::string_view name) const {
  return CreateAttribute("", name, "");
}

XmlAttribute XmlDocument::CreateAttribute(std::string_view name, std::string_view uri) const {
  const std::string held(name);
  const std::size_t colon = held.find(':');
  if (colon == std::string::npos) { return CreateAttribute("", held, uri); }
  return CreateAttribute(held.substr(0, colon), held.substr(colon + 1), uri);
}

XmlAttribute XmlDocument::CreateAttribute(std::string_view prefix,
                                          std::string_view local,
                                          std::string_view uri) const {
  xmlNodePtr holder = xmlNewNode(nullptr, Bytes("attribute"));
  const XmlHandle tree = ::agiru::detail::Detached(holder);
  xmlAttrPtr attribute = nullptr;
  if (uri.empty()) {
    attribute = xmlNewProp(holder, Bytes(std::string(local)), Bytes(""));
  } else {
    const std::string heldPrefix(prefix);
    xmlNsPtr ns =
        xmlNewNs(holder, Bytes(std::string(uri)), heldPrefix.empty() ? nullptr : Bytes(heldPrefix));
    attribute = xmlNewNsProp(holder, ns, Bytes(std::string(local)), Bytes(""));
  }
  return XmlAttribute(XmlHandle(tree.tree, attribute));
}

XmlNode
XmlDocument::CreateNode(std::string_view type, std::string_view name, std::string_view uri) const {
  if (type == "element") { return CreateElement(name, uri); }
  if (type == "attribute") { return CreateAttribute("", name, uri); }
  if (type == "text") { return CreateTextNode(""); }
  throw Error("XmlDocument.CreateNode: the node type '" + std::string(type) +
              "' is not one this runtime makes");
}

XmlNode XmlDocument::CreateTextNode(std::string_view text) const {
  return XmlNode(NewInDocument(handle_, xmlNewText(Bytes(std::string(text)))));
}

XmlNode XmlDocument::CreateCDataSection(std::string_view text) const {
  const std::string held(text);
  return XmlNode(NewInDocument(
      handle_, xmlNewCDataBlock(DocOf(handle_), Bytes(held), static_cast<int>(held.size()))));
}

XmlNode XmlDocument::CreateComment(std::string_view text) const {
  return XmlNode(NewInDocument(handle_, xmlNewComment(Bytes(std::string(text)))));
}

XmlNode XmlDocument::CreateProcessingInstruction(std::string_view target,
                                                 std::string_view data) const {
  return XmlNode(
      NewInDocument(handle_, xmlNewPI(Bytes(std::string(target)), Bytes(std::string(data)))));
}

XmlNode XmlDocument::CreateXmlDeclaration(std::string_view version,
                                          std::string_view encoding,
                                          std::string_view standalone) {
  xmlDocPtr doc = DocOf(handle_);
  if (doc == nullptr) {
    throw Error("XmlDocument.CreateXmlDeclaration: the document was never made");
  }
  xmlFree(const_cast<xmlChar *>(doc->version));
  doc->version = xmlStrdup(Bytes(std::string(version)));
  xmlFree(const_cast<xmlChar *>(doc->encoding));
  doc->encoding = encoding.empty() ? nullptr : xmlStrdup(Bytes(std::string(encoding)));
  doc->standalone = standalone == "yes" ? 1 : standalone == "no" ? 0 : -1;
  return XmlNode(XmlHandle(handle_.tree, doc));
}

XmlNode XmlDocument::CreateDocumentType(std::string_view name,
                                        std::string_view publicId,
                                        std::string_view systemId,
                                        std::string_view subset) {
  xmlDocPtr doc = DocOf(handle_);
  if (doc == nullptr) {
    throw Error("XmlDocument.CreateDocumentType: the document was never made");
  }
  static_cast<void>(subset);
  const std::string heldName(name);
  const std::string heldPublic(publicId);
  const std::string heldSystem(systemId);
  xmlDtdPtr dtd = xmlNewDtd(nullptr,
                            Bytes(heldName),
                            heldPublic.empty() ? nullptr : Bytes(heldPublic),
                            heldSystem.empty() ? nullptr : Bytes(heldSystem));
  return XmlNode(NewInDocument(handle_, reinterpret_cast<xmlNodePtr>(dtd)));
}

XmlNodeList XmlDocument::GetElementsByTagName(std::string_view name) const {
  return XmlNodeList(ByTagName(handle_, name));
}

std::string XmlDocumentType::PublicId() const {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr || node->type != XML_DTD_NODE) { return {}; }
  return ::agiru::detail::Text(reinterpret_cast<xmlDtdPtr>(node)->ExternalID);
}

std::string XmlDocumentType::SystemId() const {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr || node->type != XML_DTD_NODE) { return {}; }
  return ::agiru::detail::Text(reinterpret_cast<xmlDtdPtr>(node)->SystemID);
}

}

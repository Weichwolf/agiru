#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/Text.h"
#include "type/Variant.h"
#include "type/XmlAttribute.h"
#include "type/XmlAttributeCollection.h"
#include "type/XmlCData.h"
#include "type/XmlComment.h"
#include "type/XmlDeclaration.h"
#include "type/XmlDocument.h"
#include "type/XmlDocumentType.h"
#include "type/XmlElement.h"
#include "type/XmlHandle.h"
#include "type/XmlNameTable.h"
#include "type/XmlNamespaceManager.h"
#include "type/XmlNode.h"
#include "type/XmlNodeList.h"
#include "type/XmlProcessingInstruction.h"
#include "type/XmlReadOptions.h"
#include "type/XmlText.h"
#include "type/XmlWriteOptions.h"

#include "XmlEngine.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <libxml/tree.h>

namespace agiru {

using detail::DocOf;
using detail::Dump;
using detail::NodeOf;
using detail::XmlHandle;

namespace {

const xmlChar *Bytes(std::string_view text) {
  return reinterpret_cast<const xmlChar *>(text.data());
}

std::string ContentOf(const XmlHandle &handle) {
  xmlNodePtr node = NodeOf(handle);
  if (node == nullptr) { return {}; }
  xmlChar *text = xmlNodeGetContent(node);
  const std::string out = detail::Text(text);
  xmlFree(text);
  return out;
}

std::string SetContent(const XmlHandle &handle, std::string_view value) {
  xmlNodePtr node = NodeOf(handle);
  if (node == nullptr) { return {}; }
  const std::string held(value);
  if (node->type == XML_ATTRIBUTE_NODE) {
    auto *attribute = reinterpret_cast<xmlAttrPtr>(node);
    xmlSetNsProp(attribute->parent, attribute->ns, attribute->name, Bytes(held));
  } else {
    xmlNodeSetContent(node, Bytes(held));
  }
  return held;
}

bool ParentOf(const XmlHandle &handle, XmlElement &parent) {
  xmlNodePtr node = NodeOf(handle);
  if (node == nullptr || node->parent == nullptr || node->parent->type != XML_ELEMENT_NODE) {
    return false;
  }
  parent = XmlElement(XmlHandle(handle.tree, node->parent));
  return true;
}

bool DocumentOf(const XmlHandle &handle, XmlDocument &document) {
  if (NodeOf(handle) == nullptr || DocOf(handle) == nullptr) { return false; }
  document = XmlDocument(XmlHandle(handle.tree, DocOf(handle)));
  return true;
}

bool RemoveNode(const XmlHandle &handle) {
  xmlNodePtr node = NodeOf(handle);
  if (node == nullptr) { return false; }
  if (node->type == XML_ATTRIBUTE_NODE) {
    return xmlRemoveProp(reinterpret_cast<xmlAttrPtr>(node)) == 0;
  }
  if (node->parent == nullptr) { return false; }
  xmlUnlinkNode(node);
  return true;
}

bool WriteAll(const XmlHandle &handle, const OutStream &stream) {
  if (NodeOf(handle) == nullptr) { return false; }
  const_cast<OutStream &>(stream).WriteBytes(Dump(handle));
  return true;
}

bool WriteAll(const XmlHandle &handle, Text<0> &text) {
  if (NodeOf(handle) == nullptr) { return false; }
  text = std::string_view(Dump(handle));
  return true;
}

bool Select(const XmlHandle &from,
            std::string_view xpath,
            const std::vector<std::pair<std::string, std::string>> &namespaces,
            XmlNode &node) {
  const std::vector<XmlHandle> found = detail::XPath(from, xpath, namespaces);
  if (found.empty()) { return false; }
  node = XmlNode(found.front());
  return true;
}

bool SelectAll(const XmlHandle &from,
               std::string_view xpath,
               const std::vector<std::pair<std::string, std::string>> &namespaces,
               XmlNodeList &list) {
  list = XmlNodeList(detail::XPath(from, xpath, namespaces));
  return true;
}

const std::vector<std::pair<std::string, std::string>> &NoNamespaces() {
  static const std::vector<std::pair<std::string, std::string>> none;
  return none;
}

XmlHandle NodeFrom(const Variant &content) {
  if (content.IsText()) { return detail::Detached(xmlNewText(Bytes(content.Get<std::string>()))); }
  if (const XmlHandle *held = content.XmlHeld(); held != nullptr) { return *held; }
  throw Error("XML content must be a node or a text, and this Variant holds neither");
}

bool AddBeside(const XmlHandle &self, const Variant &content, bool after) {
  xmlNodePtr node = NodeOf(self);
  if (node == nullptr || node->parent == nullptr || node->type == XML_ATTRIBUTE_NODE) {
    return false;
  }
  const XmlHandle other = NodeFrom(content);
  xmlNodePtr added = NodeOf(other);
  if (!detail::Attachable(node->parent, added)) { return false; }
  xmlUnlinkNode(added);
  if (after) {
    xmlAddNextSibling(node, added);
  } else {
    xmlAddPrevSibling(node, added);
  }
  if (self.tree != other.tree) {
    xmlSetTreeDoc(added, DocOf(self));
    detail::KeepAlive(self.tree, other.tree);
    detail::KeepAlive(other.tree, self.tree);
  }
  return true;
}

bool Replace(const XmlHandle &self, const Variant &content) {
  xmlNodePtr node = NodeOf(self);
  if (node == nullptr || node->parent == nullptr || node->type == XML_ATTRIBUTE_NODE) {
    return false;
  }
  const XmlHandle other = NodeFrom(content);
  xmlNodePtr added = NodeOf(other);
  if (!detail::Attachable(node->parent, added)) { return false; }
  xmlUnlinkNode(added);
  xmlReplaceNode(node, added);
  if (self.tree != other.tree) {
    xmlSetTreeDoc(added, DocOf(self));
    detail::KeepAlive(self.tree, other.tree);
    detail::KeepAlive(other.tree, self.tree);
  }
  return true;
}

}

::agiru::XmlAttribute XmlAttribute::Create(std::string_view Name, std::string_view Value) {
  xmlNodePtr holder = xmlNewNode(nullptr, Bytes("attribute"));
  const XmlHandle tree = detail::Detached(holder);
  xmlAttrPtr attribute = xmlNewProp(holder, Bytes(std::string(Name)), Bytes(std::string(Value)));
  return XmlAttribute(XmlHandle(tree.tree, attribute));
}

::agiru::XmlAttribute XmlAttribute::Create(std::string_view LocalName,
                                           std::string_view NamespaceUri,
                                           std::string_view Value) {
  xmlNodePtr holder = xmlNewNode(nullptr, Bytes("attribute"));
  const XmlHandle tree = detail::Detached(holder);
  xmlNsPtr ns =
      NamespaceUri.empty() ? nullptr : xmlNewNs(holder, Bytes(std::string(NamespaceUri)), nullptr);
  xmlAttrPtr attribute =
      xmlNewNsProp(holder, ns, Bytes(std::string(LocalName)), Bytes(std::string(Value)));
  return XmlAttribute(XmlHandle(tree.tree, attribute));
}

::agiru::XmlAttribute XmlAttribute::CreateNamespaceDeclaration(std::string_view Prefix,
                                                               std::string_view Uri) {
  const std::string name = Prefix.empty() ? std::string("xmlns") : "xmlns:" + std::string(Prefix);
  return Create(name, Uri);
}

::agiru::Boolean XmlAttribute::AddAfterSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, true);
}

::agiru::Boolean XmlAttribute::AddBeforeSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, false);
}

::agiru::XmlNode XmlAttribute::AsXmlNode() {
  return XmlNode(handle_);
}

::agiru::Boolean XmlAttribute::GetDocument(::agiru::XmlDocument &Document) {
  return DocumentOf(handle_, Document);
}

::agiru::Boolean XmlAttribute::GetParent(::agiru::XmlElement &Parent) {
  return ParentOf(handle_, Parent);
}

::agiru::Boolean XmlAttribute::IsNamespaceDeclaration() {
  const std::string name = Name();
  return name == "xmlns" || name.starts_with("xmlns:");
}

std::string XmlAttribute::LocalName() {
  return NodeOf(handle_) == nullptr ? std::string{} : detail::Text(NodeOf(handle_)->name);
}

std::string XmlAttribute::Name() {
  return detail::QualifiedName(NodeOf(handle_));
}

std::string XmlAttribute::NamespacePrefix() {
  xmlNodePtr node = NodeOf(handle_);
  return node == nullptr || node->ns == nullptr ? std::string{} : detail::Text(node->ns->prefix);
}

std::string XmlAttribute::NamespaceUri() {
  xmlNodePtr node = NodeOf(handle_);
  return node == nullptr || node->ns == nullptr ? std::string{} : detail::Text(node->ns->href);
}

::agiru::Boolean XmlAttribute::Remove() {
  return RemoveNode(handle_);
}

::agiru::Boolean XmlAttribute::ReplaceWith(const ::agiru::Variant &Node) {
  return Replace(handle_, Node);
}

::agiru::Boolean XmlAttribute::SelectNodes(std::string_view XPath,
                                           const ::agiru::XmlNamespaceManager &NamespaceManager,
                                           ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NamespaceManager.Declared(), NodeList);
}

::agiru::Boolean XmlAttribute::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NoNamespaces(), NodeList);
}

::agiru::Boolean
XmlAttribute::SelectSingleNode(std::string_view XPath,
                               const ::agiru::XmlNamespaceManager &NamespaceManager,
                               ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NamespaceManager.Declared(), Node);
}

::agiru::Boolean XmlAttribute::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NoNamespaces(), Node);
}

std::string XmlAttribute::Value() {
  return ContentOf(handle_);
}

std::string XmlAttribute::Value(std::string_view NewValue) {
  return SetContent(handle_, NewValue);
}

::agiru::Boolean XmlAttribute::WriteTo(const ::agiru::OutStream &OutStream) {
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlAttribute::WriteTo(::agiru::Text<0> &Text) {
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlAttribute::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                       const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlAttribute::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                       ::agiru::Text<0> &Text) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, Text);
}

::agiru::Integer XmlNodeList::Count() {
  return static_cast<Integer>(items_.size());
}

::agiru::Boolean XmlNodeList::Get(::agiru::Integer Index, ::agiru::XmlNode &Node) {
  if (Index < 1 || static_cast<std::size_t>(Index) > items_.size()) { return false; }
  Node = XmlNode(items_[static_cast<std::size_t>(Index) - 1]);
  return true;
}

::agiru::Integer XmlAttributeCollection::Count() {
  return static_cast<Integer>(items_.size());
}

::agiru::Boolean XmlAttributeCollection::Get(::agiru::Integer Index,
                                             ::agiru::XmlAttribute &Result) {
  if (Index < 1 || static_cast<std::size_t>(Index) > items_.size()) { return false; }
  Result = XmlAttribute(items_[static_cast<std::size_t>(Index) - 1]);
  return true;
}

::agiru::Boolean XmlAttributeCollection::Get(std::string_view Name, ::agiru::XmlAttribute &Result) {
  for (const XmlHandle &item : items_) {
    if (detail::SameName(NodeOf(item), Name)) {
      Result = XmlAttribute(item);
      return true;
    }
  }
  return false;
}

void XmlAttributeCollection::Remove(std::string_view Name) {
  for (auto at = items_.begin(); at != items_.end(); ++at) {
    if (detail::SameName(NodeOf(*at), Name)) {
      xmlRemoveProp(reinterpret_cast<xmlAttrPtr>(NodeOf(*at)));
      items_.erase(at);
      return;
    }
  }
}

void XmlAttributeCollection::Remove(std::string_view LocalName, std::string_view NamespaceUri) {
  for (auto at = items_.begin(); at != items_.end(); ++at) {
    if (detail::SameLocalName(NodeOf(*at), LocalName, NamespaceUri)) {
      xmlRemoveProp(reinterpret_cast<xmlAttrPtr>(NodeOf(*at)));
      items_.erase(at);
      return;
    }
  }
}

void XmlAttributeCollection::Remove(const ::agiru::XmlAttribute &Attribute) {
  const auto at = std::ranges::find_if(
      items_, [&Attribute](const XmlHandle &item) { return item.node == Attribute.Handle().node; });
  if (at == items_.end()) { return; }
  xmlRemoveProp(reinterpret_cast<xmlAttrPtr>(NodeOf(*at)));
  items_.erase(at);
}

void XmlAttributeCollection::RemoveAll() {
  for (const XmlHandle &item : items_) {
    xmlRemoveProp(reinterpret_cast<xmlAttrPtr>(NodeOf(item)));
  }
  items_.clear();
}

void XmlAttributeCollection::Set(std::string_view Name, std::string_view Value) {
  if (items_.empty()) { return; }
  xmlNodePtr owner = reinterpret_cast<xmlAttrPtr>(NodeOf(items_.front()))->parent;
  if (owner == nullptr) { return; }
  xmlAttrPtr set = xmlSetProp(owner, Bytes(std::string(Name)), Bytes(std::string(Value)));
  if (std::ranges::none_of(items_, [set](const XmlHandle &item) { return item.node == set; })) {
    items_.emplace_back(items_.front().tree, set);
  }
}

void XmlAttributeCollection::Set(std::string_view LocalName,
                                 std::string_view NamespaceUri,
                                 std::string_view Value) {
  if (items_.empty()) { return; }
  xmlNodePtr owner = reinterpret_cast<xmlAttrPtr>(NodeOf(items_.front()))->parent;
  if (owner == nullptr) { return; }
  xmlNsPtr ns = xmlSearchNsByHref(owner->doc, owner, Bytes(std::string(NamespaceUri)));
  xmlAttrPtr set =
      xmlSetNsProp(owner, ns, Bytes(std::string(LocalName)), Bytes(std::string(Value)));
  if (std::ranges::none_of(items_, [set](const XmlHandle &item) { return item.node == set; })) {
    items_.emplace_back(items_.front().tree, set);
  }
}

void XmlNamespaceManager::AddNamespace(std::string_view Prefix, std::string_view Uri) {
  for (auto &[prefix, uri] : declared_) {
    if (prefix == Prefix) {
      uri = std::string(Uri);
      return;
    }
  }
  declared_.emplace_back(std::string(Prefix), std::string(Uri));
}

::agiru::Boolean XmlNamespaceManager::HasNamespace(std::string_view Prefix) {
  return std::ranges::any_of(declared_,
                             [Prefix](const auto &entry) { return entry.first == Prefix; });
}

::agiru::Boolean XmlNamespaceManager::LookupNamespace(std::string_view Prefix,
                                                      ::agiru::Text<0> &Result) {
  for (const auto &[prefix, uri] : declared_) {
    if (prefix == Prefix) {
      Result = std::string_view(uri);
      return true;
    }
  }
  return false;
}

::agiru::Boolean XmlNamespaceManager::LookupPrefix(std::string_view Uri, ::agiru::Text<0> &Result) {
  for (const auto &[prefix, uri] : declared_) {
    if (uri == Uri) {
      Result = std::string_view(prefix);
      return true;
    }
  }
  return false;
}

::agiru::XmlNameTable XmlNamespaceManager::NameTable() {
  return {};
}

::agiru::XmlNameTable XmlNamespaceManager::NameTable(const ::agiru::XmlNameTable &NewValue) {
  return NewValue;
}

void XmlNamespaceManager::PopScope() {}

void XmlNamespaceManager::PushScope() {}

void XmlNamespaceManager::RemoveNamespace(std::string_view Prefix, std::string_view Uri) {
  std::erase_if(declared_, [Prefix, Uri](const auto &entry) {
    return entry.first == Prefix && entry.second == Uri;
  });
}

std::string XmlNameTable::Add(std::string_view Key) {
  return std::string(Key);
}

::agiru::Boolean XmlNameTable::Get(std::string_view Key, ::agiru::Text<0> &Result) {
  Result = Key;
  return true;
}

::agiru::Boolean XmlReadOptions::PreserveWhitespace() {
  return preserveWhitespace_;
}

::agiru::Boolean XmlReadOptions::PreserveWhitespace(::agiru::Boolean NewValue) {
  preserveWhitespace_ = NewValue;
  return preserveWhitespace_;
}

::agiru::Boolean XmlWriteOptions::PreserveWhitespace() {
  return preserveWhitespace_;
}

::agiru::Boolean XmlWriteOptions::PreserveWhitespace(::agiru::Boolean NewValue) {
  preserveWhitespace_ = NewValue;
  return preserveWhitespace_;
}

}

namespace agiru {

::agiru::XmlNode *XmlNodeList::begin() {
  walked_.clear();
  for (const detail::XmlHandle &item : items_) { walked_.emplace_back(item); }
  return walked_.data();
}

::agiru::XmlNode *XmlNodeList::end() {
  return walked_.data() + walked_.size();
}

}

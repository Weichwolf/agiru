#include "XmlEngine.h"

#include "runtime/Error.h"
#include "type/Boolean.h"
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

#include <libxml/tree.h>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru {

using detail::Adopt;
using detail::Children;
using detail::Descendants;
using detail::DocOf;
using detail::Dump;
using detail::DumpChildren;
using detail::NodeOf;
using detail::XmlHandle;

namespace {

const xmlChar *Bytes(std::string_view text) {
  return reinterpret_cast<const xmlChar *>(text.data());
}

std::string Content(const XmlHandle &handle) {
  xmlNodePtr node = NodeOf(handle);
  if (node == nullptr) { return {}; }
  xmlChar *text = xmlNodeGetContent(node);
  const std::string out = detail::Text(text);
  xmlFree(text);
  return out;
}

bool IsKind(const XmlHandle &handle, xmlElementType type) {
  return NodeOf(handle) != nullptr && NodeOf(handle)->type == type;
}

template <typename T> T As(const XmlHandle &handle, xmlElementType type, std::string_view what) {
  if (!IsKind(handle, type)) {
    throw Error(std::string("The XmlNode is not an ") + std::string(what) + ".");
  }
  return T(handle);
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
  if (node == nullptr || node->parent == nullptr) { return false; }
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

XmlHandle NodeFrom(const Variant &content) {
  if (content.IsText()) {
    return detail::Detached(xmlNewText(Bytes(content.Get<std::string>())));
  }
  if (const XmlHandle *held = content.XmlHeld(); held != nullptr) { return *held; }
  throw Error("XML content must be a node or a text, and this Variant holds neither");
}

bool AddInto(const XmlHandle &parent, const Variant &content, bool first) {
  if (NodeOf(parent) == nullptr) { return false; }
  const XmlHandle child = NodeFrom(content);
  if (first && NodeOf(parent)->children != nullptr) {
    xmlNodePtr node = NodeOf(child);
    xmlUnlinkNode(node);
    xmlAddPrevSibling(NodeOf(parent)->children, node);
    if (parent.tree != child.tree) {
      xmlSetTreeDoc(node, DocOf(parent));
      detail::KeepAlive(parent.tree, child.tree);
      detail::KeepAlive(child.tree, parent.tree);
    }
    return true;
  }
  Adopt(parent, child);
  return true;
}

bool AddBeside(const XmlHandle &self, const Variant &content, bool after) {
  xmlNodePtr node = NodeOf(self);
  if (node == nullptr || node->parent == nullptr) { return false; }
  const XmlHandle other = NodeFrom(content);
  xmlNodePtr added = NodeOf(other);
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
  if (node == nullptr || node->parent == nullptr) { return false; }
  const XmlHandle other = NodeFrom(content);
  xmlNodePtr added = NodeOf(other);
  xmlUnlinkNode(added);
  xmlReplaceNode(node, added);
  if (self.tree != other.tree) {
    xmlSetTreeDoc(added, DocOf(self));
    detail::KeepAlive(self.tree, other.tree);
    detail::KeepAlive(other.tree, self.tree);
  }
  return true;
}

void ClearChildren(const XmlHandle &self) {
  xmlNodePtr node = NodeOf(self);
  if (node == nullptr) { return; }
  while (node->children != nullptr) {
    xmlNodePtr child = node->children;
    xmlUnlinkNode(child);
    xmlFreeNode(child);
  }
}

std::vector<XmlHandle> Named(std::vector<XmlHandle> nodes, std::string_view name) {
  std::vector<XmlHandle> out;
  for (XmlHandle &node : nodes) {
    if (detail::SameName(NodeOf(node), name)) { out.push_back(std::move(node)); }
  }
  return out;
}

std::vector<XmlHandle>
LocallyNamed(std::vector<XmlHandle> nodes, std::string_view local, std::string_view uri) {
  std::vector<XmlHandle> out;
  for (XmlHandle &node : nodes) {
    if (detail::SameLocalName(NodeOf(node), local, uri)) { out.push_back(std::move(node)); }
  }
  return out;
}

const std::vector<std::pair<std::string, std::string>> &NoNamespaces() {
  static const std::vector<std::pair<std::string, std::string>> none;
  return none;
}

}

::agiru::XmlDocument XmlDocument::Create() {
  xmlDocPtr doc = xmlNewDoc(Bytes("1.0"));
  return XmlDocument(XmlHandle(detail::NewTree(doc), doc));
}

::agiru::XmlDocument XmlDocument::Create(const ::agiru::Variant &Content) {
  XmlDocument document = Create();
  static_cast<void>(document.Add(Content));
  return document;
}

::agiru::Boolean XmlDocument::ReadFrom(std::string_view Text, ::agiru::XmlDocument &Result) {
  XmlHandle read;
  if (!detail::Parse(Text, false, read)) { return false; }
  Result = XmlDocument(read);
  return true;
}

::agiru::Boolean XmlDocument::ReadFrom(std::string_view Text,
                                       const ::agiru::XmlReadOptions &ReadOptions,
                                       ::agiru::XmlDocument &Result) {
  XmlHandle read;
  if (!detail::Parse(Text, const_cast<XmlReadOptions &>(ReadOptions).PreserveWhitespace(), read)) {
    return false;
  }
  Result = XmlDocument(read);
  return true;
}

::agiru::Boolean XmlDocument::ReadFrom(const ::agiru::InStream &InStream,
                                       ::agiru::XmlDocument &Result) {
  auto &stream = const_cast<::agiru::InStream &>(InStream);
  return ReadFrom(std::string_view(stream.ReadBytes(stream.Length())), Result);
}

::agiru::Boolean XmlDocument::ReadFrom(const ::agiru::InStream &InStream,
                                       const ::agiru::XmlReadOptions &ReadOptions,
                                       ::agiru::XmlDocument &Result) {
  auto &stream = const_cast<::agiru::InStream &>(InStream);
  return ReadFrom(std::string_view(stream.ReadBytes(stream.Length())), ReadOptions, Result);
}

::agiru::Boolean XmlDocument::Add(const ::agiru::Variant &Content) {
  return AddInto(handle_, Content, false);
}

::agiru::Boolean XmlDocument::AddFirst(const ::agiru::Variant &Content) {
  return AddInto(handle_, Content, true);
}

::agiru::Boolean XmlDocument::AddAfterSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  return false;
}

::agiru::Boolean XmlDocument::AddBeforeSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  return false;
}

::agiru::XmlNode XmlDocument::AsXmlNode() {
  return XmlNode(handle_);
}

::agiru::XmlNodeList XmlDocument::GetChildElements() {
  return XmlNodeList(Children(handle_, true));
}

::agiru::XmlNodeList XmlDocument::GetChildElements(std::string_view Name) {
  return XmlNodeList(Named(Children(handle_, true), Name));
}

::agiru::XmlNodeList XmlDocument::GetChildElements(std::string_view LocalName,
                                                   std::string_view NamespaceUri) {
  return XmlNodeList(LocallyNamed(Children(handle_, true), LocalName, NamespaceUri));
}

::agiru::XmlNodeList XmlDocument::GetChildNodes() {
  return XmlNodeList(Children(handle_, false));
}

::agiru::XmlNodeList XmlDocument::GetDescendantElements() {
  return XmlNodeList(Descendants(handle_, true));
}

::agiru::XmlNodeList XmlDocument::GetDescendantElements(std::string_view Name) {
  return XmlNodeList(Named(Descendants(handle_, true), Name));
}

::agiru::XmlNodeList XmlDocument::GetDescendantElements(std::string_view LocalName,
                                                        std::string_view NamespaceUri) {
  return XmlNodeList(LocallyNamed(Descendants(handle_, true), LocalName, NamespaceUri));
}

::agiru::XmlNodeList XmlDocument::GetDescendantNodes() {
  return XmlNodeList(Descendants(handle_, false));
}

::agiru::Boolean XmlDocument::GetDeclaration(::agiru::XmlDeclaration &Result) {
  if (NodeOf(handle_) == nullptr) { return false; }
  Result = XmlDeclaration(handle_);
  return true;
}

::agiru::Boolean XmlDocument::SetDeclaration(const ::agiru::XmlDeclaration &Declaration) {
  xmlDocPtr doc = DocOf(handle_);
  xmlDocPtr from = DocOf(Declaration.Handle());
  if (doc == nullptr || from == nullptr) { return false; }
  xmlFree(const_cast<xmlChar *>(doc->version));
  doc->version = xmlStrdup(from->version);
  xmlFree(const_cast<xmlChar *>(doc->encoding));
  doc->encoding = from->encoding == nullptr ? nullptr : xmlStrdup(from->encoding);
  doc->standalone = from->standalone;
  return true;
}

::agiru::Boolean XmlDocument::GetDocument(::agiru::XmlDocument &Document) {
  return DocumentOf(handle_, Document);
}

::agiru::Boolean XmlDocument::GetDocumentType(::agiru::XmlDocumentType &DocumentType) {
  xmlDocPtr doc = DocOf(handle_);
  if (doc == nullptr || doc->intSubset == nullptr) { return false; }
  DocumentType = XmlDocumentType(XmlHandle(handle_.tree, doc->intSubset));
  return true;
}

::agiru::Boolean XmlDocument::GetParent(::agiru::XmlElement &Parent) {
  static_cast<void>(Parent);
  return false;
}

::agiru::Boolean XmlDocument::GetRoot(::agiru::XmlElement &Result) {
  xmlDocPtr doc = DocOf(handle_);
  xmlNodePtr root = doc == nullptr ? nullptr : xmlDocGetRootElement(doc);
  if (root == nullptr) { return false; }
  Result = XmlElement(XmlHandle(handle_.tree, root));
  return true;
}

::agiru::XmlNameTable XmlDocument::NameTable() {
  return {};
}

::agiru::Boolean XmlDocument::Remove() {
  return false;
}

void XmlDocument::RemoveNodes() {
  ClearChildren(handle_);
}

::agiru::Boolean XmlDocument::ReplaceNodes(const ::agiru::Variant &Content) {
  if (NodeOf(handle_) == nullptr) { return false; }
  ClearChildren(handle_);
  return Add(Content);
}

::agiru::Boolean XmlDocument::ReplaceWith(const ::agiru::Variant &Node) {
  static_cast<void>(Node);
  return false;
}

::agiru::Boolean XmlDocument::SelectNodes(std::string_view XPath,
                                          const ::agiru::XmlNamespaceManager &NamespaceManager,
                                          ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NamespaceManager.Declared(), NodeList);
}

::agiru::Boolean XmlDocument::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NoNamespaces(), NodeList);
}

::agiru::Boolean XmlDocument::SelectSingleNode(std::string_view XPath,
                                               const ::agiru::XmlNamespaceManager &NamespaceManager,
                                               ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NamespaceManager.Declared(), Node);
}

::agiru::Boolean XmlDocument::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NoNamespaces(), Node);
}

::agiru::Boolean XmlDocument::WriteTo(const ::agiru::OutStream &OutStream) {
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlDocument::WriteTo(::agiru::Text<0> &Text) {
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlDocument::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                      const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlDocument::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                      ::agiru::Text<0> &Text) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlNode::AddAfterSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, true);
}

::agiru::Boolean XmlNode::AddBeforeSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, false);
}

::agiru::XmlAttribute XmlNode::AsXmlAttribute() {
  return As<XmlAttribute>(handle_, XML_ATTRIBUTE_NODE, "XmlAttribute");
}

::agiru::XmlCData XmlNode::AsXmlCData() {
  return As<XmlCData>(handle_, XML_CDATA_SECTION_NODE, "XmlCData");
}

::agiru::XmlComment XmlNode::AsXmlComment() {
  return As<XmlComment>(handle_, XML_COMMENT_NODE, "XmlComment");
}

::agiru::XmlDeclaration XmlNode::AsXmlDeclaration() {
  return As<XmlDeclaration>(handle_, XML_DOCUMENT_NODE, "XmlDeclaration");
}

::agiru::XmlDocument XmlNode::AsXmlDocument() {
  return As<XmlDocument>(handle_, XML_DOCUMENT_NODE, "XmlDocument");
}

::agiru::XmlDocumentType XmlNode::AsXmlDocumentType() {
  return As<XmlDocumentType>(handle_, XML_DTD_NODE, "XmlDocumentType");
}

::agiru::XmlElement XmlNode::AsXmlElement() {
  return As<XmlElement>(handle_, XML_ELEMENT_NODE, "XmlElement");
}

::agiru::XmlProcessingInstruction XmlNode::AsXmlProcessingInstruction() {
  return As<XmlProcessingInstruction>(handle_, XML_PI_NODE, "XmlProcessingInstruction");
}

::agiru::XmlText XmlNode::AsXmlText() {
  return As<XmlText>(handle_, XML_TEXT_NODE, "XmlText");
}

::agiru::Boolean XmlNode::GetDocument(::agiru::XmlDocument &Document) {
  return DocumentOf(handle_, Document);
}

::agiru::Boolean XmlNode::GetParent(::agiru::XmlElement &Parent) {
  return ParentOf(handle_, Parent);
}

::agiru::Boolean XmlNode::IsXmlAttribute() {
  return IsKind(handle_, XML_ATTRIBUTE_NODE);
}

::agiru::Boolean XmlNode::IsXmlCData() {
  return IsKind(handle_, XML_CDATA_SECTION_NODE);
}

::agiru::Boolean XmlNode::IsXmlComment() {
  return IsKind(handle_, XML_COMMENT_NODE);
}

::agiru::Boolean XmlNode::IsXmlDeclaration() {
  return false;
}

::agiru::Boolean XmlNode::IsXmlDocument() {
  return IsKind(handle_, XML_DOCUMENT_NODE);
}

::agiru::Boolean XmlNode::IsXmlDocumentType() {
  return IsKind(handle_, XML_DTD_NODE);
}

::agiru::Boolean XmlNode::IsXmlElement() {
  return IsKind(handle_, XML_ELEMENT_NODE);
}

::agiru::Boolean XmlNode::IsXmlProcessingInstruction() {
  return IsKind(handle_, XML_PI_NODE);
}

::agiru::Boolean XmlNode::IsXmlText() {
  return IsKind(handle_, XML_TEXT_NODE);
}

::agiru::Boolean XmlNode::Remove() {
  return RemoveNode(handle_);
}

::agiru::Boolean XmlNode::ReplaceWith(const ::agiru::Variant &Node) {
  return Replace(handle_, Node);
}

::agiru::Boolean XmlNode::SelectNodes(std::string_view XPath,
                                      const ::agiru::XmlNamespaceManager &NamespaceManager,
                                      ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NamespaceManager.Declared(), NodeList);
}

::agiru::Boolean XmlNode::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NoNamespaces(), NodeList);
}

::agiru::Boolean XmlNode::SelectSingleNode(std::string_view XPath,
                                           const ::agiru::XmlNamespaceManager &NamespaceManager,
                                           ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NamespaceManager.Declared(), Node);
}

::agiru::Boolean XmlNode::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NoNamespaces(), Node);
}

::agiru::Boolean XmlNode::WriteTo(const ::agiru::OutStream &OutStream) {
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlNode::WriteTo(::agiru::Text<0> &Text) {
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlNode::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                  const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlNode::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                  ::agiru::Text<0> &Text) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, Text);
}

::agiru::XmlElement XmlElement::Create(std::string_view Name) {
  return XmlElement(detail::Detached(xmlNewNode(nullptr, Bytes(std::string(Name)))));
}

::agiru::XmlElement XmlElement::CreateWith(std::string_view Name, const ::agiru::Variant &Content) {
  XmlElement element = Create(Name);
  static_cast<void>(element.Add(Content));
  return element;
}

::agiru::XmlElement XmlElement::Create(std::string_view LocalName, std::string_view NamespaceUri) {
  XmlElement element = Create(LocalName);
  if (!NamespaceUri.empty()) {
    xmlNodePtr node = NodeOf(element.Handle());
    xmlNsPtr ns = xmlNewNs(node, Bytes(std::string(NamespaceUri)), nullptr);
    xmlSetNs(node, ns);
  }
  return element;
}

::agiru::XmlElement XmlElement::CreateWith(std::string_view LocalName,
                                           std::string_view NamespaceUri,
                                           const ::agiru::Variant &Content) {
  XmlElement element = Create(LocalName, NamespaceUri);
  static_cast<void>(element.Add(Content));
  return element;
}

::agiru::Boolean XmlElement::Add(const ::agiru::Variant &Content) {
  return AddInto(handle_, Content, false);
}

::agiru::Boolean XmlElement::AddFirst(const ::agiru::Variant &Content) {
  return AddInto(handle_, Content, true);
}

::agiru::Boolean XmlElement::AddAfterSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, true);
}

::agiru::Boolean XmlElement::AddBeforeSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, false);
}

::agiru::XmlNode XmlElement::AsXmlNode() {
  return XmlNode(handle_);
}

::agiru::XmlAttributeCollection XmlElement::Attributes() {
  return XmlAttributeCollection(detail::Attributes(handle_));
}

::agiru::XmlNodeList XmlElement::GetChildElements() {
  return XmlNodeList(Children(handle_, true));
}

::agiru::XmlNodeList XmlElement::GetChildElements(std::string_view Name) {
  return XmlNodeList(Named(Children(handle_, true), Name));
}

::agiru::XmlNodeList XmlElement::GetChildElements(std::string_view LocalName,
                                                  std::string_view NamespaceUri) {
  return XmlNodeList(LocallyNamed(Children(handle_, true), LocalName, NamespaceUri));
}

::agiru::XmlNodeList XmlElement::GetChildNodes() {
  return XmlNodeList(Children(handle_, false));
}

::agiru::XmlNodeList XmlElement::GetDescendantElements() {
  return XmlNodeList(Descendants(handle_, true));
}

::agiru::XmlNodeList XmlElement::GetDescendantElements(std::string_view Name) {
  return XmlNodeList(Named(Descendants(handle_, true), Name));
}

::agiru::XmlNodeList XmlElement::GetDescendantElements(std::string_view LocalName,
                                                       std::string_view NamespaceUri) {
  return XmlNodeList(LocallyNamed(Descendants(handle_, true), LocalName, NamespaceUri));
}

::agiru::XmlNodeList XmlElement::GetDescendantNodes() {
  return XmlNodeList(Descendants(handle_, false));
}

::agiru::Boolean XmlElement::GetDocument(::agiru::XmlDocument &Document) {
  return DocumentOf(handle_, Document);
}

::agiru::Boolean XmlElement::GetNamespaceOfPrefix(std::string_view Prefix,
                                                  ::agiru::Text<0> &Result) {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr) { return false; }
  const std::string held(Prefix);
  xmlNsPtr ns = xmlSearchNs(node->doc, node, held.empty() ? nullptr : Bytes(held));
  if (ns == nullptr) { return false; }
  Result = std::string_view(detail::Text(ns->href));
  return true;
}

::agiru::Boolean XmlElement::GetParent(::agiru::XmlElement &Parent) {
  return ParentOf(handle_, Parent);
}

::agiru::Boolean XmlElement::GetPrefixOfNamespace(std::string_view Namespace,
                                                  ::agiru::Text<0> &Result) {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr) { return false; }
  const std::string held(Namespace);
  xmlNsPtr ns = xmlSearchNsByHref(node->doc, node, Bytes(held));
  if (ns == nullptr) { return false; }
  Result = std::string_view(detail::Text(ns->prefix));
  return true;
}

::agiru::Boolean XmlElement::HasAttributes() {
  return NodeOf(handle_) != nullptr && NodeOf(handle_)->properties != nullptr;
}

::agiru::Boolean XmlElement::HasElements() {
  return !Children(handle_, true).empty();
}

std::string XmlElement::InnerText() {
  return Content(handle_);
}

std::string XmlElement::InnerXml() {
  return DumpChildren(handle_);
}

::agiru::Boolean XmlElement::IsEmpty() {
  return NodeOf(handle_) == nullptr || NodeOf(handle_)->children == nullptr;
}

std::string XmlElement::LocalName() {
  return NodeOf(handle_) == nullptr ? std::string{} : detail::Text(NodeOf(handle_)->name);
}

std::string XmlElement::Name() {
  return detail::QualifiedName(NodeOf(handle_));
}

std::string XmlElement::NamespaceUri() {
  xmlNodePtr node = NodeOf(handle_);
  return node == nullptr || node->ns == nullptr ? std::string{} : detail::Text(node->ns->href);
}

::agiru::Boolean XmlElement::Remove() {
  return RemoveNode(handle_);
}

void XmlElement::RemoveAllAttributes() {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr) { return; }
  while (node->properties != nullptr) { xmlRemoveProp(node->properties); }
}

void XmlElement::RemoveAttribute(std::string_view Name) {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr) { return; }
  for (xmlAttrPtr attribute = node->properties; attribute != nullptr; attribute = attribute->next) {
    if (detail::SameName(reinterpret_cast<xmlNodePtr>(attribute), Name)) {
      xmlRemoveProp(attribute);
      return;
    }
  }
}

void XmlElement::RemoveAttribute(std::string_view LocalName, std::string_view NamespaceUri) {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr) { return; }
  for (xmlAttrPtr attribute = node->properties; attribute != nullptr; attribute = attribute->next) {
    if (detail::SameLocalName(reinterpret_cast<xmlNodePtr>(attribute), LocalName, NamespaceUri)) {
      xmlRemoveProp(attribute);
      return;
    }
  }
}

void XmlElement::RemoveAttribute(const ::agiru::XmlAttribute &Attribute) {
  xmlNodePtr node = NodeOf(Attribute.Handle());
  if (node != nullptr && node->type == XML_ATTRIBUTE_NODE) {
    xmlRemoveProp(reinterpret_cast<xmlAttrPtr>(node));
  }
}

void XmlElement::RemoveNodes() {
  ClearChildren(handle_);
}

::agiru::Boolean XmlElement::ReplaceNodes(const ::agiru::Variant &Content) {
  if (NodeOf(handle_) == nullptr) { return false; }
  ClearChildren(handle_);
  return Add(Content);
}

::agiru::Boolean XmlElement::ReplaceWith(const ::agiru::Variant &Node) {
  return Replace(handle_, Node);
}

::agiru::Boolean XmlElement::SelectNodes(std::string_view XPath,
                                         const ::agiru::XmlNamespaceManager &NamespaceManager,
                                         ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NamespaceManager.Declared(), NodeList);
}

::agiru::Boolean XmlElement::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NoNamespaces(), NodeList);
}

::agiru::Boolean XmlElement::SelectSingleNode(std::string_view XPath,
                                              const ::agiru::XmlNamespaceManager &NamespaceManager,
                                              ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NamespaceManager.Declared(), Node);
}

::agiru::Boolean XmlElement::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NoNamespaces(), Node);
}

void XmlElement::SetAttribute(std::string_view Name, std::string_view Value) {
  xmlNodePtr node = NodeOf(handle_);
  if (node == nullptr) { return; }
  const std::string name(Name);
  const std::string value(Value);
  const std::size_t colon = name.find(':');
  if (colon != std::string::npos) {
    xmlNsPtr ns = xmlSearchNs(node->doc, node, Bytes(name.substr(0, colon)));
    if (ns != nullptr) {
      xmlSetNsProp(node, ns, Bytes(name.substr(colon + 1)), Bytes(value));
      return;
    }
  }
  xmlSetProp(node, Bytes(name), Bytes(value));
}

::agiru::Boolean XmlElement::WriteTo(const ::agiru::OutStream &OutStream) {
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlElement::WriteTo(::agiru::Text<0> &Text) {
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlElement::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                     const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlElement::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                     ::agiru::Text<0> &Text) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, Text);
}

}

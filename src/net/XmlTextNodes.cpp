#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Stream.h"
#include "type/Text.h"
#include "type/Variant.h"
#include "type/XmlCData.h"
#include "type/XmlComment.h"
#include "type/XmlDeclaration.h"
#include "type/XmlDocument.h"
#include "type/XmlDocumentType.h"
#include "type/XmlElement.h"
#include "type/XmlHandle.h"
#include "type/XmlNamespaceManager.h"
#include "type/XmlNode.h"
#include "type/XmlNodeList.h"
#include "type/XmlProcessingInstruction.h"
#include "type/XmlText.h"
#include "type/XmlWriteOptions.h"

#include "XmlEngine.h"

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
  xmlNodeSetContent(node, Bytes(held));
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
  if (node == nullptr || node->parent == nullptr) { return false; }
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
  if (node == nullptr || node->parent == nullptr) { return false; }
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

bool Answer(std::string text, Text<0> &result) {
  result = std::string_view(text);
  return true;
}

}

::agiru::Boolean XmlText::AddAfterSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, true);
}

::agiru::Boolean XmlText::AddBeforeSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, false);
}

::agiru::XmlNode XmlText::AsXmlNode() {
  return XmlNode(handle_);
}

::agiru::Boolean XmlText::GetDocument(::agiru::XmlDocument &Document) {
  return DocumentOf(handle_, Document);
}

::agiru::Boolean XmlText::GetParent(::agiru::XmlElement &Parent) {
  return ParentOf(handle_, Parent);
}

::agiru::Boolean XmlText::Remove() {
  return RemoveNode(handle_);
}

::agiru::Boolean XmlText::ReplaceWith(const ::agiru::Variant &Node) {
  return Replace(handle_, Node);
}

::agiru::Boolean XmlText::SelectNodes(std::string_view XPath,
                                      const ::agiru::XmlNamespaceManager &NamespaceManager,
                                      ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NamespaceManager.Declared(), NodeList);
}

::agiru::Boolean XmlText::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NoNamespaces(), NodeList);
}

::agiru::Boolean XmlText::SelectSingleNode(std::string_view XPath,
                                           const ::agiru::XmlNamespaceManager &NamespaceManager,
                                           ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NamespaceManager.Declared(), Node);
}

::agiru::Boolean XmlText::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NoNamespaces(), Node);
}

::agiru::Boolean XmlText::WriteTo(const ::agiru::OutStream &OutStream) {
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlText::WriteTo(::agiru::Text<0> &Text) {
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlText::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                  const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlText::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                  ::agiru::Text<0> &Text) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlCData::AddAfterSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, true);
}

::agiru::Boolean XmlCData::AddBeforeSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, false);
}

::agiru::XmlNode XmlCData::AsXmlNode() {
  return XmlNode(handle_);
}

::agiru::Boolean XmlCData::GetDocument(::agiru::XmlDocument &Document) {
  return DocumentOf(handle_, Document);
}

::agiru::Boolean XmlCData::GetParent(::agiru::XmlElement &Parent) {
  return ParentOf(handle_, Parent);
}

::agiru::Boolean XmlCData::Remove() {
  return RemoveNode(handle_);
}

::agiru::Boolean XmlCData::ReplaceWith(const ::agiru::Variant &Node) {
  return Replace(handle_, Node);
}

::agiru::Boolean XmlCData::SelectNodes(std::string_view XPath,
                                       const ::agiru::XmlNamespaceManager &NamespaceManager,
                                       ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NamespaceManager.Declared(), NodeList);
}

::agiru::Boolean XmlCData::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NoNamespaces(), NodeList);
}

::agiru::Boolean XmlCData::SelectSingleNode(std::string_view XPath,
                                            const ::agiru::XmlNamespaceManager &NamespaceManager,
                                            ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NamespaceManager.Declared(), Node);
}

::agiru::Boolean XmlCData::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NoNamespaces(), Node);
}

::agiru::Boolean XmlCData::WriteTo(const ::agiru::OutStream &OutStream) {
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlCData::WriteTo(::agiru::Text<0> &Text) {
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlCData::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                   const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlCData::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                   ::agiru::Text<0> &Text) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlComment::AddAfterSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, true);
}

::agiru::Boolean XmlComment::AddBeforeSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, false);
}

::agiru::XmlNode XmlComment::AsXmlNode() {
  return XmlNode(handle_);
}

::agiru::Boolean XmlComment::GetDocument(::agiru::XmlDocument &Document) {
  return DocumentOf(handle_, Document);
}

::agiru::Boolean XmlComment::GetParent(::agiru::XmlElement &Parent) {
  return ParentOf(handle_, Parent);
}

::agiru::Boolean XmlComment::Remove() {
  return RemoveNode(handle_);
}

::agiru::Boolean XmlComment::ReplaceWith(const ::agiru::Variant &Node) {
  return Replace(handle_, Node);
}

::agiru::Boolean XmlComment::SelectNodes(std::string_view XPath,
                                         const ::agiru::XmlNamespaceManager &NamespaceManager,
                                         ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NamespaceManager.Declared(), NodeList);
}

::agiru::Boolean XmlComment::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NoNamespaces(), NodeList);
}

::agiru::Boolean XmlComment::SelectSingleNode(std::string_view XPath,
                                              const ::agiru::XmlNamespaceManager &NamespaceManager,
                                              ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NamespaceManager.Declared(), Node);
}

::agiru::Boolean XmlComment::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NoNamespaces(), Node);
}

::agiru::Boolean XmlComment::WriteTo(const ::agiru::OutStream &OutStream) {
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlComment::WriteTo(::agiru::Text<0> &Text) {
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlComment::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                     const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlComment::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                     ::agiru::Text<0> &Text) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlDeclaration::AddAfterSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, true);
}

::agiru::Boolean XmlDeclaration::AddBeforeSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, false);
}

::agiru::XmlNode XmlDeclaration::AsXmlNode() {
  return XmlNode(handle_);
}

::agiru::Boolean XmlDeclaration::GetDocument(::agiru::XmlDocument &Document) {
  return DocumentOf(handle_, Document);
}

::agiru::Boolean XmlDeclaration::GetParent(::agiru::XmlElement &Parent) {
  return ParentOf(handle_, Parent);
}

::agiru::Boolean XmlDeclaration::Remove() {
  return RemoveNode(handle_);
}

::agiru::Boolean XmlDeclaration::ReplaceWith(const ::agiru::Variant &Node) {
  return Replace(handle_, Node);
}

::agiru::Boolean XmlDeclaration::SelectNodes(std::string_view XPath,
                                             const ::agiru::XmlNamespaceManager &NamespaceManager,
                                             ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NamespaceManager.Declared(), NodeList);
}

::agiru::Boolean XmlDeclaration::SelectNodes(std::string_view XPath,
                                             ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NoNamespaces(), NodeList);
}

::agiru::Boolean
XmlDeclaration::SelectSingleNode(std::string_view XPath,
                                 const ::agiru::XmlNamespaceManager &NamespaceManager,
                                 ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NamespaceManager.Declared(), Node);
}

::agiru::Boolean XmlDeclaration::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NoNamespaces(), Node);
}

::agiru::Boolean XmlDeclaration::WriteTo(const ::agiru::OutStream &OutStream) {
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlDeclaration::WriteTo(::agiru::Text<0> &Text) {
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlDeclaration::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                         const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlDeclaration::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                         ::agiru::Text<0> &Text) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlDocumentType::AddAfterSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, true);
}

::agiru::Boolean XmlDocumentType::AddBeforeSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, false);
}

::agiru::XmlNode XmlDocumentType::AsXmlNode() {
  return XmlNode(handle_);
}

::agiru::Boolean XmlDocumentType::GetDocument(::agiru::XmlDocument &Document) {
  return DocumentOf(handle_, Document);
}

::agiru::Boolean XmlDocumentType::GetParent(::agiru::XmlElement &Parent) {
  return ParentOf(handle_, Parent);
}

::agiru::Boolean XmlDocumentType::Remove() {
  return RemoveNode(handle_);
}

::agiru::Boolean XmlDocumentType::ReplaceWith(const ::agiru::Variant &Node) {
  return Replace(handle_, Node);
}

::agiru::Boolean XmlDocumentType::SelectNodes(std::string_view XPath,
                                              const ::agiru::XmlNamespaceManager &NamespaceManager,
                                              ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NamespaceManager.Declared(), NodeList);
}

::agiru::Boolean XmlDocumentType::SelectNodes(std::string_view XPath,
                                              ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NoNamespaces(), NodeList);
}

::agiru::Boolean
XmlDocumentType::SelectSingleNode(std::string_view XPath,
                                  const ::agiru::XmlNamespaceManager &NamespaceManager,
                                  ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NamespaceManager.Declared(), Node);
}

::agiru::Boolean XmlDocumentType::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NoNamespaces(), Node);
}

::agiru::Boolean XmlDocumentType::WriteTo(const ::agiru::OutStream &OutStream) {
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlDocumentType::WriteTo(::agiru::Text<0> &Text) {
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlDocumentType::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                          const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlDocumentType::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                          ::agiru::Text<0> &Text) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlProcessingInstruction::AddAfterSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, true);
}

::agiru::Boolean XmlProcessingInstruction::AddBeforeSelf(const ::agiru::Variant &Content) {
  return AddBeside(handle_, Content, false);
}

::agiru::XmlNode XmlProcessingInstruction::AsXmlNode() {
  return XmlNode(handle_);
}

::agiru::Boolean XmlProcessingInstruction::GetDocument(::agiru::XmlDocument &Document) {
  return DocumentOf(handle_, Document);
}

::agiru::Boolean XmlProcessingInstruction::GetParent(::agiru::XmlElement &Parent) {
  return ParentOf(handle_, Parent);
}

::agiru::Boolean XmlProcessingInstruction::Remove() {
  return RemoveNode(handle_);
}

::agiru::Boolean XmlProcessingInstruction::ReplaceWith(const ::agiru::Variant &Node) {
  return Replace(handle_, Node);
}

::agiru::Boolean
XmlProcessingInstruction::SelectNodes(std::string_view XPath,
                                      const ::agiru::XmlNamespaceManager &NamespaceManager,
                                      ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NamespaceManager.Declared(), NodeList);
}

::agiru::Boolean XmlProcessingInstruction::SelectNodes(std::string_view XPath,
                                                       ::agiru::XmlNodeList &NodeList) {
  return SelectAll(handle_, XPath, NoNamespaces(), NodeList);
}

::agiru::Boolean
XmlProcessingInstruction::SelectSingleNode(std::string_view XPath,
                                           const ::agiru::XmlNamespaceManager &NamespaceManager,
                                           ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NamespaceManager.Declared(), Node);
}

::agiru::Boolean XmlProcessingInstruction::SelectSingleNode(std::string_view XPath,
                                                            ::agiru::XmlNode &Node) {
  return Select(handle_, XPath, NoNamespaces(), Node);
}

::agiru::Boolean XmlProcessingInstruction::WriteTo(const ::agiru::OutStream &OutStream) {
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlProcessingInstruction::WriteTo(::agiru::Text<0> &Text) {
  return WriteAll(handle_, Text);
}

::agiru::Boolean XmlProcessingInstruction::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                                   const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, OutStream);
}

::agiru::Boolean XmlProcessingInstruction::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                                   ::agiru::Text<0> &Text) {
  static_cast<void>(WriteOptions);
  return WriteAll(handle_, Text);
}

::agiru::XmlText XmlText::Create(std::string_view Content) {
  return XmlText(detail::Detached(xmlNewText(Bytes(std::string(Content)))));
}

std::string XmlText::Value() {
  return ContentOf(handle_);
}

std::string XmlText::Value(std::string_view NewValue) {
  return SetContent(handle_, NewValue);
}

::agiru::XmlCData XmlCData::Create(std::string_view Value) {
  const std::string held(Value);
  xmlDocPtr doc = xmlNewDoc(Bytes("1.0"));
  xmlNodePtr node = xmlNewCDataBlock(doc, Bytes(held), static_cast<int>(held.size()));
  return XmlCData(XmlHandle(detail::NewTree(doc), node));
}

std::string XmlCData::Value() {
  return ContentOf(handle_);
}

std::string XmlCData::Value(std::string_view NewValue) {
  return SetContent(handle_, NewValue);
}

::agiru::XmlComment XmlComment::Create(std::string_view Value) {
  return XmlComment(detail::Detached(xmlNewComment(Bytes(std::string(Value)))));
}

std::string XmlComment::Value() {
  return ContentOf(handle_);
}

std::string XmlComment::Value(std::string_view NewValue) {
  return SetContent(handle_, NewValue);
}

::agiru::XmlDeclaration XmlDeclaration::Create(std::string_view Version,
                                               std::string_view Encoding,
                                               std::string_view Standalone) {
  xmlDocPtr doc = xmlNewDoc(Bytes(std::string(Version)));
  if (!Encoding.empty()) { doc->encoding = xmlStrdup(Bytes(std::string(Encoding))); }
  doc->standalone = Standalone == "yes" ? 1 : Standalone == "no" ? 0 : -1;
  return XmlDeclaration(XmlHandle(detail::NewTree(doc), doc));
}

std::string XmlDeclaration::Encoding() {
  xmlDocPtr doc = DocOf(handle_);
  return doc == nullptr ? std::string{} : detail::Text(doc->encoding);
}

std::string XmlDeclaration::Encoding(std::string_view NewValue) {
  xmlDocPtr doc = DocOf(handle_);
  if (doc == nullptr) { return {}; }
  xmlFree(const_cast<xmlChar *>(doc->encoding));
  doc->encoding = NewValue.empty() ? nullptr : xmlStrdup(Bytes(std::string(NewValue)));
  return std::string(NewValue);
}

std::string XmlDeclaration::Standalone() {
  xmlDocPtr doc = DocOf(handle_);
  if (doc == nullptr) { return {}; }
  return doc->standalone == 1 ? "yes" : doc->standalone == 0 ? "no" : "";
}

std::string XmlDeclaration::Standalone(std::string_view NewValue) {
  xmlDocPtr doc = DocOf(handle_);
  if (doc == nullptr) { return {}; }
  doc->standalone = NewValue == "yes" ? 1 : NewValue == "no" ? 0 : -1;
  return std::string(NewValue);
}

std::string XmlDeclaration::Version() {
  xmlDocPtr doc = DocOf(handle_);
  return doc == nullptr ? std::string{} : detail::Text(doc->version);
}

std::string XmlDeclaration::Version(std::string_view NewValue) {
  xmlDocPtr doc = DocOf(handle_);
  if (doc == nullptr) { return {}; }
  xmlFree(const_cast<xmlChar *>(doc->version));
  doc->version = xmlStrdup(Bytes(std::string(NewValue)));
  return std::string(NewValue);
}

::agiru::XmlDocumentType XmlDocumentType::Create(std::string_view Name) {
  return Create(Name, "", "", "");
}

::agiru::XmlDocumentType XmlDocumentType::Create(std::string_view Name, std::string_view PublicId) {
  return Create(Name, PublicId, "", "");
}

::agiru::XmlDocumentType XmlDocumentType::Create(std::string_view Name,
                                                 std::string_view PublicId,
                                                 std::string_view SystemId,
                                                 std::string_view InternalSubset) {
  xmlDocPtr doc = xmlNewDoc(Bytes("1.0"));
  const std::string name(Name);
  const std::string publicId(PublicId);
  const std::string systemId(SystemId);
  xmlDtdPtr dtd = xmlCreateIntSubset(doc,
                                     Bytes(name),
                                     publicId.empty() ? nullptr : Bytes(publicId),
                                     systemId.empty() ? nullptr : Bytes(systemId));
  static_cast<void>(InternalSubset);
  return XmlDocumentType(XmlHandle(detail::NewTree(doc), dtd));
}

::agiru::Boolean XmlDocumentType::GetInternalSubset(::agiru::Text<0> &Result) {
  return Answer(std::string{}, Result);
}

::agiru::Boolean XmlDocumentType::GetName(::agiru::Text<0> &Result) {
  if (NodeOf(handle_) == nullptr) { return false; }
  return Answer(detail::Text(NodeOf(handle_)->name), Result);
}

::agiru::Boolean XmlDocumentType::GetPublicId(::agiru::Text<0> &Result) {
  if (NodeOf(handle_) == nullptr) { return false; }
  return Answer(detail::Text(reinterpret_cast<xmlDtdPtr>(NodeOf(handle_))->ExternalID), Result);
}

::agiru::Boolean XmlDocumentType::GetSystemId(::agiru::Text<0> &Result) {
  if (NodeOf(handle_) == nullptr) { return false; }
  return Answer(detail::Text(reinterpret_cast<xmlDtdPtr>(NodeOf(handle_))->SystemID), Result);
}

::agiru::Boolean XmlDocumentType::SetInternalSubset(std::string_view Value) {
  static_cast<void>(Value);
  return NodeOf(handle_) != nullptr;
}

::agiru::Boolean XmlDocumentType::SetName(std::string_view Value) {
  if (NodeOf(handle_) == nullptr) { return false; }
  xmlNodeSetName(NodeOf(handle_), Bytes(std::string(Value)));
  return true;
}

::agiru::Boolean XmlDocumentType::SetPublicId(std::string_view Value) {
  if (NodeOf(handle_) == nullptr) { return false; }
  auto *dtd = reinterpret_cast<xmlDtdPtr>(NodeOf(handle_));
  xmlFree(const_cast<xmlChar *>(dtd->ExternalID));
  dtd->ExternalID = Value.empty() ? nullptr : xmlStrdup(Bytes(std::string(Value)));
  return true;
}

::agiru::Boolean XmlDocumentType::SetSystemId(std::string_view Value) {
  if (NodeOf(handle_) == nullptr) { return false; }
  auto *dtd = reinterpret_cast<xmlDtdPtr>(NodeOf(handle_));
  xmlFree(const_cast<xmlChar *>(dtd->SystemID));
  dtd->SystemID = Value.empty() ? nullptr : xmlStrdup(Bytes(std::string(Value)));
  return true;
}

::agiru::XmlProcessingInstruction XmlProcessingInstruction::Create(std::string_view Target,
                                                                   std::string_view Data) {
  return XmlProcessingInstruction(
      detail::Detached(xmlNewPI(Bytes(std::string(Target)), Bytes(std::string(Data)))));
}

::agiru::Boolean XmlProcessingInstruction::GetData(::agiru::Text<0> &Result) {
  if (NodeOf(handle_) == nullptr) { return false; }
  return Answer(ContentOf(handle_), Result);
}

::agiru::Boolean XmlProcessingInstruction::GetTarget(::agiru::Text<0> &Result) {
  if (NodeOf(handle_) == nullptr) { return false; }
  return Answer(detail::Text(NodeOf(handle_)->name), Result);
}

::agiru::Boolean XmlProcessingInstruction::SetData(std::string_view Value) {
  if (NodeOf(handle_) == nullptr) { return false; }
  SetContent(handle_, Value);
  return true;
}

::agiru::Boolean XmlProcessingInstruction::SetTarget(std::string_view Value) {
  if (NodeOf(handle_) == nullptr) { return false; }
  xmlNodeSetName(NodeOf(handle_), Bytes(std::string(Value)));
  return true;
}

}

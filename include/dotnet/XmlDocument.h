#pragma once

#include "dotnet/Refused.h"
#include "dotnet/XmlNode.h"
#include "type/Boolean.h"
#include "type/Stream.h"
#include "type/Text.h"
#include "type/XmlHandle.h"
#include "type/XmlNameTable.h"

#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>

namespace agiru::dotnet {

/// \brief .NET `System.Xml.XmlElement`: a node that is an element.
class XmlElement : public XmlNode {
public:
  using XmlNode::XmlNode;
  /// \brief `XmlElement.SetAttribute(name, value)`. \param name The name. \param value The value.
  void SetAttribute(std::string_view name, std::string_view value);
  /// \brief `XmlElement.GetAttribute(name)`. \param name The name. \return The value, or empty.
  [[nodiscard]] std::string GetAttribute(std::string_view name) const;
  /// \brief `XmlElement.HasAttribute(name)`. \param name The name. \return Whether it is there.
  [[nodiscard]] ::agiru::Boolean HasAttribute(std::string_view name) const;
  /// \brief `XmlElement.RemoveAttribute(name)`. \param name The name.
  void RemoveAttribute(std::string_view name);
  /// \brief `XmlElement.GetElementsByTagName(name)`. \param name The name. \return The matches.
  [[nodiscard]] XmlNodeList GetElementsByTagName(std::string_view name) const;
  /// \brief `XmlElement.IsEmpty`, whether it is written `<a/>`. \return Whether it has no children.
  [[nodiscard]] ::agiru::Boolean IsEmpty() const;
};

/// \brief .NET `System.Xml.XmlAttribute`: a node that is an attribute.
class XmlAttribute : public XmlNode {
public:
  using XmlNode::XmlNode;
  /// \brief `XmlAttribute.OwnerElement`. \return The element.
  [[nodiscard]] XmlElement OwnerElement() const;
  /// \brief `XmlAttribute.Specified`. \return True, every attribute here was written.
  [[nodiscard]] ::agiru::Boolean Specified() const { return true; }
};

/// \brief .NET `XmlAttributeCollection`: an element's attributes.
class XmlAttributeCollection {
public:
  XmlAttributeCollection() = default;
  /// \brief The collection over an element's attributes. \param owner The element.
  explicit XmlAttributeCollection(::agiru::detail::XmlHandle owner) noexcept
      : owner_(std::move(owner)) {}
  /// \brief `XmlAttributeCollection.Count`. \return How many.
  [[nodiscard]] ::agiru::Integer Count() const;
  /// \brief `XmlAttributeCollection.Item(i)`, zero-based. \param index The index. \return It.
  [[nodiscard]] XmlAttribute Item(::agiru::Integer index) const;
  /// \brief `XmlAttributeCollection[i]`, spelled `ItemOf`. \param index The index. \return It.
  [[nodiscard]] XmlAttribute ItemOf(::agiru::Integer index) const { return Item(index); }
  /// \brief `XmlAttributeCollection[name]`, spelled `ItemOf`. \param name The name. \return It.
  [[nodiscard]] XmlAttribute ItemOf(std::string_view name) const { return GetNamedItem(name); }
  /// \brief `XmlAttributeCollection.GetNamedItem(name)`. \param name The qualified name.
  /// \return The attribute, or null.
  [[nodiscard]] XmlAttribute GetNamedItem(std::string_view name) const;
  /// \brief `XmlAttributeCollection.SetNamedItem(attribute)`. \param attribute The attribute.
  /// \return The attribute now on the element.
  XmlAttribute SetNamedItem(const XmlNode &attribute);
  /// \brief `XmlAttributeCollection.RemoveNamedItem(name)`. \param name The name.
  /// \return The removed attribute, or null.
  XmlAttribute RemoveNamedItem(std::string_view name);
  /// \brief `XmlAttributeCollection.Append(attribute)`. \param attribute The attribute.
  /// \return It.
  XmlAttribute Append(const XmlNode &attribute) { return SetNamedItem(attribute); }
  /// \brief AL `IsNull(XmlAttributeCollection)`. \return Whether there is no element behind it.
  [[nodiscard]] bool IsNullObject() const noexcept { return owner_.Empty(); }
  /// \brief The first attribute, for `foreach`.
  [[nodiscard]] std::vector<XmlAttribute>::const_iterator begin() const;
  /// \brief The end, for `foreach`.
  [[nodiscard]] std::vector<XmlAttribute>::const_iterator end() const;

private:
  ::agiru::detail::XmlHandle owner_;
  mutable std::vector<XmlAttribute> walked_;
};

/// \brief .NET `XmlNamespaceManager`: the prefixes an XPath may use.
class XmlNamespaceManager {
public:
  /// \brief The binder behind `XmlNsMgr := XmlNsMgr.XmlNamespaceManager(NameTable)`.
  struct Binder {
    /// \brief A manager over a name table. \return An empty manager.
    [[nodiscard]] class XmlNamespaceManager operator()(const ::agiru::XmlNameTable &) const;
    /// \brief A manager over a name table this runtime has not rebuilt. \return An empty one.
    template <typename T>
      requires ::agiru::dotnet::IsAbsent<T>
    [[nodiscard]] class XmlNamespaceManager operator()(const T &) const;
  };

  /// \brief `XmlNsMgr.XmlNamespaceManager(...)`, the constructor as AL calls it.
  Binder XmlNamespaceManager;

  /// \brief `XmlNamespaceManager.AddNamespace(prefix, uri)`. \param prefix The prefix.
  /// \param uri The namespace.
  void AddNamespace(std::string_view prefix, std::string_view uri);
  /// \brief `XmlNamespaceManager.LookupNamespace(prefix)`. \param prefix The prefix.
  /// \return The namespace, or empty.
  [[nodiscard]] std::string LookupNamespace(std::string_view prefix) const;
  /// \brief `XmlNamespaceManager.LookupPrefix(uri)`. \param uri The namespace.
  /// \return The prefix, or empty.
  [[nodiscard]] std::string LookupPrefix(std::string_view uri) const;
  /// \brief `XmlNamespaceManager.HasNamespace(prefix)`. \param prefix The prefix.
  /// \return Whether it was added.
  [[nodiscard]] ::agiru::Boolean HasNamespace(std::string_view prefix) const;
  /// \brief `XmlNamespaceManager.DefaultNamespace`. \return The namespace of the empty prefix.
  [[nodiscard]] std::string DefaultNamespace() const { return LookupNamespace(""); }
  /// \brief `XmlNamespaceManager.NameTable`. \return An empty name table.
  [[nodiscard]] ::agiru::XmlNameTable NameTable() const { return {}; }
  /// \brief The prefixes and namespaces, for the XPath engine.
  [[nodiscard]] const std::vector<std::pair<std::string, std::string>> &Declared() const noexcept {
    return declared_;
  }
  /// \brief AL `IsNull(XmlNamespaceManager)`. \return False; a manager is made by its binder.
  [[nodiscard]] bool IsNullObject() const noexcept { return false; }
  /// \brief `PushScope`, `PopScope`: one scope here.
  void PushScope() {}
  void PopScope() {}

private:
  std::vector<std::pair<std::string, std::string>> declared_;
};

inline class XmlNamespaceManager
XmlNamespaceManager::Binder::operator()(const ::agiru::XmlNameTable &) const {
  return ::agiru::dotnet::XmlNamespaceManager{};
}

template <typename T>
  requires ::agiru::dotnet::IsAbsent<T>
class XmlNamespaceManager XmlNamespaceManager::Binder::operator()(const T &) const {
  return ::agiru::dotnet::XmlNamespaceManager{};
}

/// \brief .NET `System.Xml.XmlDocument`, which the BaseApp's `XMLDOMManagement` wraps: a node that
///        is the whole tree.
class XmlDocument : public XmlNode {
public:
  using XmlNode::XmlNode;

  /// \brief The binder behind `XmlDoc := XmlDoc.XmlDocument()`.
  struct Binder {
    /// \brief An empty document. \return It.
    [[nodiscard]] class XmlDocument operator()() const;
  };

  /// \brief `XmlDoc.XmlDocument()`, the constructor as AL calls it.
  Binder XmlDocument;

  /// \brief `XmlDocument.Load(stream)`: parses the stream, replacing the tree.
  /// \param stream The stream.
  /// \throws Error when the text is not XML, the way .NET throws.
  void Load(const ::agiru::InStream &stream);
  /// \brief `XmlDocument.Load(reader)`, over a reader this runtime has not rebuilt.
  /// \tparam T The stub. \throws Error always (board:0035).
  template <typename T>
    requires ::agiru::dotnet::IsAbsent<T>
  void Load(const T &) {
    throw Error("XmlDocument.Load over an XmlReader is declared and not implemented yet (board:0035)");
  }
  /// \brief `XmlDocument.Load(OutStream)`, which AL writes and .NET refuses at run time: an
  ///        output stream cannot be read.
  /// \throws Error always.
  void Load(const ::agiru::OutStream &) {
    throw Error("XmlDocument.Load: an OutStream cannot be read from; the platform refuses it too");
  }
  /// \brief `XmlDocument.LoadXml(text)`. \param text The document. \throws Error on bad XML.
  void LoadXml(std::string_view text);
  /// \brief `XmlDocument.Save(stream)`. \param stream The stream.
  void Save(const ::agiru::OutStream &stream);
  /// \brief `XmlDocument.Save(InStream)`, which AL writes and cannot mean: an input stream is
  ///        not written to.
  /// \throws Error always.
  void Save(const ::agiru::InStream &) {
    throw Error("XmlDocument.Save: an InStream cannot be written to; the platform refuses it too");
  }
  /// \brief `XmlDocument.Save(stream)` over a .NET stream this runtime has not rebuilt
  ///        (`MemoryStream`): refused (board:0035).
  /// \tparam T The stub. \throws Error always.
  template <typename T>
    requires ::agiru::dotnet::IsAbsent<T>
  void Save(const T &) {
    throw Error("XmlDocument.Save over a .NET Stream is declared and not implemented yet (board:0035)");
  }
  /// \brief `XmlDocument.Save(filename)`, which this runtime does not write (board:0035).
  /// \throws Error always.
  void Save(std::string_view) {
    throw Error("XmlDocument.Save(filename) is declared and not implemented yet (board:0035)");
  }
  /// \brief `XmlDocument.DocumentElement`. \return The root, or null.
  [[nodiscard]] XmlElement DocumentElement() const;
  /// \brief `XmlDocument.DocumentType`. \return The DTD node, or null.
  [[nodiscard]] XmlNode DocumentType() const;
  /// \brief `XmlDocument.CreateElement(name)`. \param name The qualified name. \return It.
  [[nodiscard]] XmlElement CreateElement(std::string_view name) const;
  /// \brief `XmlDocument.CreateElement(name, uri)`. \param name The name. \param uri The namespace.
  /// \return It.
  [[nodiscard]] XmlElement CreateElement(std::string_view name, std::string_view uri) const;
  /// \brief `XmlDocument.CreateElement(prefix, local, uri)`. \param prefix The prefix.
  /// \param local The local name. \param uri The namespace. \return It.
  [[nodiscard]] XmlElement
  CreateElement(std::string_view prefix, std::string_view local, std::string_view uri) const;
  /// \brief `XmlDocument.CreateAttribute(name)`. \param name The name. \return It.
  [[nodiscard]] XmlAttribute CreateAttribute(std::string_view name) const;
  /// \brief `XmlDocument.CreateAttribute(name, uri)`. \param name The qualified name.
  /// \param uri The namespace. \return It.
  [[nodiscard]] XmlAttribute CreateAttribute(std::string_view name, std::string_view uri) const;
  /// \brief `XmlDocument.CreateAttribute(prefix, local, uri)`. \param prefix The prefix.
  /// \param local The local name. \param uri The namespace. \return It.
  [[nodiscard]] XmlAttribute
  CreateAttribute(std::string_view prefix, std::string_view local, std::string_view uri) const;
  /// \brief `XmlDocument.CreateNode(type, name, uri)` with the type as text (`element`).
  /// \param type The node type. \param name The name. \param uri The namespace. \return It.
  [[nodiscard]] XmlNode
  CreateNode(std::string_view type, std::string_view name, std::string_view uri) const;
  /// \brief `XmlDocument.CreateTextNode(text)`. \param text The text. \return It.
  [[nodiscard]] XmlNode CreateTextNode(std::string_view text) const;
  /// \brief `XmlDocument.CreateCDataSection(text)`. \param text The text. \return It.
  [[nodiscard]] XmlNode CreateCDataSection(std::string_view text) const;
  /// \brief `XmlDocument.CreateComment(text)`. \param text The text. \return It.
  [[nodiscard]] XmlNode CreateComment(std::string_view text) const;
  /// \brief `XmlDocument.CreateProcessingInstruction(target, data)`. \param target The target.
  /// \param data The data. \return It.
  [[nodiscard]] XmlNode CreateProcessingInstruction(std::string_view target,
                                                    std::string_view data) const;
  /// \brief `XmlDocument.CreateXmlDeclaration(version, encoding, standalone)`: sets the
  ///        document's own declaration and answers the document node.
  /// \param version The version. \param encoding The encoding. \param standalone yes, no or empty.
  /// \return The document node.
  XmlNode CreateXmlDeclaration(std::string_view version,
                               std::string_view encoding,
                               std::string_view standalone);
  /// \brief `XmlDocument.CreateDocumentType(name, public, system, subset)`. \param name The name.
  /// \param publicId The public id. \param systemId The system id. \param subset The subset.
  /// \return The DTD node.
  XmlNode CreateDocumentType(std::string_view name,
                             std::string_view publicId,
                             std::string_view systemId,
                             std::string_view subset);
  /// \brief `CreateDocumentType` over .NET strings this runtime has not rebuilt (board:0035).
  /// \tparam Arguments The stubs. \throws Error always.
  template <typename... Arguments>
    requires(sizeof...(Arguments) == 4 && (::agiru::dotnet::IsAbsent<Arguments> || ...))
  XmlNode CreateDocumentType(const Arguments &...) {
    throw Error("XmlDocument.CreateDocumentType over .NET String is declared and not implemented yet (board:0035)");
  }
  /// \brief `XmlDocument.GetElementsByTagName(name)`. \param name The name. \return The matches.
  [[nodiscard]] XmlNodeList GetElementsByTagName(std::string_view name) const;
  /// \brief `XmlDocument.NameTable`. \return An empty name table.
  [[nodiscard]] ::agiru::XmlNameTable NameTable() const { return {}; }
  /// \brief `XmlDocument.PreserveWhitespace` read. \return The setting.
  [[nodiscard]] ::agiru::Boolean PreserveWhitespace() const { return preserveWhitespace_; }
  /// \brief `XmlDocument.PreserveWhitespace` write. \param value The setting. \return It.
  ::agiru::Boolean PreserveWhitespace(::agiru::Boolean value) {
    preserveWhitespace_ = value;
    return value;
  }
  /// \brief `XmlDocument.ImportNode`, not rebuilt.
  ::agiru::dotnet::Refused ImportNode{{.type = "XmlDocument", .member = "ImportNode"}};
  /// \brief `XmlDocument.Schemas`, not rebuilt.
  ::agiru::dotnet::Refused Schemas{{.type = "XmlDocument", .member = "Schemas"}};
  /// \brief `XmlDocument.Validate`, not rebuilt.
  ::agiru::dotnet::Refused Validate{{.type = "XmlDocument", .member = "Validate"}};
  /// \brief `XmlDocument.ValidationEventHandler`, not rebuilt.
  ::agiru::dotnet::Refused ValidationEventHandler{
      {.type = "XmlDocument", .member = "ValidationEventHandler"}};

private:
  ::agiru::Boolean preserveWhitespace_ = false;
};

inline class XmlDocument XmlDocument::Binder::operator()() const {
  return ::agiru::dotnet::XmlDocument(::agiru::detail::EmptyDocument());
}

}

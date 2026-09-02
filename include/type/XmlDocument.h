#pragma once

#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/Stream.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `XmlDocument` -- the surface the platform documentation declares.

namespace agiru {

class XmlDeclaration;
class XmlDocumentType;
class XmlElement;
class XmlNameTable;
class XmlNamespaceManager;
class XmlNode;
class XmlNodeList;
class XmlReadOptions;
class XmlWriteOptions;

/// \brief AL `XmlDocument`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/xmldocument/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class XmlDocument {
public:
  /// \brief AL `XmlDocument.Add(Any)`. Adds the specified content as a child of this document.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(const ::agiru::Variant &Content);

  /// \brief AL `XmlDocument.AddAfterSelf(Any)`. Adds the specified content immediately after this
  /// node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddAfterSelf(const ::agiru::Variant &Content);

  /// \brief AL `XmlDocument.AddBeforeSelf(Any)`. Adds the specified content immediately before this
  /// node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddBeforeSelf(const ::agiru::Variant &Content);

  /// \brief AL `XmlDocument.AddFirst(Any)`. Adds the specified content at the start of the child
  /// list of this document.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddFirst(const ::agiru::Variant &Content);

  /// \brief AL `XmlDocument.AsXmlNode()`. Converts the node to an XmlNode.
  /// \return The AL `XmlNode`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNode AsXmlNode();

  /// \brief AL `XmlDocument.Create()`. Creates an XmlDocument.
  /// \return The AL `XmlDocument`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlDocument Create();

  /// \brief AL `XmlDocument.Create(Any)`. Creates an XmlDocument.
  /// \param Content The AL `Any`.
  /// \return The AL `XmlDocument`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlDocument Create(const ::agiru::Variant &Content);

  /// \brief AL `XmlDocument.GetChildElements()`. Gets a list containing the child elements for this
  /// document, in document order.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetChildElements();

  /// \brief AL `XmlDocument.GetChildElements(Text)`. Gets a list containing the child elements for
  /// this document, in document order.
  /// \param Name The AL `Text`.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetChildElements(std::string_view Name);

  /// \brief AL `XmlDocument.GetChildElements(Text, Text)`. Gets a list containing the child
  /// elements for this document, in document order.
  /// \param LocalName The AL `Text`.
  /// \param NamespaceUri The AL `Text`.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetChildElements(std::string_view LocalName, std::string_view NamespaceUri);

  /// \brief AL `XmlDocument.GetChildNodes()`. Gets a list containing the child elements for this
  /// document, in document order.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetChildNodes();

  /// \brief AL `XmlDocument.GetDeclaration(XmlDeclaration)`. Gets the XML declaration for this
  /// document.
  /// \param Result The AL `XmlDeclaration`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetDeclaration(::agiru::XmlDeclaration &Result);

  /// \brief AL `XmlDocument.GetDescendantElements()`. Gets a list containing the descendant
  /// elements for this document, in document order.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetDescendantElements();

  /// \brief AL `XmlDocument.GetDescendantElements(Text)`. Gets a list containing the descendant
  /// elements for this document, in document order.
  /// \param Name The AL `Text`.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetDescendantElements(std::string_view Name);

  /// \brief AL `XmlDocument.GetDescendantElements(Text, Text)`. Gets a list containing the
  /// descendant elements for this document, in document order.
  /// \param LocalName The AL `Text`.
  /// \param NamespaceUri The AL `Text`.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetDescendantElements(std::string_view LocalName,
                                             std::string_view NamespaceUri);

  /// \brief AL `XmlDocument.GetDescendantNodes()`. Gets a list containing the descendant nodes for
  /// this document, in document order.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetDescendantNodes();

  /// \brief AL `XmlDocument.GetDocument(XmlDocument)`. Gets the XmlDocument for this node.
  /// \param Document The AL `XmlDocument`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetDocument(::agiru::XmlDocument &Document);

  /// \brief AL `XmlDocument.GetDocumentType(XmlDocumentType)`. Gets the Document Type Definition
  /// (DTD) for this document.
  /// \param DocumentType The AL `XmlDocumentType`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetDocumentType(::agiru::XmlDocumentType &DocumentType);

  /// \brief AL `XmlDocument.GetParent(XmlElement)`. Gets the parent XmlElement of this node.
  /// \param Parent The AL `XmlElement`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetParent(::agiru::XmlElement &Parent);

  /// \brief AL `XmlDocument.GetRoot(XmlElement)`. Gets the root element of the XML tree for this
  /// document.
  /// \param Result The AL `XmlElement`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetRoot(::agiru::XmlElement &Result);

  /// \brief AL `XmlDocument.NameTable()`. Gets the XmlNameTable associated with this document.
  /// \return The AL `XmlNameTable`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNameTable NameTable();

  /// \brief AL `XmlDocument.ReadFrom(InStream, XmlDocument)`. Reads and parses the XML document
  /// from the given data source.
  /// \param InStream The AL `InStream`.
  /// \param Result The AL `XmlDocument`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean ReadFrom(const ::agiru::InStream &InStream, ::agiru::XmlDocument &Result);

  /// \brief AL `XmlDocument.ReadFrom(InStream, XmlReadOptions, XmlDocument)`. Reads and parses the
  /// XML document from the given data source.
  /// \param InStream The AL `InStream`.
  /// \param ReadOptions The AL `XmlReadOptions`.
  /// \param Result The AL `XmlDocument`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean ReadFrom(const ::agiru::InStream &InStream,
                                   const ::agiru::XmlReadOptions &ReadOptions,
                                   ::agiru::XmlDocument &Result);

  /// \brief AL `XmlDocument.ReadFrom(Text, XmlDocument)`. Reads and parses the XML document from
  /// the given data source.
  /// \param Text The AL `Text`.
  /// \param Result The AL `XmlDocument`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean ReadFrom(std::string_view Text, ::agiru::XmlDocument &Result);

  /// \brief AL `XmlDocument.ReadFrom(Text, XmlReadOptions, XmlDocument)`. Reads and parses the XML
  /// document from the given data source.
  /// \param Text The AL `Text`.
  /// \param ReadOptions The AL `XmlReadOptions`.
  /// \param Result The AL `XmlDocument`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean ReadFrom(std::string_view Text,
                                   const ::agiru::XmlReadOptions &ReadOptions,
                                   ::agiru::XmlDocument &Result);

  /// \brief AL `XmlDocument.Remove()`. Removes this node from its parent element.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Remove();

  /// \brief AL `XmlDocument.RemoveNodes()`. Removes the child nodes from this document.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void RemoveNodes();

  /// \brief AL `XmlDocument.ReplaceNodes(Any)`. Replaces the children nodes of this document with
  /// the specified content.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReplaceNodes(const ::agiru::Variant &Content);

  /// \brief AL `XmlDocument.ReplaceWith(Any)`. Replaces this node with the specified content.
  /// \param Node The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReplaceWith(const ::agiru::Variant &Node);

  /// \brief AL `XmlDocument.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)`. Selects a list of
  /// nodes matching the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath,
                               const ::agiru::XmlNamespaceManager &NamespaceManager,
                               ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlDocument.SelectNodes(Text, XmlNodeList)`. Selects a list of nodes matching the
  /// XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlDocument.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)`. Selects the
  /// first XmlNode that matches the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath,
                                    const ::agiru::XmlNamespaceManager &NamespaceManager,
                                    ::agiru::XmlNode &Node);

  /// \brief AL `XmlDocument.SelectSingleNode(Text, XmlNode)`. Selects the first XmlNode that
  /// matches the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node);

  /// \brief AL `XmlDocument.SetDeclaration(XmlDeclaration)`. Sets the XML declaration for this
  /// document.
  /// \param Declaration The AL `XmlDeclaration`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SetDeclaration(const ::agiru::XmlDeclaration &Declaration);

  /// \brief AL `XmlDocument.WriteTo(OutStream)`. Serializes and saves the current node to the given
  /// variable.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlDocument.WriteTo(Text)`. Serializes and saves the current node to the given
  /// variable.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(std::string &Text);

  /// \brief AL `XmlDocument.WriteTo(XmlWriteOptions, OutStream)`. Serializes and saves the current
  /// node to the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                           const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlDocument.WriteTo(XmlWriteOptions, Text)`. Serializes and saves the current node
  /// to the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions, std::string &Text);
};

} // namespace agiru

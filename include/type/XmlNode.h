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
/// \brief AL `XmlNode` -- the surface the platform documentation declares.

namespace agiru {

class XmlAttribute;
class XmlCData;
class XmlComment;
class XmlDeclaration;
class XmlDocument;
class XmlDocumentType;
class XmlElement;
class XmlNamespaceManager;
class XmlNodeList;
class XmlProcessingInstruction;
class XmlText;
class XmlWriteOptions;

/// \brief AL `XmlNode`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/xmlnode/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class XmlNode {
public:
  /// \brief AL `XmlNode.AddAfterSelf(Any)`. Adds the specified content immediately after this node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddAfterSelf(const ::agiru::Variant &Content);

  /// \brief AL `XmlNode.AddBeforeSelf(Any)`. Adds the specified content immediately before this
  /// node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddBeforeSelf(const ::agiru::Variant &Content);

  /// \brief AL `XmlNode.AsXmlAttribute()`. Converts the node to an XmlAttribute node. The operation
  /// will fail if the node is not an XmlAttribute.
  /// \return The AL `XmlAttribute`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlAttribute AsXmlAttribute();

  /// \brief AL `XmlNode.AsXmlCData()`. Converts the node to an XmlCData node. The operation will
  /// fail if the node is not an XmlCData.
  /// \return The AL `XmlCData`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlCData AsXmlCData();

  /// \brief AL `XmlNode.AsXmlComment()`. Converts the node to an XmlComment node. The operation
  /// will fail if the node is not an XmlComment.
  /// \return The AL `XmlComment`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlComment AsXmlComment();

  /// \brief AL `XmlNode.AsXmlDeclaration()`. Converts the node to an XmlDeclaration node. The
  /// operation will fail if the node is not an XmlDeclaration.
  /// \return The AL `XmlDeclaration`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlDeclaration AsXmlDeclaration();

  /// \brief AL `XmlNode.AsXmlDocument()`. Converts the node to an XmlDocument node. The operation
  /// will fail if the node is not an XmlDocument.
  /// \return The AL `XmlDocument`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlDocument AsXmlDocument();

  /// \brief AL `XmlNode.AsXmlDocumentType()`. Converts the node to an XmlDocumentType node. The
  /// operation will fail if the node is not an XmlDocumentType.
  /// \return The AL `XmlDocumentType`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlDocumentType AsXmlDocumentType();

  /// \brief AL `XmlNode.AsXmlElement()`. Converts the node to an XmlElement node. The operation
  /// will fail if the node is not an XmlElement.
  /// \return The AL `XmlElement`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlElement AsXmlElement();

  /// \brief AL `XmlNode.AsXmlProcessingInstruction()`. Converts the node to an
  /// XmlProcessingInstruction node. The operation will fail if the node is not an
  /// XmlProcessingInstruction.
  /// \return The AL `XmlProcessingInstruction`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlProcessingInstruction AsXmlProcessingInstruction();

  /// \brief AL `XmlNode.AsXmlText()`. Converts the node to an XmlText node. The operation will fail
  /// if the node is not an XmlText.
  /// \return The AL `XmlText`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlText AsXmlText();

  /// \brief AL `XmlNode.GetDocument(XmlDocument)`. Gets the XmlDocument for this node.
  /// \param Document The AL `XmlDocument`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetDocument(::agiru::XmlDocument &Document);

  /// \brief AL `XmlNode.GetParent(XmlElement)`. Gets the parent XmlElement of this node.
  /// \param Parent The AL `XmlElement`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetParent(::agiru::XmlElement &Parent);

  /// \brief AL `XmlNode.IsXmlAttribute()`. Gets a value indicating whether this node is an
  /// XmlAttribute.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlAttribute();

  /// \brief AL `XmlNode.IsXmlCData()`. Gets a value indicating whether this node is an XmlCData.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlCData();

  /// \brief AL `XmlNode.IsXmlComment()`. Gets a value indicating whether this node is an
  /// XmlComment.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlComment();

  /// \brief AL `XmlNode.IsXmlDeclaration()`. Gets a value indicating whether this node is an
  /// XmlDeclaration.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlDeclaration();

  /// \brief AL `XmlNode.IsXmlDocument()`. Gets a value indicating whether this node is an
  /// XmlDocument.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlDocument();

  /// \brief AL `XmlNode.IsXmlDocumentType()`. Gets a value indicating whether this node is an
  /// XmlDocumentType.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlDocumentType();

  /// \brief AL `XmlNode.IsXmlElement()`. Gets a value indicating whether this node is an
  /// XmlElement.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlElement();

  /// \brief AL `XmlNode.IsXmlProcessingInstruction()`. Gets a value indicating whether this node is
  /// an XmlProcessingInstruction.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlProcessingInstruction();

  /// \brief AL `XmlNode.IsXmlText()`. Gets a value indicating whether this node is an XmlText.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsXmlText();

  /// \brief AL `XmlNode.Remove()`. Removes this node from its parent element.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Remove();

  /// \brief AL `XmlNode.ReplaceWith(Any)`. Replaces this node with the specified content.
  /// \param Node The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReplaceWith(const ::agiru::Variant &Node);

  /// \brief AL `XmlNode.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)`. Selects a list of
  /// nodes matching the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath,
                               const ::agiru::XmlNamespaceManager &NamespaceManager,
                               ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlNode.SelectNodes(Text, XmlNodeList)`. Selects a list of nodes matching the XPath
  /// expression.
  /// \param XPath The AL `Text`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlNode.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)`. Selects the first
  /// XmlNode that matches the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath,
                                    const ::agiru::XmlNamespaceManager &NamespaceManager,
                                    ::agiru::XmlNode &Node);

  /// \brief AL `XmlNode.SelectSingleNode(Text, XmlNode)`. Selects the first XmlNode that matches
  /// the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node);

  /// \brief AL `XmlNode.WriteTo(OutStream)`. Serializes and saves the current node to the given
  /// variable.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlNode.WriteTo(Text)`. Serializes and saves the current node to the given
  /// variable.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(std::string &Text);

  /// \brief AL `XmlNode.WriteTo(XmlWriteOptions, OutStream)`. Serializes and saves the current node
  /// to the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                           const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlNode.WriteTo(XmlWriteOptions, Text)`. Serializes and saves the current node to
  /// the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions, std::string &Text);
};

}

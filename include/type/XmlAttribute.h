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
/// \brief AL `XmlAttribute` -- the surface the platform documentation declares.

namespace agiru {

class XmlDocument;
class XmlElement;
class XmlNamespaceManager;
class XmlNode;
class XmlNodeList;
class XmlWriteOptions;

/// \brief AL `XmlAttribute`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/xmlattribute/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class XmlAttribute {
public:
  /// \brief AL `XmlAttribute.AddAfterSelf(Any)`. Adds the specified content immediately after this
  /// node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddAfterSelf(const ::agiru::Variant &Content);

  /// \brief AL `XmlAttribute.AddBeforeSelf(Any)`. Adds the specified content immediately before
  /// this node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddBeforeSelf(const ::agiru::Variant &Content);

  /// \brief AL `XmlAttribute.AsXmlNode()`. Converts the node to an XmlNode.
  /// \return The AL `XmlNode`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNode AsXmlNode();

  /// \brief AL `XmlAttribute.Create(Text, Text)`. Creates an XmlAttribute node.
  /// \param Name The AL `Text`.
  /// \param Value The AL `Text`.
  /// \return The AL `XmlAttribute`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlAttribute Create(std::string_view Name, std::string_view Value);

  /// \brief AL `XmlAttribute.Create(Text, Text, Text)`. Creates an XmlAttribute node.
  /// \param LocalName The AL `Text`.
  /// \param NamespaceUri The AL `Text`.
  /// \param Value The AL `Text`.
  /// \return The AL `XmlAttribute`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlAttribute
  Create(std::string_view LocalName, std::string_view NamespaceUri, std::string_view Value);

  /// \brief AL `XmlAttribute.CreateNamespaceDeclaration(Text, Text)`. Creates an attribute that
  /// represents a namespace declaration.
  /// \param Prefix The AL `Text`.
  /// \param NamespaceUri The AL `Text`.
  /// \return The AL `XmlAttribute`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlAttribute CreateNamespaceDeclaration(std::string_view Prefix,
                                                   std::string_view NamespaceUri);

  /// \brief AL `XmlAttribute.GetDocument(XmlDocument)`. Gets the XmlDocument for this node.
  /// \param Document The AL `XmlDocument`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetDocument(::agiru::XmlDocument &Document);

  /// \brief AL `XmlAttribute.GetParent(XmlElement)`. Gets the parent XmlElement of this node.
  /// \param Parent The AL `XmlElement`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetParent(::agiru::XmlElement &Parent);

  /// \brief AL `XmlAttribute.IsNamespaceDeclaration()`. Determines if this attribute is a namespace
  /// declaration.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsNamespaceDeclaration();

  /// \brief AL `XmlAttribute.LocalName()`. Gets the local name of the attribute.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string LocalName();

  /// \brief AL `XmlAttribute.Name()`. The qualified name of the attribute.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Name();

  /// \brief AL `XmlAttribute.NamespacePrefix()`. Gets the prefix of the attribute (if any).
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string NamespacePrefix();

  /// \brief AL `XmlAttribute.NamespaceUri()`. Gets the namespace URI of the attribute.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string NamespaceUri();

  /// \brief AL `XmlAttribute.Remove()`. Removes this node from its parent element.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Remove();

  /// \brief AL `XmlAttribute.ReplaceWith(Any)`. Replaces this node with the specified content.
  /// \param Node The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReplaceWith(const ::agiru::Variant &Node);

  /// \brief AL `XmlAttribute.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)`. Selects a list
  /// of nodes matching the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath,
                               const ::agiru::XmlNamespaceManager &NamespaceManager,
                               ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlAttribute.SelectNodes(Text, XmlNodeList)`. Selects a list of nodes matching the
  /// XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlAttribute.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)`. Selects the
  /// first XmlNode that matches the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath,
                                    const ::agiru::XmlNamespaceManager &NamespaceManager,
                                    ::agiru::XmlNode &Node);

  /// \brief AL `XmlAttribute.SelectSingleNode(Text, XmlNode)`. Selects the first XmlNode that
  /// matches the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node);

  /// \brief AL `XmlAttribute.Value(Text)`. Gets or sets the value of the attribute.
  /// \param NewValue The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Value(std::string_view NewValue);

  /// \brief AL `XmlAttribute.WriteTo(OutStream)`. Serializes and saves the current node to the
  /// given variable.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlAttribute.WriteTo(Text)`. Serializes and saves the current node to the given
  /// variable.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(std::string &Text);

  /// \brief AL `XmlAttribute.WriteTo(XmlWriteOptions, OutStream)`. Serializes and saves the current
  /// node to the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                           const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlAttribute.WriteTo(XmlWriteOptions, Text)`. Serializes and saves the current node
  /// to the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions, std::string &Text);
};

} // namespace agiru

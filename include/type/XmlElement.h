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
/// \brief AL `XmlElement` -- the surface the platform documentation declares.

namespace agiru {

class XmlAttribute;
class XmlAttributeCollection;
class XmlDocument;
class XmlNamespaceManager;
class XmlNode;
class XmlNodeList;
class XmlWriteOptions;

/// \brief AL `XmlElement`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/xmlelement/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class XmlElement {
public:
  /// \brief AL `XmlElement.Add(Any)`. Adds the specified content as a child of this element.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(const ::agiru::Variant &Content);

  /// \brief AL `XmlElement.AddAfterSelf(Any)`. Adds the specified content immediately after this
  /// node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddAfterSelf(const ::agiru::Variant &Content);

  /// \brief AL `XmlElement.AddBeforeSelf(Any)`. Adds the specified content immediately before this
  /// node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddBeforeSelf(const ::agiru::Variant &Content);

  /// \brief AL `XmlElement.AddFirst(Any)`. Adds the specified content at the start of the child
  /// list of this element.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddFirst(const ::agiru::Variant &Content);

  /// \brief AL `XmlElement.AsXmlNode()`. Converts the node to an XmlNode.
  /// \return The AL `XmlNode`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNode AsXmlNode();

  /// \brief AL `XmlElement.Attributes()`. Gets a collection of the attributes of this element.
  /// \return The AL `XmlAttributeCollection`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlAttributeCollection Attributes();

  /// \brief AL `XmlElement.Create(Text, Any)`. Creates an XmlElement node.
  /// \param Name The AL `Text`.
  /// \param Content The AL `Any`.
  /// \return The AL `XmlElement`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlElement Create(std::string_view Name, const ::agiru::Variant &Content);

  /// \brief AL `XmlElement.Create(Text)`. Creates an XmlElement node.
  /// \param Name The AL `Text`.
  /// \return The AL `XmlElement`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlElement Create(std::string_view Name);

  /// \brief AL `XmlElement.Create(Text, Text, Any)`. Creates an XmlElement node.
  /// \param LocalName The AL `Text`.
  /// \param NamespaceUri The AL `Text`.
  /// \param Content The AL `Any`.
  /// \return The AL `XmlElement`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlElement Create(std::string_view LocalName,
                             std::string_view NamespaceUri,
                             const ::agiru::Variant &Content);

  /// \brief AL `XmlElement.Create(Text, Text)`. Creates an XmlElement node.
  /// \param LocalName The AL `Text`.
  /// \param NamespaceUri The AL `Text`.
  /// \return The AL `XmlElement`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlElement Create(std::string_view LocalName, std::string_view NamespaceUri);

  /// \brief AL `XmlElement.GetChildElements()`. Gets a list containing the child elements for this
  /// element, in document order.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetChildElements();

  /// \brief AL `XmlElement.GetChildElements(Text)`. Gets a list containing the child elements for
  /// this element, in document order.
  /// \param Name The AL `Text`.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetChildElements(std::string_view Name);

  /// \brief AL `XmlElement.GetChildElements(Text, Text)`. Gets a list containing the child elements
  /// for this element, in document order.
  /// \param LocalName The AL `Text`.
  /// \param NamespaceUri The AL `Text`.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetChildElements(std::string_view LocalName, std::string_view NamespaceUri);

  /// \brief AL `XmlElement.GetChildNodes()`. Gets a list containing the child elements for this
  /// element, in document order.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetChildNodes();

  /// \brief AL `XmlElement.GetDescendantElements()`. Gets a list containing the descendant elements
  /// for this element, in document order.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetDescendantElements();

  /// \brief AL `XmlElement.GetDescendantElements(Text)`. Gets a list containing the descendant
  /// elements for this element, in document order.
  /// \param Name The AL `Text`.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetDescendantElements(std::string_view Name);

  /// \brief AL `XmlElement.GetDescendantElements(Text, Text)`. Gets a list containing the
  /// descendant elements for this element, in document order.
  /// \param LocalName The AL `Text`.
  /// \param NamespaceUri The AL `Text`.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetDescendantElements(std::string_view LocalName,
                                             std::string_view NamespaceUri);

  /// \brief AL `XmlElement.GetDescendantNodes()`. Gets a list containing the descendant nodes for
  /// this element, in document order.
  /// \return The AL `XmlNodeList`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNodeList GetDescendantNodes();

  /// \brief AL `XmlElement.GetDocument(XmlDocument)`. Gets the XmlDocument for this node.
  /// \param Document The AL `XmlDocument`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetDocument(::agiru::XmlDocument &Document);

  /// \brief AL `XmlElement.GetNamespaceOfPrefix(Text, Text)`. Gets the namespace associated with a
  /// particular prefix for this element.
  /// \param Prefix The AL `Text`.
  /// \param Result The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetNamespaceOfPrefix(std::string_view Prefix, std::string &Result);

  /// \brief AL `XmlElement.GetParent(XmlElement)`. Gets the parent XmlElement of this node.
  /// \param Parent The AL `XmlElement`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetParent(::agiru::XmlElement &Parent);

  /// \brief AL `XmlElement.GetPrefixOfNamespace(Text, Text)`. Gets the prefix associated with a
  /// namespace URI for this element.
  /// \param Namespace The AL `Text`.
  /// \param Result The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetPrefixOfNamespace(std::string_view Namespace, std::string &Result);

  /// \brief AL `XmlElement.HasAttributes()`. Gets a boolean value indicating whether this element
  /// has at least one attribute.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean HasAttributes();

  /// \brief AL `XmlElement.HasElements()`. Gets a value indicating whether this element has at
  /// least one child element.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean HasElements();

  /// \brief AL `XmlElement.InnerText()`. Gets the concatenated values of the node and all its child
  /// nodes.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string InnerText();

  /// \brief AL `XmlElement.InnerXml()`. Gets the markup representing only the child nodes of this
  /// node.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string InnerXml();

  /// \brief AL `XmlElement.IsEmpty()`. Gets a value indicating whether this element contains no
  /// content.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsEmpty();

  /// \brief AL `XmlElement.LocalName()`. Gets the local name of this element.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string LocalName();

  /// \brief AL `XmlElement.Name()`. Gets the fully qualified name of this element.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Name();

  /// \brief AL `XmlElement.NamespaceUri()`. Gets the namespace URI of this element.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string NamespaceUri();

  /// \brief AL `XmlElement.Remove()`. Removes this node from its parent element.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Remove();

  /// \brief AL `XmlElement.RemoveAllAttributes()`. Removes the attributes of this element.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void RemoveAllAttributes();

  /// \brief AL `XmlElement.RemoveAttribute(Text)`. Removes the specified attribute from this
  /// element.
  /// \param Name The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void RemoveAttribute(std::string_view Name);

  /// \brief AL `XmlElement.RemoveAttribute(Text, Text)`. Removes the specified attribute from this
  /// element.
  /// \param LocalName The AL `Text`.
  /// \param NamespaceUri The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void RemoveAttribute(std::string_view LocalName, std::string_view NamespaceUri);

  /// \brief AL `XmlElement.RemoveAttribute(XmlAttribute)`. Removes the specified attribute from
  /// this element.
  /// \param Attribute The AL `XmlAttribute`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void RemoveAttribute(const ::agiru::XmlAttribute &Attribute);

  /// \brief AL `XmlElement.RemoveNodes()`. Removes the child nodes from this element.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void RemoveNodes();

  /// \brief AL `XmlElement.ReplaceNodes(Any)`. Replaces the children nodes of this element with the
  /// specified content.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReplaceNodes(const ::agiru::Variant &Content);

  /// \brief AL `XmlElement.ReplaceWith(Any)`. Replaces this node with the specified content.
  /// \param Node The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReplaceWith(const ::agiru::Variant &Node);

  /// \brief AL `XmlElement.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)`. Selects a list of
  /// nodes matching the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath,
                               const ::agiru::XmlNamespaceManager &NamespaceManager,
                               ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlElement.SelectNodes(Text, XmlNodeList)`. Selects a list of nodes matching the
  /// XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlElement.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)`. Selects the first
  /// XmlNode that matches the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath,
                                    const ::agiru::XmlNamespaceManager &NamespaceManager,
                                    ::agiru::XmlNode &Node);

  /// \brief AL `XmlElement.SelectSingleNode(Text, XmlNode)`. Selects the first XmlNode that matches
  /// the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node);

  /// \brief AL `XmlElement.SetAttribute(Text, Text)`. Sets the value of the specified attribute or
  /// create it if is not part of the element's attribute collection.
  /// \param Name The AL `Text`.
  /// \param Value The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetAttribute(std::string_view Name, std::string_view Value);

  /// \brief AL `XmlElement.SetAttribute(Text, Text, Text)`. Sets the value of the specified
  /// attribute or create it if is not part of the element's attribute collection.
  /// \param LocalName The AL `Text`.
  /// \param NamespaceUri The AL `Text`.
  /// \param Value The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void
  SetAttribute(std::string_view LocalName, std::string_view NamespaceUri, std::string_view Value);

  /// \brief AL `XmlElement.WriteTo(OutStream)`. Serializes and saves the current node to the given
  /// variable.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlElement.WriteTo(Text)`. Serializes and saves the current node to the given
  /// variable.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(std::string &Text);

  /// \brief AL `XmlElement.WriteTo(XmlWriteOptions, OutStream)`. Serializes and saves the current
  /// node to the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                           const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlElement.WriteTo(XmlWriteOptions, Text)`. Serializes and saves the current node
  /// to the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions, std::string &Text);
};

}

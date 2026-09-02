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
/// \brief AL `XmlCData` -- the surface the platform documentation declares.

namespace agiru {

class XmlDocument;
class XmlElement;
class XmlNamespaceManager;
class XmlNode;
class XmlNodeList;
class XmlWriteOptions;

/// \brief AL `XmlCData`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/xmlcdata/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class XmlCData {
public:
  /// \brief AL `XmlCData.AddAfterSelf(Any)`. Adds the specified content immediately after this
  /// node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddAfterSelf(const ::agiru::Variant &Content);

  /// \brief AL `XmlCData.AddBeforeSelf(Any)`. Adds the specified content immediately before this
  /// node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddBeforeSelf(const ::agiru::Variant &Content);

  /// \brief AL `XmlCData.AsXmlNode()`. Converts the node to an XmlNode.
  /// \return The AL `XmlNode`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNode AsXmlNode();

  /// \brief AL `XmlCData.Create(Text)`. Creates an XmlCData node.
  /// \param Value The AL `Text`.
  /// \return The AL `XmlCData`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::XmlCData Create(std::string_view Value);

  /// \brief AL `XmlCData.GetDocument(XmlDocument)`. Gets the XmlDocument for this node.
  /// \param Document The AL `XmlDocument`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetDocument(::agiru::XmlDocument &Document);

  /// \brief AL `XmlCData.GetParent(XmlElement)`. Gets the parent XmlElement of this node.
  /// \param Parent The AL `XmlElement`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetParent(::agiru::XmlElement &Parent);

  /// \brief AL `XmlCData.Remove()`. Removes this node from its parent element.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Remove();

  /// \brief AL `XmlCData.ReplaceWith(Any)`. Replaces this node with the specified content.
  /// \param Node The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReplaceWith(const ::agiru::Variant &Node);

  /// \brief AL `XmlCData.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)`. Selects a list of
  /// nodes matching the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath,
                               const ::agiru::XmlNamespaceManager &NamespaceManager,
                               ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlCData.SelectNodes(Text, XmlNodeList)`. Selects a list of nodes matching the
  /// XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlCData.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)`. Selects the first
  /// XmlNode that matches the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath,
                                    const ::agiru::XmlNamespaceManager &NamespaceManager,
                                    ::agiru::XmlNode &Node);

  /// \brief AL `XmlCData.SelectSingleNode(Text, XmlNode)`. Selects the first XmlNode that matches
  /// the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node);

  /// \brief AL `XmlCData.Value(Text)`. Gets or sets the value of this node.
  /// \param NewValue The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Value(std::string_view NewValue);

  /// \brief AL `XmlCData.WriteTo(OutStream)`. Serializes and saves the current node to the given
  /// variable.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlCData.WriteTo(Text)`. Serializes and saves the current node to the given
  /// variable.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(std::string &Text);

  /// \brief AL `XmlCData.WriteTo(XmlWriteOptions, OutStream)`. Serializes and saves the current
  /// node to the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                           const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlCData.WriteTo(XmlWriteOptions, Text)`. Serializes and saves the current node to
  /// the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions, std::string &Text);
};

} // namespace agiru

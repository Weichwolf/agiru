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
/// \brief AL `XmlDocumentType` -- the surface the platform documentation declares.

namespace agiru {

class XmlDocument;
class XmlElement;
class XmlNamespaceManager;
class XmlNode;
class XmlNodeList;
class XmlWriteOptions;

/// \brief AL `XmlDocumentType`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/xmldocumenttype/` states, so a call site compiles and is CHECKED; the
///          body refuses by name rather than returning a plausible wrong answer (board:0035).
class XmlDocumentType {
public:
  /// \brief AL `XmlDocumentType.AddAfterSelf(Any)`. Adds the specified content immediately after
  /// this node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddAfterSelf(const ::agiru::Variant &Content);

  /// \brief AL `XmlDocumentType.AddBeforeSelf(Any)`. Adds the specified content immediately before
  /// this node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddBeforeSelf(const ::agiru::Variant &Content);

  /// \brief AL `XmlDocumentType.AsXmlNode()`. Converts the node to an XmlNode.
  /// \return The AL `XmlNode`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNode AsXmlNode();

  /// \brief AL `XmlDocumentType.Create(Text)`. Creates an XmlDocumentType node.
  /// \param Name The AL `Text`.
  /// \return The AL `XmlDocumentType`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlDocumentType Create(std::string_view Name);

  /// \brief AL `XmlDocumentType.Create(Text, Text)`. Creates an XmlDocumentType node.
  /// \param Name The AL `Text`.
  /// \param PublicId The AL `Text`.
  /// \return The AL `XmlDocumentType`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlDocumentType Create(std::string_view Name, std::string_view PublicId);

  /// \brief AL `XmlDocumentType.Create(Text, Text, Text)`. Creates an XmlDocumentType node.
  /// \param Name The AL `Text`.
  /// \param PublicId The AL `Text`.
  /// \param SystemId The AL `Text`.
  /// \return The AL `XmlDocumentType`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlDocumentType
  Create(std::string_view Name, std::string_view PublicId, std::string_view SystemId);

  /// \brief AL `XmlDocumentType.Create(Text, Text, Text, Text)`. Creates an XmlDocumentType node.
  /// \param Name The AL `Text`.
  /// \param PublicId The AL `Text`.
  /// \param SystemId The AL `Text`.
  /// \param InternalSubSet The AL `Text`.
  /// \return The AL `XmlDocumentType`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlDocumentType Create(std::string_view Name,
                                  std::string_view PublicId,
                                  std::string_view SystemId,
                                  std::string_view InternalSubSet);

  /// \brief AL `XmlDocumentType.GetDocument(XmlDocument)`. Gets the XmlDocument for this node.
  /// \param Document The AL `XmlDocument`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetDocument(::agiru::XmlDocument &Document);

  /// \brief AL `XmlDocumentType.GetInternalSubset(Text)`. Gets the internal subset for this
  /// Document Type Definition (DTD).
  /// \param Result The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetInternalSubset(std::string &Result);

  /// \brief AL `XmlDocumentType.GetName(Text)`. Gets the name for this Document Type Definition
  /// (DTD).
  /// \param Result The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetName(std::string &Result);

  /// \brief AL `XmlDocumentType.GetParent(XmlElement)`. Gets the parent XmlElement of this node.
  /// \param Parent The AL `XmlElement`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetParent(::agiru::XmlElement &Parent);

  /// \brief AL `XmlDocumentType.GetPublicId(Text)`. Gets the public identifier for this Document
  /// Type Definition (DTD).
  /// \param Result The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetPublicId(std::string &Result);

  /// \brief AL `XmlDocumentType.GetSystemId(Text)`. Gets the system identifier for this Document
  /// Type Definition (DTD).
  /// \param Result The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetSystemId(std::string &Result);

  /// \brief AL `XmlDocumentType.Remove()`. Removes this node from its parent element.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Remove();

  /// \brief AL `XmlDocumentType.ReplaceWith(Any)`. Replaces this node with the specified content.
  /// \param Node The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReplaceWith(const ::agiru::Variant &Node);

  /// \brief AL `XmlDocumentType.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)`. Selects a
  /// list of nodes matching the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath,
                               const ::agiru::XmlNamespaceManager &NamespaceManager,
                               ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlDocumentType.SelectNodes(Text, XmlNodeList)`. Selects a list of nodes matching
  /// the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlDocumentType.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)`. Selects the
  /// first XmlNode that matches the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath,
                                    const ::agiru::XmlNamespaceManager &NamespaceManager,
                                    ::agiru::XmlNode &Node);

  /// \brief AL `XmlDocumentType.SelectSingleNode(Text, XmlNode)`. Selects the first XmlNode that
  /// matches the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node);

  /// \brief AL `XmlDocumentType.SetInternalSubset(Text)`. Sets the internal subset for this
  /// Document Type Definition (DTD).
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SetInternalSubset(std::string_view Value);

  /// \brief AL `XmlDocumentType.SetName(Text)`. Sets the name for this Document Type Definition
  /// (DTD).
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SetName(std::string_view Value);

  /// \brief AL `XmlDocumentType.SetPublicId(Text)`. Sets the public identifier for this Document
  /// Type Definition (DTD).
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SetPublicId(std::string_view Value);

  /// \brief AL `XmlDocumentType.SetSystemId(Text)`. Sets the system identifier for this Document
  /// Type Definition (DTD).
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SetSystemId(std::string_view Value);

  /// \brief AL `XmlDocumentType.WriteTo(OutStream)`. Serializes and saves the current node to the
  /// given variable.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlDocumentType.WriteTo(Text)`. Serializes and saves the current node to the given
  /// variable.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(std::string &Text);

  /// \brief AL `XmlDocumentType.WriteTo(XmlWriteOptions, OutStream)`. Serializes and saves the
  /// current node to the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                           const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlDocumentType.WriteTo(XmlWriteOptions, Text)`. Serializes and saves the current
  /// node to the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions, std::string &Text);
};

} // namespace agiru

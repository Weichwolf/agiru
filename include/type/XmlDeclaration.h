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
#include "type/StringValue.h"
#include "type/Time.h"
#include "type/Variant.h"
#include "type/XmlHandle.h"

#include <string>
#include <string_view>
#include <utility>

/// \file
/// \brief AL `XmlDeclaration` -- the surface the platform documentation declares.

namespace agiru {

class XmlDocument;
class XmlElement;
class XmlNamespaceManager;
class XmlNode;
class XmlNodeList;
class XmlWriteOptions;

/// \brief AL `XmlDeclaration`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/xmldeclaration/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class XmlDeclaration {
public:
  /// \brief AL `XmlDeclaration.AddAfterSelf(Any)`. Adds the specified content immediately after
  /// this node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddAfterSelf(const ::agiru::Variant &Content);

  /// \brief AL `AddAfterSelf(Content: Any, ...)` with more than one content: each in turn, in
  /// order.
  /// \tparam First The first content's type. \tparam Rest The others' types.
  /// \param first The first. \param rest The others.
  /// \return Whether every one was added.
  template <typename First, typename... Rest>
    requires(sizeof...(Rest) > 0)::agiru::Boolean
  AddAfterSelf(const First &first, const Rest &...rest) {
    return AddAfterSelf(::agiru::Variant(first)) && (AddAfterSelf(::agiru::Variant(rest)) && ...);
  }

  /// \brief AL `XmlDeclaration.AddBeforeSelf(Any)`. Adds the specified content immediately before
  /// this node.
  /// \param Content The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AddBeforeSelf(const ::agiru::Variant &Content);

  /// \brief AL `AddBeforeSelf(Content: Any, ...)` with more than one content: each in turn, in
  /// order.
  /// \tparam First The first content's type. \tparam Rest The others' types.
  /// \param first The first. \param rest The others.
  /// \return Whether every one was added.
  template <typename First, typename... Rest>
    requires(sizeof...(Rest) > 0)::agiru::Boolean
  AddBeforeSelf(const First &first, const Rest &...rest) {
    return AddBeforeSelf(::agiru::Variant(first)) && (AddBeforeSelf(::agiru::Variant(rest)) && ...);
  }

  /// \brief AL `XmlDeclaration.AsXmlNode()`. Converts the node to an XmlNode.
  /// \return The AL `XmlNode`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNode AsXmlNode();

  /// \brief AL `XmlDeclaration.Create(Text, Text, Text)`. Creates an XmlDeclaration node.
  /// \param Version The AL `Text`.
  /// \param Encoding The AL `Text`.
  /// \param Standalone The AL `Text`.
  /// \return The AL `XmlDeclaration`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::XmlDeclaration
  Create(std::string_view Version, std::string_view Encoding, std::string_view Standalone);

  /// \brief AL `XmlDeclaration.Encoding(Text)`. Gets or sets the encoding of the XML document.
  /// \param NewValue The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `XmlDeclaration.Encoding()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] XmlDeclaration.Encoding([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Encoding();

  std::string Encoding(std::string_view NewValue);

  /// \brief AL `XmlDeclaration.GetDocument(XmlDocument)`. Gets the XmlDocument for this node.
  /// \param Document The AL `XmlDocument`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetDocument(::agiru::XmlDocument &Document);

  /// \brief AL `XmlDeclaration.GetParent(XmlElement)`. Gets the parent XmlElement of this node.
  /// \param Parent The AL `XmlElement`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetParent(::agiru::XmlElement &Parent);

  /// \brief AL `XmlDeclaration.Remove()`. Removes this node from its parent element.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Remove();

  /// \brief AL `XmlDeclaration.ReplaceWith(Any)`. Replaces this node with the specified content.
  /// \param Node The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReplaceWith(const ::agiru::Variant &Node);

  /// \brief AL `XmlDeclaration.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)`. Selects a list
  /// of nodes matching the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath,
                               const ::agiru::XmlNamespaceManager &NamespaceManager,
                               ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlDeclaration.SelectNodes(Text, XmlNodeList)`. Selects a list of nodes matching
  /// the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NodeList The AL `XmlNodeList`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList);

  /// \brief AL `XmlDeclaration.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)`. Selects the
  /// first XmlNode that matches the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param NamespaceManager The AL `XmlNamespaceManager`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath,
                                    const ::agiru::XmlNamespaceManager &NamespaceManager,
                                    ::agiru::XmlNode &Node);

  /// \brief AL `XmlDeclaration.SelectSingleNode(Text, XmlNode)`. Selects the first XmlNode that
  /// matches the XPath expression.
  /// \param XPath The AL `Text`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node);

  /// \brief AL `XmlDeclaration.Standalone(Text)`. Gets or sets the standalone property for this
  /// document.
  /// \param NewValue The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `XmlDeclaration.Standalone()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] XmlDeclaration.Standalone([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Standalone();

  std::string Standalone(std::string_view NewValue);

  /// \brief AL `XmlDeclaration.Version(Text)`. Gets or sets the version property for this document.
  /// \param NewValue The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `XmlDeclaration.Version()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] XmlDeclaration.Version([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Version();

  std::string Version(std::string_view NewValue);

  /// \brief AL `XmlDeclaration.WriteTo(OutStream)`. Serializes and saves the current node to the
  /// given variable.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlDeclaration.WriteTo(Text)`. Serializes and saves the current node to the given
  /// variable.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(::agiru::Text<0> &Text);

  /// \brief AL `XmlDeclaration.WriteTo(XmlWriteOptions, OutStream)`. Serializes and saves the
  /// current node to the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                           const ::agiru::OutStream &OutStream);

  /// \brief AL `XmlDeclaration.WriteTo(XmlWriteOptions, Text)`. Serializes and saves the current
  /// node to the given variable.
  /// \param WriteOptions The AL `XmlWriteOptions`.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::XmlWriteOptions &WriteOptions, ::agiru::Text<0> &Text);

  /// \brief The node this value refers to; empty for a value never assigned.
  [[nodiscard]] const detail::XmlHandle &Handle() const noexcept { return handle_; }

  /// \brief The AL XML type this is, for a Variant.
  static constexpr detail::XmlKind kKind = detail::XmlKind::Declaration;

  /// \brief A value over a node, made by the engine.
  /// \param handle The node.
  explicit XmlDeclaration(detail::XmlHandle handle) noexcept : handle_(std::move(handle)) {}

  /// \brief A value that refers to nothing yet, which AL's declaration is.
  XmlDeclaration() = default;

private:
  detail::XmlHandle handle_;
};

}

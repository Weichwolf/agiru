#pragma once

#include "dotnet/Refused.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"
#include "type/XmlHandle.h"
#include "type/XmlNameTable.h"

#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace agiru::dotnet {

class XmlAttributeCollection;
class XmlDocument;
class XmlElement;
class XmlNamespaceManager;
class XmlNodeList;

/// \brief .NET `System.Xml.XmlNode`, over the same libxml2 tree the AL XML types walk
///        (board:0646). A null reference is an empty handle, which `IsNull` sees.
class XmlNode {
public:
  /// \brief A null reference.
  XmlNode() = default;

  /// \brief A node over a handle, made by the engine.
  /// \param handle The node.
  explicit XmlNode(::agiru::detail::XmlHandle handle) noexcept : handle_(std::move(handle)) {}

  /// \brief The node behind this reference.
  [[nodiscard]] const ::agiru::detail::XmlHandle &Handle() const noexcept { return handle_; }

  /// \brief AL `IsNull(XmlNode)`.
  [[nodiscard]] bool IsNullObject() const noexcept { return handle_.Empty(); }

  /// \brief `XmlNode.AppendChild(node)`: appends and answers the appended node.
  /// \param node The node; it moves into this tree.
  /// \return The node.
  XmlNode AppendChild(const XmlNode &node);
  /// \brief `XmlNode.Attributes`. \return The element's attributes; empty for any other node.
  [[nodiscard]] XmlAttributeCollection Attributes() const;
  /// \brief `XmlNode.ChildNodes`. \return The children, in document order.
  [[nodiscard]] XmlNodeList ChildNodes() const;
  /// \brief `XmlNode.FirstChild`. \return The first child, or null.
  [[nodiscard]] XmlNode FirstChild() const;
  /// \brief `XmlNode.LastChild`. \return The last child, or null.
  [[nodiscard]] XmlNode LastChild() const;
  /// \brief `XmlNode.NextSibling`. \return The next sibling, or null.
  [[nodiscard]] XmlNode NextSibling() const;
  /// \brief `XmlNode.ParentNode`. \return The parent, or null.
  [[nodiscard]] XmlNode ParentNode() const;
  /// \brief `XmlNode.OwnerDocument`. \return The document this node lives in.
  [[nodiscard]] XmlDocument OwnerDocument() const;
  /// \brief `XmlNode.HasChildNodes`. \return Whether there is a child.
  [[nodiscard]] ::agiru::Boolean HasChildNodes() const;
  /// \brief `XmlNode.InnerText` read. \return The concatenated text.
  [[nodiscard]] ::agiru::Text<0> InnerText() const;
  /// \brief `XmlNode.InnerText` write. \param text The new content. \return It.
  ::agiru::Text<0> InnerText(std::string_view text);
  /// \brief `XmlNode.InnerXml` read. \return The children's markup.
  [[nodiscard]] ::agiru::Text<0> InnerXml() const;
  /// \brief `XmlNode.InnerXml` write. \param markup The new children. \return It.
  ::agiru::Text<0> InnerXml(std::string_view markup);
  /// \brief `XmlNode.OuterXml`. \return This node's markup.
  [[nodiscard]] ::agiru::Text<0> OuterXml() const;
  /// \brief `XmlNode.LocalName`. \return The name without its prefix.
  [[nodiscard]] ::agiru::Text<0> LocalName() const;
  /// \brief `XmlNode.Name`. \return The qualified name.
  [[nodiscard]] ::agiru::Text<0> Name() const;
  /// \brief `XmlNode.NamespaceURI`. \return The namespace, or empty.
  [[nodiscard]] ::agiru::Text<0> NamespaceURI() const;
  /// \brief `XmlNode.Prefix`. \return The prefix, or empty.
  [[nodiscard]] ::agiru::Text<0> Prefix() const;
  /// \brief `XmlNode.Value` read. \return A text or attribute node's value.
  [[nodiscard]] ::agiru::Text<0> Value() const;
  /// \brief `XmlNode.Value` write. \param text The value. \return It.
  ::agiru::Text<0> Value(std::string_view text);
  /// \brief `XmlNode.RemoveChild(node)`. \param node The child. \return It.
  XmlNode RemoveChild(const XmlNode &node);
  /// \brief `XmlNode.ReplaceChild(new, old)`. \param made The new. \param old The old.
  /// \return The old.
  XmlNode ReplaceChild(const XmlNode &made, const XmlNode &old);
  /// \brief `XmlNode.InsertBefore(new, ref)`. \param made The new. \param before The reference.
  /// \return The new.
  XmlNode InsertBefore(const XmlNode &made, const XmlNode &before);
  /// \brief `XmlNode.SelectNodes(xpath)`. \param xpath The expression. \return The matches.
  [[nodiscard]] XmlNodeList SelectNodes(std::string_view xpath) const;
  /// \brief `XmlNode.SelectNodes(xpath, nsmgr)`. \param xpath The expression.
  /// \param manager The prefixes. \return The matches.
  [[nodiscard]] XmlNodeList SelectNodes(std::string_view xpath,
                                        const XmlNamespaceManager &manager) const;
  /// \brief `XmlNode.SelectSingleNode(xpath)`. \param xpath The expression.
  /// \return The first match, or null.
  [[nodiscard]] XmlNode SelectSingleNode(std::string_view xpath) const;
  /// \brief `XmlNode.SelectSingleNode(xpath, nsmgr)`. \param xpath The expression.
  /// \param manager The prefixes. \return The first match, or null.
  [[nodiscard]] XmlNode SelectSingleNode(std::string_view xpath,
                                         const XmlNamespaceManager &manager) const;
  /// \brief `XmlNode.NodeType`, compared against an `XmlNodeType` this runtime has not rebuilt.
  ::agiru::dotnet::Refused NodeType{{.type = "XmlNode", .member = "NodeType"}};
  /// \brief `XmlNode.WriteTo(XmlWriter)`, over a writer this runtime has not rebuilt.
  ::agiru::dotnet::Refused WriteTo{{.type = "XmlNode", .member = "WriteTo"}};
  /// \brief `XmlNode.Clone()` and `CloneNode`, not rebuilt.
  ::agiru::dotnet::Refused CloneNode{{.type = "XmlNode", .member = "CloneNode"}};
  /// \brief `XmlNode.RemoveAll()`, not rebuilt.
  ::agiru::dotnet::Refused RemoveAll{{.type = "XmlNode", .member = "RemoveAll"}};
  /// \brief `XmlNode.Normalize()`, not rebuilt.
  ::agiru::dotnet::Refused Normalize{{.type = "XmlNode", .member = "Normalize"}};

  /// \brief Becomes an absent .NET type a wrapper stores it in: an empty stub (board:0035).
  /// \tparam T The stub.
  /// \return An empty one.
  template <typename T>
    requires ::agiru::dotnet::IsAbsent<T>
  operator T() const {
    return T{};
  }

protected:
  ::agiru::detail::XmlHandle handle_; ///< The node.
};

/// \brief .NET `XmlNodeList`: the nodes a selection or `ChildNodes` answered.
class XmlNodeList {
public:
  XmlNodeList() = default;

  /// \brief A list over nodes. \param items The nodes.
  explicit XmlNodeList(std::vector<::agiru::detail::XmlHandle> items) noexcept
      : items_(std::move(items)) {}

  /// \brief `XmlNodeList.Count`. \return How many.
  [[nodiscard]] ::agiru::Integer Count() const;
  /// \brief `XmlNodeList.Item(i)`, zero-based. \param index The index. \return The node, or null.
  [[nodiscard]] XmlNode Item(::agiru::Integer index) const;

  /// \brief `XmlNodeList[i]`, spelled `ItemOf`. \param index The index. \return The node.
  [[nodiscard]] XmlNode ItemOf(::agiru::Integer index) const { return Item(index); }

  /// \brief AL `IsNull(XmlNodeList)`: a list is never null once made.
  [[nodiscard]] bool IsNullObject() const noexcept { return false; }

  /// \brief AL `foreach Node in List`: the nodes, as XmlNode values.
  [[nodiscard]] std::vector<XmlNode> Nodes() const;
  /// \brief The first node, for `foreach`.
  [[nodiscard]] std::vector<XmlNode>::const_iterator begin() const;
  /// \brief The end, for `foreach`.
  [[nodiscard]] std::vector<XmlNode>::const_iterator end() const;

private:
  std::vector<::agiru::detail::XmlHandle> items_;
  mutable std::vector<XmlNode> walked_;
};

}

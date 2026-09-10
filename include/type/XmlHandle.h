#pragma once

#include <cstdint>

namespace agiru::detail {

/// \brief One parsed or built XML tree, owned by reference count: every AL XML value that refers
///        into it holds one count, and the tree is freed with the last of them.
struct XmlTree;

/// \brief Takes one count on a tree.
/// \param tree The tree, or nothing.
void XmlRetain(XmlTree *tree) noexcept;

/// \brief Gives one count back; the last one frees the tree.
/// \param tree The tree, or nothing.
void XmlRelease(XmlTree *tree) noexcept;

/// \brief What an AL XML value refers to: a node inside a tree.
///
/// \note IT IS A REFERENCE, AS THE AL TYPES ARE. Two `XmlElement` variables that came from one
///       `SelectSingleNode` see the same element, and an `Add` through either shows through the
///       other (`xmlelement-data-type.md`). The count is intrusive rather than a `shared_ptr`
///       because this header reaches the Variant and the door, where `<memory>` is a measured
///       cost (board:0589).
struct XmlHandle {
  XmlTree *tree = nullptr; ///< The tree, counted.
  void *node = nullptr;    ///< The `xmlNode` inside it; nothing for an empty value.

  XmlHandle() = default;

  /// \brief A handle on a node, taking a count.
  XmlHandle(XmlTree *inTree, void *at) noexcept : tree(inTree), node(at) { XmlRetain(tree); }

  XmlHandle(const XmlHandle &o) noexcept : tree(o.tree), node(o.node) { XmlRetain(tree); }

  XmlHandle(XmlHandle &&o) noexcept : tree(o.tree), node(o.node) {
    o.tree = nullptr;
    o.node = nullptr;
  }

  XmlHandle &operator=(const XmlHandle &o) noexcept {
    if (this != &o) {
      XmlRetain(o.tree);
      XmlRelease(tree);
      tree = o.tree;
      node = o.node;
    }
    return *this;
  }

  XmlHandle &operator=(XmlHandle &&o) noexcept {
    if (this != &o) {
      XmlRelease(tree);
      tree = o.tree;
      node = o.node;
      o.tree = nullptr;
      o.node = nullptr;
    }
    return *this;
  }

  ~XmlHandle() { XmlRelease(tree); }

  /// \brief Whether the value refers to anything.
  [[nodiscard]] bool Empty() const noexcept { return node == nullptr; }
};

/// \brief A new, empty document: what .NET `new XmlDocument()` and AL `XmlDocument.Create()` make.
/// \return A handle on the document node.
XmlHandle EmptyDocument();

/// \brief Which AL XML type a node is seen as, for the Variant.
enum class XmlKind : std::uint8_t {
  Document,
  Element,
  Attribute,
  Text,
  CData,
  Comment,
  Declaration,
  DocumentType,
  ProcessingInstruction,
  Node,
  NodeList,
  AttributeCollection,
  NamespaceManager,
  NameTable,
};

}

#pragma once

#include <cstdint>

namespace agiru::detail {

/// \brief How a Newtonsoft token travels in a `Variant`: which of the rebuilt `JObject` family
///        the variable that put it there was. \see dotnet::JToken
enum class JsonKind : std::uint8_t { Token, Object, Array, Value, Property };

/// \brief One JSON document, owned by reference count: every AL JSON value that refers into it
///        holds one count, and the document is freed with the last of them.
struct JsonTree;

/// \brief Takes one count on a document.
/// \param tree The document, or nothing.
void JsonRetain(JsonTree *tree) noexcept;

/// \brief Gives one count back; the last one frees the document.
/// \param tree The document, or nothing.
void JsonRelease(JsonTree *tree) noexcept;

/// \brief What an AL JSON value refers to: a node inside a document.
///
/// \note IT IS A REFERENCE, AS THE AL TYPES ARE. `jsonobject-data-type.md`: "JsonObject is a
///       reference type", so two variables that came from one `Get` see the same node and an
///       `Add` through either shows through the other. The count is intrusive rather than a
///       `shared_ptr` because this header reaches the door, where `<memory>` is a measured cost
///       (board:0589), and `XmlHandle` solved the same problem the same way (board:0646).
struct JsonHandle {
  JsonTree *tree = nullptr; ///< The document, counted.
  void *node = nullptr;     ///< The node inside it; nothing for a value that holds none.

  JsonHandle() = default;

  /// \brief A handle on a node, taking a count.
  /// \param inTree The document. \param at The node.
  JsonHandle(JsonTree *inTree, void *at) noexcept : tree(inTree), node(at) { JsonRetain(tree); }

  /// \brief A second handle on the same node. \param o The other.
  JsonHandle(const JsonHandle &o) noexcept : tree(o.tree), node(o.node) { JsonRetain(tree); }

  /// \brief Takes the other's handle. \param o The other.
  JsonHandle(JsonHandle &&o) noexcept : tree(o.tree), node(o.node) {
    o.tree = nullptr;
    o.node = nullptr;
  }

  /// \brief Points at what the other points at. \param o The other. \return This.
  JsonHandle &operator=(const JsonHandle &o) noexcept {
    if (this != &o) {
      JsonRetain(o.tree);
      JsonRelease(tree);
      tree = o.tree;
      node = o.node;
    }
    return *this;
  }

  /// \brief Takes the other's handle. \param o The other. \return This.
  JsonHandle &operator=(JsonHandle &&o) noexcept {
    if (this != &o) {
      JsonRelease(tree);
      tree = o.tree;
      node = o.node;
      o.tree = nullptr;
      o.node = nullptr;
    }
    return *this;
  }

  ~JsonHandle() { JsonRelease(tree); }

  /// \brief Whether the value refers to anything. \return Whether it is empty.
  [[nodiscard]] bool Empty() const noexcept { return node == nullptr; }
};

/// \brief A new, empty object: what an AL `JsonObject` variable holds before anything is added.
/// \return A handle on it.
[[nodiscard]] JsonHandle NewJsonObject();

/// \brief A new, empty array. \return A handle on it.
[[nodiscard]] JsonHandle NewJsonArray();

/// \brief A new, null value. \return A handle on it.
[[nodiscard]] JsonHandle NewJsonValue();

}

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
#include "type/Time.h"
#include "type/Variant.h"
#include "type/XmlHandle.h"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

/// \file
/// \brief AL `XmlNodeList` -- the surface the platform documentation declares.

namespace agiru {

class XmlNode;

/// \brief AL `XmlNodeList`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/xmlnodelist/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class XmlNodeList {
public:
  /// \brief AL `XmlNodeList.Count()`. Gets the number of nodes in the XmlNodeList.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Count();

  /// \brief AL `XmlNodeList.Get(Integer, XmlNode)`. Gets a node at the given index.
  /// \param Index The AL `Integer`.
  /// \param Node The AL `XmlNode`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Get(::agiru::Integer Index, ::agiru::XmlNode &Node);

  /// \brief AL `foreach XmlNode in XmlNodeList` -- the first node.
  /// \return A pointer to the first node, so a range-for binds `XmlNode &`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNode *begin();

  /// \brief AL `foreach XmlNode in XmlNodeList` -- one past the last node.
  /// \return A pointer one past the last node.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::XmlNode *end();

  /// \brief The nodes, in document order.
  [[nodiscard]] const std::vector<detail::XmlHandle> &Items() const noexcept { return items_; }

  /// \brief The AL XML type this is, for a Variant.
  static constexpr detail::XmlKind kKind = detail::XmlKind::NodeList;

  /// \brief A list over nodes, made by the engine.
  /// \param items The nodes.
  explicit XmlNodeList(std::vector<detail::XmlHandle> items) noexcept : items_(std::move(items)) {}

  /// \brief An empty list, which AL's declaration is.
  XmlNodeList() = default;

private:
  std::vector<detail::XmlHandle> items_;
  mutable std::vector<XmlNode> walked_;
};

}

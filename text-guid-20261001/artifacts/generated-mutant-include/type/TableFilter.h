#pragma once

#include <string>
#include <string_view>

/// \file
/// \brief AL `TableFilter` -- a filter stored in a field, and applied to ANOTHER table.

namespace agiru {

/// \brief AL `TableFilter`.
///
/// From `fieldtype-option.md`: "This data type is used to apply a filter to another table.
/// Currently, this can only be used to apply security filters from the Permission table."
///
/// \note IT IS A FIELD TYPE AND NOT A VALUE TYPE, which is why it holds the filter as TEXT: what a
///       `Permission` row stores is the expression, and the table it applies to is decided by the
///       row rather than by this type. Parsing it is the filter language's job (board:0018) and it
///       happens where the filter is APPLIED, not where it is stored.
class TableFilter {
public:
  /// \brief An empty filter, which is what a blank field holds.
  TableFilter() = default;

  /// \brief A filter from its stored expression.
  /// \param expression The filter, as the column holds it.
  explicit TableFilter(std::string_view expression) : expression_(expression) {}

  /// \brief The stored expression.
  /// \return It, empty when the field is blank.
  [[nodiscard]] std::string_view Value() const { return expression_; }

  /// \brief Whether the field is blank, which is what `TestField` asks.
  /// \return True when no filter is stored.
  [[nodiscard]] bool IsEmpty() const { return expression_.empty(); }

  /// \brief Compares two filters.
  /// \param o The other.
  /// \return True when they store the same expression.
  [[nodiscard]] bool operator==(const TableFilter &o) const = default;

private:
  std::string expression_;
};

}

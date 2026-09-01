#pragma once

#include "meta/TableDef.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

/// \file
/// \brief AL's filter expression -- one parse, and consumers that read the same tree.

namespace agiru::detail {

/// What one atom of a filter asks of a value.
enum class Compare : std::uint8_t {
  Equal,        ///< `1`, `=1`
  NotEqual,     ///< `<>1`
  Less,         ///< `<1`
  LessOrEqual,  ///< `<=1`
  Greater,      ///< `>1`
  GreaterEqual, ///< `>=1`
  Between,      ///< `1..9`, `..9`, `1..`
  Like,         ///< `*Ltd*`, `?x`
  NotLike,      ///< `<>*Ltd*`
};

/// One comparison against one value, or a range between two.
struct Atom {
  Compare compare = Compare::Equal;
  std::string value;      ///< The operand, unquoted; the lower bound of a range.
  std::string upper;      ///< The upper bound of a range; empty when open at that end.
  bool openLower = false; ///< `..9` -- no lower bound.
  bool openUpper = false; ///< `1..` -- no upper bound.
};

/// A conjunction of atoms: `>=1000&<=2000`.
using All = std::vector<Atom>;

/// A disjunction of conjunctions, which is the whole expression: `1|2|>=10&<=20`.
///
/// `&` BINDS TIGHTER THAN `|`, which is why the shape is a list of lists rather than a flat one.
using Expression = std::vector<All>;

/// \brief Parses an AL filter expression.
///
/// \param text The expression, as `SetFilter` was handed it.
/// \return The parsed expression; an empty one when the text is empty.
/// \throws Error when the text is not a filter expression.
[[nodiscard]] Expression ParseFilter(std::string_view text);

/// \brief Whether one rendered value satisfies an expression.
///
/// \param expression The parsed filter.
/// \param value      The value, rendered as the field renders it.
/// \param def        The field, which decides how two values compare.
/// \return True when the value passes.
///
/// This is the IN-MEMORY consumer -- what a temporary record uses, and what a filter has to answer
/// with no database in reach.
[[nodiscard]] bool
Matches(const Expression &expression, std::string_view value, const FieldDef &def);

} // namespace agiru::detail

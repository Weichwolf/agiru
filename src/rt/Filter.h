#pragma once

#include "meta/TableDef.h"

#include <cstdint>
#include <optional>
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

/// A closed interval of integers, both ends included.
struct Interval {
  std::int64_t low;  ///< The smallest member.
  std::int64_t high; ///< The largest member; a value below `low` describes an empty interval.
};

/// \brief Compares two intervals.
/// \param a One interval.
/// \param b The other.
/// \return True when both ends agree.
///
/// A FREE FUNCTION, so that `Interval` stays a pure aggregate the way `FieldDef` and `KeyDef` are.
/// A member would give it methods beside public data, which is a different kind of thing.
[[nodiscard]] constexpr bool operator==(const Interval &a, const Interval &b) {
  return a.low == b.low && a.high == b.high;
}

/// A set of integers as disjoint intervals, ascending and non-adjacent.
using Intervals = std::vector<Interval>;

/// \brief The integers an expression admits within a domain.
///
/// \param expression The parsed filter; an empty one admits the whole domain.
/// \param domain     The values the field can hold at all.
/// \pre `domain.low` is above the smallest `int64` and `domain.high` below the largest, so that a
///      bound one step outside it is representable. `Integer`'s domain is +/- 1 000 000 000.
/// \return The admitted set, or nothing when the expression does not reduce to intervals.
///
/// A DISJUNCTION OF CONJUNCTIONS IS A UNION OF INTERSECTIONS, which is why this is a function of
/// the filter language rather than a prop for one table: each atom contributes a set, a conjunction
/// intersects them, the expression unions the results, and the answer is normalised so that no two
/// intervals touch or overlap.
///
/// `<>5` contributes TWO intervals rather than none -- it punches a hole -- so a conjunction can
/// hold a set and not merely a range. That is why the intersection is over sets throughout.
///
/// It answers NOTHING for a wildcard: AL allows `*1*` on an integer field and no interval describes
/// it. A caller that cannot fall back on scanning must refuse rather than guess, which is what the
/// `Integer` virtual table does -- it has no rows to scan.
[[nodiscard]] std::optional<Intervals> IntegerIntervals(const Expression &expression,
                                                        Interval domain);

/// \brief How many integers a set holds.
/// \param intervals The set.
/// \return The count.
///
/// A sequence table answers `Count()` from this and never asks the database: the arithmetic is
/// exact and costs the number of intervals rather than the number of rows.
[[nodiscard]] std::int64_t CountOf(const Intervals &intervals);

} // namespace agiru::detail

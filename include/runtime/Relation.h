#pragma once

#include "meta/TableDef.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::detail {

/// \brief One `where(...)` term of a resolved `TableRelation`: the related table's field and the
///        filter text it takes, already evaluated against the record (`field(X)` is X's value,
///        `const(V)` is V, `filter(F)` is F).
struct RelationFilter {
  std::string field; ///< The related table's field, by AL name.
  std::string text;  ///< The filter text, as `SetFilter` would take it.
};

/// \brief What a field's `TableRelation` resolves to for ONE record: the branch whose condition
///        the record satisfies.
struct ResolvedRelation {
  std::string table;                   ///< The related table, by AL name.
  std::string field;                   ///< Its field, by AL name; empty for the primary key.
  std::vector<RelationFilter> filters; ///< The `where(...)` terms, evaluated.
};

/// \brief Reads a field's `TableRelation` against the record it belongs to.
///
/// `devenv-set-relationships-between-tables.md`: `<Table>[.<Field>] [where(<filters>)]`, or
/// `if (<conditions>) <that> else <TableRelation>`, where a condition is `Field = const(V)` or
/// `Field = filter(F)` and a `where` term is `Field = const(V)`, `= filter(F)` or `= field(G)`.
/// The branches are tried in order and the first whose every condition the record satisfies is
/// the answer; a trailing branch with no condition always is.
/// \param record The record, whose fields the conditions and `field(...)` terms read.
/// \param table  Its declaration.
/// \param def    The field, whose `relation` (or `relationTable` / `relationField`) is read.
/// \return The branch, or nothing when the field declares no relation or no branch applies.
/// \throws Error when the text is not in the documented grammar.
[[nodiscard]] std::optional<ResolvedRelation>
ResolveRelation(const void *record, const TableDef &table, const FieldDef &def);

}

# 0018 — SQL and temporary records will evaluate the same filter grammar

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

`src/rt/Filter.cpp` is an existing parser; `RecordState.cpp` separately parses views using parentheses counts. Its splitter does not track quoted field/filter text. Temporary and SQL paths apply filters through different evaluators, so a passing SQL case does not prove the temporary case.

## Implementation for Sol

1. Define one typed filter AST and quote-aware view grammar. Cover escaped quotes, commas/parentheses inside literals, OR/AND precedence, wildcard modifiers, blank values, open ranges and field types.
2. Run the same fixtures through SQL and Temporary.cpp, including filter groups, SetRange replacement, CopyFilter, marks, GetRangeMin/Max and RecordId values.
3. Separate UI date/token expansion from SetFilter parsing; resolve metadata filter tokens through declared platform hooks rather than guessing.
4. Reject malformed input with a named diagnostic. Avoid repeatedly parsing immutable CalcFormula/TableRelation text on hot reads.

## Acceptance

Differential fixtures return the same ordered keys for SQL and temporary tables. Include quoted delimiter characters and closing dates; remove quoting or a filter-group term as negative controls. Preserve benchmark bounds for large filtered reads.

## References

Platform: devenv-entering-criteria-in-filters.md and methods-auto/record filter overloads. AL: No. Series temporary lookups and declared CalcFormula filters. Predecessor: WI-870/889/1063/1136 and the filter findings referenced in the old 0044.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `sourcetableview`, `sourcetableview-pages`, `sourcetableview-xmlports`.

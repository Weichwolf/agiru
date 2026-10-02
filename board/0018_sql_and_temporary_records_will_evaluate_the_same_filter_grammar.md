# 0018 — SQL and temporary records will evaluate the same filter grammar

Status: open | Priority: P1 | Stage: UT | Reviewed: 2026-09-28
Depends on: 0044 record state.

## Evidence

- `Filter.cpp` parses filters; `RecordState.cpp` separately splits views without tracking quoted delimiters.
- SQL and temporary evaluation are separate implementations. Parity must include filter-group composition, not only matching leaf predicates.

## Implementation

1. Compile immutable declaration filters once; bind only session/record values per execution. Keep typed SQL parameters and bounded temporary scans.
2. Define one typed filter AST and quote-aware view grammar. Cover escaped quotes, commas/parentheses inside literals, OR/AND precedence, wildcard modifiers, blank values, open ranges and field types.
3. Run the same fixtures through SQL and Temporary.cpp, including filter groups, SetRange replacement, CopyFilter, marks, GetRangeMin/Max and RecordId values.
4. Separate UI date/token expansion from SetFilter parsing; resolve metadata filter tokens through declared platform hooks rather than guessing.
5. Reject malformed input with a named diagnostic. Avoid repeatedly parsing immutable CalcFormula/TableRelation text on hot reads.

## Acceptance

- Differential fixtures return the same ordered keys for SQL and temporary tables. Include quoted delimiter characters and closing dates; remove quoting or a filter-group term as negative controls. Preserve benchmark bounds for large filtered reads.

## References

Code: `src/rt/Filter.cpp`, `src/rt/Where.cpp`, `src/rt/Selection.cpp`, `src/rt/Temporary.cpp`.

Platform: devenv-entering-criteria-in-filters.md and methods-auto/record filter overloads. AL: No. Series temporary lookups and declared CalcFormula filters. Predecessor: WI-870/889/1063/1136 and the filter findings referenced in the old 0044.

Property scope: `sourcetableview`, `sourcetableview-pages`, `sourcetableview-xmlports`.

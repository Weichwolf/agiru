# 0064 — Queries will stream correct joins, filters and typed aggregates

Status: open | Priority: P2 | Reviewed: 2026-09-22

## Current evidence

QueryWriter.cpp and src/rt/Query.cpp exist and QueryOpen uses Cursor. The old no-parser/no-writer claim is obsolete. Build constructs SELECT/JOIN/GROUP/HAVING and deserves semantic fixtures independent of generated metadata.

## Implementation for Sol

1. Gate all join kinds and nesting order. Child DataItemTableFilter belongs in ON for an outer join; root filters and column/HAVING filters have different placement.
2. Verify filter overwrite versus conjunction rules, grouped nonaggregate columns, integer Average, NULL defaults, ReverseSign, ordering and TopNumberOfRows.
3. Honor read isolation through 0012 and security/company context through 0062. Preserve bounded streaming for exports and propagate stream errors.
4. Add query API publication only after runtime Open/Read/Close semantics are complete; metadata reachability is not an endpoint.

## Acceptance

Fixture includes unmatched parent rows, filtered child rows, NULL aggregate inputs and runtime filter replacement. Compare generated query output to hand-written SQL and assert fetch memory stays bounded.

## References

Platform: queryinstance overloads; SqlJoinType, Method, ColumnFilter, DataItemTableFilter and ReadState properties. AL: query declarations. Existing QueryGate outer-join regression is the starting point, not a replacement target.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `columnfilter`, `dataitemlink-query`, `dataitemtablefilter`, `method`, `orderby`, `querycategory`, `querytype`, `reversesign`, `sqljointype`.

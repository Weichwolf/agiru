# 0064 — Queries will stream correct joins, filters and typed aggregates

Status: open | Priority: P2 | Stage: All; promote on a demonstrated UT blocker | Reviewed: 2026-09-28
Depends on: 0018 filters; 0019 aggregates; 0012 isolation.

## Evidence

- `QueryWriter.cpp` and `Query.cpp` produce SELECT/JOIN/GROUP/HAVING and use Cursor; semantic completeness is unproved.

## Implementation

1. Compile a typed query plan with distinct ON/WHERE/HAVING slots and explicit NULL-to-AL conversion. Feed its projected columns directly into bounded cursor decoding.
2. Gate all join kinds and nesting order. Child DataItemTableFilter belongs in ON for an outer join; root filters and column/HAVING filters have different placement.
3. Verify filter overwrite versus conjunction rules, grouped nonaggregate columns, integer Average, NULL defaults, ReverseSign, ordering and TopNumberOfRows.
4. Honor read isolation through 0012 and security/company context through 0062. Preserve bounded streaming for exports and propagate stream errors.
5. Add query API publication only after runtime Open/Read/Close semantics are complete; metadata reachability is not an endpoint.

## Acceptance

- Fixture includes unmatched parent rows, filtered child rows, NULL aggregate inputs and runtime filter replacement. Compare generated query output to hand-written SQL and assert fetch memory stays bounded.

## References

Code: `src/rt/Query.cpp`, `src/gen/QueryWriter.cpp`, `test/gate/QueryGate.cpp`.

Platform: queryinstance overloads; SqlJoinType, Method, ColumnFilter, DataItemTableFilter and ReadState properties. AL: query declarations. Existing QueryGate outer-join regression is the starting point, not a replacement target.

Property scope: `columnfilter`, `dataitemlink-query`, `dataitemtablefilter`, `method`, `orderby`, `querycategory`, `querytype`, `reversesign`, `sqljointype`.

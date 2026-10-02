# 0045 — Reads will remain bounded and partial records will be real

Status: open | Priority: P2 | Stage: Clients bounds; All scale | Reviewed: 2026-09-28
Depends on: 0012 transaction epochs; 0018 filters.

## Evidence

- `Cursor.cpp` fetches blocks; `Navigate.cpp` selects every stored column. SetLoadFields/AddLoadFields/LoadFields accept fields without projection.
- Cursor destruction checks transaction epoch/depth; Step/Fetch does not establish that a cached block remains valid after Commit/rollback.

## Implementation

1. Validate transaction generation before both cached-row delivery and FETCH; resume according to documented dynamic-result semantics. A pool lease cannot be returned while its cursor is live.
2. Preserve bounded cursor reads while making transaction generations and cursor invalidation explicit. Reopen from a stable key when AL requires continued navigation after writes/commit.
3. Implement selected-field state, initial projection and JIT loading with observed-rowversion checks. Public typed member reads need a deliberate generated access mechanism; do not pretend recording field numbers alone implements JIT.
4. Load BLOB payloads only through the documented CalcFields/load path. Keep media identifiers cheap; storage work is 0074.
5. Verify each maintained key index and ORDER BY prefix/tiebreak against actual plans. Benchmark with realistic cardinality and concurrent sessions, not the small gate fixture.

## Acceptance

- Peak memory remains proportional to fetch block and selected field widths as row count grows. Unloaded BLOBs are not transferred. JIT detects modified/deleted/renamed rows, and post-rollback cursor cleanup never aborts a later transaction.

## References

Platform: devenv-partial-records.md, its FAQ, record-findset-boolean-method.md, key properties and BLOB contract. Repository: Cursor.cpp, Navigate.cpp, Selection.cpp. Predecessor: consult cursor/partial-record findings before selecting resume semantics.

Property scope: `clustered`, `compressed`, `includedfields`, `keys`, `maintainsiftindex`, `maintainsqlindex`, `optimizefortextsearch`, `subtype-blob`, `unique`.

# 0045 — Reads will remain bounded and partial records will be real

Status: open | Priority: P2 | Reviewed: 2026-09-22

## Current evidence

`Cursor.cpp` already FETCHes blocks, so the old whole-result-set claim is obsolete. `Navigate.cpp` still selects Columns(table), and Table.h accepts SetLoadFields/LoadFields without implementing projection. Cursor validity is approximated by rollback count and boundary depth.

## Implementation for Sol

1. Preserve bounded cursor reads while making transaction generations and cursor invalidation explicit. Reopen from a stable key when AL requires continued navigation after writes/commit.
2. Implement selected-field state, initial projection and JIT loading with observed-rowversion checks. Public typed member reads need a deliberate generated access mechanism; do not pretend recording field numbers alone implements JIT.
3. Load BLOB payloads only through the documented CalcFields/load path. Keep media identifiers cheap; storage work is 0074.
4. Verify each maintained key index and ORDER BY prefix/tiebreak against actual plans. Benchmark with realistic cardinality and concurrent sessions, not the small gate fixture.

## Acceptance

Peak memory remains proportional to fetch block and selected field widths as row count grows. Unloaded BLOBs are not transferred. JIT detects modified/deleted/renamed rows, and post-rollback cursor cleanup never aborts a later transaction.

## References

Platform: devenv-partial-records.md, its FAQ, record-findset-boolean-method.md, key properties and BLOB contract. Repository: Cursor.cpp, Navigate.cpp, Selection.cpp. Predecessor: consult cursor/partial-record findings before selecting resume semantics.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `clustered`, `compressed`, `includedfields`, `keys`, `maintainsiftindex`, `maintainsqlindex`, `optimizefortextsearch`, `subtype-blob`, `unique`.

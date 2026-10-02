# 0019 — FlowFields will preserve semantics and use bounded aggregate queries

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

FieldClass and CalcFields support are already present. FlowField filtering and aggregation still need coherent SQL/temporary proofs; old claims that every FlowField is a stored column are stale. Stored properties alone do not prove SIFT behaviour.

## Implementation for Sol

1. Gate all seven CalcFormula operations, typed results, empty-set defaults, FlowFilter substitution and negated formulas. Compile declaration structure once into typed metadata.
2. For filters on FlowFields emit a correlated subquery over the correct table/company and combine it with ordinary filters. Exercise nested parentheses and NULL aggregates.
3. Verify CalcSums and selected key/filter behaviour from the current overload documentation; do not inherit the old title's blanket current-key restriction without checking it.
4. Implement maintained aggregates only after measuring plain SQL. Honor MaintainSiftIndex and consistency under insert/modify/delete/rollback from two writers.

## Acceptance

SQL and temporary fixtures agree; FlowFields never become stored columns. A second connection and rollback test prove aggregate consistency. Explain EXPLAIN plans and lock duration before claiming SIFT performance.

## References

Platform: devenv-flowfields.md, properties/devenv-calcformula-property.md, methods-auto/record/record-calcsums-method.md. AL: aggregate/filter users. Predecessor: WI-890 records activation regressions from FlowField filters.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `autocalcfield`, `calcfields`, `calcformula`, `fieldclass`.

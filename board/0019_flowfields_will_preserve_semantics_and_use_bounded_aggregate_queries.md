# 0019 — FlowFields will preserve semantics and use bounded aggregate queries

Status: open | Priority: P1 | Stage: UT totals; All aggregate scale | Reviewed: 2026-09-30
Depends on: 0018 filter AST; 0044 record primitives.

## Evidence

- FlowFields/CalcFields already exist. Selection.cpp silently skips a FlowField predicate if FlowFieldColumn returns empty; unresolved formulas can broaden reads.
- `Table.cpp::CalcFieldIfCarried` returns false for an absent target; page callers discard that result. Direct CalcFields refuses. User Personalization language/region formulas name Windows Language, which has no registered provider. Missing targets must not become blank successful displays.
- Its Full Name formula reads FIELD(User ID), itself a FlowField. Ordered explicit calculation of User ID then Full Name works; PredicateOf otherwise reads the current source value. Verify native dependency/evaluation guarantees before changing generic calculation, never special-case this table.

## Implementation

1. Make failed formula/provider resolution an explicit diagnostic, never a dropped predicate or blank success. Keep automatic page calculation and explicit CalcFields on one resolution contract. Compare one record's aggregate with the set query and temporary evaluator before optimizing.
2. Gate all seven CalcFormula operations, typed results, empty-set defaults, FlowFilter substitution and negated formulas. Compile declaration structure once into typed metadata.
3. For filters on FlowFields emit a correlated subquery over the correct table/company and combine it with ordinary filters. Exercise nested parentheses and NULL aggregates.
4. Verify CalcSums and selected key/filter behaviour from the current overload documentation; do not inherit the old title's blanket current-key restriction without checking it.
5. Implement maintained aggregates only after measuring plain SQL. Honor MaintainSiftIndex and consistency under insert/modify/delete/rollback from two writers.

## Acceptance

- SQL and temporary fixtures agree; FlowFields never become stored columns. A second connection and rollback test prove aggregate consistency. Explain EXPLAIN plans and lock duration before claiming SIFT performance.

## References

Code: `src/rt/{Table,Selection,PageRecord}.cpp`, `include/runtime/Record.h`, `src/gen/TableWriter.cpp`; fixture `test/gate/UserPersonalizationGate.cpp`. Source: `work/symbols/src/Tenant Database Tables/UserPersonalization.Table.al`. Declaration/provider ownership: 0034; page binding: 0030.

Platform: devenv-flowfields.md, properties/devenv-calcformula-property.md, methods-auto/record/record-calcsums-method.md. AL: aggregate/filter users. Predecessor: WI-890 records activation regressions from FlowField filters.

Property scope: `autocalcfield`, `calcfields`, `calcformula`, `fieldclass`.

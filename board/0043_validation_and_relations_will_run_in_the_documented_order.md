# 0043 — Validation and relations will run in the documented order

Status: open | Priority: P1 | Stage: UT | Reviewed: 2026-09-28
Depends on: 0044 record operations; 0718 xRec ownership.

## Evidence

- `Relation.cpp` evaluates conditional relations; correct validation/event order and unknown-target refusal still need reduced fixtures.

## Implementation

1. Use one validation dispatcher with explicit origin (AL assignment, Record.Validate, page edit, import). Default checks precede field triggers only where that origin's contract requires them; keep 0057 event dispatch reusable.
2. Gate default validation, table field OnValidate, page control OnValidate and platform/extension events in documented order. Include failure restoration and xRec visibility.
3. Represent relation targets, branches and filters as typed generated metadata; unknown installed targets must refuse rather than silently accept a value.
4. Implement property-specific enforcement at its documented boundary. UI-only NotBlank/MinValue rules must not be copied blindly onto all assignments or Record.Validate calls.
5. Verify ValidateTableRelation=false, lookup overrides, fieldgroups and Rename propagation with real related tables. Batch metadata/header changes after the schema is agreed.

## Acceptance

- A two-table fixture proves missing-parent refusal, conditional branches, filtered relation, opted-out validation, extension order and cascade rollback. A trace proves the field trigger never runs after rejected default validation.

## References

Code: `src/rt/Relation.cpp`, `src/rt/Table.cpp`, `include/runtime/Table.h`, `src/gen/TableWriter.cpp`.

Platform: properties/devenv-tablerelation-property.md, devenv-validatetablerelation-property.md, each validation property, table/page OnValidate triggers. AL: TableRelation declarations. Predecessor: validation-order findings; WI-781/1078/1137/1156 for xRec.

Property scope: `autoincrement`, `charallowed`, `decimalplaces`, `initvalue`, `maxvalue`, `minvalue`, `notblank`, `numeric`, `tablerelation`, `validatetablerelation`, `valuesallowed`.

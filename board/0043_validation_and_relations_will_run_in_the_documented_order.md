# 0043 — Validation and relations will run in the documented order

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

`src/rt/Relation.cpp` now parses conditional relation text and runtime validation exists. The old no-op description is obsolete. Missing related tables and trigger sequencing still need end-to-end fixtures; parsing metadata is not proof that Validate enforces it.

## Implementation for Sol

1. Gate default validation, table field OnValidate, page control OnValidate and platform/extension events in documented order. Include failure restoration and xRec visibility.
2. Represent relation targets, branches and filters as typed generated metadata; unknown installed targets must refuse rather than silently accept a value.
3. Implement property-specific enforcement at its documented boundary. UI-only NotBlank/MinValue rules must not be copied blindly onto all assignments or Record.Validate calls.
4. Verify ValidateTableRelation=false, lookup overrides, fieldgroups and Rename propagation with real related tables. Batch metadata/header changes after the schema is agreed.

## Acceptance

A two-table fixture proves missing-parent refusal, conditional branches, filtered relation, opted-out validation, extension order and cascade rollback. A trace proves the field trigger never runs after rejected default validation.

## References

Platform: properties/devenv-tablerelation-property.md, devenv-validatetablerelation-property.md, each validation property, table/page OnValidate triggers. AL: TableRelation declarations. Predecessor: validation-order findings; WI-781/1078/1137/1156 for xRec.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `autoincrement`, `charallowed`, `decimalplaces`, `initvalue`, `maxvalue`, `minvalue`, `notblank`, `numeric`, `tablerelation`, `validatetablerelation`, `valuesallowed`.

# 0043 — Validation and relations will run in the documented order

Status: open | Priority: P1 | Stage: UT | Reviewed: 2026-10-01
Depends on: 0044 record operations; 0718 xRec ownership; 0033 qualified declaration identity.

## Evidence

- `Relation.cpp` evaluates conditional relations; correct validation/event order and unknown-target refusal still need reduced fixtures.
- Own Record Link diagnostic: Clang/GCC each compile and execute 11 checks, four red. Both `RelationBranches` and `ResolveRelation` misread original `System.Environment.Company.Name` as one table with no field, ignoring correct split metadata. Split-only controls pass; original relation remains retained. `build/record-link-20261001/{RelationProbe.cpp,artifacts/relation-probe-*.json}`; no SQL/navigation proof.
- `DeclaredTargetOf`/`TargetOf` duplicate target parsing; `TableWriter.cpp::FieldProperties` also splits bare names at the first dot. Do not repair this with a third parser or Company-specific alias.

## Implementation

1. Use one validation dispatcher with explicit origin (AL assignment, Record.Validate, page edit, import). Default checks precede field triggers only where that origin's contract requires them; keep 0057 event dispatch reusable.
2. Gate default validation, table field OnValidate, page control OnValidate and platform/extension events in documented order. Include failure restoration and xRec visibility.
3. Bind relation targets once against 0033's app/namespace identity; emit typed immutable branches/filters. Runtime declaration queries and record evaluation consume the same metadata, not separate string parsers. Preserve original text for diagnostics; unknown installed targets must refuse rather than silently accept a value.
4. Implement property-specific enforcement at its documented boundary. UI-only NotBlank/MinValue rules must not be copied blindly onto all assignments or Record.Validate calls.
5. Verify ValidateTableRelation=false, lookup overrides, fieldgroups and Rename propagation with real related tables. Batch metadata/header changes after the schema is agreed.

## Acceptance

- A two-table fixture proves missing-parent refusal, conditional branches, filtered relation, opted-out validation, extension order and cascade rollback. A trace proves the field trigger never runs after rejected default validation.
- Turn the retained 11-check qualified-relation diagnostic green under both compilers. Cover quoted names containing dots, same-named namespace targets, extension branch order and ValidateTableRelation=false lookup; prove a missing/ambiguous target refuses without dropping its declaration.

## References

Code: `src/rt/Relation.cpp`, `src/rt/Table.cpp`, `include/runtime/Table.h`, `src/gen/TableWriter.cpp`.

Platform: properties/devenv-tablerelation-property.md, devenv-validatetablerelation-property.md, each validation property, table/page OnValidate triggers. AL: TableRelation declarations. Predecessor: validation-order findings; WI-781/1078/1137/1156 for xRec.

Qualified target: pinned System `src/Tenant Database Tables/RecordLink.Table.al`; BCApps main `a9ea4d84534cebba852c44bf0f841c2ea149de4e`, `src/{Apps/W1/HybridBaseDeployment/app/src/tables/ReplicationRecordLinkBuffer,Layers/W1/BaseApp/System/Permissions/UserPermissionsBuffer}.Table.al`. Earlier 1071: retain every conditional branch/filter, do not flatten; 1132: disabling validation must not erase lookup metadata.

Property scope: `autoincrement`, `charallowed`, `decimalplaces`, `initvalue`, `maxvalue`, `minvalue`, `notblank`, `numeric`, `tablerelation`, `validatetablerelation`, `valuesallowed`.

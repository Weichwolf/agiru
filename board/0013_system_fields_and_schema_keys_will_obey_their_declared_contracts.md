# 0013 — System fields and schema keys will obey their declared contracts

Status: open | Priority: P1 | Stage: UT → Clients | Reviewed: 2026-09-30
Depends on: 0033 declaring-app identity.

## Evidence

- `Storage.cpp` generates keys and system fields, but writes lack observed-rowversion predicates. Field presence is not a uniqueness/audit/concurrency guarantee.
- Integrated `Stored()` repair retains Normal fields marked Removed, as AppSourceCop AS0016/AS0002 require; AL reference restrictions are separate. Shared rule, no object-specific branch. Archived old metadata/runtime: seven red (three metadata checks, column count, three SQL columns); current probe seven green. UserPersonalization 320/320 Clang/GCC, including SQL roundtrip of obsolete Boolean values; own suites 86 cases / 66 toolchain tests green both; main 86/66 green. Frozen activation pending. Existing audit/constraint/migration gaps remain open.

## Implementation

1. Use one database-wide sequence for rowversion; include the returned version in RecordState and expose it to 0012 without widening every generated field wrapper.
2. Audit SystemId uniqueness, supplied-ID Insert overloads, created/modified stamps and database-wide monotonic SystemRowVersion. Assign versions in PostgreSQL so independent tiers cannot collide.
3. Verify key properties: Unique, MaintainSqlIndex, IncludedFields, Enabled, AutoIncrement and SqlTimestamp. Encode constraints in DDL; preserve explicit non-applicability of SQL Server clustering.
4. Return platform-owned fields from writes and preserve identity on Rename. Coordinate version checks with 0012 and company-qualified sequences with its company work.
5. Use typed generated metadata for constraints and migration comparisons; never infer a field number from display order.
   Retain obsolete Normal columns and their data through synchronization; exclude FlowFields/FlowFilters independently. Do not conflate Removed with declaring-app migration/Moved (0033).

## Acceptance

- Two connections writing different tables receive distinct increasing versions. Duplicate supplied SystemId fails, Rename retains it, audit stamps follow documented trigger order, and disabled/nonmaintained keys generate the intended DDL.

## References

Code: `src/rt/Storage.cpp`, `src/gen/TableWriter.cpp`, `include/meta/TableDef.h`.

Platform: devenv-table-system-fields.md, methods-auto/record/record-insert-boolean-boolean-method.md, key property pages and `analyzers/appsourcecop-as{0002,0016}.md`. AL: pinned `UserPersonalization.Table.al` and current-main `System Application/App/AI/src/Copilot/CopilotSettings.Table.al`. Fixture: `test/gate/UserPersonalizationGate.cpp`; own `build/user-personalization-proof/stored-negative-runtime.log`. User-settings intent adds no separate obsoletion guarantee. Predecessor 913 concerns Moved/merge identity, not permission to drop Removed columns.

Property scope: `autoincrement`, `sqldatatype`, `sqltimestamp`.

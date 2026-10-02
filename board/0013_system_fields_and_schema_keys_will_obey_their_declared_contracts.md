# 0013 — System fields and schema keys will obey their declared contracts

Status: open | Priority: P1 | Stage: UT → Clients | Reviewed: 2026-09-28
Depends on: 0033 declaring-app identity.

## Evidence

- `Storage.cpp` generates keys and system fields, but writes lack observed-rowversion predicates. Field presence is not a uniqueness/audit/concurrency guarantee.

## Implementation

1. Use one database-wide sequence for rowversion; include the returned version in RecordState and expose it to 0012 without widening every generated field wrapper.
2. Audit SystemId uniqueness, supplied-ID Insert overloads, created/modified stamps and database-wide monotonic SystemRowVersion. Assign versions in PostgreSQL so independent tiers cannot collide.
3. Verify key properties: Unique, MaintainSqlIndex, IncludedFields, Enabled, AutoIncrement and SqlTimestamp. Encode constraints in DDL; preserve explicit non-applicability of SQL Server clustering.
4. Return platform-owned fields from writes and preserve identity on Rename. Coordinate version checks with 0012 and company-qualified sequences with its company work.
5. Use typed generated metadata for constraints and migration comparisons; never infer a field number from display order.

## Acceptance

- Two connections writing different tables receive distinct increasing versions. Duplicate supplied SystemId fails, Rename retains it, audit stamps follow documented trigger order, and disabled/nonmaintained keys generate the intended DDL.

## References

Code: `src/rt/Storage.cpp`, `src/gen/TableWriter.cpp`, `include/meta/TableDef.h`.

Platform: devenv-table-system-fields.md, methods-auto/record/record-insert-boolean-boolean-method.md and key property pages. AL: table/key declarations. Predecessor: inspect SystemId/rowversion findings before changing write order.

Property scope: `autoincrement`, `sqldatatype`, `sqltimestamp`.

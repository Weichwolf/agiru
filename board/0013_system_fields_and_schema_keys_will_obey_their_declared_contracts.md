# 0013 — System fields and schema keys will obey their declared contracts

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

System-field declarations and storage support exist; the old claim that none are generated is obsolete. `Storage.cpp` builds indexes, but its write predicates only identify keys. Audit schema guarantees separately from fields being present.

## Implementation for Sol

1. Audit SystemId uniqueness, supplied-ID Insert overloads, created/modified stamps and database-wide monotonic SystemRowVersion. Assign versions in PostgreSQL so independent tiers cannot collide.
2. Verify key properties: Unique, MaintainSqlIndex, IncludedFields, Enabled, AutoIncrement and SqlTimestamp. Encode constraints in DDL; preserve explicit non-applicability of SQL Server clustering.
3. Return platform-owned fields from writes and preserve identity on Rename. Coordinate version checks with 0012 and company-qualified sequences with its company work.
4. Use typed generated metadata for constraints and migration comparisons; never infer a field number from display order.

## Acceptance

Two connections writing different tables receive distinct increasing versions. Duplicate supplied SystemId fails, Rename retains it, audit stamps follow documented trigger order, and disabled/nonmaintained keys generate the intended DDL.

## References

Platform: devenv-table-system-fields.md, methods-auto/record/record-insert-boolean-boolean-method.md and key property pages. AL: table/key declarations. Predecessor: inspect SystemId/rowversion findings before changing write order.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `autoincrement`, `sqldatatype`, `sqltimestamp`.

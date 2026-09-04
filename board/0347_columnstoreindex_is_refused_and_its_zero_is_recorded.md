Type:     task
Status:   open
Parent:   0045
Area:     gen
Source:   developer/properties/devenv-columnstoreindex-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `ColumnStoreIndex` is refused, and its zero is recorded

> Sets the fields that are added to the ColumnStore index inside SQL Server. The property creates a
> **nonclustered columnstore index** on the table. **There can be only one** on a table. Using one
> can improve the performance for **analytical queries on large tables**.

A SQL Server storage feature with no PostgreSQL equivalent in core -- the nearest thing is BRIN, which
is not the same structure and does not answer the same queries. Mapping one onto the other would be
the kind of "papered over" divergence board:0004 refuses.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ColumnStoreIndex =`: **0 declarations.**

## The IST-state

Unknown to the parser's consumers and to the schema.

## The choice

Refuse in the generator, naming the property and the table, with the reason recorded here: no
PostgreSQL equivalent, and no declaration to serve. This is the third property in this sweep whose
population is 0 (board:0327, board:0346), and all three take the same decision for the same reason.

**What would change it**: an analytical workload measured against the base table, showing the scan is
the cost. That is a measurement nobody has taken here, so the honest state is a refusal and not a
silent drop.

## Ordering

With board:0067's census.

## Gate, and its negative control

A table declaring `ColumnStoreIndex` fails to transpile.

**The negative control is the BaseApp transpiling with the refusal in place**, which proves the count
of 0 rather than assuming it.

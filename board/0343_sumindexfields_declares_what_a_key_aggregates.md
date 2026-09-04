Type:     task
Status:   open
Parent:   0045
Area:     gen, db
Source:   developer/properties/devenv-sumindexfields-property.md
Verdict:  fehlt
Class:    activation

# `SumIndexFields` declares what a key aggregates

> Specify which fields should be the "aggregation fields" in a **SumIndexField Technology (SIFT)**
> index, if applicable.
>
> ```AL
> key(<key name>; <comma-separated list of lookup fields>) {
>     SumIndexFields = <comma-separated list of aggregation fields>;
> }
> ```

SIFT is C/SIDE's answer to the FlowField: the key's fields are the grouping, the `SumIndexFields` are
the measures, and SQL Server holds an INDEXED VIEW that keeps the running totals. A `Sum` FlowField
whose filters match a SIFT key reads one row of that view instead of aggregating the table.

**PostgreSQL has no indexed view**, and that is this item's whole question. The candidates are a
materialised view refreshed on write (wrong: not transactional with the writer), a summary table
maintained by trigger (what SIFT actually is, and what the write cost buys), or nothing at all -- let
the aggregate run against the base table and its index.

**Do not decide it here and do not decide it by preference.** board:0012 named PostgreSQL's missing
dirty read and measured the divergence rather than mapping it away; this is the same shape and gets
the same treatment: implement the aggregate against the base table first, measure a `Sum` over the
largest CRONUS table against the same `SUM` from `psql`, and only then decide whether a maintained
summary is worth its write cost. That is CLAUDE.md's benchmark rule applied to the one place where BC
bought read speed with write speed.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`SumIndexFields =`: **762 declarations** across 3 272 declared keys -- so roughly one key in four
carries aggregates.

## The IST-state

`include/meta/TableDef.h:98` -- `KeyDef` carries `name`, `fields` and `clustered`, and nothing else.
No SIFT anywhere. The schema writer is `src/rt/Storage.cpp:94` -- `CreateTable` emits one column per
field, a `PRIMARY KEY` from `keys[0]`, and one plain `CREATE INDEX` per further key -- and it has no
notion of an aggregate.

## The choice

The declaration lands in `KeyDef` as a span of `FieldNo` regardless of how it is executed, because
the metadata is what board:0340's planner needs in order to ask "is there a key that matches these
filters" at all. **Storing the declaration is separable from maintaining a structure for it**, and
this item is the first half.

## Ordering

Behind board:0340, which is the only consumer. The schema half waits on its measurement.

## Gate, and its negative control

A `Sum` FlowField whose filters match a key's fields returns the same number as the same `SUM` from
`psql`, and the ratio between the two is recorded.

**The negative control is the ratio** -- a gate that only checks the NUMBER cannot tell a one-row
read from a table scan, and this property exists for nothing but that difference.

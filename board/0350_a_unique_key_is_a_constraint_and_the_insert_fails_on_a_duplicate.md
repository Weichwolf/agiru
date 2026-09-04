Type:     task
Status:   open
Parent:   0045
Area:     gen, db
Source:   developer/properties/devenv-unique-property.md
Verdict:  fehlt
Class:    activation

# A `Unique` key is a constraint, and a duplicate insert fails

> **Version**: Available or changed with **runtime version 3.0**.
>
> Sets a value that indicates whether a SQL Server unique constraint that corresponds to the key
> should be created. **The default is false.**
>
> A unique index ensures that records in a table do not have identical field values. With a unique
> index, **when a table is validated**, values of the field that makes up the key are checked for
> uniqueness. If the table includes records with duplicate values, the validation fails.
>
> **NOTE: The `Unique` property cannot be used in table extension objects.**

This is the one property in the key theme that is a CONSTRAINT rather than a performance declaration.
`MaintainSqlIndex`, `MaintainSiftIndex`, `Clustered` and `IncludedFields` all decide what the database
builds; this one decides what it refuses.

**And PostgreSQL has it natively**, which makes it the cheapest item in the theme: `UNIQUE` on the
index and the error comes back from the database. The only work is turning that error into AL's --
which is board:0055's, and a `unique_violation` from libpq is not a message a BC user has ever seen.

`tableextension` may not declare it, so that is a `static_assert`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Unique =`: **13 declarations.** Small, and every one of them is a rule somebody relied on -- an email
that must not repeat, a code that must be one row.

## The IST-state

`KeyDef` at `include/meta/TableDef.h:98` carries `name`, `fields`, `clustered`.

**The primary key IS already unique**: `src/rt/Storage.cpp:99` emits `PRIMARY KEY (...)` from
`keys[0]`, so PostgreSQL enforces it. What is missing is exactly the 13 secondary keys that declare
the property -- `src/rt/Storage.cpp:112` builds every one of them as a plain `CREATE INDEX`, with no
`UNIQUE`.

## The choice

One bit on `KeyDef` and `UNIQUE` in the emitted index. The refusal comes from PostgreSQL and is
translated into an AL `Error` naming the table and the key's fields, the way BC words it.

## Ordering

One word in the statement the schema writer already emits. Ahead of most of this theme, because it
changes what the database ACCEPTS
and the others only change what it costs.

## Gate, and its negative control

Inserting a second row with the same values on a `Unique` key raises; the same insert on a key without
the property succeeds.

**The negative control is the second insert on a non-unique key** -- an implementation that makes
every declared key unique passes the first half and refuses legitimate rows on 3 259 keys.

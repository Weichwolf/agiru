Type:     task
Status:   open
Parent:   0045
Area:     gen, db, rt
Source:   developer/properties/devenv-includedfields-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `IncludedFields` are carried by the index and ignored by `SetCurrentKey`

> **Version**: Available or changed with **runtime version 8.0**.
>
> Sets the fields that are included as **non-key columns** in the index on SQL Server.
>
> You **can't** use this property on primary keys or clustered secondary keys (`Clustered = true`).
>
> **Fields that are part of an `IncludedFields` definition are not used when searching for a matching
> key with `Record.SetCurrentKey`.**

Two statements, and the second is a RUNTIME rule rather than a schema one. A covering index carries
extra columns so a read never touches the heap -- and the runtime must not then treat those columns
as part of the sort order. `SetCurrentKey` matches against the key's FIELDS and ignores its included
ones, so a key `(A, B)` including `C` is a match for `SetCurrentKey(A, B)` and never for
`SetCurrentKey(A, B, C)`.

That is exactly the sort of rule that is invisible until it is wrong: an implementation that appends
the included fields to the key's field list finds a "matching key" that sorts differently, and the
rows come back in a different order with no error.

The two prohibitions are `static_assert`s: not on a primary key, not on a clustered secondary key.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`IncludedFields =`: **789 declarations** across 3 272 keys -- **24 %** of declared keys carry covering
columns. It is not a rarity.

## The IST-state

`KeyDef` at `include/meta/TableDef.h:98` carries `name`, `fields`, `clustered`. No included fields.
The schema writer at `src/rt/Storage.cpp:112` emits each secondary index from the key's field list
alone, so the 789 keys that declare covering columns get an index without them and every read over
one of those keys goes back to the heap. `SetCurrentKey`'s key matching is board:0044's.

## The choice

A second span of `FieldNo` on `KeyDef`, kept SEPARATE from `fields` and never concatenated -- the
separation IS the runtime rule, and one span with a length marker would be the shape that invites the
bug the page warns about. The schema writer emits them as `INCLUDE (...)`, which PostgreSQL has had
since 11.

## Ordering

The index half is an `INCLUDE (...)` clause on the statement `src/rt/Storage.cpp:112` already emits.
The matching rule goes with board:0044 -- the two halves
land together or the matching is written against a metadata shape that does not exist.

## Gate, and its negative control

`SetCurrentKey(A, B)` selects the key `(A, B) INCLUDE (C)`; `SetCurrentKey(A, B, C)` does not select
it, and falls through to whatever else matches.

**The negative control is `SetCurrentKey(A, B, C)`** -- an implementation that concatenates the two
spans passes the first assertion and picks the wrong key on the second, which is a silently different
row order.

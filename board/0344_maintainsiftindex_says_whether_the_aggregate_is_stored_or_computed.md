Type:     task
Status:   open
Parent:   0045
Area:     gen, db
Source:   developer/properties/devenv-maintainsiftindex-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `MaintainSiftIndex` says whether the aggregate is stored or computed

> Sets the value to determine whether SIFT structures (indexed views) should be created in SQL Server
> to support the corresponding `SumIndexFields` part of the key. **The default is true.**
>
> If the SIFT structures exist, summing the fields is faster, especially for large sets of records,
> **but modifications to the table are slower because the SIFT structures must also be maintained**.
>
> In situations where `SumIndexFields` must be created on a key to enable FlowField calculations, but
> the calculations are performed **infrequently or on small sets of data**, you can disable this
> property to prevent slow modifications to the table.

The page states the trade in full and it is the trade board:0343 has to measure: read speed bought
with write speed, per key, decided by the developer. **70 keys in the BaseApp say the read is not
worth it.**

That is a fact worth having before building anything: Microsoft's own developers turned SIFT OFF on
70 of the 762 keys that declare aggregates -- 9 % -- which means the structure is not free even where
SQL Server has native support for it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`MaintainSiftIndex =`: **70 declarations**, all necessarily `false` since `true` is the default.

## The IST-state

`KeyDef` carries no aggregate declaration at all (`include/meta/TableDef.h:98`), so there is nothing
to maintain and nothing to switch off.

## The choice

One bit on `KeyDef` beside board:0343's field list. It is cheap and it has to exist before any
maintained structure does, because a maintained structure that ignored it would pay the write cost on
70 keys whose author declined it.

**If board:0343's measurement says PostgreSQL computes the aggregate from the base table, this
property becomes a no-op** -- and then it is recorded as known-and-ignored the way board:0333 is,
with the measurement as the reason. That is a legitimate outcome and it still needs the bit, because
the decision has to be provable from the metadata rather than from a commit message.

## Ordering

With board:0343.

## Gate, and its negative control

A key declaring `MaintainSiftIndex = false` creates no maintained aggregate structure; the FlowField
over it still returns the right number.

**The negative control is the number** -- switching the structure off must not change the answer, only
where it comes from, and a gate that checks only the structure cannot see a wrong total.

Type:     task
Status:   open
Parent:   0045
Area:     gen, db
Source:   developer/properties/devenv-sqlindex-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `SqlIndex` lets the index differ from the key, and nobody declares it

> Sets the actual fields that are used in the corresponding index on SQL Server. The fields in the
> SQL index can **differ in number** from the fields defined in the key -- there can be fewer or more
> -- and can be **arranged in a different order**.
>
> If you use this property on a key that is **not** the primary key, the index contains exactly the
> fields you specify and **will not be a unique index**. A unique index will only be created if it
> contains all of the primary key fields.
>
> If you use this property for the **primary key**, it must include all the fields defined in the
> primary key. You can add extra fields and rearrange them.

So the AL key and the physical index come apart: the key decides the SORT the runtime offers, the
property decides the INDEX the database builds. The page's own example makes a primary key
`(MyField1, MyField2)` clustered as `(MyField2, MyField1)`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`SqlIndex =`: **0 declarations.** Not one key in the BaseApp.

## The IST-state

`KeyDef` at `include/meta/TableDef.h:98` has no such member. The schema writer
(`src/rt/Storage.cpp:112`) builds each secondary index from the key's own field list in the key's own
order, which is the behaviour this property exists to override.

## The choice

**Refuse it, as board:0327 refuses `SignDisplacement`, and for the same arithmetic**: 0 declarations
means refusing costs nothing today and a translation error is the notification if that ever changes.
Building a key-versus-index divergence for zero call sites is a code path that can only be wrong.

The two constraints on the page are written into the refusal's item anyway, because they are what a
future implementation has to hold: an index over a non-primary key is never unique unless it covers
the primary key, and a primary key's index must contain every primary-key field.

## Ordering

With board:0067's property census. No runtime work.

## Gate, and its negative control

A table declaring `SqlIndex` fails to transpile with a message naming the property and the key.

**The negative control is the whole BaseApp transpiling** -- if it does not, the population was
measured wrong, and 0 is exactly the count that has to be checked rather than trusted.

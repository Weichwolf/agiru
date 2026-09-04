Type:     task
Status:   open
Parent:   0013
Area:     gen, rt
Source:   developer/properties/devenv-sqltimestamp-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `SqlTimestamp` exposes the rowversion as a declared field

> Specifies a field to be a timestamp field. **The default is false.**
>
> **Each table includes a hidden timestamp field.** The timestamp field contains **row version
> numbers for records as maintained in SQL Server**. **This property exposes the timestamp field in
> the table object, and enables you to write code against it.** This property only applies to fields
> that have the data type **BigInteger**.

So the rowversion always exists and this property gives it a NAME an AL procedure can use. It is not
a second column -- it is a declared alias for the one board:0013 owns.

**And board:0013 says the hard part**: the rowversion is monotonic across the DATABASE and not per
table (`@@DBTS`), and one that is merely present is worse than none. A field that exposed a per-table
counter under this property would hand AL code a number that looks like a rowversion and orders
wrongly across tables -- which is exactly the failure board:0013 was filed to prevent, arriving
through a different door.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`SqlTimestamp =`: **2 declarations.**

Two, because the platform's own `SystemRowVersion` covers the same ground for everyone else. The
population makes this a small item and not a free one: the two fields that declare it are fields AL
code reads.

## The IST-state

`include/meta/TableDef.h` -- `kSystemFieldCount = 5`, and `SystemRowVersion` is NOT among them
(board:0013's own finding). So the underlying value does not exist yet either, and this property has
nothing to alias.

## The choice

The declared field becomes a read-only alias of the table's rowversion: same storage, same value, an
AL name. The `BigInteger`-only rule is a `static_assert`.

**Not a separate column.** Two columns holding the same rowversion is two values that can disagree,
and the one AL reads would be the one that is not the row's.

## Ordering

**Strictly behind board:0013.** There is nothing to expose until the rowversion is a real,
database-wide, monotonic value.

## Gate, and its negative control

The declared field's value equals the row's `SystemRowVersion` and increases when the row is modified.

**The negative control is a modification in ANOTHER table** -- the rowversion is database-wide, so a
per-table counter passes "increases when modified" and fails this, and it is the failure board:0013
names.

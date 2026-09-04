Type:     task
Status:   open
Parent:   0013
Area:     rt, gen, db
Source:   developer/devenv-table-system-fields.md
Verdict:  teilweise
Class:    silent-wrong-data

# The six system fields have numbers, and the audit fields are stamped between triggers

board:0013 is "every table carries the system fields" and this page is its specification. It gives the
field numbers, the stamping point relative to the triggers, and two exceptions.

## The fields and their numbers

| field | type | number |
|---|---|---|
| `SystemId` | **Guid** | **2000000000** |
| `SystemCreatedAt` | DateTime | 2000000001 |
| `SystemCreatedBy` | **Guid** | 2000000002 |
| `SystemModifiedAt` | DateTime | 2000000003 |
| `SystemModifiedBy` | Guid | 2000000004 |
| `SystemRowVersion` | the timestamp | -- |

> **"System fields are assigned numbers in the range 2000000000-2147483647. This range is RESERVED.
> You'll get a DESIGN-TIME ERROR if you give a field a number in this range."**

**That reserved range is a `static_assert`**, and it is one line: a declared field number ≥ 2 000 000
000 is a translation error.

**`include/meta/TableDef.h` records `kSystemFieldCount = 5` and board:0013 already found that
`SystemRowVersion` is not among them.** This page confirms it is a sixth thing with different rules --
it is the SQL rowversion, it is read-only from AL, and it is not in the numbered range the other five
occupy.

## `SystemId` -- five rules

> - **"ALL records must have a value."**
> - You may assign your own on insert; otherwise the platform generates one.
> - **"Once the `SystemId` has been set, IT CAN'T BE CHANGED."**
> - **"There's ALWAYS A UNIQUE SECONDARY KEY on the `SystemId` field."**
> - `TableRelation = Customer.SystemId` is legal -- **a relation may target it** (board:0331).

**The always-unique secondary key is a schema fact**, and board:0350 records that
`src/rt/Storage.cpp:112` emits every secondary key as a plain `CREATE INDEX` with no `UNIQUE`. So this
index is a second thing the schema writer owes, independent of any declared key.

**And `Insert(Boolean, Boolean)` is the overload that carries the rule** -- CLAUDE.md names exactly
this: "the SystemId rule lives in `record-insert-boolean-boolean-method.md`, not in the file next to
it."

## The audit fields are stamped at a point BETWEEN the triggers

> The platform assigns values:
> - **after all `OnBeforeInsert`/`OnBeforeModify` triggers run**,
> - **after the `OnInsert`/`OnModify` triggers run**,
> - **before all `OnAfterInsert`/`OnAfterModify` triggers run.**
>
> **"You CAN assign the values, but the values written to the database are ALWAYS PROVIDED BY THE
> PLATFORM."**

**So the stamp happens after the table's own trigger and before the subscribers'** -- which means an
`OnInsert` trigger reads blank audit fields and an `OnAfterInsert` subscriber reads real ones. That is
a precise ordering constraint on board:0057's event dispatch and board:0228-0233's triggers, and it is
not derivable from either.

> - **"When a new record is created, BEFORE calling `Insert`, the audit fields are given BLANK GUIDs
>   and BLANK DATES."**
> - On first insert, `SystemCreatedBy` and `SystemModifiedBy` get **the same value**; so do the two
>   timestamps.
> - `SystemCreatedBy` and `SystemCreatedAt` **never change after that**.
> - **"If a record is copied into a TEMPORARY table, the audit values are copied as well. The values
>   AREN'T CHANGED by the server when calling modify or insert."**

**The temporary-table exception is board:0032's**: a temporary record does not get stamped, so
`RuntimeInsert`'s stamping must be conditional on the record not being temporary.

> The platform **won't** populate them when **copying a company** or **synchronizing the table schema**,
> and **"audit fields can't be imported with configuration packages."**

## The rowversion is database-wide, and two methods prove it

> `SystemRowVersion` -- **"you CAN'T WRITE to the field."**
>
> | method | |
> |---|---|
> | `Database.LastUsedRowVersion` | **"does the same as `@@DBTS`"** |
> | `Database.MinimumActiveRowVersion` | **"the lowest rowversion of ANY UNCOMMITTED ROWS. Rows with a lower timestamp are GUARANTEED TO BE COMMITTED. If there are no active transactions, the value is `LastUsedRowVersion + 1`."** |

**`MinimumActiveRowVersion` is the sentence board:0013 needs**: it makes the rowversion a
database-wide, monotonic, transaction-aware watermark, not a per-row counter. PostgreSQL has no
`@@DBTS`; a sequence gives monotonicity and not the uncommitted-row guarantee, and
`pg_snapshot_xmin(pg_current_snapshot())` gives that guarantee over transaction ids rather than
rowversions. **The divergence is named here and board:0013 owns it.**

## Population

The six fields are on all 1 609 tables by construction; there is nothing to count.

## The IST-state

`include/meta/TableDef.h` -- `kSystemFieldCount = 5`, `SystemRowVersion` not among them (board:0013).
`src/rt/Table.cpp` -- `StampInserted` and `StampModified` exist and write `SystemCreatedAt`,
`SystemCreatedBy` and the modified pair; `RuntimeInsert` calls `StampInserted` at
`src/rt/Table.cpp:310`. **So four of the six are stamped.** Whether the stamp happens at the
documented point relative to the triggers is this item's check -- `include/runtime/Table.h:353` calls
`OnInsert` and then `Insert()`, which stamps, so the ordering may already be right by construction.

## The choice

`SystemRowVersion` as a sixth system field with its own rules; the reserved-range `static_assert`; the
unique index on `SystemId` in `src/rt/Storage.cpp`; the stamping made conditional on non-temporary;
and the two `Database` methods mapped onto PostgreSQL with the divergence recorded.

## Ordering

Inside board:0013. The `static_assert` and the unique index are independent and cheap; the rowversion
is the hard half.

## Gate, and its negative control

An `OnInsert` trigger reads a blank `SystemCreatedAt`; an `OnAfterInsert` subscriber reads a real one;
a second `Modify` changes `SystemModifiedAt` and not `SystemCreatedAt`; a field declared with number
2000000005 fails to transpile.

**The negative control is the `OnInsert` trigger's blank value** -- an implementation that stamps
before the triggers gives it a real timestamp, which looks more correct and is not what BC does; and
every gate that only checks the final row passes.

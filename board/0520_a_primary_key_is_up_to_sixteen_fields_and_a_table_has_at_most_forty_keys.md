Type:     task
Status:   open
Parent:   0045
Area:     gen, db
Source:   developer/devenv-table-keys.md
Verdict:  teilweise
Class:    silent-wrong-data

# A primary key is up to sixteen fields, and a table has at most forty keys

board:0045 is "a key is an index and a read is a cursor" and CLAUDE.md counts the population: **1 609
tables declare 3 272 keys; `Sales Line` alone has 17.** This page is the specification, and it carries
four numeric limits that are `static_assert`s and one behaviour the schema writer is currently getting
wrong.

## The limits, and every one is decidable from the declaration

> - **"The primary key is composed of UP TO 16 FIELDS."**
> - **"In the development environment it's technically possible to create a primary key based on up to
>   20 fields. However, because of SQL Server limitations, ONLY THE FIRST 16 ARE USED."**
> - **"UP TO 40 KEYS can be associated with a table."**
> - **"The first key defined in a table object is the primary key"**, and there can be only one.

**The 20-versus-16 clause is a trap, not a limit.** A table declaring 18 primary-key fields COMPILES in
AL and silently uses 16 -- so two records differing only in fields 17 and 18 collide. board:0081 is
"every documented limit is a `static_assert`", and this is the one where the assert must be at **16**
even though AL accepts 20: **refusing what BC silently truncates is the only way the truncation cannot
happen.** That is a deviation, it is stricter, and it is recorded.

## What the schema writer gets wrong today

> **"When you define a secondary key and mark it as ENABLED, an index is automatically maintained."**
>
> **"A secondary key CAN BE DISABLED so that it DOESN'T OCCUPY DATABASE SPACE or use time during
> updates to maintain its index."**

board:0402 found `Enabled` applies to a table key and board:0345 found `src/rt/Storage.cpp:112`
emits an index for **every** key unconditionally. **This page is the confirmation**: a disabled key is
an index that must not exist, alongside board:0345's 158 `MaintainSqlIndex = false` keys.

> **"There's ALWAYS a unique secondary key on the `SystemId` field."**

board:0511 records the same. Two pages, one obligation the schema writer does not meet.

> A unique key: **"when the table is validated, the key value is checked for uniqueness"**, and
> **"unlike primary keys, it's possible to define MULTIPLE unique secondary keys."** The `Unique`
> property **"isn't supported in table extension objects"** (board:0350).

## Included fields exist to BYPASS the field limit

> "Using included fields lets you create indexes that cover more queries, and **LETS YOU BYPASS THE
> MAXIMUM NUMBER OF FIELDS IN A KEY.**"
>
> "The performance improves because the query optimizer can locate all the column values within the
> index, and **doesn't access table or clustered index data**, which results in fewer disk I/O
> operations."

board:0351 filed `IncludedFields` at 789 declarations and recorded that `SetCurrentKey` must ignore
them. **This page adds WHY they exist** -- to carry more columns than a key may have -- which confirms
that concatenating them into the key's field list would both break `SetCurrentKey` and exceed the
limit.

## Columnstore is documented as the SIFT replacement

> "You can use a non-clustered columnstore index to efficiently run real-time operational analytics
> **WITHOUT THE NEED TO DEFINE SIFT INDEXES UP FRONT (and without the LOCKING ISSUES that SIFT indexes
> sometimes impose on the system).** Whenever you would normally add a SIFT key ... use a
> non-clustered columnstore key instead."
>
> Two SIFT keys replaced by one: `ColumnStoreIndex = WareHouseId,Color,ItemId,Size,OnStock`.

**Microsoft's own advice is to stop using SIFT**, and it says SIFT causes locking problems. board:0343
left open whether agiru maintains SIFT structures at all; **this page is evidence for the answer NO** --
and board:0347 refuses `ColumnStoreIndex` on its zero population, so agiru would have neither. That
combination is defensible only if the base-table aggregate is fast enough, which is board:0343's
measurement, and this page raises its priority.

## Key modifications are a migration contract

> Don't delete primary keys · don't add or remove primary-key fields or change their order · don't
> change properties of existing primary keys · **don't add more unique keys** · **don't add more
> clustered keys** · don't add keys that are fields of the base table.

Six rules for a NEW VERSION of an extension. They are board:0070's upgrade concern and they are
checkable between two translations of the same app -- which no item owns yet, and which is named here.

## Table-extension keys have three rules

> - keys in a table extension may include fields from the base table **and** the extension,
> - **"a single key CAN'T include fields from BOTH"**,
> - not possible to key fields defined in **another** table extension,
> - the same key NAME may be reused unless the key contains base-table fields.

board:0033 merges extensions at translation time, so all three are `static_assert`s over the merged
result.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

CLAUDE.md: **3 272 keys across 1 609 tables.** board:0348: `Clustered` **4 565**. board:0351:
`IncludedFields` **789**. board:0345: `MaintainSqlIndex` **158**. board:0350: `Unique` **13**.

## The IST-state

`include/meta/TableDef.h:98` -- `KeyDef` carries `name`, `fields`, `clustered`.
`src/rt/Storage.cpp:99` emits `PRIMARY KEY` from `keys[0]`; `:112` emits a plain `CREATE INDEX` for
every further key. **So the primary key is right and every secondary key is unconditional.**

## The choice

Four `static_assert`s -- 16 primary-key fields, 40 keys per table, one primary key, the three
extension rules -- emitted beside every table, which is CLAUDE.md's "the transpiler EMITS them beside
every object".

The schema writer reads `enabled`, `unique` and `MaintainSqlIndex` before emitting an index, and emits
the `SystemId` unique index unconditionally.

## Ordering

board:0045's core. The `static_assert`s are independent and cheap; the index conditions go with
board:0345, board:0350 and board:0402.

## Gate, and its negative control

A table declaring 17 primary-key fields fails to transpile; a table with 41 keys fails; a disabled key
produces no index; `SystemId` has a unique index.

**The negative control is the 17-field primary key** -- AL accepts it and BC uses 16, so an
implementation that follows BC produces a table where two distinct records collide. The gate asserts a
translation FAILURE, which is the deviation this item takes deliberately.

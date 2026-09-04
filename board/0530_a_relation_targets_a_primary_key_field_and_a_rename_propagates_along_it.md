Type:     task
Status:   open
Parent:   0331
Area:     rt, gen, db
Source:   developer/devenv-set-relationships-between-tables.md
Verdict:  fehlt
Class:    activation

# A relation targets a primary-key field, and a rename propagates along it

board:0331 filed `TableRelation` as a grammar. **This page adds two rules the property page does not
state**, and one of them is a whole runtime mechanism.

## The target must be part of the primary key

> **"NOTE: You can define a relationship ONLY TO A FIELD THAT IS A MEMBER OF THE PRIMARY KEY GROUP."**

**So `TableRelation = Customer.Name` is illegal** and `TableRelation = Customer."No."` is legal, because
`No.` is the primary key. That is decidable from two declarations -- the relation's target field and
the target table's first key -- so it is a `static_assert`, and it is the cheapest possible guard
against a relation that would need a non-unique lookup.

**And it explains why board:0331's grammar has a `[.<FieldName>]` at all**: omitting it means the whole
primary key.

**One apparent conflict, recorded**: board:0511 quotes the system-fields page saying
`TableRelation = Customer.SystemId` is legal, and `SystemId` is NOT the primary key -- it has a unique
secondary key. So either the rule means "a field with a unique index" or `SystemId` is a documented
exception. **Two pages, two readings; the AL source decides**, and the answer changes whether the
assertion checks `keys[0]` or "any unique key".

## A rename propagates along the relation

> "You can use relationships to: **validate data entries**, **perform lookup functions**, and
> **PROPAGATE CHANGES AUTOMATICALLY from one table to other tables.**"
>
> "if you change one of the currency codes in the **Currency Code** table, then **the change is
> AUTOMATICALLY PROPAGATED to all tables that refer to this code.**"

**That is what `Rename` does, and it is the third thing a relation is for.** board:0331 names two --
the lookup and the validation -- and this page names the third: renaming a record rewrites every
foreign key pointing at it.

**`include/runtime/Table.h:1141` records `Rename` as a variadic refusal citing board:0035.** So the
mechanism is declared and absent, and this page is what says how much is behind it: **a rename is a
cascade over every table with a relation to the renamed one**, which requires the reverse index of
board:0331's 40 221 relations -- for each table, which other tables point at it.

**That reverse index is `constexpr` and built by the generator**, since every relation is a
declaration. It is the same shape board:0512's dispatch table has, and it is the thing that makes
`Rename` possible at all without the runtime knowing any AL object.

**And it explains board:0512's `OnBeforeRenameEvent` / `OnAfterRenameEvent` signature carrying `xRec`**:
the old primary key is what the cascade searches for.

## The relation is only for validation, lookup and propagation

The page's `<TableFilter>` grammar is narrower than board:0331's -- **`CONST` and `FILTER` only, no
`FIELD`** -- which matches board:0475's finding that the term grammar has two subsets. board:0331 reads
the full six-shape form from the property page; this page shows the two-shape form. **Both appear in
the documentation for the same property**, and board:0331 already takes the six-shape reading, which is
the superset and therefore safe.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0331: `TableRelation` **40 221**. board:0332: `ValidateTableRelation` **2 240**.

**The rename cascade's cost is 40 221 relations' worth of reverse edges**, and how many tables point at
the busiest one -- `Customer`, `Item`, `G/L Account` -- is the number that decides whether a rename is
feasible. That count belongs to this item and is its first task.

## The IST-state

`include/runtime/Table.h:1141` -- `Rename` is a variadic refusal (board:0035).
`src/rt/Table.cpp:350` -- `CheckRelation` is an empty body (board:0331). `include/meta/TableDef.h:67`
-- `FieldDef` carries no relation.

## The choice

A `constexpr` reverse-relation index per table, emitted by the generator: for each table, the
`{ table, field }` pairs that reference it. `Rename` walks it and issues one `UPDATE` per referencing
table, inside the caller's transaction.

**One `UPDATE` per table, not per row** -- the cascade is a set operation and board:0045's row counts
make a row loop the process.

The primary-key-target rule is a `static_assert` once the `SystemId` question above is settled.

## Ordering

Behind board:0331's relation metadata, which supplies the edges. Ahead of board:0512's rename events,
which fire around the cascade.

## Gate, and its negative control

Renaming a `Currency Code` record rewrites the code in every table declaring a relation to it, in one
transaction; a `TableRelation` to a non-key field fails to transpile.

**The negative control is a table that does NOT declare the relation but holds the same value** -- it
must be untouched. An implementation that rewrites by column NAME rather than by declared relation
corrupts unrelated tables, and every gate that only checks the related ones passes.

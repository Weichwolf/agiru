Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/properties/devenv-autosplitkey-property.md
Verdict:  fehlt
Class:    activation

# `AutoSplitKey` puts a new line between two existing ones

> Sets whether a key is automatically created for a new record placed between the current record and
> the previous record. Applies to: **Request Page**, **Page**. **The default is false.**
>
> To set this to true, the following conditions must be met: **the current key must be the primary
> key**, and **the last field in the primary key must be Integer, BigInteger, GUID or Decimal**.
>
> When enabled, a value is automatically calculated for the last field of the primary key when a new
> record is inserted between two existing records. **The new key value is set to a value halfway
> between the keys of the surrounding records.**
>
> **Negative key values** (2025 release wave 1 and later): if a new row is inserted before the first
> record in the list, **a negative key value may be generated** so that the new record sorts before
> all existing records. Both 0 and negative values are allowed for numeric key fields, though
> `AutoSplitKey` doesn't currently generate 0.

This is how a user inserts a line into the middle of a sales document. `Line No.` runs 10000, 20000,
30000 and the new line between the first two gets 15000 -- BC's document lines are numbered in
ten-thousands for exactly this reason.

**The halving runs out**, and the page does not say what happens then: between 10000 and 10001 there
is no integer halfway point. BC's answer is a renumbering of the lines, and that is not on this page
-- it is behaviour the AL source or the user documentation has to supply, and this item looks it up
rather than inventing it.

**The negative half is dated and current.** "Doesn't currently generate 0" is a statement about the
present implementation and not a guarantee, so it is recorded as observed behaviour rather than built
as a rule.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AutoSplitKey =`: **622 declarations** -- every document line subform in the BaseApp.

## The IST-state

Pages carry no properties beyond `SourceTable` (`src/gen/PageWriter.cpp`), so nothing.

## The choice

One bit on the page, and the split runs where the page inserts a new record -- board:0030's
`OnNewRecord` path, not the table's `Insert`. The table knows nothing about "the previous record in
the list"; only the page's cursor does, which is why this property is on a page and not on a table.

Both preconditions are `static_assert`s: the current key is the primary key, and its last field is one
of the four types.

## Ordering

Inside board:0030, with the page's insert path. It cannot precede a page that can insert.

## Gate, and its negative control

On a list over lines 10000 and 20000, inserting between them yields 15000; inserting before the first
yields a value below 10000.

**The negative control is the exhausted range** -- lines 10000 and 10001 with an insert between them.
An implementation that computes `(a + b) / 2` in integers returns 10000, which is a duplicate primary
key, and only a gate that tries it sees the failure.

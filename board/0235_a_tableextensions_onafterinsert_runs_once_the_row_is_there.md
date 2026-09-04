Type:     task
Status:   open
Parent:   0029
Area:     rt, gen
Source:   developer/triggers-auto/tableextension/devenv-onafterinsert-tableextension-trigger.md
Verdict:  fehlt
Class:    activation

# A tableextension's `OnAfterInsert` runs once the row is there, and can still undo it

`OnAfterInsert` runs after the default insert behaviour has put the row in. It is the far side of
the bracket 0234 opens, and the two differ in one way that matters: **at `OnAfterInsert` the row
exists**, so the trigger can read what the platform assigned -- the `SystemId`, the audit fields, an
`AutoIncrement` number (board:0013).

An error in it still undoes the insert, because the whole write is inside the caller's transaction
boundary -- not because the trigger is before the row.

## The IST-state

`include/runtime/Table.h:353` runs `OnInsert` and then `Insert()`, and returns. Nothing runs after
the row.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnAfterInsert()` on a tableextension: **72 declarations.**

## The choice

One line after `Insert()` in `Insert(RunTrigger)`, under the same `if constexpr` guard, with the
per-extension member naming 0234 describes.

**The order against the platform-assigned values is the whole point**: `Insert()` writes the
`SystemId` and the audit fields into the record (`Table.h:318`'s `\note` explains why `Insert` is
not `const`), so `OnAfterInsert` must run after that assignment and not merely after the SQL.

## Ordering

With 0234; they are two call sites in one method and share the merge work.

## Gate, and its negative control

A tableextension whose `OnAfterInsert` reads `Rec.SystemId` and writes it into another field: the
value is the one the platform generated, not the blank GUID.

**The negative control is the blank GUID.** A runtime that runs the trigger before the platform
fills the record passes any assertion that only checks the trigger RAN.

Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/table/devenv-onafterinsertevent-table-trigger.md
Verdict:  fehlt
Class:    activation

# `OnAfterInsertEvent` fires once the row and the system fields are written

```al
local procedure MyProcedure(var Rec: Record; RunTrigger: Boolean)
```

Same signature as 0244 and the opposite moment: "Executed after a record is inserted in a table."
`Rec` carries what the platform assigned -- the `SystemId`, `SystemCreatedAt`, `SystemCreatedBy`
and any `AutoIncrement` value (board:0013) -- which is the difference that makes subscribers choose
this event over the before one.

It is step 5 of board:0029's five.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**194 subscriptions** to `'OnAfterInsertEvent'`.

## The IST-state

`include/runtime/Table.h:353` returns immediately after `Insert()`. Nothing is raised.

## The choice

The raise sits after `Insert()` in `Insert(RunTrigger)`, unconditionally, with the record by
reference and the flag.

**After the platform's field assignment, not merely after the SQL.** `Insert()` writes the system
fields into the RECORD as well as the row (`Table.h:318`'s `\note`), and a raise between the two
would hand every one of the 194 subscribers a blank `SystemId`.

## Ordering

With 0244; one method, two call sites.

## Gate, and its negative control

A subscriber that copies `Rec.SystemId` into a log table: the log holds the generated id.

**The negative control is the blank GUID** -- it passes any assertion that only checks the
subscriber ran, which is what a first gate would check.

Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/table/devenv-onaftermodifyevent-table-trigger.md
Verdict:  fehlt
Class:    activation

# `OnAfterModifyEvent` fires once the update landed, and `xRec` still holds what it replaced

```al
local procedure MyProcedure(var Rec: Record; var xRec: Record; RunTrigger: Boolean)
```

The same three parameters as 0246 at the opposite end: the row is written, `Rec` carries the
platform's `SystemModifiedAt` and the advanced `SystemRowVersion` (board:0013), and `xRec` still
holds the values the row had. It is step 5 of board:0029's order around `Modify`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**101 subscriptions** to `'OnAfterModifyEvent'` -- more than the 64 on its before-partner, because
the commonest use is to react to a change rather than to police it.

## The IST-state

`include/runtime/Table.h:381` returns straight after `Modify()`.

## The choice

The raise sits after `Modify()`, unconditionally, inside the same `xRec` scope 0246 opens -- which
is why the scope has to cover the whole method rather than just the pre-write half.

## Ordering

With 0246, behind board:0042.

## Gate, and its negative control

A subscriber that writes `xRec.Field` and `Rec.Field` into a log: the log holds the old value and
the new one.

**The negative control is the pair being equal.** A runtime that rebuilds `xRec` from the record
after the write logs the same value twice and passes any assertion that only checks the subscriber
ran.

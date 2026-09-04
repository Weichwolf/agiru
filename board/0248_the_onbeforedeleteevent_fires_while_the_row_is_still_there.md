Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/table/devenv-onbeforedeleteevent-table-trigger.md
Verdict:  fehlt
Class:    activation

# `OnBeforeDeleteEvent` fires while the row is still there, so a subscriber can refuse

```al
local procedure MyProcedure(var Rec: Record; RunTrigger: Boolean)
```

No `xRec` -- a delete has one image. The row still exists when this fires, which is what lets a
subscriber count dependents and RAISE to cancel the delete. Step 1 of board:0029's order around
`Delete`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**68 subscriptions** to `'OnBeforeDeleteEvent'`.

## The IST-state

`include/runtime/Table.h:406` runs `OnDelete()` and then `Delete()`. Nothing is raised.

## The choice

The raise sits ahead of the trigger call in `Delete(RunTrigger)`, unconditionally.

**And it must also fire from `DeleteAll(true)`** -- board:0044 records that `DeleteAll` reverts to
row-by-row when subscribers to `OnBeforeDelete` exist, which is the platform saying that the event
fires per row there too. A runtime that raises only from `Delete` makes `DeleteAll(true)` silently
skip 68 subscriptions.

## Ordering

After board:0057's dispatcher. The `DeleteAll` call site follows board:0044.

## Gate, and its negative control

A subscriber that raises: `Delete(true)` raises and the row survives. The same set through
`DeleteAll(true)`: the run raises and nothing is deleted.

**The negative control is the `DeleteAll` case**, for the reason board:0044 gives -- the two look
like one call and are not.

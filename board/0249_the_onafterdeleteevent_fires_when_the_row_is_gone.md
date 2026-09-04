Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/table/devenv-onafterdeleteevent-table-trigger.md
Verdict:  fehlt
Class:    activation

# `OnAfterDeleteEvent` fires when the row is gone and the record still holds it

```al
local procedure MyProcedure(var Rec: Record; RunTrigger: Boolean)
```

The row is deleted; `Rec` still carries the values, which is what subscribers read to clean up
whatever referenced the key. Step 5 of board:0029's order around `Delete`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**253 subscriptions** to `'OnAfterDeleteEvent'` -- the second-largest of the ten table events, after
`OnBeforeInsertEvent`'s 563, and by a wide margin the largest of the delete family. Cleanup is what
the BaseApp's layers hook.

## The IST-state

`include/runtime/Table.h:406` returns straight after `Delete()`.

## The choice

The raise sits after `Delete()`, unconditionally, with the record NOT cleared -- `Delete()` removes
the row and leaves the variable alone, which is what 253 subscribers depend on.

The `DeleteAll(true)` call site applies here as it does to 0248.

## Ordering

With 0248. High by population among the table events.

## Gate, and its negative control

A subscriber that reads the primary key and writes it into a log: the log holds the deleted key.

**The negative control is a blank key** -- a runtime that clears the record after the delete passes
"the subscriber ran" and logs nothing, which is exactly the shape that survives a careless gate.

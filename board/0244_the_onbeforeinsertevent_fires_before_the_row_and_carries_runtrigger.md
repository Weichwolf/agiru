Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/table/devenv-onbeforeinsertevent-table-trigger.md
Verdict:  fehlt
Class:    activation

# `OnBeforeInsertEvent` fires before the row, and tells the subscriber whether triggers were asked for

```al
[EventSubscriber(ObjectType::Table, Database::<Table>, 'OnBeforeInsertEvent', '', <SkipLicense>, <SkipPermission>)]
local procedure MyProcedure(var Rec: Record; RunTrigger: Boolean)
```

Two things the signature settles:

- **`Rec` is by `var`**, so a subscriber may CHANGE the record before it is written. That is what
  the BaseApp's own subscribers do, and it means the event is part of the write path rather than a
  notification beside it.
- **`RunTrigger` is passed through.** The subscriber learns whether the caller wrote `Insert()` or
  `Insert(true)` -- so the event fires either way, and the flag is information rather than a
  condition. A runtime that raised the event only under `Insert(true)` would silently drop 563
  subscriptions on the commonest call form.

It is step 1 of board:0029's five: this event, then the table's `OnInsert` (board:0228), then
`OnDatabaseInsert`, then the row, then `OnAfterInsertEvent` (0245).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**563 subscriptions** to `'OnBeforeInsertEvent'` -- the largest of the ten table trigger events.

## The IST-state

`include/runtime/Table.h:353` calls the trigger and then `Insert()`. No event is raised anywhere in
the tree, and board:0057 records that 3 753 subscribers are never called.

## The choice

The raise sits in `Insert(RunTrigger)` ahead of the trigger call, and takes the record by reference
plus the flag. **It is raised unconditionally**, outside the `if (RunTrigger)` block that guards the
trigger -- which is the shape the signature demands and the one a reader would get wrong by putting
the raise inside the block that already exists.

## Ordering

After board:0057's dispatcher and after 0196 (subscribers bind). It is the first table event worth
raising, by population.

## Gate, and its negative control

A subscriber that changes a field in `OnBeforeInsertEvent`: the row carries the changed value. The
same insert through `Insert()` without the argument: the subscriber still runs, and `RunTrigger` is
`false`.

**The negative control is the `Insert()` case** -- a raise placed inside the `if (RunTrigger)` block
passes the first assertion and drops the event on the call form the BaseApp uses most.

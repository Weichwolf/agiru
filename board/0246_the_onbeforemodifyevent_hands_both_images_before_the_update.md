Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/table/devenv-onbeforemodifyevent-table-trigger.md
Verdict:  fehlt
Class:    activation

# `OnBeforeModifyEvent` hands both images before the update

```al
local procedure MyProcedure(var Rec: Record; var xRec: Record; RunTrigger: Boolean)
```

Unlike the insert events, the modify events carry **`xRec`** -- the stored values -- because a
subscriber deciding whether to allow or adjust a change needs to see what is changing. It is step 1
of board:0029's order around `Modify`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**64 subscriptions** to `'OnBeforeModifyEvent'` -- the smallest of the four insert/modify/delete
before-events, and fewer than its after-partner (101), which is the reverse of the insert pair.

## The IST-state

`include/runtime/Table.h:381` runs `OnModify()` and then `Modify()`. Nothing is raised, and the
before-image this event needs is not built on the modify path at all -- board:0229 records that gap
for the trigger and it is the same one.

## The choice

The raise sits ahead of the trigger call in `Modify(RunTrigger)`, unconditionally, inside an `xRec`
scope opened for the whole method. **The scope must wrap the event, the trigger and the write**, so
that all three see the same before-image -- opening it per consumer would give the trigger a
`xRec` that a subscriber had already changed.

## Ordering

Behind board:0042's `xRec`, which `Validate` builds and this path does not. With 0247.

## Gate, and its negative control

A subscriber that raises when `Rec.Field <> xRec.Field`: changing the field raises before the row is
written; leaving it alone modifies.

**The negative control is the unchanged case** -- with no before-image both sides are equal, nothing
raises, and the test passes for the wrong reason.

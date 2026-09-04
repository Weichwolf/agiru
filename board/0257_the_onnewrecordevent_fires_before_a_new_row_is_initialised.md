Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/page/devenv-onnewrecordevent-page-trigger.md
Verdict:  fehlt
Class:    activation

# `OnNewRecordEvent` fires before a new row is initialised, and knows where it will sit

```al
local procedure MyProcedure(var Rec: Record; BelowxRec: Boolean; var xRec: Record)
```

"Executed after the OnNewRecord trigger, which is called **before a new record is initialized**" --
so before `Init` (board:0055's rule that `Init` skips the primary key), and before the user has
typed anything.

`BelowxRec` appears here as it does in 0258, and for the same reason: the new line's position in the
list decides its key when `AutoSplitKey` is on. There is no veto -- a page cannot refuse to start a
new row.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**2 subscriptions** with `ObjectType::Page` to `'OnNewRecordEvent'` -- with
`OnBeforeValidateEvent` (page) the smallest non-zero page event.

## The IST-state

No page runtime, and no producer for `BelowxRec` (0258 records the same).

## The choice

The raise sits in the page's new-record path, before `Init` runs, with the three parameters.

**Before `Init` is the point.** A subscriber that sets a default on `Rec` and then has `Init` clear
it has done nothing, so the order is the whole behaviour -- and it is the reverse of what a reader
would assume from the name "after the OnNewRecord trigger".

## Ordering

Blocked on board:0030. Low by population, but it shares `BelowxRec` with 0258, so the two are one
piece of page state.

## Gate, and its negative control

A subscriber that writes a field: after the new row is initialised the field still holds the value.

**The negative control is `Init`** -- a raise placed after it wipes the subscriber's work, and the
test that only asserts "the subscriber ran" passes.

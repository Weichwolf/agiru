Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/page/devenv-onaftergetcurrrecordevent-page-trigger.md
Verdict:  fehlt
Class:    activation

# `OnAfterGetCurrRecordEvent` fires when the CURRENT row changes, not when a row is fetched

```al
local procedure MyProcedure(var Rec: Record)
```

"Executed after the OnAfterGetCurrRecord trigger, which is called after the current record is
retrieved from the table."

**The distinction from 0261 is the whole item.** `OnAfterGetRecordEvent` fires per row rendered;
this one fires when the user's CURRENT row changes -- once per selection, not once per line. A page
runtime that raised both from the same place would raise this one fifty times on a fifty-row list,
and every FactBox and Cue that subscribes would recompute fifty times.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**24 subscriptions** with `ObjectType::Page` to `'OnAfterGetCurrRecordEvent'` -- against 0 for its
per-row sibling, which is the ratio that says which of the two the BaseApp actually uses.

## The IST-state

No page runtime. board:0030 records that a page background task is CANCELLED when the current record
changes, so the same moment already has a second consumer waiting for it.

## The choice

The raise sits where the page's current record is established -- on open, on navigation, and after a
write that repositions -- and NOT in the row-fetch loop.

## Ordering

Blocked on board:0030. Ahead of 0261 by population, and the two must be built together so the
distinction is made once.

## Gate, and its negative control

Move through five rows of a list: the subscriber fires five times, once per move. Render the same
list without moving: it fires once.

**The negative control is the render** -- a raise in the fetch loop fires five times on a page the
user has not touched, which is the defect that costs a FactBox its performance.

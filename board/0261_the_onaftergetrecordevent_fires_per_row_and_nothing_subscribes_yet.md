Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/page/devenv-onaftergetrecordevent-page-trigger.md
Verdict:  fehlt
Class:    activation

# `OnAfterGetRecordEvent` fires per ROW, before it is shown, and nothing subscribes to it yet

```al
local procedure MyProcedure(var Rec: Record)
```

"Executed after the OnAfterGetCurrRecord trigger, which is called after the record is retrieved from
the table **but before it is displayed to the user**."

**It fires per row**, which makes it the one page event whose cost scales with the list: a page
showing 50 rows raises it 50 times. That is the reason to know its population before building it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**0 subscriptions** with `ObjectType::Page` to `'OnAfterGetRecordEvent'`. Nothing in the read roots
uses it -- and given the per-row cost, that is worth recording rather than assuming.

## The IST-state

No page runtime.

## The choice

The raise sits in the page's row-fetch path. **With zero subscribers the dispatcher must cost
nothing**: board:0057's table is `constexpr` and sorted, so an event with no entries is one
comparison per row rather than a lookup -- which at 50 rows per page render is the difference
between free and measurable.

## Ordering

Blocked on board:0030. Last of the thirteen with 0255, both at zero subscribers.

## Gate, and its negative control

A page over five rows with one subscriber: it fires five times, before each row is rendered.

**The negative control is the count** -- a raise placed in the page's open path rather than its row
path fires once and passes any assertion that only checks it fired.

Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/pagefield/devenv-ondrilldown-pagefield-trigger.md, developer/triggers-auto/pagefieldextension/devenv-ondrilldown-pagefieldextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page field's `OnDrillDown` opens what a computed value came from

```al
trigger OnDrillDown()
```

Drilldown is the gesture on a computed field -- a FlowField total, a Cue -- that opens the rows
behind the number. CLAUDE.md names it among the things a BC user works in: "role centers with cues,
Tell Me, card/list/document layout, lookups, **drilldowns**".

Declaring the trigger replaces the platform's own drilldown, which for a FlowField is derived from
its `CalcFormula` (board:0047) and for a field with `DrillDownPageId` from that property.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnDrillDown()` on a page field or extension: **1 214 declarations.**

## The IST-state

No page runtime. `TestField::DrillDown` exists in `src/rt/TestPage.cpp` and reaches `Unopened()`.

## The choice

`TestField::DrillDown` and the page's own gesture consult the trigger when the generated class
declares it, and fall back to `DrillDownPageId` and then to the FlowField's own source.

**The fallback chain is the item, not the trigger.** The trigger is three lines; deciding what a
field with no trigger does -- and getting the FlowField case right, where the target is derived from
the `CalcFormula`'s table and filters -- is the work, and it is why this sits with board:0047 as much
as with board:0030.

## Ordering

Blocked on board:0030 and, for the FlowField fallback, on board:0047.

## Gate, and its negative control

A FlowField with no `OnDrillDown`: the gesture opens the summed table filtered by the
`CalcFormula`'s own `where` clause. The same field WITH a trigger: the trigger's page opens instead.

**The negative control is the filter** -- a fallback that opens the summed table unfiltered shows
every row in the company and passes any assertion that only checks a page opened.

Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-onaftergetcurrrecord-page-trigger.md, developer/triggers-auto/pageextension/devenv-onaftergetcurrrecord-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnAfterGetCurrRecord` runs when the SELECTION moves, not when a row is read

```al
trigger OnAfterGetCurrRecord()
```

It runs after the CURRENT record is retrieved -- once per selection change, where
`OnAfterGetRecord` (0284) runs once per row rendered. The pair is the same distinction 0261 and
0262 make for the events, and it is the one a page runtime gets wrong by raising both from the row
loop.

It is where FactBoxes and Cues refresh, which is why it is the second-busiest of the fourteen.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnAfterGetCurrRecord()` on a page or pageextension: **1 520 declarations**, against 8 136 on
`OnAfterGetRecord`. Both large; entirely different frequencies at run time.

## The IST-state

No page runtime.

## The choice

The call sits where the current record is established -- on open, on navigation, and after a write
that repositions -- with `OnAfterGetCurrRecordEvent` (0262) raised immediately after.

**And board:0030 records a third consumer of the same moment**: a page background task is cancelled
when the current record changes (board:0090). One moment, three things hanging off it, which is the
argument for making it a named point in the page runtime rather than three call sites.

## Ordering

Blocked on board:0030, and paired with 0284 so the distinction is made once.

## Gate, and its negative control

Move through five rows: the trigger runs five times. Render the same list without moving: once.

**The negative control is the render** -- a call in the row loop runs it five times on a page nobody
touched, and every FactBox recomputes five times.

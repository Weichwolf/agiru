Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/triggers-auto/report/devenv-oninitreport-report-trigger.md
Verdict:  fehlt
Class:    activation

# A report's `OnInitReport` runs first and can end the run before the request page

```al
trigger OnInitReport()
```

It is step 1 of board:0063's sequence, and the page that documents the sequence is explicit about
what it may do: "**If the OnInitReport doesn't end the processing of the report**, then the request
page for the report is run, if one is enabled."

So the sequence is a chain of gates: `OnInitReport`, the request page (0301), `OnPreReport` (0303),
the dataitems, `OnPostReport` (0304) -- and each of the first three can stop it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnInitReport()`: **479 declarations**, against 2 134 `.Report.al` files.

## The IST-state

Report has no generator: board:0034's object-kind table lists it among the kinds with none, and
board:0063 is the item. 280 of 676 reports in the read roots are `ProcessingOnly` and need no
layout, so the driver is reachable well before a renderer.

## The choice

The report driver calls it first, before anything else, and honours an early exit -- which in AL is
`CurrReport.Quit` rather than a return value.

**`OnInitReport` runs before the request page, so it cannot read the user's filters.** A trigger
that needs them belongs in `OnPreReport`, and that distinction is the reason both exist.

## Ordering

Blocked on board:0063. First of the four report triggers.

## Gate, and its negative control

A report whose `OnInitReport` quits: the request page never opens and no dataitem is processed.

**The negative control is the request page** -- a driver that opens it and then checks passes any
assertion about the dataset being empty.

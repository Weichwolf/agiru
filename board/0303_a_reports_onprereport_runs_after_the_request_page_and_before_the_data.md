Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/triggers-auto/report/devenv-onprereport-report-trigger.md, developer/triggers-auto/reportextension/devenv-onprereport-reportextension-trigger.md
Verdict:  fehlt
Class:    activation

# A report's `OnPreReport` runs after the request page and before any data is read

```al
trigger OnPreReport()
```

Step 3 of board:0063's sequence: "When the OnPreReport trigger has been run, the first data item is
processed unless the processing of the report was ended in the OnPreReport trigger."

**It is the first trigger that can read the user's choices**, because the request page has closed --
which is what separates it from `OnInitReport` (0302) and is why the BaseApp's filter-validation
code lives here.

The `reportextension` page describes the same trigger contributed by an extension; they are one call
site, as the table triggers are (board:0228).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnPreReport()`: **856 declarations** -- nearly double `OnInitReport`'s 479, for the reason
above.

## The IST-state

No report generator (board:0034, board:0063).

## The choice

Called after the request page closes and before the first dataitem, honouring an early exit.

**An extension's `OnPreReport` runs alongside the report's own**, in a declared order -- the merge
rule board:0033 owns, and the same ordering question board:0234 raises for tableextension triggers.

## Ordering

Blocked on board:0063, after 0301's request page and 0302.

## Gate, and its negative control

A report whose `OnPreReport` reads a request-page option and quits on one value: with that value no
dataitem is processed; with the other, all are.

**The negative control is reading the option** -- a driver that calls `OnPreReport` before the
request page gives it the default rather than the user's choice, and both branches then take the
same path.

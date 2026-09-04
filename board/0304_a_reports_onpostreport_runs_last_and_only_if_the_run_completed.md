Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/triggers-auto/report/devenv-onpostreport-report-trigger.md, developer/triggers-auto/reportextension/devenv-onpostreport-reportextension-trigger.md
Verdict:  fehlt
Class:    activation

# A report's `OnPostReport` runs last, and only when there were no more dataitems

```al
trigger OnPostReport()
```

Step 5 of board:0063's sequence: "When there are no more data items, the **OnPostReport** trigger is
called to do any necessary post processing, for example, removing temporary files."

**It does not run when the report was ended early.** A quit in `OnInitReport` (0302) or
`OnPreReport` (0303) skips it, which matters because its documented job is cleanup -- and cleanup
that runs after a run that never started deletes something that was never made.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnPostReport()`: **431 declarations.**

## The IST-state

No report generator.

## The choice

Called after the last dataitem, on the completion path only, with the extension's contribution
alongside in a declared order.

**"Only on the completion path" includes errors.** A report whose dataitem raises does not reach
`OnPostReport`, so a temporary file it would have deleted survives -- which is BC's behaviour and
the reason `OnPostReport` is not a destructor.

## Ordering

Blocked on board:0063. Last of the four report triggers.

## Gate, and its negative control

A report that completes: the trigger runs once. A report whose `OnPreReport` quits: it does not run
at all.

**The negative control is the quit** -- a driver that runs `OnPostReport` unconditionally cleans up
after a run that produced nothing, and every assertion about the completed case still passes.

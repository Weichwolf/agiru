Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/triggers-auto/reportdataitem/devenv-onaftergetrecord-reportdataitem-trigger.md
Verdict:  fehlt
Class:    activation

# A report dataitem's `OnAfterGetRecord` runs per row, and is where the report computes what it prints

```al
trigger OnAfterGetRecord()
```

board:0063: "After a record is retrieved from the data item, the **OnAfterGetRecord** trigger is
called. If there's an indented data item, its records are also processed" -- so this trigger is the
point the child dataitems hang off, and the place a report calls `CalcFields`, formats a caption or
skips a row with `CurrReport.Skip`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnAfterGetRecord()` on a report dataitem: **3 613 declarations** -- the largest trigger
population in the report family and the second-largest per-row trigger in the tree after the page's
8 136.

## The IST-state

No report generator.

## The choice

Called per row, after the read and before the child dataitems and the output.

**`CurrReport.Skip` is what makes this more than a callback.** A row skipped here produces no output
and no child rows, so the trigger's control-flow verbs are part of the driver's loop rather than
something the trigger returns -- and board:0063's dataset half cannot be built without them.

## Ordering

Blocked on board:0063, after 0306.

## Gate, and its negative control

A report over five rows whose `OnAfterGetRecord` skips the third: the output has four rows and the
third's children were not processed.

**The negative control is the children** -- a driver that honours the skip for the row and still
descends into the child dataitem produces orphaned detail lines, which is invisible on a report with
no indentation.

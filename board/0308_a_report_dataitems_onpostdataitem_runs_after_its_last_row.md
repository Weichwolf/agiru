Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/triggers-auto/reportdataitem/devenv-onpostdataitem-reportdataitem-trigger.md
Verdict:  fehlt
Class:    activation

# A report dataitem's `OnPostDataItem` runs after its last row, once per parent iteration

```al
trigger OnPostDataItem()
```

board:0063: "After the last record has been processed, the **OnPostDataItem** trigger is called." It
is where a dataitem writes its totals -- the sum it accumulated across its own rows.

**Like `OnPreDataItem` (0306) it runs once per parent iteration**, not once per report, so a child's
totals are per parent row. A driver that called it once would print one grand total where the report
means one per group.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnPostDataItem()`: **1 002 declarations**, against 2 356 on `OnPreDataItem` -- fewer
dataitems total than filter.

## The IST-state

No report generator.

## The choice

Called after the dataitem's last row, inside the parent's iteration, with the accumulated state the
`OnAfterGetRecord` bodies built.

**It runs even when the dataitem produced NO rows.** "After the last record has been processed" is
true of an empty set too, which is what lets a report print a zero total rather than nothing -- and
a driver that skips it on an empty dataitem drops the line the user expects.

## Ordering

Blocked on board:0063, after 0307.

## Gate, and its negative control

A parent with two rows and a child that has rows under the first and none under the second: the
child's `OnPostDataItem` runs twice, and the second time with a zero total.

**The negative control is the empty child** -- a driver that calls the trigger only when rows were
read prints one total instead of two.

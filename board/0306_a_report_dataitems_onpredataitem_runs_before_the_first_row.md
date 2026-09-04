Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/triggers-auto/reportdataitem/devenv-onpredataitem-reportdataitem-trigger.md
Verdict:  fehlt
Class:    activation

# A report dataitem's `OnPreDataItem` runs before its first row, and is where its filters are set

```al
trigger OnPreDataItem()
```

board:0063's per-dataitem sequence: "Before the first record is retrieved, the **OnPreDataItem**
trigger is called", then each record with `OnAfterGetRecord` (0307), then `OnPostDataItem` (0308).

It is where a dataitem sets the filters the request page did not -- and where `SetRange` on the
dataitem's own record does what `DataItemTableView` could not express statically.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnPreDataItem()`: **2 356 declarations** -- the largest of the report triggers by a factor
of three, because nearly every dataitem narrows itself.

## The IST-state

No report generator (board:0034, board:0063).

## The choice

Called per dataitem, before its first read, with the dataitem's record in scope.

**Per dataitem and per ITERATION of its parent.** board:0063 records that an indented dataitem's
records are processed inside each of the parent's, so a child's `OnPreDataItem` runs once per parent
row -- not once per report. A driver that calls it once sets the child's filters from the first
parent row and reuses them.

## Ordering

Blocked on board:0063. First of the three dataitem triggers, and the largest.

## Gate, and its negative control

A report with a parent and an indented child, where the child's `OnPreDataItem` filters on the
parent's key: each parent row yields only its own children.

**The negative control is the second parent row** -- a driver that calls the trigger once gives every
parent the first one's children, and a single-row test passes.

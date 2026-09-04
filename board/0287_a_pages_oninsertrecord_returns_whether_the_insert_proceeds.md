Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-oninsertrecord-page-trigger.md, developer/triggers-auto/pageextension/devenv-oninsertrecord-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnInsertRecord` RETURNS whether the insert proceeds

```al
trigger OnInsertRecord(BelowxRec: Boolean): Boolean
```

It runs before the row is inserted and its return value decides: `false` cancels. That is the page's
own veto, ahead of the subscribers' `AllowInsert` (0258) -- **two independent refusals on one
operation**, and the insert needs both to agree.

`BelowxRec` again, for the same reason as 0286.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnInsertRecord(` on a page or pageextension: **442 declarations.**

## The IST-state

No page runtime.

## The choice

The call sits in the page's insert path, its Boolean combined with the event's `AllowInsert`, and
only then does the record's own `Insert` run -- which brings the table's `OnInsert` (0228) and the
table events (0244, 0245) with it.

**One user pressing Enter on a new line therefore crosses five refusal points**: this trigger,
`OnInsertRecordEvent`'s `AllowInsert`, the table's `OnInsert`, `OnBeforeInsertEvent`, and the
platform's duplicate check. Naming them together is the point of this item; implementing them as one
check is the mistake.

## Ordering

Blocked on board:0030, and behind 0228 and 0244, which it calls into.

## Gate, and its negative control

A page whose `OnInsertRecord` returns `false`: no row appears and the table's `OnInsert` never ran.

**The negative control is the table trigger** -- a page runtime that inserts first and asks after
runs the table trigger on a row it then removes, and side effects the trigger produced stay.

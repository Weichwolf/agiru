Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-onmodifyrecord-page-trigger.md, developer/triggers-auto/pageextension/devenv-onmodifyrecord-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnModifyRecord` returns whether the update proceeds

```al
trigger OnModifyRecord(): Boolean
```

The modify counterpart of 0287: it runs before the row is written and `false` cancels, ahead of the
subscribers' `AllowModify` (0259).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnModifyRecord()` on a page or pageextension: **357 declarations.**

## The IST-state

No page runtime.

## The choice

The call sits in the page's save path, combined with the event's `AllowModify`, ahead of the
record's `Modify` -- and therefore ahead of the table's `OnModify` (0229) and the table events
(0246, 0247).

**The page's `xRec` is what this trigger compares against**, and 0259 records why that is not the
record-level before-image: a page's `xRec` is the row as it was when editing began, not as it was
before this particular `Modify`.

## Ordering

Blocked on board:0030, behind 0229 and 0246.

## Gate, and its negative control

A page whose `OnModifyRecord` returns `false`: the row is unchanged and the table's `OnModify` never
ran.

**The negative control is the table trigger**, as in 0287.

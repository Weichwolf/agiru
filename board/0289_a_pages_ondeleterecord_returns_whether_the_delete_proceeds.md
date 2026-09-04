Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-ondeleterecord-page-trigger.md, developer/triggers-auto/pageextension/devenv-ondeleterecord-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnDeleteRecord` returns whether the delete proceeds, and usually asks the user first

```al
trigger OnDeleteRecord(): Boolean
```

The delete counterpart of 0287 and 0288. In the BaseApp its commonest body is a `Confirm` -- the
"Delete this record?" dialog -- so under a test runner this trigger is where board:0194's
`ConfirmHandler` is reached.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnDeleteRecord()` on a page or pageextension: **420 declarations.**

## The IST-state

No page runtime, and `Confirm` is a door refusal -- so even once the trigger is called, its
commonest body cannot run without board:0194.

## The choice

The call sits in the page's delete path, combined with `OnDeleteRecordEvent`'s `AllowDelete` (0260),
ahead of the record's `Delete` and therefore ahead of `OnDelete` (0230) and the table events.

## Ordering

Blocked on board:0030, and its useful gate is blocked on board:0194 -- which makes this the page
trigger that ties the UI half of the milestone to the handler half.

## Gate, and its negative control

A page whose `OnDeleteRecord` confirms and a handler answering `false`: the row survives and the
table's `OnDelete` never ran.

**The negative control is the handler answering `true`** -- the row must go. A runtime that treats
an unanswered `Confirm` as `true` passes the second case and silently deletes on the first.

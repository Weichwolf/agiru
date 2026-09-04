Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-oninit-page-trigger.md, developer/triggers-auto/pageextension/devenv-oninit-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnInit` runs before anything else, and before the source record exists

```al
trigger OnInit()
```

It is the first thing a page runs -- before `OnOpenPage` (0279) and before the source table is
attached. That is the constraint: a variable initialised here is available to every later trigger,
and a record read here is not, because there is no current record yet.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnInit()` on a page or pageextension: **1 458 declarations.**

## The IST-state

No page runtime; the trigger is emitted as a member and never called.

## The choice

The call sits first in the page's open sequence, before the source record is attached.

**Before the record is the whole item.** A runtime that attached the record first would let an
`OnInit` body read `Rec` and get something -- which would work in agiru and fail in BC, and the
difference surfaces as a page that behaves correctly here and wrongly there. Getting the order right
costs nothing now.

There is no `OnInitEvent`: `OnInit` is one of the three page triggers with no event beside it, so
its call site has no second consumer.

## Ordering

Blocked on board:0030, and first within the open sequence.

## Gate, and its negative control

A page whose `OnInit` sets a variable that `OnOpenPage` reads: the value arrives.

**The negative control is an `OnInit` that reads `Rec`** -- it must see no current record, which is
what proves the trigger runs before the attach rather than merely early.

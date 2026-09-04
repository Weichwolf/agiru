Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/page/devenv-onopenpageevent-page-trigger.md
Verdict:  fehlt
Class:    activation

# `OnOpenPageEvent` fires after the page's own `OnOpenPage`, and it is the busiest page event

```al
[EventSubscriber(ObjectType::Page, Page::<Page Name>, 'OnOpenPageEvent', '', ...)]
local procedure MyProcedure(var Rec: Record)
```

"Executed **after** the OnOpenPage trigger, which is called after a page is initialized and run."
So the order is: the page is initialised, its own `OnOpenPage` runs, then every subscriber.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**57 subscriptions** with `ObjectType::Page` to `'OnOpenPageEvent'` -- the largest of the thirteen
page trigger events.

## The IST-state

There is no page runtime: `include/runtime/test/TestPage.h` derives from the generated page class
and every `TestField` body reaches `Unopened()` (`src/rt/TestPage.cpp`). No page opens, so no page
event fires.

## The choice

The raise sits at the end of the page's open sequence, after the object's own `OnOpenPage`, with the
page's `Rec` by reference.

**And a `TestPage` must raise it too.** board:0030 records that the predecessor filed this twice as
its own defect (openerp WI-1169, WI-1170): `Page.run` raised the page events and the TestPage path
did not, so 108 subscribers on 55 pages were live one way and dead the other. The two paths share
one raise or they diverge.

## Ordering

Blocked on board:0030 (a page that opens). Ahead of the other twelve page events by population.

## Gate, and its negative control

A page whose own `OnOpenPage` sets a field and a subscriber that reads it: the subscriber sees the
value, which proves the order.

**The negative control is driving the same page through a `TestPage`** -- the subscriber must fire
there too. That is the case the predecessor missed twice.

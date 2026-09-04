Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/page/devenv-onclosepageevent-page-trigger.md
Verdict:  fehlt
Class:    activation

# `OnClosePageEvent` fires only when the page actually closed

```al
local procedure MyProcedure(var Rec: Record)
```

"Executed after the OnClosePage trigger, which is called when page closes **after the
OnQueryClosePage trigger is executed**." So it is the last step of the sequence 0255 opens, and it
must not run when the close was vetoed.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**13 subscriptions** with `ObjectType::Page` to `'OnClosePageEvent'`.

## The IST-state

No page runtime.

## The choice

The raise sits after the page's own `OnClosePage`, inside the branch that the close survived. It has
no veto parameter -- by the time it runs the decision is made.

**A `TestPage` closes too.** The predecessor's WI-1169/WI-1170 pair (board:0030) is about exactly
this asymmetry on the open side; the close side has the same shape and the same 13 subscribers.

## Ordering

Blocked on board:0030, with 0255 -- one sequence, two items, and 0255 is the one that decides
whether this one runs.

## Gate, and its negative control

Close a page normally: the subscriber runs. Close it with 0255's veto in place: it does not.

**The negative control is the vetoed close** -- and it is the same case 0255 uses, which is why the
two items name each other rather than each building half a sequence.

Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/page/devenv-onqueryclosepageevent-page-trigger.md
Verdict:  fehlt
Class:    activation

# `OnQueryClosePageEvent` vetoes the close, and nothing in the read roots subscribes to it

```al
local procedure MyProcedure(var Rec: Record; var AllowClose: Boolean)
```

"Executed after the OnQueryClosePage trigger, which is called as a page closes and **before the
OnClosePage trigger executes**." So the page's close sequence is: `OnQueryClosePage`, this event,
then -- only if the close survives -- `OnClosePage` and `OnClosePageEvent` (0256).

The fourth of the veto events, and the only one whose veto stops something the user asked for
rather than a database write.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**0 subscriptions** with `ObjectType::Page` to `'OnQueryClosePageEvent'`. Nothing in the read roots
uses it, which sets the ordering: last of the thirteen.

## The IST-state

No page runtime.

## The choice

The raise sits between `OnQueryClosePage` and `OnClosePage`, with `AllowClose` following 0260's rule
-- initialised `true`, only turned off, carried through the chain.

**The veto must SKIP the rest of the close sequence**, not merely report it: a page that raises
`OnClosePageEvent` after a refused close tells its subscribers the page closed when it did not.

## Ordering

Blocked on board:0030. **Last of the page events by population**, and it is filed rather than
skipped because the close sequence has to be built once and the veto is part of it -- discovering
this event after the sequence exists means building it twice.

## Gate, and its negative control

A subscriber setting `AllowClose := false`: the page stays open AND `OnClosePage` never runs.

**The negative control is `OnClosePage`** -- a runtime that honours the veto for the window and runs
the rest of the sequence anyway passes the visible half.

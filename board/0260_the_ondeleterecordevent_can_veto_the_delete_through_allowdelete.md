Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/page/devenv-ondeleterecordevent-page-trigger.md
Verdict:  fehlt
Class:    activation

# `OnDeleteRecordEvent` can VETO the delete, through a `var Boolean` the caller reads back

```al
local procedure MyProcedure(var Rec: Record; var AllowDelete: Boolean)
```

"Executed after the OnDeleteRecord trigger, which is called **before** a record is deleted." The
subscriber does not raise to stop the delete -- **it sets `AllowDelete := false`**, and the page
reads the value back after the chain.

It is one of four page events with a veto parameter: this one, `OnInsertRecordEvent` (`AllowInsert`),
`OnModifyRecordEvent` (`AllowModify`) and `OnQueryClosePageEvent` (`AllowClose`).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**18 subscriptions** with `ObjectType::Page` to `'OnDeleteRecordEvent'`.

## The IST-state

No page runtime, so nothing raises it.

## The choice

The dispatcher must carry a `var Boolean` THROUGH the whole subscriber chain and hand it back to the
caller -- which is the same `var`-through-dispatch path board:0066's `OnResolveAutoFormat` and
board:0057's `IsHandled` need. **Three families, one mechanism**, and this is the family where
getting it wrong deletes a row somebody vetoed.

**The initial value is `true`.** A page with no subscribers deletes, and each subscriber may only
turn it off -- a dispatcher that reset the flag per subscriber would let the last one overrule a
veto the first one set.

## Ordering

Blocked on board:0030. Behind board:0057's `var`-parameter dispatch, which it shares with 0258,
0259 and 0255.

## Gate, and its negative control

Two subscribers, the first setting `AllowDelete := false` and the second leaving it alone: the row
survives.

**The negative control is the second subscriber** -- a dispatcher that re-initialises the flag per
call deletes the row and passes any test with only one subscriber.

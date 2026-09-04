Type:     task
Status:   open
Parent:   0057
Area:     rt
Source:   developer/triggers-auto/events/table/devenv-onbeforerenameevent-table-trigger.md
Verdict:  fehlt
Class:    activation

# `OnBeforeRenameEvent` fires before the key moves, with both keys in hand

```al
local procedure MyProcedure(var Rec: Record; var xRec: Record; RunTrigger: Boolean)
```

`Rec` holds the NEW primary key and `xRec` the old one. It is the only event pair where the two
images differ in the KEY rather than in a field, and a subscriber that raises here stops the rename
before any dependent table is touched.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**13 subscriptions** to `'OnBeforeRenameEvent'` -- the smallest of the ten, against 121 on its
after-partner.

## The IST-state

`include/runtime/Table.h:1141` -- `Rename` is a variadic refusal, so there is no operation and no
call site. Board:0231 and board:0240 record the same.

## The choice

The raise sits at the top of the `Rename(Keys..., RunTrigger)` overload board:0231 introduces,
ahead of the trigger and ahead of the relation cascade.

## Ordering

Blocked on board:0231, itself blocked on board:0043.

## Gate, and its negative control

A subscriber that raises: the key does not move and no dependent row is touched.

**The negative control is the dependent row** -- a cascade that runs before the event passes the
"key unchanged" assertion and leaves the graph half-renamed.

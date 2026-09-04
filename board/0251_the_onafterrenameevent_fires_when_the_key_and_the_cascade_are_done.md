Type:     task
Status:   open
Parent:   0057
Area:     rt
Source:   developer/triggers-auto/events/table/devenv-onafterrenameevent-table-trigger.md
Verdict:  fehlt
Class:    activation

# `OnAfterRenameEvent` fires when the key and the cascade are done

```al
local procedure MyProcedure(var Rec: Record; var xRec: Record; RunTrigger: Boolean)
```

`Rec` holds the new key, `xRec` the old one, and every table whose `TableRelation` pointed at the old
key has already been updated. That is what makes the event useful: a subscriber fixes up the
references the platform's cascade does not know about, and it needs both keys to do it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**121 subscriptions** to `'OnAfterRenameEvent'`, against 13 on the before-partner -- a ratio of nine
to one, and the sharpest before/after asymmetry of the ten. Renaming is something extensions react
to, not something they police.

## The IST-state

No `Rename` operation exists (`include/runtime/Table.h:1141`).

## The choice

The raise sits at the end of `Rename(Keys..., RunTrigger)`, after the cascade rather than between
the key update and the cascade.

## Ordering

Blocked on board:0231 and board:0043, with 0250. **Ahead of 0250 by population** when the two are
worked.

## Gate, and its negative control

A subscriber that reads a dependent table and asserts it points at the new key.

**That assertion IS the negative control** -- it fails if the event fires between the key update and
the cascade, and passes if the event is simply missing, so the gate must be written around it rather
than around whether the subscriber ran.

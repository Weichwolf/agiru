Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/page/devenv-onmodifyrecordevent-page-trigger.md
Verdict:  fehlt
Class:    activation

# `OnModifyRecordEvent` vetoes the modify and carries both images

```al
local procedure MyProcedure(var Rec: Record; var xRec: Record; var AllowModify: Boolean)
```

"Executed after the OnModifyRecord trigger, which is called **before** a record is modified." The
subscriber sees the new values and the stored ones, and sets `AllowModify := false` to stop the
write.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**39 subscriptions** with `ObjectType::Page` to `'OnModifyRecordEvent'` -- the second-largest page
event after `OnOpenPageEvent`.

## The IST-state

No page runtime.

## The choice

The raise sits in the page's modify path, inside the `xRec` scope the page holds for the current
record -- which is a DIFFERENT scope from the record-level one board:0229 needs: a page's `xRec` is
the row as it was when the user started editing, not as it was before this particular `Modify`.

**That distinction is the item.** A page runtime that reused the record-level before-image would
give subscribers the values from the last validate rather than from the last read, and the
difference is every field the user changed before pressing save.

`AllowModify` follows 0260's rule.

## Ordering

Blocked on board:0030 and on board:0057's `var`-parameter dispatch.

## Gate, and its negative control

Edit two fields on a page and save with a subscriber that compares `Rec` and `xRec`: both changes
are visible.

**The negative control is the second field** -- a runtime whose `xRec` is refreshed per validate
shows only the last one changed.

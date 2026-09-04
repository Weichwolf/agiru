Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/triggers-auto/events/page/devenv-oninsertrecordevent-page-trigger.md
Verdict:  fehlt
Class:    activation

# `OnInsertRecordEvent` vetoes through `AllowInsert`, and `BelowxRec` says where the row goes

```al
local procedure MyProcedure(var Rec: Record; BelowxRec: Boolean; var xRec: Record; var AllowInsert: Boolean)
```

Four parameters, and two of them exist nowhere else:

- **`BelowxRec`** -- whether the new row is being inserted BELOW the record `xRec` holds. It is what
  a list page knows and a table does not: the user pressed down-arrow on the last line rather than
  up-arrow on the first, and `AutoSplitKey` (board:0067) uses it to compute the new key.
- **`AllowInsert`** by `var` -- the veto, like 0260.

"Executed after the OnInsertRecord trigger, which is called **before** a record is inserted."

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**20 subscriptions** with `ObjectType::Page` to `'OnInsertRecordEvent'`.

## The IST-state

No page runtime, so nothing raises it, and `BelowxRec` has no producer: nothing in the tree knows
where in a list a new row was requested.

## The choice

The raise sits in the page's insert path with all four parameters. **`BelowxRec` is the page's own
state**, not the record's -- it comes from which navigation produced the new row, so the page
runtime has to keep it rather than derive it.

`AllowInsert` follows 0260's rule: initialised `true`, only turned off, carried through the chain.

## Ordering

Blocked on board:0030, and on whatever implements `AutoSplitKey`, which is the consumer that makes
`BelowxRec` matter.

## Gate, and its negative control

A subscriber that reads `BelowxRec` and writes it into a field: inserting below the current line
gives `true`, above gives `false`.

**The negative control is the "above" case** -- a page runtime that always passes `true` passes the
first assertion, and `AutoSplitKey` then numbers every new line as if it went to the end.

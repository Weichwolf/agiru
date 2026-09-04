Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-onnewrecord-page-trigger.md, developer/triggers-auto/pageextension/devenv-onnewrecord-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnNewRecord` runs AFTER `Init` and before the row is inserted

```al
trigger OnNewRecord(BelowxRec: Boolean)
```

"Runs **after a new record is initialized**, but before it is inserted as a record in the table."

**That is the opposite order from its event.** `OnNewRecordEvent` (0257) is documented as running
before initialisation and this trigger after it -- so the sequence is: the event, `Init`, then this
trigger. A runtime that placed the two together would put one of them on the wrong side of `Init`,
and whichever it is loses its defaults to the initialisation.

`BelowxRec` is the same page-held state 0257 and 0258 need: whether the new line goes below the
current one, which `AutoSplitKey` reads.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnNewRecord(` on a page or pageextension: **886 declarations** -- where a card or a
document line sets its defaults.

## The IST-state

No page runtime, and no producer for `BelowxRec`.

## The choice

The call sits after `Init` in the page's new-record path, with `BelowxRec` from the page's
navigation state.

## Ordering

Blocked on board:0030. With 0257, which is the same path on the other side of `Init`.

## Gate, and its negative control

A page whose `OnNewRecord` sets a default: the new line carries it after initialisation.

**The negative control is a subscriber to `OnNewRecordEvent` setting the SAME field** -- the
trigger's value must win, because it runs later, and a runtime with the two on the same side of
`Init` gets that backwards.

Type:     task
Status:   open
Parent:   0029
Area:     rt, gen
Source:   developer/triggers-auto/tableextension/devenv-onbeforemodify-tableextension-trigger.md
Verdict:  fehlt
Class:    activation

# A tableextension's `OnBeforeModify` runs before the update, and sees both images

`OnBeforeModify` runs before the default modify behaviour. At that point `Rec` holds the new values
and `xRec` the stored ones, so it is the trigger an extension uses to refuse a change -- raising
leaves the row as it was.

## The IST-state

`include/runtime/Table.h:381` runs `OnModify()` and then `Modify()`. No `OnBeforeModify` call
exists, and neither does the `xRec` scope this trigger reads (board:0229 records that gap for the
base trigger; it is the same gap).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnBeforeModify()` on a tableextension: **62 declarations.**

## The choice

One line before the existing `OnModify` call, under `if constexpr (requires ...)`, with the
per-extension member naming board:0234 describes -- and inside the `xRec` scope, which has to open
before the first of the three triggers rather than around each.

## Ordering

With 0234 and 0237; and behind board:0042, because a `OnBeforeModify` that cannot read `xRec` is
the trigger without its purpose.

## Gate, and its negative control

A tableextension whose `OnBeforeModify` raises when a field changed: changing it raises and the row
keeps its old value; leaving it alone modifies.

**The negative control is the unchanged case** -- without a before-image both comparisons come out
equal and the trigger never raises, which reads as a pass.

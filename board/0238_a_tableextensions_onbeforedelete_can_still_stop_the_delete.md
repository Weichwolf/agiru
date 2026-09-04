Type:     task
Status:   open
Parent:   0029
Area:     rt, gen
Source:   developer/triggers-auto/tableextension/devenv-onbeforedelete-tableextension-trigger.md
Verdict:  fehlt
Class:    activation

# A tableextension's `OnBeforeDelete` can still stop the delete, because the row is still there

`OnBeforeDelete` runs before the default delete behaviour. The row exists, so the trigger can read
it, count what depends on it and RAISE -- which cancels the delete. It is the extension's half of
what board:0230 records for the base table's `OnDelete`.

## The IST-state

`include/runtime/Table.h:406` runs `OnDelete()` and then `Delete()`. No `OnBeforeDelete` call
exists.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnBeforeDelete()` on a tableextension: **74 declarations.**

## The choice

One line before the existing `OnDelete` call, same guard, same per-extension naming.

**And `DeleteAll(true)` must reach it too** -- board:0044 records that `DeleteAll(true)` runs the
triggers over a COPY of the record with its initial values, so the extension's brackets fire there
as well and see the same copy. A runtime that wires the brackets only into `Delete` leaves
`DeleteAll(true)` half-triggered, which is worse than not wiring them at all.

## Ordering

With 0239. After board:0044 decides `DeleteAll`'s shape, because that is the second call site.

## Gate, and its negative control

A tableextension whose `OnBeforeDelete` raises: `Delete(true)` raises and the row survives. The same
through `DeleteAll(true)`: the run raises and no row of the set is gone.

**The negative control is the `DeleteAll` case** -- wiring only `Delete` passes the first and
deletes everything in the second.

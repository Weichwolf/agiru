Type:     task
Status:   open
Parent:   0029
Area:     rt, gen
Source:   developer/triggers-auto/tableextension/devenv-onaftermodify-tableextension-trigger.md
Verdict:  fehlt
Class:    activation

# A tableextension's `OnAfterModify` runs once the row is written, and the rowversion has moved

`OnAfterModify` runs after the default modify behaviour. The row is written, so the trigger sees
what the platform changed -- `SystemModifiedAt`, `SystemModifiedBy` and the advanced
`SystemRowVersion` (board:0013).

## The IST-state

`include/runtime/Table.h:381` returns straight after `Modify()`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnAfterModify()` on a tableextension: **57 declarations.**

## The choice

One line after `Modify()`, same guard, same per-extension naming.

**It must run after the platform's own field assignment and not merely after the `UPDATE`.** The
audit fields are written into the RECORD as well as the row, and a trigger placed between the SQL
and that assignment would read the old values -- the same ordering point board:0235 makes for
`Insert`.

## Ordering

With 0236.

## Gate, and its negative control

A tableextension whose `OnAfterModify` copies `Rec.SystemModifiedAt` into a field: the value is the
one this modify produced, not the previous one.

**The negative control is the previous value** -- it passes any assertion that only checks the
trigger ran.

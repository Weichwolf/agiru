Type:     task
Status:   open
Parent:   0029
Area:     rt, gen
Source:   developer/triggers-auto/tableextension/devenv-onafterdelete-tableextension-trigger.md
Verdict:  fehlt
Class:    activation

# A tableextension's `OnAfterDelete` runs when the row is gone, and `Rec` still holds it

`OnAfterDelete` runs after the default delete behaviour. The ROW is gone but the RECORD VARIABLE
still holds the values it had, which is what the trigger reads to clean up whatever pointed at it.

## The IST-state

`include/runtime/Table.h:406` returns straight after `Delete()`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnAfterDelete()` on a tableextension: **89 declarations** -- the largest of the eight
brackets, because cleanup is the commonest reason to extend a table's delete.

## The choice

One line after `Delete()`, same guard, same per-extension naming. **The record must NOT be cleared
between the delete and the trigger** -- `Delete()` removes the row and leaves the variable alone,
which is what makes the trigger useful and is the current behaviour, so this is a call site and not
a change of state.

## Ordering

With 0238, including the `DeleteAll(true)` call site.

## Gate, and its negative control

A tableextension whose `OnAfterDelete` reads the primary key it just deleted and writes it into a
log table: the log holds the key.

**The negative control is a blank key.** A runtime that clears the record after the delete passes
"the trigger ran" and logs nothing.

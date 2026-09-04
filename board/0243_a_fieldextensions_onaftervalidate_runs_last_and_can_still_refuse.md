Type:     task
Status:   open
Parent:   0029
Area:     rt, gen
Source:   developer/triggers-auto/fieldextension/devenv-onaftervalidate-fieldextension-trigger.md
Verdict:  fehlt
Class:    activation

# A fieldextension's `OnAfterValidate` runs last and can still refuse the entry

`OnAfterValidate` runs "after the default validation behavior is executed on a record field entry".
It is the last of the four steps board:0242 lists, and it can still raise: "An error message
displays if an error occurs in the trigger code. In case of an error, the user entry is not written
to the database."

So it is the extension's chance to reject a value the base field accepted -- which is the whole
reason it exists, and why it must run INSIDE the restore.

## The IST-state

`include/runtime/Table.h:1373` ends at `RunOnValidate(no)`; nothing runs after it. The trigger is
discarded by `TableWriter.cpp:592`'s name filter, like 0233 and 0242.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnAfterValidate()` on a fieldextension: **1 316 declarations** -- eight times its partner,
and the third-largest trigger population in the tree after `OnValidate` and `OnInsert`. Extensions
overwhelmingly hook the END of a validate rather than its start.

## The choice

A fourth `constexpr` map -- `kOnAfterValidate` -- consulted after `RunOnValidate` and inside the same
try block.

**The population is the argument for doing this one first of the four maps.** 1 316 declarations
against 165 for its partner and 1 197 for `OnLookup`: the extensions' after-validate hooks are how
the BaseApp's own layers add behaviour to base fields, so a validate that does not reach them is
missing most of what an extended field does.

## Ordering

With 0242, and ahead of it by population.

## Gate, and its negative control

A fieldextension whose `OnAfterValidate` raises on a value the base field accepts: the validate
raises and the field holds its old value.

**The negative control is the old value** -- the base trigger already accepted the new one, so a
runtime that raises without restoring leaves the record holding a value the extension rejected,
which the next `Modify` writes.

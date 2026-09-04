Type:     task
Status:   open
Parent:   0029
Area:     rt, gen
Source:   developer/triggers-auto/fieldextension/devenv-onbeforevalidate-fieldextension-trigger.md
Verdict:  fehlt
Class:    activation

# A fieldextension's `OnBeforeValidate` runs before the DEFAULT validation, not before the field's own trigger

The page is precise about which default: it runs before "the default validation behavior ...
which are default checks such as **data type validation**". So the order around a validate is

1. `OnBeforeValidate` (fieldextension)
2. the platform's own checks -- type, length, and the `TableRelation` (board:0043)
3. the field's `OnValidate` (board:0232)
4. `OnAfterValidate` (fieldextension, 0243)

and an error at any of them leaves the field with its old value: "In case of an error, the user
entry is not written to the database."

**It applies to a field the extension does NOT own** -- "an already existing table field when it is
being modified in a table extension" -- which is what distinguishes it from declaring `OnValidate`
on a field the extension added.

## The IST-state

`include/runtime/Table.h:1373` does the assignment, `CheckRelation`, then `RunOnValidate`. There is
no step before the check, and `TableWriter.cpp:592` takes only the trigger named `onvalidate` from
each field's list -- so a fieldextension's `OnBeforeValidate` is discarded exactly as `OnLookup` is
(board:0233).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnBeforeValidate()` on a fieldextension: **165 declarations.**

## The choice

A third `constexpr` map beside `kOnValidate` and `kOnLookup` -- `kOnBeforeValidate`, field number to
lambda -- consulted at the top of `Validate`, inside the try block so a raise restores the record
like any other failure.

**Inside the try and not before it.** The restore is what makes "the user entry is not written"
true, and a trigger that raised outside the try would leave the assigned value in place.

## Ordering

With 0243; both need the same widening of `TableWriter.cpp`'s trigger filter that board:0233 needs.

## Gate, and its negative control

A fieldextension whose `OnBeforeValidate` raises: the field holds its OLD value and the field's own
`OnValidate` never ran.

**The negative control is the field's own trigger** -- a runtime that runs the brackets after
`RunOnValidate` passes the "it raised" assertion while having already run the body the extension
meant to prevent.

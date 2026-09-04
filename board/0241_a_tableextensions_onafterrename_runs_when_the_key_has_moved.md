Type:     task
Status:   open
Parent:   0029
Area:     rt
Source:   developer/triggers-auto/tableextension/devenv-onafterrename-tableextension-trigger.md
Verdict:  fehlt
Class:    activation

# A tableextension's `OnAfterRename` runs when the key has moved and the cascade is done

`OnAfterRename` runs after the default rename behaviour -- so after the row carries its new primary
key and after every table whose `TableRelation` pointed at the old one has been updated. That is
what an extension needs in order to fix up anything the platform's cascade does not know about.

## The IST-state

`Rename` is a variadic refusal (`include/runtime/Table.h:1141`), so neither this trigger nor its
partner has a call site.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnAfterRename()` on a tableextension: **20 declarations.**

## The choice

One line after the rename completes, inside the same overload board:0231 introduces, and **after the
relation cascade rather than between the key update and the cascade** -- the page's "after the
default rename behavior" covers both, and the cascade is part of it.

## Ordering

Blocked on board:0231 and board:0043, with 0240.

## Gate, and its negative control

A tableextension whose `OnAfterRename` reads a dependent table and asserts it now points at the new
key.

**The negative control is exactly that assertion** -- it is the one that fails if the trigger runs
between the key update and the cascade, and passes if the trigger is simply missing, so the gate
has to be written around it rather than around whether the trigger ran.

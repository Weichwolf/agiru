Type:     task
Status:   open
Parent:   0029
Area:     rt
Source:   developer/triggers-auto/tableextension/devenv-onbeforerename-tableextension-trigger.md
Verdict:  fehlt
Class:    activation

# A tableextension's `OnBeforeRename` sees both keys, and cannot run because `Rename` refuses

`OnBeforeRename` runs before the default rename behaviour. `Rec` holds the NEW primary key and
`xRec` the old one -- the only trigger where the pair differs in the key rather than in a field.

## The IST-state

`include/runtime/Table.h:1141` -- `Rename` is a variadic refusal, so there is no operation to
bracket. Board:0231 records the same for the base table's `OnRename`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnBeforeRename()` on a tableextension: **13 declarations** -- the smallest of the eight.

## The choice

One line before the base `OnRename` call, in the `Rename(Keys..., RunTrigger)` overload board:0231
introduces. Nothing about the bracket is unusual; the item exists because the ORDER around the
relation cascade is not: the trigger runs before the key moves and therefore before every dependent
table is updated, so an extension that raises here leaves the whole graph untouched.

## Ordering

Blocked on board:0231, which is itself blocked on board:0043's relation metadata.

## Gate, and its negative control

A tableextension whose `OnBeforeRename` raises: the key does not move AND no dependent row is
touched.

**The negative control is the dependent row** -- a cascade that runs before the trigger passes the
"key unchanged" assertion and leaves the graph half-renamed.

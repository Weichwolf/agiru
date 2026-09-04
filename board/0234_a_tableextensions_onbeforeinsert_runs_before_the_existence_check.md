Type:     task
Status:   open
Parent:   0029
Area:     rt, gen
Source:   developer/triggers-auto/tableextension/devenv-onbeforeinsert-tableextension-trigger.md
Verdict:  fehlt
Class:    activation

# A tableextension's `OnBeforeInsert` runs before the platform's existence check

> This trigger is run **before default insert behavior, which checks that the record to be inserted
> does not already exist** before the insertion occurs. ... **The new record is not inserted if an
> error occurs in the trigger code.**

So it brackets the base table's `OnInsert` (board:0228) on the near side, and both are ahead of the
duplicate-key check -- which is board:0029's "the trigger runs BEFORE the platform's own check",
stated by the platform for the extension half.

## The IST-state

`include/runtime/Table.h:353` calls `record.OnInsert()` and then `Insert()`. There is no
`OnBeforeInsert` call and no place for one: the extension's triggers are merged into the generated
class by board:0033, so the METHOD would exist once the merge names it -- and the merge takes the
base trigger only.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnBeforeInsert()` on a tableextension: **77 declarations.**

## The choice

`Insert(RunTrigger)` gains one line before the existing one, under the same
`if constexpr (requires ...)` guard, so a table whose extensions declare none costs nothing:

```cpp
if constexpr (requires(Derived &r) { r.OnBeforeInsert(); }) { Self().OnBeforeInsert(); }
if constexpr (requires(Derived &r) { r.OnInsert(); })       { Self().OnInsert(); }
```

**The generator must emit the extension's trigger under a distinct member name.** board:0033 merges
a `tableextension` into the base class; two extensions each declaring `OnBeforeInsert` would collide
on one member. The merge names them per extension and the base class calls each in a declared order
-- which is the same ordering rule board:0053 needs for enum values.

## Ordering

After board:0033's merge can carry more than one trigger of a name. Before the bracket EVENTS
(0244), which fire outside this pair.

## Gate, and its negative control

A tableextension whose `OnBeforeInsert` raises: `Insert(true)` raises and NO row appears -- including
when the row would have been a duplicate, because the trigger ran first.

**The negative control is the duplicate.** A runtime that checks existence first reports a
duplicate-key error instead of the trigger's, and the test sees the wrong message.

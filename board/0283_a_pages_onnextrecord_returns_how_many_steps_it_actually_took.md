Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-onnextrecord-page-trigger.md, developer/triggers-auto/pageextension/devenv-onnextrecord-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnNextRecord` returns how many steps it ACTUALLY took

```al
trigger OnNextRecord(Steps: Integer): ActualSteps
```

`Steps` is how many rows to move, "a negative value indicates steps backwards", and the return value
is how many the trigger managed. **The difference is how the page learns it hit the end** -- asking
for 5 and getting 3 means three rows remained, which is the same contract `Record.Next(Steps)` has
(board:0056).

With `OnFindRecord` (0282) it completes the pair that lets a page present a set the database does
not hold.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnNextRecord(` on a page or pageextension: **95 declarations**, against 317 on
`OnFindRecord` -- so two thirds of the pages that override the find do NOT override the step, and
fall back to the record's own `Next`.

## The IST-state

No page runtime.

## The choice

The page's row source consults it under `if constexpr (requires ...)` and falls back to
`Rec.Next(Steps)` otherwise -- which is what the 222-page gap between the two populations requires:
overriding one must not disable the other.

## Ordering

Blocked on board:0030, with 0282.

## Gate, and its negative control

A page over a three-row buffer asked for five steps: the trigger returns 3 and the page stops at
the last row.

**The negative control is the return value** -- a runtime that ignores it and trusts `Steps` walks
past the end, which on a buffer is a read of whatever is next in memory.

Type:     task
Status:   open
Parent:   0016
Area:     rt, gen
Source:   developer/properties/devenv-closingdates-property.md
Verdict:  fehlt
Class:    activation

# A `ClosingDates` field accepts a closing date, and every other Date field refuses one

> Sets a value that determines whether users can enter a closing date in this field. The default
> value is False.
>
> **All dates have a corresponding closing date.** A closing date is a period following the given
> date, but before the next date. Closing dates are sorted immediately after the corresponding date
> but before the next date.

So the property is a PERMISSION and the default is refusal: 187 fields may hold a closing date and
every other Date field in 1 609 tables may not. That is the half board:0016 does not cover -- it owns
the ORDERING of a closing date under a filter and a key; this owns who may hold one.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ClosingDates =`: **187 declarations.** Small, and unavoidable: they are the fiscal-year-end fields
every closing entry lands on.

## The IST-state

Not in `FieldDef` (`include/meta/TableDef.h:67`). board:0016 owns the representation; nothing owns
the permission, and `System.ClosingDate(Date)` refuses the door at `src/rt/Builtins.cpp:87`.

## The choice

One bit on `FieldDef`, checked wherever a Date value reaches a field -- and here the UI-only rule of
its neighbours does NOT obviously apply: the page says "determines whether users can enter", but a
closing date that reached the column from AL is still a value the sort order has to handle. The
conservative reading is that the CHECK is at the UI and the REPRESENTATION is everywhere, which is
also what board:0016 assumes.

**Say what is unresolved rather than round it off**: whether `Rec."Posting Date" := ClosingDate(D)`
on a field without the property is legal. The documentation does not answer it; the AL source will,
and 187 fields is a population small enough to read.

## Ordering

Behind board:0016, which has to hold the value before anything can permit it.

## Gate, and its negative control

Entering `C31122026` through a `TestPage` succeeds on a `ClosingDates` field and raises on one
without the property.

**The negative control is the field WITHOUT the property** -- a runtime that accepts a closing date
everywhere passes the positive half and has no rule at all.

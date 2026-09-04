Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-freezecolumn-property.md
Verdict:  fehlt
Class:    activation

# `FreezeColumn` keeps the left of a wide list in view

> Specifies the columns in a list that remain in view on a page, even when you scroll right. Applies
> to: **Page Group.**
>
> To set the property, **you select a column. The column that you select AND ALL COLUMNS BEFORE IT
> remain in view.**

**The named column is the LAST frozen one, not the only one.** A property that reads like "freeze
this column" freezes a prefix, and an implementation that froze the single named column would leave
the columns to its left scrolling away -- which is visibly wrong and is exactly what the name
suggests.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`FreezeColumn =`: **37 declarations.**

Small, and every one of them is a wide document list where the line number and description must stay
visible.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

The generator resolves the named control to its INDEX in the repeater's control list, and the
descriptor carries the index -- so the renderer freezes a prefix by count and never searches for a
name.

**A `FreezeColumn` naming a control the repeater does not have is a `static_assert`.**

## Ordering

With board:0030's repeater rendering.

## Gate, and its negative control

Scrolling a list right leaves the named column and everything left of it in place.

**The negative control is the column before the named one** -- it must also stay, and an
implementation that freezes one column passes any gate that names the first column.

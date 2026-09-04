Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-hidevalue-property.md
Verdict:  fehlt
Class:    activation

# `HideValue` blanks a control while leaving it in place

> **Version**: runtime 3.2. Applies to: **Page Label, Page Field.**
>
> Sets whether to **show or hide a VALUE** for the user **based on an expression**.

**Three properties in this theme hide three different things and they must not be confused:**

| property | what disappears | what remains |
|---|---|---|
| `Visible` (board:0401) | the control | nothing -- not in the fragment |
| `Enabled` (board:0402) | the ability to type | the control and its value |
| `HideValue` | **the value** | the control, its caption, its place in the layout |

`HideValue` is what an indented tree list uses: the group row shows a total and the repeated
description is blanked so the eye follows the indentation. The cell is still there.

**And the value is an expression**, like board:0407's `ShowMandatory` -- re-evaluated per row, which
on a repeater means per row per render.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`HideValue =`: **184 declarations.**

Small, and concentrated where it would be: indented list pages and the FactBoxes that summarise them.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

A generated predicate on the control, evaluated where the cell's value is written, emitting an empty
cell rather than omitting the element.

**Per row on a repeater is the cost to watch.** 184 declarations is small, but each sits on a list
page and is evaluated once per visible row, so the predicate has to be cheap and the expression is
usually one field comparison -- which the generator can constant-fold when it is a literal.

## Ordering

With board:0407 -- both are control-level AL predicates evaluated at render time, and they share the
call site.

## Gate, and its negative control

A row where the expression is true renders an empty cell; the cell is still present and the column
keeps its width.

**The negative control is the cell's presence** -- an implementation that reuses board:0401's removal
shifts every following column left on that row, which no screenshot of a single row would show.

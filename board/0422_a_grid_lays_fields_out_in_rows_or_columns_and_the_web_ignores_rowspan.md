Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-gridlayout-property.md, developer/properties/devenv-columnspan-property.md, developer/properties/devenv-rowspan-property.md
Verdict:  fehlt
Class:    activation

# A grid lays fields out in rows or columns, and the web client ignores `RowSpan`

**Three pages, one item**: `GridLayout` declares the grid, `ColumnSpan` and `RowSpan` place a field
inside it, and both span pages name `GridLayout` as their context. Neither span means anything outside
a grid.

> **GridLayout** (Page Group): `Rows` or `Columns`. **This property is only supported on grids.** By
> default, fields in a FastTab are arranged automatically in two columns based on the number of
> fields; a Grid control customises that.
>
> **ColumnSpan** (Page Label, Page Field): the number of columns a field spans. **The field occupies
> the cells to its right, and existing fields in the occupied cells are moved to the right.**
>
> **RowSpan**: the number of rows. **IMPORTANT: The `RowSpan` property is not supported by the web
> client. If the page displays in the web client, then the property is IGNORED and the field will not
> span any rows.**

**`RowSpan` is a documented no-op here.** agiru has only a web client, so the property is declared,
parsed and deliberately not acted on -- and that has to be RECORDED, because "we implemented
`ColumnSpan` and not `RowSpan`" and "the web client ignores `RowSpan`" look identical in the output
and are completely different states.

The overflow rule is the part with content: a span pushes existing fields RIGHT rather than
overwriting them, so the grid's cell assignment is a placement algorithm and not a coordinate lookup.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`GridLayout =` **32** · `ColumnSpan =` **7** · `RowSpan =` **6**.

**Forty-five declarations in 2.56 million lines.** The grid control is rare, and its two span
properties are used seven and six times. That is the smallest theme in this sweep and it is filed at
its true size rather than skipped.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

A grid descriptor on the group and a `{ colSpan }` on the control -- `RowSpan` parsed, asserted to be
within range, and dropped with this item as the citation.

The placement algorithm runs in the GENERATOR: the grid's cell assignment is decidable from the
declarations, so the renderer emits a CSS grid with fixed positions and never computes an overflow.

## Ordering

With board:0030's group rendering. Low, on population.

## Gate, and its negative control

A grid with `GridLayout = Columns` and a field declaring `ColumnSpan = 2` renders that field over two
cells and moves the displaced field right.

**The negative control is `RowSpan`** -- a field declaring it must render spanning ONE row, matching
the web client, and an implementation that honours it is more capable than BC and differs from it.

Type:     task
Status:   open
Parent:   0064
Area:     gen, rt, db
Source:   developer/properties/devenv-orderby-property.md
Verdict:  fehlt
Class:    activation

# `OrderBy` sorts a view and a query, and a view takes only one direction

> **Version**: Available or changed with **runtime version 3.0**.
>
> Sorts table fields in the page view in ascending or descending order. Applies to: **Page View**,
> **Query**.
>
> `OrderBy = ascending (Name), descending (Quantity)` -- separate multiple columns with a comma.
>
> **You cannot sort on the same column more than once.**
>
> **For views you can only use one direction; either Ascending or Descending.**

Two objects, one syntax, and two different restrictions -- which is why this is one item and not two:
the parser and the emitted shape are shared, and only the validation differs.

**Both restrictions are decidable from the declaration**, so both are `static_assert`s: a repeated
column, and a page VIEW mixing `ascending` with `descending`. A query may mix them; a view may not.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`OrderBy =`: **113 declarations.**

## The IST-state

Queries are not generated (board:0064, board:0034) and page views are not either (board:0030). The
property has nowhere to land.

## The choice

A span of `{ FieldNo, direction }` on the view and on the query, resolved by the generator into the
`ORDER BY` the read emits. For a query it is the statement's own clause; for a page view it is the
sort the list opens with, which the user may then change.

**A page view's sort is not a `SetCurrentKey`.** It orders the rows the page shows and does not
select a key -- so if no index supports it, it is board:0045's "a sort of the table", and the
transpiler is the only place that could say so. Whether it should warn is a question this item asks
of board:0045 rather than answering alone.

## Ordering

Inside board:0064 for the query half and board:0030 for the view half. Neither exists yet.

## Gate, and its negative control

A query with `OrderBy = ascending (Name), descending (Quantity)` returns rows in that order and the
same order as the equivalent `ORDER BY` from `psql`. A view declaring both directions fails to
compile; so does one naming a column twice.

**The negative control is the mixed-direction view** -- a checker written once and applied to both
object kinds refuses a legal query, and one applied to neither accepts an illegal view.

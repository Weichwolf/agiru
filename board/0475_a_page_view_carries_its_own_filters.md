Type:     task
Status:   open
Parent:   0030
Area:     al, gen, rt
Source:   developer/properties/devenv-filters-property.md
Verdict:  fehlt
Class:    activation

# A page view carries its own filters

> **Version**: runtime 3.0. Applies to: **Page View.**
>
> ```
> Filters = [WHERE(<TableFilters>)]
> <TableFilter> ::= <RunObjectFieldName> = CONST(<FieldConst>) | FILTER(<FilterExpression>)
> ```
>
> The example is a `pagecustomization` adding a view named `BalanceLCY` to the Customer List.

**A page VIEW is a named, saved filter-and-sort the user picks from a list's view pane** -- the same
pane board:0460's `ClearViews` empties. This property is its filter half; `OrderBy` (board:0352) is
its sort half, and `Caption` (board:0382) names it.

The term grammar is the two-shape subset -- `CONST` and `FILTER`, no `FIELD` -- which is the same
subset `SubPageView` and `RunPageView` use (board:0430, board:0433) and NOT the six-shape one
`TableRelation` and `SubPageLink` use. **Two subsets of one grammar**, and the difference is whether
there is a parent record to read a field from: a view has none.

That distinction is worth recording once here, because a parser that accepted `FIELD(...)` in a view
would accept a declaration with nothing to resolve against.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Filters =`: **179 declarations.**

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; there is no view pane and board:0018's filter
parser does not exist.

## The choice

A `constexpr` span of filter terms on the view descriptor, parsed by board:0018's parser restricted to
the two-shape subset -- **restricted by the grammar, not by a run-time check**, so a `FIELD` term in a
view is a translation error.

## Ordering

Behind board:0018's filter parser. With board:0352 and board:0460, which are the view's other two
halves.

## Gate, and its negative control

Selecting a view applies its filters and its sort together; clearing the view restores the unfiltered
list.

**The negative control is a `FIELD` term in a view** -- it must fail to transpile, which is what
proves the two subsets are actually distinguished rather than one permissive parser being reused.

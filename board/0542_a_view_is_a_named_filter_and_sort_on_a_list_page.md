Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/devenv-views.md, developer/devenv-views-legacy.md, developer/devenv-view-table-data.md
Verdict:  fehlt
Class:    activation

# A view is a named filter and sort on a list page

**Three pages, one item**: the view object, its legacy form and the data-viewing page. board:0475 filed
the `Filters` property and board:0352 the `OrderBy`; **this is the object they belong to.**

> "Views ... **define a different view of the data on a given page.** Views can be defined for
> **Pages, Page Extensions, and Page Customization**."
>
> A view offers **filtering on multiple fields**, **sorting on multiple fields BUT ONLY IN ONE
> DIRECTION -- either ascending or descending**, and **layout changes** -- "modifying page columns,
> moving them, etc."
>
> **"Views are defined DIRECTLY IN CODE, on the list page that they modify. The defined views are
> available to the user through the FILTER PANE and appear IN THE SEQUENCE THAT THEY'RE DEFINED IN
> CODE."**
>
> ```al
> view(OnlyApproved) {
>     Caption = 'Approved';
>     Filters = where(Approved = const(true));
>     OrderBy = ascending("Balance (LCY)", Name);
> }
> ```

**Three things a view carries and only two have property items**: the filter (board:0475), the sort
(board:0352), and **LAYOUT CHANGES** -- which no property page covers. So a view may reorder and hide
columns, which makes it a partial page customization scoped to a named view, and board:0033's merge has
to produce that.

**The one-direction sort is board:0352's restriction seen from its home**: `OrderBy` on a query may mix
directions, on a view it may not. Same property, two rules, and board:0352 already asserts both.

**Source order again** -- views appear in declaration order, like board:0538's actions and board:0539's
area contents. Three places, one rule, and board:0033's extension merge needs an answer for all three.
board:0539 found that ACTIONS have anchors; **views do not**, so for views the order between extensions
must be declared by agiru.

> **"`allowDebugging` doesn't apply to views. Views defined in an extension with `allowDebugging` set
> to `false` can still be COPIED using Designer."**

A note about a tool, recorded and not actionable here.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0475: `Filters` **179**. board:0352: `OrderBy` **113**, pages and queries together.
board:0460: `ClearViews` **8**. **The `view(` block count is a declaration block, not a
`Name = Value` property, so this sweep's pattern does not reach it** -- stated rather than guessed, and
it is this item's first task.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; no views, no filter pane, no layout deltas.
board:0018's filter parser does not exist.

## The choice

A `constexpr` view list on the page descriptor, each carrying a caption, a filter term span
(board:0475), a sort (board:0352) and a layout delta. The renderer draws the filter pane from it and
applies the selected view's three parts to the list read.

**The layout delta is the same structure board:0033's page customization produces** -- so a view is a
customization with a name and a filter, and the two mechanisms share one representation rather than
having two.

## Ordering

Behind board:0018's filter parser and board:0537's list rendering. With board:0460's `ClearViews`,
which empties this list.

## Gate, and its negative control

Selecting a view applies its filter, its sort and its column layout together; views appear in
declaration order; a view declaring both `ascending` and `descending` fails to transpile.

**The negative control is the LAYOUT** -- an implementation that applies filter and sort and ignores the
column changes shows the right rows in the right order with the wrong columns, which every row-oriented
assertion passes.

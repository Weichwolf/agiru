Type:     task
Status:   open
Parent:   0063
Area:     gen, rt
Source:   developer/devenv-report-object.md, developer/devenv-report-dataset.md, developer/devenv-walktrough-designing-reports-multiple-tables.md, developer/devenv-get-report-parameters-with-virtual-tables.md, developer/devenv-testing-reports.md
Verdict:  fehlt
Class:    activation

# A report is a dataset, and a column may be an expression

**Five pages, one item**: the report object, its dataset, the multi-table walkthrough, the virtual-table
parameter trick and the testing page. board:0063 is the root and board:0450, board:0451, board:0452,
board:0454-0457 and board:0547 file its properties; **this is the object's own shape.**

## The section order is part of the syntax

> ```AL
> report ObjectId ReportName
> {
>     // properties
>     dataset {}
>     requestpage {}    // optional
>     rendering {}      // optional, but recommended for reports that have a layout
>     // AL code
> }
> ```
>
> **"The ORDER IN WHICH THE SECTIONS APPEAR MATTERS."**

**A syntactic ordering constraint on an object body**, which the parser must enforce rather than
accept in any order.

## Three report kinds, and only two have a layout

> - **analytical** -- output for online consumption
> - **document** -- output for print
> - **processing-only** -- **"there's NO OUTPUT. The report object is typically used WITH A REQUEST
>   PAGE to let the user set filters/options for the operation."**

board:0436 measured `ProcessingOnly` at **767** and concluded the batch-job shape is the common one.
**This page confirms it is a first-class kind**, not a switch on a report -- and that a processing-only
report still normally has a request page, which is why board:0454's filter fields matter to it.

## A column is not always a field

> "A data item is a table. **A column can be: a FIELD in a table, a VARIABLE, an EXPRESSION, or a TEXT
> CONSTANT.**"

**Four column kinds**, and three of them have no field behind them. So board:0396's `IncludeCaption`
only applies to the first kind, and board:0491's `AutoFormat` resolution needs a field to look up a
currency -- **a column that is an expression has neither.**

**That makes the dataset schema a tagged list**, like board:0548's XMLport nodes: four column shapes,
and the properties that apply depend on the shape.

## A query may be a data item, through a trick

> "Instead of building the report dataset directly from tables, you can also **use a QUERY object**. To
> achieve this, you must: **add a global variable that points to the query object; use an INTEGER in
> the data item definition; add `OnPreDataItem` and `OnAfterGetRecord` triggers.**"

**The `Integer` virtual table again** (board:0523) -- a report iterates it as a counter and the triggers
pull rows from the query. So board:0523's "`Integer` is how AL writes a `for` loop" has a second, named
use: **it is also how AL attaches a non-table source to a report.**

**And `devenv-get-report-parameters-with-virtual-tables.md` is a third such trick**, which is why the
two pages are one item: the `Integer` and `Date` virtual tables are the report's escape hatch from
table-shaped data.

## Relations between data items

> "When a report is based on **more than one table**, you must set relations between the data items so
> that you can retrieve and organize the data."

board:0450 filed `DataItemLink` and `DataItemLinkReference` and recorded the documentation's own
statement that a link IS a `SetRange`. This page is the same fact from the object's side.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

CLAUDE.md: **668 reports in scope**, 1 508 objects total. board:0436: `ProcessingOnly` **767**.
board:0451: `DataItemTableView` **7 710**. board:0452: `RDLCLayout` **768**.

**The `dataitem(` and `column(` block counts are declarations, not properties** -- stated rather than
guessed. `DataItemTableView`'s 7 710 is the closest proxy: roughly eleven data items per report.

## The IST-state

board:0063: reports have no generator (board:0034). `src/al/Parser.cpp` parses AL; **whether it parses
`dataset`, `dataitem`, `column`, `requestpage` and `rendering` is this item's first check** and is not
measured here.

## The choice

A report descriptor with an ordered data-item tree, each item carrying its table, its link
(board:0450), its view (board:0451) and a **tagged column list of four shapes**. The `requestpage` and
`rendering` sections are optional members.

**The section order is a parser rule, not a generator one** -- it is syntax, and accepting any order
would accept AL that BC rejects.

**A processing-only report never reaches the rendering path** (board:0436), so the descriptor's
rendering member is absent rather than empty for 767 reports.

## Ordering

board:0063's core, before board:0547's layouts -- the dataset is what a layout binds to. Behind
board:0523's `Integer` virtual table, which is how a query becomes a data item.

## Gate, and its negative control

A report with two linked data items emits, for each parent row, only its children; a column that is an
expression appears in the dataset with the computed value; a report whose sections are out of order
fails to parse.

**The negative control is the expression column** -- an implementation that assumes every column has a
field behind it either drops it or crashes looking up its caption, and a fields-only fixture never
shows it.

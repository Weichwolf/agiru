Type:     task
Status:   open
Parent:   0064
Area:     gen, db
Source:   developer/devenv-query-object.md, developer/devenv-query-links-joins.md, developer/devenv-api-querytype.md
Verdict:  fehlt
Class:    activation

# A query nests its data items, and the hierarchy is the join order

**Three pages, one item**: the query object, its links and joins, and the API query type. board:0461
filed `SqlJoinType` and board:0453 the filters; **this is the object they hang on.**

> **"The HIERARCHY of the `dataitem` and `column` controls is IMPORTANT because it DETERMINES THE
> SEQUENCE IN WHICH DATA ITEMS ARE LINKED, which in turn controls the results."**
>
> "Working from **top-to-bottom**, you start by adding the `dataitem` for the first table ... For the
> next table, you add another `dataitem` **EMBEDDED WITHIN the first**."
>
> **"Both properties must be set ON THE LOWER DATAITEM."**
>
> ```AL
> dataitem(DataItem1; Table1) {
>   column(Column1; Field1) { }
>   dataitem(DataItem2; Table2) {
>     DataItemLink = FieldY = DataItem1.FieldX;
>     SqlJoinType = InnerJoin;
> ```

**The nesting IS the join tree**, and the link and the join type live on the CHILD. So a query's
`SELECT` is assembled by walking the tree depth-first, each level contributing one `JOIN` clause with
its own type -- and board:0461's five join types are per level, not per query.

**`DataItemLink = FieldY = DataItem1.FieldX`** names the parent data item by its ALIAS, not by table --
so two data items over the same table are distinguishable, which a table-keyed representation would
lose.

## Two query types and only one is a UI object

> "There are **two types**: **normal** and **API**. ... **API query objects are used to generate web
> service endpoints and CAN'T BE DISPLAYED IN THE USER INTERFACE.**"

board:0464 measured `QueryType =` at **346**, all necessarily `API`, and concluded most queries in the
BaseApp are endpoints. **This page confirms the two are mutually exclusive in where they can appear** --
so board:0464's finding is a real split of the population, not a labelling.

## The codeunit is the simplest object and carries `this`

> A codeunit "is a **container for AL code** ... You typically implement business logic in codeunits."
>
> The example shows `TableNo = Customer` with an `OnRun` taking `Rec` (board:0489), callable **both** as
> `codeunit.run(customer)` **and** as `createcustomer.CheckSize(customer)`.
>
> **"The `this` keyword can be used in codeunits in AL as a SELF-REFERENCE, and it allows PASSING THE
> CURRENT OBJECT AS AN ARGUMENT to methods."**

**`this` in AL is a value, not a pointer** -- it is passed as an argument, which in C++ means `*this`
and a reference parameter. board:0037 owns the codeunit handle and this is what it must support.

**And a codeunit is reachable two ways**: `Codeunit.Run` (board:0077, with its commit-and-raise
semantics) and a direct procedure call (ordinary). **The same object, two entry points with different
transaction behaviour** -- which board:0077 records and which this page states plainly.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0464: `QueryType` **346**. board:0461: `SqlJoinType` **216**. board:0453:
`DataItemTableFilter` **230**. board:0489: `TableNo` **1 341**.

**The `query` and `codeunit` object counts are declarations** -- CLAUDE.md records the tree at 9 300 AL
objects and 1 508 in scope. Stated rather than guessed.

## The IST-state

board:0064: queries have no generator (board:0034). `src/gen/CodeunitWriter.cpp` generates codeunits
and consumes `TableNo` and `Subtype` (board:0489, board:0472). **Whether `this` is supported is
board:0037's question and is not measured here.**

## The choice

A query descriptor as a data-item TREE, each node carrying its table, alias, columns, link terms and
join type, and the `SELECT` assembled by a depth-first walk -- **one `JOIN` per level, in tree order.**

**Not a flat list with parent pointers**: the documentation says the hierarchy determines the sequence,
so the tree IS the semantics and flattening it would need the order stored separately.

`this` becomes `*this` and a reference parameter.

## Ordering

board:0064's core, with board:0461's joins and board:0453's filters -- the three assemble one
statement. `this` with board:0037.

## Gate, and its negative control

A three-level query emits two joins in tree order with the declared types; two data items over the same
table are distinguished by alias in the link.

**The negative control is the two-data-items-one-table case** -- an implementation keyed by table id
resolves both links to the same node and produces a self-join or a wrong one, and every
distinct-tables fixture passes.

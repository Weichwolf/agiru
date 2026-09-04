Type:     task
Status:   open
Parent:   0064
Area:     gen, db
Source:   developer/properties/devenv-sqljointype-property.md
Verdict:  fehlt
Class:    activation

# A query joins its data items five ways

> Sets the data item link type between data items in a query. Applies to: **Query Data Item.**
>
> `LeftOuterJoin` · `InnerJoin` · `RightOuterJoin` · `FullOuterJoin` · `CrossJoin`
>
> **"When setting up a data item link between two data items, you always set up the `SqlJoinType`
> property ON THE LOWER DATA ITEM."**
>
> **"IMPORTANT: Cross Join does not require any comparisons between fields of data items, so the
> `DataItemLink` property MUST BE LEFT BLANK."**
>
> "Except for `CrossJoin`, the property works together with `DataItemLink` ... The `DataItemLink`
> property sets up an 'equal to' (=) comparison between two or more fields."

**This is the one place in AL where a real SQL join is declared**, and it is the difference between a
query and a report: board:0450 records that a REPORT's data item link is a `SetRange` and a nested
loop; a QUERY's is a join, and the property names which. Same property NAME on two object kinds, two
completely different executions -- the `Scope` situation (board:0361) again, and here the divergence is
in the semantics rather than the values.

**`CrossJoin` with a `DataItemLink` is a `static_assert`**: the documentation says the link must be
blank, and both are declarations.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`SqlJoinType =`: **216 declarations.**

## The IST-state

Queries have no generator (board:0064, board:0034).

## The choice

A five-valued enumerator on the data item, emitted straight into the `SELECT`'s `JOIN` clause --
PostgreSQL has all five, so this is one of the few properties in the sweep that translates exactly and
needs no divergence note.

**The join is SQL's and not the runtime's.** A query streams (board:0045); joining in the read loop
would be the predecessor's shape and would defeat the object's whole purpose.

## Ordering

Inside board:0064, with the data-item list and board:0453's filters -- one `SELECT` is assembled from
all of them.

## Gate, and its negative control

A `LeftOuterJoin` query returns every row of the upper data item including those with no match; the
same query as `InnerJoin` returns fewer.

**The negative control is the unmatched row** -- the two join types differ only there, and a fixture
where every row matches gives the same answer for all five.

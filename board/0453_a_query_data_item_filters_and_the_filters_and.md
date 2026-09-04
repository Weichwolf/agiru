Type:     task
Status:   open
Parent:   0064
Area:     al, gen, rt, db
Source:   developer/properties/devenv-dataitemtablefilter-property.md, developer/properties/devenv-columnfilter-property.md, developer/properties/devenv-dataitemlink-query-property.md
Verdict:  fehlt
Class:    activation

# A query's filters combine with AND, and `SetFilter` overwrites one of them

**Three pages, one item**: the two filter properties and the query's own data-item link. The
`DataItemTableFilter` page defines the combination rule for all of them, so they cannot be read apart.

> **DataItemTableFilter** (Query Data Item): filters on fields of the underlying table. **"You can
> filter on ANY field in the table, not just those included as columns."**
>
> **ColumnFilter** (Query Column): a filter on a column.
>
> **"A filter set by the `ColumnFilter` property, `SetFilter`, or `SetRange` is JOINED to the filter
> set by `DataItemTableFilter`. In logical terms, and in SQL SELECT statements, this combination
> corresponds to an AND operator."**
>
> Example: `DataItemTableFilter` says `< 50` and `ColumnFilter` says `> 20` -- the result is
> `20 < value < 50`. **"If `SetFilter` is called from AL and sets `> 10`, then `SetFilter` OVERWRITES
> the `ColumnFilter` property"** and the result is `10 < value < 50`.
>
> **"In an SQL SELECT statement, a filter set by `DataItemTableFilter` would correspond to a WHERE
> clause."**

**Three filter sources and two different combination rules**, stated in one paragraph:
`DataItemTableFilter` ANDs with everything and cannot be removed; `ColumnFilter` ANDs with it but is
REPLACED by an AL `SetFilter` on the same field. An implementation with one filter list per field
gets the second wrong, and the failure is a wider result set that looks plausible.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`DataItemTableFilter =` **230** · `DataItemLink =` **2 023** (reports and queries together,
board:0450) · `ColumnFilter` measured with the query theme.

## The IST-state

Queries have no generator (board:0064, board:0034); board:0018's filter parser does not exist.

## The choice

The query descriptor keeps the two property-declared filters in SEPARATE spans -- the separation is
the overwrite rule -- and the runtime's `SetFilter` replaces the column span's entry for that field
while the data-item span is untouchable.

Both are emitted into one `WHERE` clause, which is what the documentation says they are.

## Ordering

Inside board:0064. Behind board:0018's filter parser.

## Gate, and its negative control

A query with `DataItemTableFilter = "x" = filter(<50)` and `ColumnFilter = filter(>20)` returns rows
with `20 < x < 50`; calling `SetFilter` with `>10` returns `10 < x < 50`.

**The negative control is the `SetFilter` case** -- an implementation that ANDs all three returns
`20 < x < 50` again, which is a subset of the right answer and passes any "the rows are correct" gate
that does not count them.

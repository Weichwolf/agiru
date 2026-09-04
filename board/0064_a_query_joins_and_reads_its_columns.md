Type: arc
State: open
Area: al, gen, rt, db
Tags: target

# A query joins its dataitems and reads its columns, and the join it does not declare is LEFT OUTER

`Query` has no parser, no writer and no door header. `methods-auto/query/` holds 22 signature pages
-- `Open`, `Read`, `Close`, `SetFilter`, `SetRange`, `GetFilter`, `GetFilters`, `TopNumberOfRows`,
`ColumnName`, `ColumnNo`, `ColumnCaption`, `SecurityFiltering`, `SaveAsXml`, `SaveAsCsv`,
`SaveAsJson` -- and the door carries none of them. **17 of the 22 are `queryinstance-*` pages the
completeness counter never reads** (board:0059), so the surface baseline records this type as five
methods.

**164 `.Query.al` files in the read roots** (base 154, system 7, system_test_library 3), and one
`[Test]` procedure of the 2 305 declares a `Query` variable. So this is a TARGET item, ranked with
board:0063 and behind board:0057.

## What the platform documents

`devenv-query-object.md`, `devenv-query-links-joins.md`, `devenv-query-totals-grouping.md`,
`devenv-query-accessing-columns.md`, `devenv-query-filters.md`.

- A query is `elements { dataitem(name; Table) { column(name; Field) filter(name; Field) } }` --
  **the same `<kind>(<name>[; <source>]) { ... }` grammar** board:0034 found under the page layout,
  the report dataset and the xmlport schema. One reader, four headers.
- **The dataitems NEST rather than sit side by side**: the second is written INSIDE the first, the
  third inside the second, and `DataItemLink` and `SqlJoinType` are set on the LOWER one. So the
  nesting IS the join order, and it is what makes the linking cumulative -- the third dataitem joins
  the result of the first two, which the page states and the syntax enforces.
- **`FlowFields and FlowFilters cannot be used in SQL join operations`**
  (`coverage/diagnostics.md`): a `DataItemLink` across a FlowField is refused, not slow.
- **`SqlJoinType` DEFAULTS TO `LeftOuterJoin`**, in as many words: "By default, the SqlJoinType
  property is `LeftOuterJoin`, so if you omit this property, a `LeftOuterJoin` is performed." The
  five values are `InnerJoin`, `LeftOuterJoin`, `RightOuterJoin`, `FullOuterJoin`, `CrossJoin`.
- `DataItemLink` is an equality between a lower dataitem's field and an upper one's column, and
  **the linking is CUMULATIVE**: "lower data items are linked to the resulting dataset of the linked
  data items above it", not pairwise to the one directly above.
- Totals: a `column` carries `Method = Sum|Count|Min|Max|Average` and every non-aggregated column
  then becomes a grouping column -- a `GROUP BY` derived from the element list rather than declared.
- `filter(...)` elements are FLOWFILTER-shaped: a filter the caller sets by name, which the query
  applies to the dataitem it belongs to.

**A query is the one AL object that is already a SQL statement**, which is what makes it worth
having early and cheap to get subtly wrong.

## What the predecessor paid for

| item | finding | measured |
|---|---|---|
| **WI-1227** | a dataitem with no `SqlJoinType` was generated as **INNER** instead of LEFT OUTER -- the default in the property page, not the one in the property | 2 254 -> 2 255 |
| **WI-1133** | the query's own procedures were never emitted, so a query object became a **silent no-op** | GAINED 4 |
| **WI-1129** | a runtime filter that did not apply to any dataitem was **dropped silently** rather than refused | -- |

All three are the same class: a query that returns rows nobody can tell are the wrong rows. An inner
join where AL says left outer drops exactly the records the report using it was written to show.

## The choice

- **The parse is the layout grammar** (board:0034), so the work is a header, a writer and the
  property table -- not a grammar.
- **The generated form is a `constexpr` STATEMENT SHAPE, not a string.** The dataitems, their links,
  their join types and the column list are knowable at translation time and belong in `.rodata` like
  every other piece of object metadata; what is built at run time is the WHERE clause from the
  filters, which is board:0018's parser and board:0044's binder, unchanged.
- **`Open`/`Read`/`Close` is a CURSOR**, which board:0045 already builds for `FindSet`: `Read` steps
  it, and a query over a large join must not materialise.
- **An omitted `SqlJoinType` is written as `LEFT OUTER` explicitly in the emitted statement**, so the
  default is visible in the artefact rather than implied by whoever wrote the emitter (WI-1227).
- **A filter naming no dataitem is a translation ERROR**, not a dropped clause (WI-1129).

## Gate

Two dataitems, rows in the upper with no match below: the default join returns them with the lower
columns empty, and `SqlJoinType = InnerJoin` drops them. A third dataitem links to the RESULT of the
first two and not to the second alone. A `Method = Sum` column groups by every other column.
`TopNumberOfRows` limits and `Read` walks. A filter naming a column no dataitem has fails the
translation.

**Negative control**: emit `INNER JOIN` for the omitted `SqlJoinType` and require the first case to
go red -- that is the exact defect WI-1227 records, and a gate whose fixtures all match on both
sides cannot see it.

## THE AGGREGATES, THE IMPLICIT GROUP BY, AND A FILTER PRECEDENCE WITH ONE IMMOVABLE LEVEL

Four root pages read in full 2026-09-04 (board:0071): `devenv-query-object`, `-links-joins`,
`-totals-grouping`, `-filters`.

**Five aggregates, and one of them truncates.** `Method` on a `column` takes `Sum`, `Average`, `Min`,
`Max` or `Count`; all but `Count` require the field to be `Decimal`, `Integer`, `BigInteger` or
`Duration` -- the same four board:0047's `Sum` takes. And:

> When averaging fields that have an integer data type (such as `Integer` or `BigInteger`),
> **integer division is used. The result isn't rounded, and the remainder is discarded. For example,
> 5÷2=2 instead of 2.5.**

**That is the opposite of board:0088's rule for AL's `/`**, which yields a Decimal whatever its
operands. So a query's `Average` over integers truncates and `Sum / Count` in AL does not -- two
places, two answers, both documented, and an implementation that shares one helper between them is
wrong in one of them. It is exactly the kind of pair a gate has to hold side by side.

**Setting an aggregate GROUPS implicitly**: "Setting an aggregate method on a column will
automatically group the resultant data set by the other columns in the query ... It's similar to the
GROUP BY clause in SQL SELECT statements." So the `GROUP BY` is DERIVED from which columns carry a
`Method` and which do not -- no `group by` clause exists in AL, and the generator computes it.

**Three filter kinds, and the precedence has one level AL cannot reach:**

| where the filter is declared | overwritten by `SetFilter` / `SetRange`? |
|---|---|
| `DataItemTableFilter` on a `dataitem` -- may name ANY field of the table, not only a column | **NO** -- "A data item filter can't be overwritten from AL code" |
| `ColumnFilter` on a `column` | yes |
| `ColumnFilter` on a `filter` control (a filter ROW -- a field filtered but not returned) | yes |

**The immovable one is the finding.** A `DataItemTableFilter` is a permanent narrowing of the
dataitem, so it is part of the query's DEFINITION and belongs in the `WHERE` the generator emits;
the other two are per-instance state that `SetFilter` replaces, and they belong beside the record's
filters (board:0018). Treating all three the same way makes the first overridable, which silently
widens a query somebody wrote to be narrow.

**Five join types** -- `InnerJoin`, `LeftOuterJoin`, `RightOuterJoin`, `FullJoin`, `CrossJoin` --
set by `SqlJoinType` beside `DataItemLink`, and this item already records that the DEFAULT is
`LeftOuterJoin`. `devenv-query-links-joins.md` gives a worked four-row example with a NULL
salesperson code, which is the case that distinguishes inner from left outer and is therefore the
gate.

`devenv-query-retrieve-date-data.md` adds one more column-level transform: a **date method** on a
column returns only the Year, Month or Day of a date field, which is a `date_part` in the emitted
SQL and another reason the column list is a projection rather than a field list.

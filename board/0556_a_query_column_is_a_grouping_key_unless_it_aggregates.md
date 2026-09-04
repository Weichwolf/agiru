Type:     task
Status:   open
Parent:   0064
Area:     gen, db, rt
Source:   developer/devenv-query-totals-grouping.md, developer/devenv-query-filters.md, developer/devenv-query-retrieve-date-data.md, developer/devenv-query-accessing-columns.md, developer/devenv-query-using-instead-record-variables.md
Verdict:  fehlt
Class:    silent-wrong-data

# A query column is a grouping key unless it aggregates

**Five pages, one item**: the aggregation model, the filter model, the date methods, the column read
and the worked comparison against record variables. board:0550 owns the join tree, board:0453 the two
filter properties and board:0462 the `Method` property; **this is the SELECT those three assemble
into**, and it corrects one of 0462's gates.

## The rule the property page does not state

**There is no `GROUP BY` in AL.** Setting an aggregate method on one column "will AUTOMATICALLY GROUP
the resultant data set BY THE OTHER COLUMNS in the query" -- so the grouping key is derived by
exclusion, and the derivation has an exception:

> **"A column that applies a DATE METHOD IS STILL PART OF THE GROUP, unlike columns that apply an
> aggregate method."**

**So `Method` is two property families with opposite grouping behaviour**, and the split is not
visible in the property's name:

| method | in `GROUP BY` | result type |
|---|---|---|
| `Day`, `Month`, `Year` | **yes** -- the DERIVED value is the key | `Integer` |
| `Sum`, `Average`, `Min`, `Max` | no | **`Decimal`** |
| `Count` | no | `Integer` |
| (none) | yes | the field's own type |

**The result type is the method's, not the field's**: "the data type is an INTEGER for the `Count`
method and a DECIMAL FOR ALL OTHER TOTALING METHODS." A `Sum` over an `Integer` column yields a
`Decimal` -- which matters here more than in most systems, because CLAUDE.md forbids a binary float
carrying an amount and `agiru::Decimal` is the type that must come out.

**`Count` has no source field at all.** "The `column` element definition CANNOT include a source
table; only a name", and "records are identified and counted based on the PRIMARY KEY of the data item
table" -- so it is `COUNT(*)` over the data item, not over a column.

**And a `Count` column with its value thrown away is the documented idiom for `SELECT DISTINCT`**,
which AL has no other way to express. So a query whose `Count` column is never read is not dead code;
it is the grouping being requested. An optimiser that dropped an unread column would change the row
count.

## The filter model is a lattice with four positions

| mechanism | filters on | dynamic | against `SetFilter`/`SetRange` | SQL |
|---|---|---|---|---|
| `DataItemTableFilter` on a `dataitem` | **any field of the table**, column or not | **no** | **COMBINED with AND** | `WHERE` |
| `ColumnFilter` on a `column` | a field in the dataset | yes | **overwritten** | `WHERE` |
| `ColumnFilter` on a `column` carrying a totals method | the aggregate | yes | overwritten | **`HAVING`** |
| `ColumnFilter` on a `filter` row | **a field NOT in the dataset** | yes | overwritten | `WHERE` |

board:0453 has the AND rule from the property page; **the two things it does not have are on this
page**: that a filter on an AGGREGATED column becomes a `HAVING` rather than a `WHERE`, and that a
`filter` row exists precisely to filter on a field the dataset does not carry -- "you might want to
filter a date field on a specific date, but you don't want to include the date in the dataset."

**`SetFilter` and `SetRange` address a column BY ITS NAME**, not by field, and may be called either
from the caller or **from the query's own `OnBeforeOpen` trigger**, where the query names itself
`currQuery`.

**A `filter` row is a TYPED member on the query variable even though it is not in the dataset.** The
worked example writes `ItemMovements.SetRange(Entry_Type, ItemMovements.Entry_Type::Sale)` -- so the
filter row carries the source field's OPTION SCOPE and resolves `::Sale` through it. That is the same
option-member mechanism a record field has, on a control the dataset never returns, and it is why a
filter row cannot be a bare name in a filter list.

**`TopNumberOfRows` is a property AND a method**: declared on the object, and set at run time as
`ItemMovements.TopNumberOfRows(5)` before `Open`. Four declarations against a method the example
prefers, so the run-time form is the one that matters.

## Reading is a state, and a column exists only inside it

> "If the query is IN THE READING STATE, you can retrieve the value of columns in the CURRENT ACTIVE
> ROW ... A column of a row can only be accessed after the query has been opened by using a call to
> `Open` followed by a call to `Read`."

`ColumnValue := QueryVariable.ColumnName` -- so a query variable has a member per column whose type is
the table above, and reading it outside the `Open`/`Read` window is an error rather than a blank. This
is CLAUDE.md's streaming requirement in its natural form: `Open`, `Read` until false, `Close`, one row
in hand at a time.

## The date methods are computed in UTC, and that is a documented divergence

> "The day, month, or year is calculated ON THE SQL SERVER, and then returned to the query dataset as
> an integer, WHICH DOESN'T CONSIDER THE REGIONAL SETTINGS."

| time zone | value in BC | `Day` | `Month` | `Year` |
|---|---|---:|---:|---:|
| Pacific (UTC -8) | 12-31-2025 17:00:00 | 31 | 12 | 2025 |
| Middle European (UTC +1) | 01-01-2025 00:59:00 | 1 | 1 | 2025 |

Both rows return the LOCAL day, so the conversion runs the other way than the prose suggests -- the
value is stored as UTC and the extraction happens on the stored value. The page's own advice is to
avoid the date methods on `DateTime` entirely. **agiru reproduces the behaviour and does not fix it**:
extract on the stored value, no session time zone applied.

## THIS CORRECTS board:0462

board:0462 states, from `devenv-method-property.md`: *"If the day in the date expression is 0, then 1
is returned ... if the year is 0, then 1900 is returned"*, and its gate is **"a `Year` method on a zero
date returns 1900."**

**That gate is wrong for the version this tree translates.**

| BC version | `Day` | `Month` | `Year` |
|---|---:|---:|---:|
| **26 and earlier** | 1 | 1 | 1753 |
| **27 and later** | **0** | **0** | **0** |

`~/Git/BCApps` on `main` is **30.0** and the demo database is 28.4, so both are on the 27-and-later
side and **the answer is 0, 0, 0.** The property page describes the old behaviour and the concept page
carries the version table; neither is wrong, and 0462's gate has to move. Note the two rules are also
about different inputs -- "a date expression whose day component is 0" against "a BLANK date, `0D`,
stored in SQL as 01-01-1753" -- and the 1753 in the old row is exactly that SQL sentinel leaking
through. Under 27-and-later it no longer leaks.

**board:0462's negative control survives unchanged** and is the better half of that item: `Average`
over a `Decimal` column must NOT truncate.

## The rename the page contradicts itself about

The `Min` and `Max` sections say **"the name of the `Quantity` column AUTOMATICALLY CHANGES to
`Min_Quantity`"** -- and the result table printed directly beneath each says the column is `Qty`, the
declared name. The `Sum` and `Average` sections show no rename at all. The date page's tables DO show
the prefix: `Day_Order_Date` for a column declared `Order_Date`.

**The source settles the AL half and nothing else.** Measured 2026-09-04 over `~/Git/BCApps/src` by
`\.(Min|Max|Day|Month|Year)_[A-Za-z0-9_]+`: **zero references, for all five prefixes.** And the source
disambiguates by hand instead -- `Quality Management/QltyInspectionValues.Query.al` declares
`finishedAtDay`, `finishedAtMonth`, `finishedAtYear` over one `Finished Date` field, and
`CustomerInteractionStats.Query.al` declares `MaxEntryNo`. **A platform rename would break every one
of those**, since `MaxEntryNo` would have to be read as `Max_Entry_No_` and nothing in 2.56 million
lines does.

**And the documentation's own worked example agrees.**
`devenv-query-using-instead-record-variables.md` declares `column(Sum_Quantity; Quantity) { Method =
Sum; }` and reads it back as `ItemMovements.Sum_Quantity` -- the prefix is written BY HAND into the
declared name, exactly as `MaxEntryNo` is in the source, and the read uses the declared name unchanged.
A platform that renamed would have produced `Sum_Sum_Quantity`.

**So: AL sees the DECLARED name.** Whether the exposed OData/API column carries a prefix is settled by
neither the documentation nor the source, and it is recorded as unsettled rather than guessed -- it
costs nothing now, because board:0550's `API` query type is where it would be observable.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`query <id>` **464** objects. Properties by the standard `(^|[{;])\s*<Name>\s*=` pattern:

| | count |
|---|---:|
| `DataItemLink =` | 2 023 |
| `DataAccessIntent =` | 367 |
| `QueryType =` | 346 -- `Normal` 233, `API` 113 |
| `Method =` | **268** |
| `DataItemTableFilter =` | 230 |
| `SqlJoinType =` | 216 -- `InnerJoin` 161, `LeftOuterJoin` 46, `CrossJoin` 8, `FullOuterJoin` 1 |
| `ColumnFilter =` | 214 |
| `QueryCategory =` | 136 |
| `OrderBy =` | 113 |
| `TopNumberOfRows =` | **4** |

`Method` by value: **`Sum` 223, `Count` 36, `Year` 2, `Month` 2, `Max` 2, `Day` 2, `Min` 1** -- summing
to 268, so the census is complete.

**`Average` is declared ZERO times.** So the integer-truncation trap board:0462 exists for has no call
site in this tree today. It stays, because the documentation is the specification and a documented
behaviour without a gate case is a gap -- but the ORDER it earns is last among the seven, and that is
what the number is for.

**Two counts are NOT SEPARABLE by object kind and are given as they are.** `column(` **105 669** and
`dataitem(` **9 046** count report, query and XMLport controls together, because the pattern sees a
line and not an enclosing object. The query share is smaller than both by orders of magnitude -- 464
query objects against 668 reports -- and no rounded guess is offered in its place. `filter(` **669**
has the same problem from the other side: a `DataItemTableFilter` value contains the token `filter(`,
and a wrapped line puts it at column 0.

`QueryType` at 346 against 464 objects leaves **118 queries with no `QueryType`**, taking the default.

## The IST-state

**There is no query anywhere in the front end.** `grep -n QueryObject src/al/Ast.h` is empty -- the AST
has `TableObject`, `PageObject`, `PageExtensionObject` and the rest, and no query node. `ObjectKind::Query`
exists (`src/gen/Scope.h:16`) and maps to the string `"query"` (`src/gen/Names.cpp:72`,
`src/gen/Scope.cpp:120`), and `src/gen/CodeunitWriter.cpp:129` knows `Query` as a VARIABLE type -- so
a codeunit holding `MyQuery: Query "..."` is recognised while the object it names is not parsed.

That is the hole board:0034 counts, seen from the inside: the name exists, the object does not, and
nothing reports the difference.

## The choice

**One `SELECT` assembled from the tree in a single pass, and the `GROUP BY` is a partition of the
column list rather than a separate structure.**

```cpp
enum class QueryMethod : std::uint8_t { None, Sum, Average, Min, Max, Count, Day, Month, Year };

constexpr bool Aggregates(QueryMethod m) {
  return m == QueryMethod::Sum || m == QueryMethod::Average || m == QueryMethod::Min ||
         m == QueryMethod::Max || m == QueryMethod::Count;
}
```

and the `GROUP BY` is every column with `!Aggregates(method)` -- which puts the date columns in by
construction, because `Day`, `Month` and `Year` are not aggregates. **The exception in the
documentation becomes the absence of a special case**, which is the shape worth having: a boolean per
column would have needed the rule written down twice.

**`constexpr` and checked at translation time:**

- `Aggregates` is `constexpr`, so the partition is computed once per query and lands in `.rodata`.
- **`static_assert` that a `Count` column names no field** -- the platform refuses it, so the
  transpiler refuses it.
- **`static_assert` that a `Day`/`Month`/`Year` column's field is `Date` or `DateTime`** -- likewise
  refused by the platform, and both types are known to the generator.
- **`static_assert` that at most one filter mechanism per field is DYNAMIC.** The static one combines;
  two dynamic ones on one field would race for the same slot.

**`Average` over an integer column emits integer division, never `avg`** -- board:0462's finding, and
it stands: PostgreSQL's `avg(integer)` is `numeric` and would hand back 2.5 where BC gives 2.

**Filters land in two buckets decided by `Aggregates(method)` of the column they name**: `WHERE` for
the rest, `HAVING` for an aggregated one. The bucket is `constexpr` for a property filter; a
`SetFilter` at run time re-uses the same decision, because the column's method does not change.

**A `filter` row is a column that is filtered and not selected.** So it is the same `ControlDef` with
a `selected` flag off -- not a second structure -- which keeps `SetFilter`'s by-name lookup over one
list.

**Reading is a cursor, per CLAUDE.md's streaming requirement.** `Open` declares it, `Read` fetches one
row, `Close` drops it; the column members are written from the fetch buffer and reading one before
`Read` is an error rather than a blank.

## Ordering

**Inside board:0064, after board:0550's join tree** -- there is no `SELECT` to filter until the `FROM`
exists -- **and before board:0453**, whose two properties are positions in the lattice above.

**`Sum` and `Count` are 259 of the 268 declarations, so they are the whole first pass.** `Min`, `Max`
and the three date methods are 9 between them and `Average` is 0; they follow, in that order, and the
order is the measurement rather than the alphabet.

## Gate, and its negative control

Over two customers with three and two sales lines:

1. `column(No.)`, `column(Name)`, `column(Qty; Quantity) { Method = Sum; }` gives ONE ROW PER CUSTOMER
   with the summed quantity -- the `GROUP BY` was never written down
2. adding `column(D; "Order Date") { Method = Day; }` **splits the rows by day** -- the date column
   entered the group
3. `column(C) { Method = Count; }` over a query with no other aggregate returns the row count per
   group and needs no source field
4. `ColumnFilter` on the summed column becomes a `HAVING` and removes a GROUP, not a row
5. `DataItemTableFilter = Quantity = filter(> 10)` plus `SetRange(Qty, 0, 5)` yields the AND of both,
   which is empty -- the static filter was not overwritten
6. a `Year` method over a blank date returns **0**

**The negative control is case 2, and it is the whole reason this item is separate from board:0462.**
Treat `Day` as an aggregate -- the reading a single "is this column a method column" flag produces --
and case 2 collapses back to one row per customer while cases 1, 3, 4, 5 and 6 all stay green. A gate
that checks only that `Sum` sums proves nothing about the partition.

**Second control, for case 5:** make `DataItemTableFilter` overwritable. Case 5 goes red and every
other case stays green -- and case 5 is the only one where a wrong answer is a plausible number rather
than an error.

## Class

`silent-wrong-data` on every count above: a wrong `GROUP BY` returns rows, a truncating `Average`
returns a number, an overwritten static filter returns more rows than BC would. Nothing throws. That
is the class this item is filed under and the reason it outranks its own population.

Type:     task
Status:   open
Parent:   0064
Area:     gen, db
Source:   developer/properties/devenv-method-property.md
Verdict:  fehlt
Class:    activation

# A query column groups and aggregates, and `Average` truncates on integers

> Sets either a **date method** for retrieving the year, month or day from a date field, or a **totals
> method** for performing calculations. Applies to: **Query Column.**
>
> **Date methods** -- `Day`, `Month`, `Year`, usable only on `Date` and `DateTime` fields. **"If the
> day in the date expression is 0, then 1 is returned"**; the same for month; **"if the year is 0,
> then 1900 is returned."**
>
> **Totals methods** -- `Sum`, `Count`, `Average`, `Min`, `Max`.
>
> **"The `Count` method is associated with the DataItem and not with a specific column, so the
> `DataSource` property must be blank."**
>
> **"When averaging fields that have an integer data type, INTEGER DIVISION is used. This means the
> result is NOT ROUNDED and the remainder is discarded. For example, 5÷2=2 instead of 2.5."**
>
> "When you set up a totals method on another column, rows in the resulting dataset are **GROUPED** by
> the day, month or year."

**Three documented behaviours that an implementation would otherwise get right and thereby wrong:**

1. **A zero date part returns a substitute**, not zero and not an error -- day 0 gives 1, year 0 gives
   1900. That is BC's own zero-date handling and it has to be reproduced, not fixed.
   **CORRECTED by board:0556: that is the behaviour of BC 26 AND EARLIER.** From version 27 the three
   date methods return **0, 0, 0** on a blank date, and this tree translates BCApps 30.0 against a
   28.4 demo database -- both on the later side. The 1900 and the 1753 beside it are the SQL blank-date
   sentinel leaking through, and it stops leaking in 27.
2. **`Average` over integers truncates.** PostgreSQL's `avg(integer)` returns `numeric` -- so a direct
   translation gives 2.5 where BC gives 2, and this is the kind of silent numeric difference
   CLAUDE.md's determinism invariant exists for.
3. **A date method on another column IMPLIES the grouping** -- there is no `GROUP BY` property, the
   presence of a totals method makes every non-aggregated column a grouping key. So the `GROUP BY` is
   DERIVED from the column list, which is exactly the derivation board:0064 has to get right.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Method =`: **268 declarations** -- `Sum` 223, `Count` 36, `Year` 2, `Month` 2, `Max` 2, `Day` 2,
`Min` 1, and **`Average` 0**. So the truncation trap this item is titled for has no call site in the
tree, which decides its order rather than its existence: it is last of the seven.

## The IST-state

Queries have no generator (board:0064, board:0034), and no PARSER either: `src/al/Ast.h` has no query
node at all, while `src/gen/CodeunitWriter.cpp:129` already knows `Query` as a variable type. The name
exists and the object does not.

## The choice

An enumerator per column, and the generator derives the `GROUP BY` from which columns carry a totals
method. `Average` over an integer column emits an explicit integer division rather than `avg`, and
`Count` asserts an empty `DataSource`.

## Ordering

Inside board:0064, with board:0461's joins -- one `SELECT`.

## Gate, and its negative control

An `Average` over integer values 5 and 2 returns 2, not 3.5; **a `Year` method on a blank date returns
0** -- board:0556 carries the version table this was corrected from, and the earlier text here said
1900.

**The negative control is `Average` over a DECIMAL column** -- it must NOT truncate, and an
implementation that applies integer division everywhere passes the first gate and rounds every amount
in the system.

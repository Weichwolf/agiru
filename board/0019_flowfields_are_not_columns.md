Type: root
State: open
Area: rt, db, gen
Tags: navision, semantics, blocker

# FlowFields are not columns, and SIFT is why NAV was fast

A FlowField has no column. It carries a `CalcFormula` -- `Sum("Amount" WHERE (...))`, `Count`,
`Lookup`, `Exist`, `Min`, `Max`, `Average` -- and it holds nothing until `CALCFIELDS` runs the
query behind it. `Customer."Balance (LCY)"` is a Sum over `Detailed Cust. Ledg. Entry`; nothing in
the Customer row holds a balance, and BC has never stored one.

Three mechanisms hang together here and none of them exists in this tree yet.

**The field class.** `FieldClass = FlowField` and `FieldClass = FlowFilter`. `FieldDef` has a type
and no class, so a FlowField is currently emitted as an ordinary column -- which means the
generated CREATE TABLE has a column BC does not have, and an INSERT writes a value BC never wrote.
That is a defect in the schema and not only in the calculation. The same missing field class blocks
BLOBs (board:0017).

**The FlowFilter.** A field of class FlowFilter is not data either: it is a filter the USER sets,
which the CalcFormula of another field reads. `Customer.SETFILTER("Date Filter", '..%1', Today)`
followed by `CALCFIELDS("Balance (LCY)")` gives the balance as of a date. So a FlowFilter needs the
filter language (board:0018) and a place on the record that is neither a column nor a value.

**SIFT.** A key with `SumIndexFields` was, in NAV, a maintained aggregate -- a separate table
updated on every insert, which is why a customer balance was instant over millions of entries. BC
on SQL Server keeps indexed views for it.

**And `devenv-sift-and-sql-server.md` gives the whole mechanism** (read 2026-09-04, board:0071),
which turns the open question into a comparison with a named baseline:

```sql
CREATE VIEW GLEntry$VSIFT$1 AS
  SELECT SUM(Amount) AS SUM$Amount, AccountNo, PostingDate
  FROM GLEntry GROUP BY AccountNo, PostingDate;
CREATE UNIQUE CLUSTERED INDEX VSIFTIDX ON GLEntry$VSIFT$1(AccountNo, PostingDate);
```

and the read is `SELECT SUM(SUM$Amount) FROM GLEntry$VSIFT$1 WITH(NOEXPAND) WHERE ...`.

| | |
|---|---|
| one INDEXED VIEW per enabled SIFT key | `MaintainSIFTIndex = true` creates it, `false` drops it and stops maintaining the totals |
| **always at the FINEST granularity of the key** | a key on `AccountNo, PostingDate` stores a row per account per DAY, "so in the worst case 365 records must be summed to generate the total for each account for a year" |
| at most **20 SumIndexFields per key** | and only `Decimal`, `Integer`, `BigInteger`, `Duration` |
| SQL Server maintains the view on every write to the base table | which is where the locking cost NCCI's page complains about comes from |

**PostgreSQL has `MATERIALIZED VIEW` and does NOT maintain it automatically**, so BC's mechanism does
not transfer as written -- and that is the divergence this item has to name and measure, exactly as
board:0012 names the missing dirty read. The three candidates are a trigger-maintained aggregate
table (NAV's own answer), a plain `SUM` over a covering index, and a materialised view refreshed on
a schedule, which is wrong for a balance and worth saying so. Whether agiru needs the same, or whether PostgreSQL's
aggregate over a covering index is fast enough on the target hardware, is EXACTLY the kind of
question that gets a measurement rather than an opinion (board:0006). It is written down here so
that the answer is chosen rather than defaulted into.

## AND MICROSOFT'S OWN ADVICE IS TO STOP ADDING SIFT KEYS

`devenv-table-keys.md`, read 2026-09-04 (board:0071), on the non-clustered columnstore index:

> You can use a non-clustered columnstore index to efficiently run real-time operational analytics
> on the database **without the need to define SIFT indexes up front** (and without the locking
> issues that SIFT indexes sometimes impose on the system.) **Whenever you would normally add a SIFT
> key on fields to do summation/count operations on, use a non-clustered columnstore key to add all
> the fields to the index instead.**

Its worked example replaces two SIFT keys with one `ColumnStoreIndex`, and `devenv-ncci-overview.md`
and `devenv-migrating-from-sift-to-ncci.md` are three more pages about the same move.

**That does not decide anything here and it changes the question.** PostgreSQL has no columnstore
index, so neither of BC's two answers transfers -- but the platform saying its OWN maintained
aggregate is the older and lock-heavier route means the choice this item leaves open is not
"maintained totals or nothing". It is between a maintained aggregate, a plain aggregate over a
covering index, and whatever PostgreSQL offers in that direction, and the measurement this item
asks for should include the third.

## The benchmark

The number of FlowFields and FlowFilters in the BaseApp, counted rather than guessed, and the wall
time of one `CALCFIELDS("Balance (LCY)")` on a CRONUS customer -- with an aggregate query and, if
it comes to it, with a maintained total -- on x86_64 and on the Pi.

## Closed when

A FlowField is absent from the generated schema, empty until CALCFIELDS, and correct afterwards for
all seven formula kinds; and the SIFT question has a measured answer either way.

## SMARTSQL IS THE SHAPE, AND THE PLATFORM'S OWN QUERY IS PRINTED

`administration/Troubleshooting-Queries-Involving-FlowFields-By-Disabling-SmartSQL.md` (read
2026-09-04, board:0071) prints the SQL BC generates for `Customer List`, whose three FlowFields are
`Balance (LCY)`, `Balance Due (LCY)` and `Sales (LCY)`. The shape is one statement:

```sql
SELECT TOP (50) "Customer".<columns>,
       ISNULL("SUB$Balance (LCY)"."...$SUM$Amount (LCY)", @3) AS "Balance (LCY)", ...
  FROM dbo."MSFT$Customer" AS "Customer"
OUTER APPLY ( SELECT TOP (1) ISNULL(SUM(...), @3)
                FROM dbo."MSFT$Detailed Cust_ Ledg_ Entry$VSIFT$5" AS ...
               WHERE ..."Customer No_" = "Customer"."No_" AND <the CalcFormula's filters> )
       AS "SUB$Balance (LCY)"
OUTER APPLY ( ... ) AS "SUB$Balance Due (LCY)"
OUTER APPLY ( ... ) AS "SUB$Sales (LCY)"
 WHERE ...
 ORDER BY "No_" ASC
```

**Three things this settles for the design rather than leaving to taste:**

- **One `OUTER APPLY` per FlowField, against the SIFT VIEW.** "For each FlowField in the table, an
  OUTER APPLY clause is added to the SQL statement." The table name `...$VSIFT$5` is this item's own
  claim in BC's own naming: a SIFT bucket is an indexed VIEW over the key numbered 5, and the
  FlowField reads it rather than the base table. **PostgreSQL's equivalent of `OUTER APPLY` is
  `LEFT JOIN LATERAL`**, so the shape translates without inventing anything.
- **The join key is the CalcFormula's own filters**, inlined into the `WHERE` of the subquery --
  which is why board:0047's `CalcFormula` has to be `constexpr` data rather than a string: the
  generator writes this `WHERE` clause.
- **`ISNULL(..., @3)` is everywhere**, so a FlowField with no matching rows is 0 and never NULL --
  the same rule as the column defaults board:0013 now carries.

**And the calculation is not optional.** "It won't help to customize the page or change the
visibility of the field either. **If a FlowField is contained in the metadata of the page, it will
be calculated.**" board:0047 says the same from `CalcFields`' side; this page says it about the page
renderer, which is where a reader would expect an optimisation to be allowed.

**SmartSQL queries are NOT CACHED**, which the page names as what "amplifies the issue" -- so the
data cache described in `optimize-sql-data-access.md` covers `Get`, the `Find` family, `Count`,
`IsEmpty` and `CalcFields`, but not this fused query. agiru has no row cache at all (see the note
below), so the divergence is only in the direction of doing more work.

**Two documented exceptions where filtering on a FlowField is NOT one statement**, and both are
declaration-shaped, so the generator knows them: `ValueIsFilter` on a field that has a value, and a
second predicate on a source field that already carries one -- "if you specify two or more filters
on the same source field, then filtering doesn't issue a single SQL statement".

**`MIN` and `MAX` CalcFormulas never use SIFT**: "MIN and MAX formulas use SQL Server MIN and MAX
functions exclusively", while `Count` and `Average` can. One more row of the CalcFormula table that
is decided by the METHOD rather than by the key.

## SIFT HAS A DOCUMENTED SUCCESSOR, AND IT DOES NOT APPLY HERE

`devenv-ncci-overview.md` and `devenv-migrating-from-sift-to-ncci.md` (read 2026-09-04, board:0071):
the **nonclustered columnstore index** "is envisioned to be the successor of SumIndexField
Technology (SIFT)" and lets BC "quickly calculate the sums of numeric data type (Decimal, Integer,
BigInteger, and Duration) columns in tables ... with millions of records", replacing one or more SIFT
keys on a table.

**PostgreSQL has no columnstore index**, so the successor is not available and the predecessor is
what agiru implements. That is a NAMED non-applicability rather than a gap: the four pages
(`devenv-ncci-overview`, `-and-sql-server`, `-performance`, `-tuning-and-tracing`, plus the migration
page) describe a SQL Server storage feature, and this item's own answer -- SIFT as a materialised
aggregate the platform maintains -- stands.

What the NCCI pages DO settle is the type list: sums are over **Decimal, Integer, BigInteger and
Duration**, the same four `devenv-flowfields.md` gives for `Sum` and `Average`. Two pages, one
answer, and it is the list board:0047's `static_assert` uses.

Type: root
State: open
Area: db

# A declared key is a real index, and a read streams through a cursor

**A TABLE OF 100 MILLION ROWS IS ORDINARY IN BC**, and two decisions in `src/db` are made as though
it held a thousand.

- **1 663 of the 3 272 declared keys become nothing.** `CreateTable` emits the primary key and stops;
  `Sales Line` alone declares 17 keys and gets one index. `SetCurrentKey` picks one of the other 16,
  and on 100 million rows that is a sort of the table.
- **`FindSet` was going to hold the whole result set** in the record variable's state. One session
  reading a journal is nothing; a thousand sessions doing it is the per-session budget board:0006
  measures, spent in one place -- and one session over 100 million rows is the process.

## Reference

`devenv-table-keys.md` and `devenv-keys-property.md`: a key is a sort order AND an index, and
`Enabled = false` says which ones the platform does not maintain. `SumIndexFields` are the ones BC
maintains an aggregate for, which is what makes `CalcSums` affordable and is board:0019's problem,
not this one.

**Read in full 2026-09-04 (board:0071), the page carries six rules this item did not:**

| | |
|---|---|
| the FIRST key declared is the primary key | and it is "always active"; records are logically stored sorted by it |
| a primary key is at most **16 fields** | 20 are declarable and "because of SQL Server limitations, only the first 16 are used" |
| **there is ALWAYS a unique secondary key on `SystemId`** | so board:0013's field is not only a column, it is an index the schema owes |
| a secondary key does NOT reject duplicates | "if two or more records contain identical information in the secondary key, SQL Server uses the table's **primary key** to resolve this conflict" -- which is board:0056's ORDER BY rule stated from the index side, and it is a GUARANTEE rather than an optimisation |
| `Unique` makes a secondary key a unique constraint | checked when the table is validated; **not supported in a table extension** |
| a disabled key can be re-enabled | at the cost of a full table scan to rebuild the index |
| `IncludedFields` are SQL **included columns** | non-key columns carried in the index so a query never touches the table; also "lets you bypass the maximum number of fields in a key" |
| `Clustered` | one per table, "by default the primary is configured as a clustered key" -- and PostgreSQL has no maintained clustered index (board:0067) |

**SQL SERVER GIVES BC A SERVER-SIDE CURSOR AND POSTGRESQL HAS THE SAME THING.** `DECLARE <name>
CURSOR FOR SELECT ... ORDER BY ...` then `FETCH FORWARD n`: the position lives in the server, the
session costs the fetch buffer, and `Next` past the buffer fetches the next block rather than
re-querying. A cursor lives inside a transaction, which is where a session already is -- board:0012
pins the connection for exactly that. `WITH HOLD` carries one across a commit at the cost of a
materialisation, so it is taken only where AL's behaviour needs it.

`record-findset-boolean-method.md` says `FindSet` "will request all rows at once" and `Find` pages.
That is a statement about the ROUND TRIP and not an instruction to hold every row in the client: a
cursor with a fetch block is one request whose rows arrive as they are read.

## The choice

- **`Clustered` decides which key is the physical order, and 1 771 keys declare it** (measured
  2026-09-04 over the read roots; the generator reads neither it nor `Enabled` -- board:0067).
  PostgreSQL has no clustered index: `CLUSTER` is a one-off reordering and not a maintained
  property, so this is a divergence to NAME and measure rather than map away, the way board:0012
  names `READUNCOMMITTED`. The primary key is the candidate for it and usually is the declared one.
- **Every declared key with `Enabled` unset or true becomes a `CREATE INDEX`**, named after the AL
  key, over its fields in order and in its declared direction. A key the table already has as its
  primary key is not repeated.
- **A read is a cursor.** `FindSet` declares one over the filtered, ordered set and fetches a block;
  `Next` walks the block and fetches the next when it runs out; `Close`, a `Reset` or the end of the
  transaction closes it. The block size is a MEASURED number with its origin beside it, not a guess.
- **`IsEmpty` is `SELECT 1 ... LIMIT 1` and never `Count() != 0`** -- the predecessor measured a
  90-second per-test timeout from exactly that shortcut. `Count()` is `COUNT(*)`, which on 100
  million rows is expensive in BC too, and AL code that calls it in a loop is AL's problem.
- **`FindFirst`/`FindLast` ask for one row** with `LIMIT 1` over the order and its reverse, rather
  than reading a set and taking an end of it.

## Gate

A table with a secondary key: the index exists after `CreateTable`, and `EXPLAIN` shows the plan
using it for a `SetCurrentKey` read rather than a sequential scan. A cursor over more rows than the
fetch block walks all of them and holds only the block. `IsEmpty` on a large filtered set does not
read it. The negative control drops the index and the plan must change.

## A RESULT SET IS DYNAMIC, AND THAT IS A GUARANTEE RATHER THAN AN OPTIMISATION

`administration/optimize-sql-al-Database-methods-and-performance-on-server.md`, read 2026-09-04
(board:0071 -- the `administration/` family was outside the sweep's first denominator):

> Any result set that is returned from a call to the Find methods ... is **dynamic**. That means that
> the result set is **guaranteed to contain any changes that you make further ahead in the result
> set**. However, this feature comes at a cost. If any modifications are made to a table being
> traversed, then Business Central might have to issue an **extra SQL statement** to guarantee that
> the result set is dynamic.

**That is a constraint on the cursor this item is built around.** A `FindSet` loop that modifies rows
ahead of the current position must see the modified values when it reaches them -- and the BaseApp's
posting routines do exactly that, which is why the platform pays for it. PostgreSQL's
`DECLARE ... CURSOR` reads the transaction's snapshot: changes the SAME transaction makes are
visible to a non-materialised cursor, and a `WITH HOLD` cursor materialises and is NOT. So the
divergence is decided by which form this item takes, and it has to be decided deliberately rather
than discovered when a posting reads a stale line.

**The other rules on the same page, and each one names a method this item's `Find` family owes:**

| method | what the platform does |
|---|---|
| `Get` | optimised for one record by PRIMARY KEY |
| `Find` | one record by the primary key **plus any filter or range set** |
| **`Find('-')` / `Find('+')`** | **a self-tuning `TOP X`, where X changes over time based on statistics of the number of rows read** -- for the case where the caller may stop early |
| **`FindSet`** | the complete set in the filter, and **NOT a `TOP X` call** |
| `FindFirst` / `FindLast` | the single first or last in the filter |
| **`Next`** | **if it is not called as part of retrieving a continuous result set, the platform issues a SEPARATE SQL statement to find the next record** |

`FindSet(ForUpdate, UpdateKey)`'s second parameter is documented as INERT: "The *UpdateKey* parameter
doesn't influence the efficiency of this method ... such as it did in NAV 2009." So the signature is
carried and the parameter does nothing, which is a rare case where doing nothing is the faithful
behaviour -- and it should be stated at the declaration rather than left to look like an omission.

**The `Find('-')` self-tuning TOP X is the one shape agiru cannot copy directly** and the one worth
measuring: a fetch block on a cursor is the same idea (board:0045's own design), and whether it
should ADAPT the way BC's does is a measurement rather than a deduction.

## `SetCurrentKey` FIXES THE ORDER AND NOT THE INDEX

`administration/optimize-sql-table-keys-and-performance.md`, read 2026-09-04:

> **SQL Server automatically chooses which index to use** in order to retrieve data in the most
> efficient way. SQL Server calculates the cost of retrieving data using different indexes and then
> chooses the path that has the smallest cost. For Business Central, that calculation is **based only
> on the statistical distribution of values in a column.**

and the page's own example makes the point sharper than the sentence does: with
`SetCurrentKey("LowSelectivityColumn")` and filters on both columns, "SQL Server chooses an index
that contains the **HighSelectivityColumn** and then **sorts the rows** by the
LowSelectivityColumn."

**So `SetCurrentKey` is an `ORDER BY` and not an index hint.** The key decides the ORDER -- which is
a semantic this item already carries, down to the primary-key tiebreak -- and which index satisfies
it is the planner's business. That settles a design question the other way round from the obvious
reading: agiru emits the ORDER from the current key and lets PostgreSQL choose the scan, rather than
forcing a scan of the matching index.

It does not weaken this item's other half. **A `SetCurrentKey` onto a key with no index is still a
sort of the table** -- the planner just tells you so with a `Sort` node instead of a hint being
ignored -- which is why 3 272 declared keys have to be real indexes either way.

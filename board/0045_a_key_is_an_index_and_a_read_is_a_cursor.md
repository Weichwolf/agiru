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

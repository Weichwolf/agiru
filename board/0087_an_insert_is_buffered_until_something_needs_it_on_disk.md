Type: root
State: open
Area: rt, gen

# An `Insert` is BUFFERED until something needs it on disk, and the flush points are documented

`administration/optimize-sql-bulk-inserts.md` (read 2026-09-04, board:0071 -- the
`dev-itpro/administration/` family was outside the sweep's first denominator):

> **By default, Business Central automatically buffers inserts** in order to send them to Microsoft
> SQL Server at one time. ... Bulk inserts also improve scalability by **delaying the actual insert
> until the last possible moment in the transaction**. This reduces the amount of time that tables
> are locked; especially tables that contain SIFT indexes.

This is not an implementation detail the runtime may ignore. The buffer has OBSERVABLE edges, and
the page names every one of them.

## The rules, and where each is decided

**A buffer is FLUSHED when:**

| | and it is decided |
|---|---|
| `Commit` is called | at run time |
| `Modify` or `Delete` is called **on that table** | at run time |
| any `Find` or `Calc` method is called **on that table** | at run time |

**A table's inserts are NOT buffered at all when:**

| | and it is decided |
|---|---|
| **the return value of `Insert` is used** -- `if GLEntry.Insert() then` | **TRANSLATION time, at the call site** |
| the table contains a **BLOB** field | **translation time** -- the field table |
| the table contains a field with **`AutoIncrement = true`** | **translation time** -- the field table |

**The first row is CLAUDE.md's value-context trap, and the generator already decides it.** The
tabulated failure mode is "AL decides at consumption-versus-discard whether a failure throws or
yields `false`", and the guard is that "the contexts are named: assignment, `if`/`while`, `exit`,
argument, `case` selector". So `Rec.Insert()` as a STATEMENT and `if Rec.Insert() then` are already
two different emissions, and the buffering decision rides on the distinction that exists.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`, all `.al`

| | |
|---|---:|
| `.Insert(` call sites | **39 353** |
| of them in an `if` -- the return value USED, so never buffered | **2 181 (5.5 %)** |
| `field(... ; Blob)` declarations | 616 |
| `AutoIncrement` declarations | 434 |

**So 94.5 % of the insert call sites are in the buffered shape**, and the tables that opt out do so
for a reason the field table already knows.

## Why it matters BEYOND performance

- **A duplicate-key error surfaces at the FLUSH, not at the `Insert`.** The page's own worked
  example is a posting loop over `G/L Entry`, and the difference between its two versions is where a
  `FindLast` sits. A runtime that inserts eagerly reports the violation at a different statement
  from the one BC reports it at -- and an AL test that wraps the insert in `asserterror` then passes
  or fails depending on which. That is intended behaviour and tests compare it (board:0055).
- **It is the documented reason SIFT-heavy tables stay unlocked longer**, which is board:0012's
  subject from the other end: the lock is taken when the row is written, and BC deliberately writes
  as late as it can.
- **`LockTable` before the loop is the pattern the page shows**, and it does not flush -- so the
  lock is held and the rows are still buffered, which is exactly the combination that makes the
  posting loop fast.

## The choice

**A per-table insert buffer on the session, and a `constexpr bool` beside each table saying whether
it may be used.**

- The generator emits `kBufferedInsertAllowed` from the field table: false when any field is a BLOB
  or carries `AutoIncrement`, true otherwise. One byte in `.rodata`, decided once.
- `Table<Derived>::Insert()` -- the STATEMENT form -- appends to the session's buffer for that table
  when the flag allows; `Insert(Boolean)` and the value-returning form write through, because the
  caller is asking for the answer now.
- The three flush points are the three methods named above, plus the transaction boundary. A
  `Commit` flushes everything; a `Find`, `Calc`, `Modify` or `Delete` flushes ONE table's buffer, and
  the buffer is keyed by table id so that is a lookup.
- **The buffer is per SESSION and dies with the transaction**, because CLAUDE.md's first invariant
  makes a rolled-back boundary undo everything inside it -- and a buffered row that was never sent
  is undone by discarding the buffer, which is cheaper than the rollback it replaces.

**What this item does NOT do is make buffering optional.** BC has `BufferedInsertEnabled` in
`CustomSettings.config` for troubleshooting; agiru has no reason to carry a switch whose only
purpose is to change where an error appears.

## Gate, and its negative control

Insert three rows into a table with no BLOB and no `AutoIncrement`, then read the table from a
SECOND record variable in the same transaction -- the rows must be there, because the read flushes.
Then insert three rows and `Commit` -- the rows must be on disk. Then insert a DUPLICATE key as a
statement and require the error to arrive at the flush, and the same insert in an `if` to return
`false` immediately.

**The negative control is the last pair**: they must differ. A runtime that writes through always
passes the first three cases and fails only that one, which is why it is the case the item is
written around.

Classification: **activation** -- inserts write through today, so every case that depends on the
delay is currently green for the wrong reason; the A/B is over the suite, and a posting loop's
statement count is the measurement.

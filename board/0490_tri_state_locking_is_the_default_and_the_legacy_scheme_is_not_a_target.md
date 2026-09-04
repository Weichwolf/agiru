Type:     task
Status:   open
Parent:   0012
Area:     rt, db
Source:   developer/devenv-tri-state-locking.md
Verdict:  fehlt
Class:    activation

# Tri-state locking is the default, and the legacy scheme is not a target

board:0012 is "a read takes the lock AL says it takes" and this page is its specification. Three
statements settle what the three states actually are:

> "**AL-based read operations that FOLLOW WRITE operations are performed OPTIMISTICALLY**, rather than
> with strict consistency and low concurrency."
>
> | | versions 22 and earlier | **tri-state locking** |
> |---|---|---|
> | default isolation for subsequent operations | `UpdLock` | **`ReadCommitted`** |
> | locking behaviour | update lock until commit or rollback | **shared lock when reading** |
>
> **"NOTE: Explicitly using the `LockTable` method in code MAINTAINS THE SAME BEHAVIOR, disabling
> optimistic reads."**
>
> **"Tri-state locking is ENABLED BY DEFAULT for Business Central online and on-premises."** The
> legacy scheme is a **Feature Management** toggle -- in version 25, *Feature: Enable legacy locking
> scheme in AL*.

So the three states, in order, are:

| state | when | SQL |
|---|---|---|
| 1 | before the session writes to that table | `READUNCOMMITTED` |
| 2 | **after** the session writes to that table | `READCOMMITTED` |
| 3 | after `LockTable()` on that table | `UPDLOCK` |

**And state 2 is the one that changed.** In version 22 a write escalated every later read on that
table to `UPDLOCK`; since 23 it escalates to a shared lock, and only `LockTable` still takes the
update lock. board:0012's own description is the tri-state one -- so it is right, and this page is
what proves it rather than the version-22 behaviour a reader might infer from older sources.

**The legacy scheme is a documented NON-TARGET**, and saying so is worth an item on its own: it is
switchable in BC, it is not the default anywhere, and building both would double the state machine
for a mode Microsoft is removing.

## What PostgreSQL does with the three states

**State 1 has no counterpart.** PostgreSQL has no dirty read; `READ UNCOMMITTED` is accepted and
behaves as `READ COMMITTED`. board:0012 already names this divergence and it is the largest one in
this tree's database layer -- so states 1 and 2 COLLAPSE, and the whole tri-state machine reduces to
two observable states here.

**That collapse is more correct, not less**, and it is measurable: a read that BC would answer from
uncommitted data answers from committed data instead. The item's job is to record where that can be
SEEN -- a session reading its own uncommitted write is unaffected (its own transaction sees it), and
a session reading ANOTHER session's uncommitted write is where BC and agiru differ.

**State 3 maps exactly**: `SELECT ... FOR UPDATE` is PostgreSQL's `UPDLOCK`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`LockTable(` call sites: measured with this item when it is pulled -- it is a method call and not a
property, so this sweep's declaration pattern does not apply, and that is said rather than guessed.

## The IST-state

board:0012 records it: no isolation state machine, one connection kind, `src/rt/Storage.cpp` issues
plain statements.

## The choice

A per-table state on the session's transaction, two observable values here, with `LockTable`
promoting to `FOR UPDATE`. **State 1 and state 2 are the same SQL and must still be distinct in the
model**, because `LockTable` promotes from either and board:0463's `ReadState` bypasses the machine
per statement.

**The legacy scheme is refused rather than configurable**: one machine, no toggle, and this page is
the citation.

## Ordering

Inside board:0012, as its specification. Before board:0458's `TransactionType` and board:0463's
`ReadState`, which are both declarations over this machine.

## Gate, and its negative control

A session that writes to a table then reads it takes a shared lock, not an update lock; after
`LockTable` it takes an update lock and a second session's write blocks.

**The negative control is the second session** -- lock behaviour is invisible from one session, so
every assertion here needs a concurrent writer, and a gate without one passes under any isolation
level.

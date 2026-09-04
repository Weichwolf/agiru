Type:     task
Status:   open
Parent:   0012
Area:     gen, rt, db
Source:   developer/properties/devenv-readstate-property.md
Verdict:  fehlt
Class:    activation

# `ReadState` overrules the transaction type for a query

> Specifies which records are read and **how they are locked** when a query is executed. Applies to:
> **Query.**
>
> `ReadUncommitted` -- a **dirty read**. "No additional locks are placed on the read data."
> `ReadShared` -- committed data only, **share locks held until the transaction is committed**;
> "translates to reading the data with Repeatable Read."
> `ReadExclusive` -- committed data only, **update locks**; "translates to reading the data with
> UpdLocks."
>
> **"The `ReadState` property will OVERRULE the current transaction type of the data as set by a
> `CurrentTransactionType` call in the AL code, because QUERIES IGNORE the `CurrentTransactionType`
> method call."**
>
> **"Each query will use the specified `ReadState` regardless of other queries that have already been
> executed. This means that you can read UNCOMMITTED data and COMMITTED data from the same tables IN
> THE SAME TRANSACTION. However, the STRICTEST LOCK placed on a row will remain until the transaction
> is committed."**

**Two rules that break the isolation model everything else in this tree follows.** board:0012 owns a
per-table isolation STATE MACHINE that only ever escalates; this property says a query ignores it,
that two queries in one transaction may read at different levels, and that the strictest lock
nevertheless persists.

So the state machine has an exception, and the exception is per statement rather than per table.
That is not derivable from board:0012's own reference and it is the reason this item is filed under it
rather than under board:0064.

**And `ReadUncommitted` is board:0012's named divergence** -- PostgreSQL has no dirty read -- so one
of the three values cannot be translated and inherits whatever board:0012 measured.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ReadState =`: **8 declarations.**

Eight, so the DEFAULT governs every other query -- and the page does not state one. That gap is
recorded rather than guessed at; `devenv-tri-state-locking.md` is where board:0012 looks.

## The IST-state

Queries have no generator (board:0064, board:0034); board:0012 records the isolation state.

## The choice

A three-valued enumerator on the query, applied per statement, bypassing the table's isolation state
and NOT updating it -- except that the lock, once taken, outlives the statement, which is the
documentation's own last sentence.

**The bypass must be explicit in the code**, not an emergent property of where the enumerator is read.
board:0012's machine and this exception are one design and the exception is the part somebody will
otherwise remove as redundant.

## Ordering

Behind board:0012. Inside board:0064 for the declaration.

## Gate, and its negative control

Two queries in one transaction over the same table, one `ReadUncommitted` and one `ReadShared`, read
at their own levels; a `CurrentTransactionType` call between them changes neither.

**The negative control is the `CurrentTransactionType` call** -- an implementation that routes queries
through the table's isolation state passes both single-query gates and obeys a call the documentation
says queries ignore.

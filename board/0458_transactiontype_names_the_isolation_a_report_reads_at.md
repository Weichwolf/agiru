Type:     task
Status:   open
Parent:   0012
Area:     rt, db, gen
Source:   developer/properties/devenv-transactiontype-property.md
Verdict:  fehlt
Class:    activation

# `TransactionType` names the isolation a report reads at

> Applies to: **Xml Port, Report.**
>
> | value | behaviour |
> |---|---|
> | `Browse` | all reads at **READ UNCOMMITTED** |
> | `Report` | maps to `Browse` |
> | `Snapshot` | all reads at **REPEATABLE READ** |
> | `UpdateNoLocks` | READ UNCOMMITTED **until the table is modified or `LockTable` is called** |
> | `Update` | REPEATABLE READ until the table is modified or `LockTable` is called |
>
> "Each transaction type defines the behavior of a transaction and takes effect **from the start of a
> transaction**. A transaction starts at the start of the outermost code or immediately after
> `Commit` is called ... **if a method in a codeunit calls another codeunit, then a new transaction is
> NOT started.**"

**This is board:0012's tri-state locking declared per object**, and board:0012 already records the
hard part: **PostgreSQL has no dirty read**, so `READ UNCOMMITTED` cannot be translated and the
divergence is named and measured rather than mapped away. Three of the five values ask for it.

**And the transaction-boundary sentence is the one to keep.** A nested codeunit does not start a new
transaction, so the type is a property of the OUTERMOST object -- which means a report's declared type
governs everything it calls, and a codeunit called from two reports runs at two different isolations.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`TransactionType =`: **5 declarations.**

**Five, in 668 reports and every XMLport.** So the DEFAULT is what matters -- and the property page
does not state one, which is a gap this item records rather than guesses at. `devenv-tri-state-locking.md`
is the root page board:0012 cites and it is read separately.

## The IST-state

board:0012 owns the isolation state machine; reports and XMLports have no generator (board:0063,
board:0065).

## The choice

A five-valued enumerator on the report and the XMLport, feeding board:0012's per-table isolation state
machine -- **the same machine, not a second one**, because `LockTable` raising the level is in both
this property's description and board:0012's.

**The `READ UNCOMMITTED` values inherit board:0012's divergence**: whatever it decides PostgreSQL does
instead, these three values decide it too, and this item must not invent a second answer.

## Ordering

Behind board:0012. Inside board:0063 and board:0065 for the declaration.

## Gate, and its negative control

A report declaring `Snapshot` reads the same rows twice within one run even when another session
commits between the reads.

**The negative control is the concurrent commit** -- a single-session gate passes under every
isolation level, so the control needs a second session writing between two reads.

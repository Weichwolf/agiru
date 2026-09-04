Type:     task
Status:   open
Parent:   0057
Area:     rt, gen, db
Source:   developer/devenv-events-isolated.md
Verdict:  fehlt
Class:    activation

# An isolated event gives each subscriber its own transaction

> "An isolated event ensures the event publisher **CONTINUES ITS CODE EXECUTION after calling an
> event**. If a subscriber's code causes an error, **its transaction and associated table changes will
> be ROLLED BACK. The execution continues to the NEXT event subscriber**, or is handed back to the
> caller."
>
> "Implemented by **separating each event subscriber INTO ITS OWN TRANSACTION**. The transaction is
> created before invoking the subscriber, then **COMMITTED afterwards**."
>
> Declared by an attribute argument: `[InternalEvent(IncludeSender: Boolean, Isolated: Boolean)]` --
> `[InternalEvent(true, true)]`.

**This is a `Commit` per subscriber, and it meets the first invariant.** CLAUDE.md: "A POSTING IS ALL
OR NOTHING, AND THAT OUTRANKS EVERY OTHER GOAL HERE ... a `Commit()` makes what came before it durable
and no later rollback may undo that."

**The page draws the boundary itself, and it is what keeps the invariant intact:**

> **"Read-only transactions are allowed to call isolated events directly, but WRITE TRANSACTIONS
> SHOULD EXPLICITLY BE COMMITTED BEFORE INVOKING an isolated event. OTHERWISE, THE ISOLATED EVENT WILL
> BE INVOKED LIKE A NORMAL EVENT, that is, errors inside a subscriber will cause the entire operation
> to fail."**

**So isolation is CONDITIONAL on there being no pending write.** An isolated event raised inside an
uncommitted write transaction is NOT isolated -- it degrades silently to a normal event. That is the
rule an implementation would never invent, and it means a posting cannot lose half its work to an
isolated subscriber: inside a posting, the isolation does not apply.

## What is rolled back, and what is not

> **"ONLY changes done via Modify/Delete/Insert calls on records of type `TableType: Normal` will be
> automatically rolled back. Other state changes -- HTTP CALLS, VARIABLE ALTERATIONS, CHANGES TO
> SINGLE INSTANCE CODEUNIT'S MEMBERS -- WON'T be rolled back."**
>
> "If an integer variable passed by VAR is modified by a failing subscriber, **its changes will
> persist.**"

**Three exclusions, each naming another board item**: `TableType` (board:0364, so a temporary table's
changes survive too), single-instance codeunit members (board:0471), and `var` parameters -- which
CLAUDE.md lists as an inherited failure mode in the other direction ("an out parameter never written")
and which here is the reverse: a `var` parameter written by a rolled-back subscriber KEEPS its value.

## The install and upgrade exception

> **"When the operation is installing, uninstalling, or upgrading extensions, ISOLATED EVENTS AREN'T
> RUN ISOLATED."** Those operations require everything in one transaction, so **"explicit `Commit`
> calls can't be made"** during them.

board:0500's drivers must switch isolation off for their duration -- a mode flag on the session, not a
property of the event.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Isolated` is a positional argument inside three attributes, and this sweep's declaration pattern does
not reach inside an attribute's arguments. board:0191 and board:0196 own the attribute counts.
**Stated rather than guessed** -- and the count decides the item's size, because at zero the whole
mechanism is a refusal.

## The IST-state

board:0057: no dispatch. `src/al/Parser.cpp:926` ignores attribute arguments, so `Isolated` is not
visible to the generator. board:0012 owns the transaction.

## The choice

A bit on the publisher's `constexpr` entry, and a dispatch loop that -- when the bit is set, the
session has no pending write, and no install/upgrade driver is running -- opens a **SAVEPOINT** per
subscriber, releases it on success and rolls back to it on failure, then continues.

**Savepoint, not `Commit`.** BC's "committed afterwards" is inside its own transaction model; here the
outer transaction must survive, and a savepoint gives the same observable behaviour without ending it.
**A deviation in mechanism and not in behaviour**, and it keeps the posting invariant literally true
rather than approximately.

**The three exclusions fall out of savepoints for free** -- a savepoint rolls back table changes and
nothing else, which is exactly what the page specifies.

## Ordering

Behind board:0512's dispatch and board:0012's transaction. Ahead of board:0500's drivers, which need
the off switch.

## Gate, and its negative control

Two subscribers to an isolated event, the first of which raises: the second still runs, the first's
inserts are gone, the publisher continues, and a counter passed by `var` keeps the first's increment.

**The negative control is the `var` counter** -- it must NOT be rolled back, and an implementation that
snapshots the arguments restores it, which is more correct and is not what BC does. The second control
is an isolated event raised inside an uncommitted write: it must behave as a NORMAL event and take the
whole operation down.

Type: root
State: open
Area: rt, db

# `Commit` swaps the savepoint, and `Codeunit.Run` is the boundary that reads it

AL's `Commit` is durable: what it wrote survives any later rollback up to the enclosing transaction.
AL's `Codeunit.Run` is a boundary: an error inside it rolls the database back to where the run began
and returns `false`. `asserterror` reads that same boundary. None of it exists here, and every UT
test stands on all three.

## Reference

**Platform documentation** (`devenv-transactions.md`, `codeunit-run-method.md`): a codeunit run
through `Codeunit.Run` gets its own transaction scope; an error unwinds to the start of it; the
return value is how the caller learns, because the error does not propagate.

**Predecessor, and this is the expensive part it already paid for.**
`~/Git/openerp/openerp/runtime/transaction.py` keeps a STACK of AL-level savepoints, and its comment
records a revert worth copying rather than rediscovering:

> The cell indirection lets `al_commit` swap a *live* block's savepoint for a fresh one: AL `Commit`
> ends the current write transaction and STARTS A NEW ONE, so a later error must still roll back --
> to the commit point. Releasing the savepoint without replacing it (the earlier behaviour) left the
> enclosing block with no rollback at all, so everything written after an inner `Commit` survived an
> `asserterror`.

**So the mechanism is: a stack of savepoints, and `Commit` REPLACES the top one instead of releasing
it.** Releasing is the obvious implementation and it is wrong in a way that makes tests pass for the
wrong reason -- which is the failure class this tree counts.

The bottom entry is the per-test boundary the runner opens and rolls back at teardown. That is how
2 392 tests run against one database without leaking into each other.

## What agiru has to decide that openerp did not

PostgreSQL savepoints are per CONNECTION, and a session's connection is pinned for its transaction
(board:0012). So the stack is per session, not global, and it is not a `ContextVar` -- that was a
Python answer to a Python problem and CLAUDE.md says not to port it.

## What will be true

- [ ] `Codeunit.Run` opens a savepoint, returns `false` on an error, and leaves the database as it
      was at the call.
- [ ] `Commit` swaps the top savepoint rather than releasing it, proven by a gate case: write,
      `Commit`, raise inside an `asserterror`, and require the committed write to SURVIVE.
- [ ] **Negative control**: implement `Commit` as a release and require that case to go red. If it
      does not, the case is not testing what it claims.
- [ ] The stack is per session and two sessions do not see each other's savepoints.

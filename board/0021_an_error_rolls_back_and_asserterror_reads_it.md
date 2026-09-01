Type: root
State: open
Area: rt, db
Tags: navision, semantics, blocker

# An error rolls back to where the codeunit started, and asserterror is how a test reads it

AL's `Error()` is not an exception that a caller catches. It ABORTS the write, and what it aborts
back to is a boundary NAV defines rather than the call stack: `Codeunit.Run` opens one, and an
error inside it rolls the database back to that point and returns `false` to the caller. Anything
the codeunit wrote is gone. That is why BaseApp posting routines are wrapped in `Codeunit.Run` and
why `IF NOT CODEUNIT.RUN(...) THEN` is the idiom for "try this".

`asserterror` is the test-side half:

    asserterror SalesPost.Run(SalesHeader);
    Assert.ExpectedError('The Quantity must not be negative');

Two things have to be true for that to mean anything. The message has to match -- `ExpectedError`
matches a SUBSTRING, which is why the wording of TestField and FieldError is load-bearing and
already noted in `type/StringValue.h`. And the write set has to be gone: a test that asserts an
error and then counts rows will see the rows if the rollback did not happen, and it will pass or
fail for a reason that has nothing to do with what it tests.

## What this tree has

`runtime/Error.h` throws. Nothing rolls back, because nothing opens a scope to roll back TO. The
database layer has a connection and no savepoint.

## The shape

A PostgreSQL `SAVEPOINT` per `Codeunit.Run`, released on success and rolled back to on error, and
nested so that a codeunit calling another codeunit nests too. AL's `Commit` in the middle of a
codeunit is a real thing BC allows and it interacts with this: a `Commit` releases the outer
transaction, so an error afterwards cannot roll back past it. That is NAV behaviour, not a bug, and
the tests rely on both halves.

## The benchmark

A gate: a codeunit that inserts a row and then errors, run through `Codeunit.Run`; the row is
absent afterwards and `Run` returned false. Then the same with a `Commit` before the error, where
the row survives. And `asserterror` catching the message, substring-matched the way
`Assert.ExpectedError` matches it.

## Predecessor

`openerp/runtime/transaction.py` and `openerp/runtime/asserterror.py` did this, and they correct
this item on the point it got half right.

**What this item said**: a `Commit` releases the boundary, so an error afterwards cannot roll back
past it. True, and not the whole rule. What the predecessor paid for:

> AL `Commit` ends the current write transaction and STARTS A NEW ONE, so a later error must still
> roll back -- to the commit point. Releasing the savepoint without replacing it (the earlier
> behaviour) left the enclosing block with no rollback at all, so everything written after an inner
> `Commit` survived an `asserterror`.

So a `Commit` does not release the block's savepoint; it SWAPS it for a fresh one taken at the
commit point. The savepoint is held through an indirection for exactly that reason.

**And a second, which lands on board:0012.** The savepoint must be opened on the SAME connection
handle the queries run on -- otherwise it opens on an unrelated or absent one and the rollback that
is supposed to clear an aborted transaction clears nothing. That is the pinned-connection rule
CLAUDE.md already states, reached from the other direction.

**Where the predecessor is a hint and not a verdict**: its `asserterror` swallows the exception and
captures the text, and its savepoint stack exists mostly to make a per-test rollback boundary work
under test isolation. Whether BC's `Codeunit.Run` boundary and the test-runner boundary are the
same mechanism or two is a question the DOCUMENTATION has to answer here, not that file.

## Closed when

Both halves pass, nested two deep, against PostgreSQL.

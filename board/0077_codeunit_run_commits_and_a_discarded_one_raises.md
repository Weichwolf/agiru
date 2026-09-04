Type: root
State: open
Area: rt, gen
Tags: navision, semantics, blocker

# `Codeunit.Run` COMMITS when its answer is used, and RAISES when it is not

`include/runtime/Codeunit.h:186` opens a `detail::Scope`, calls `OnRun`, and on success calls
`scope.Keep()` -- a savepoint released into the enclosing transaction. On failure it calls
`scope.Discard()` and returns `false`, always, in every context.

`codeunit-run-method.md` says something else, twice, and both halves are load-bearing:

> **Transaction semantics.** When the return value of the `Codeunit.Run` method is used, for example
> using the `if Codeunit.Run() then` pattern, **any changes done to the database will be committed at
> the end of the codeunit**, unless an error occurs. If you're already in a transaction you must
> commit first before calling `Codeunit.Run`.

> **[Optional] Ok** ... **If you omit this optional return value and the operation does not execute
> successfully, a runtime error will occur.**

## Two defects, and they are opposite in direction

**1. The value form does not commit; it releases a savepoint.** BC makes the codeunit's writes
DURABLE at its end. agiru folds them into the enclosing transaction, so a later error rolls back
what BC would have kept. CLAUDE.md's first invariant is exactly this sentence from the other side:
"a `Commit()` makes what came before it durable and no later rollback may undo that". A posting run
that calls `if PostingCodeunit.Run(Line) then` and then fails downstream keeps its entries in BC and
loses them here -- **silent-wrong-data on the ledger itself**, which is the worst address in this
tree.

**2. The discard form does not raise; it reports.** `CashFlowCheck.Run(Line);` as a STATEMENT must
raise on failure. agiru returns `false` into nothing, so a failed codeunit is indistinguishable from
a successful one and execution continues. That is CLAUDE.md's `value context` trap, the same one
board:0055 answers for `Get` and board:0056 for `Find` -- and the header currently argues the
opposite:

> THE RETURN VALUE IS THE ERROR HANDLING. `Codeunit.Run` does not propagate.

That is true of the VALUE form and false of the discard form, and the page is explicit.

## What else the page settles

- **`CommitBehavior` does not touch the implicit commit.** `[CommitBehavior(Ignore)]` and
  `[CommitBehavior(Error)]` change what a `Commit()` INSIDE the scope does; the commit
  `Codeunit.Run` performs at its end is not affected. The attribute is parsed nowhere today (78
  declarations, board:0067) and this is the rule it has to arrive with.
- **"If you're already in a transaction you must commit first."** BC states a precondition rather
  than a behaviour, so what agiru does when one is open is a choice: refuse loudly, or commit. The
  refusal is the one that cannot silently lose a write, and it is what a gate can assert.
- **An unknown codeunit id is a run-time error**, and **a record from a table other than the
  codeunit's is a run-time error.** Both need the by-number entry point, which does not exist
  (board:0025): `Codeunit.Run(Number [, var Record])` and `Codeunit.Run(FullyQualifiedName: Text
  [, var Record])` are two of the five `codeunit/` pages and both are absent.

## What the predecessor paid for

`~/Git/openerp`'s transaction module records the savepoint stack whose `Commit` must SWAP rather
than release as the defect that cost it most, and board:0040 records that agiru already has that
half right (`Boundaries::Commit` releases from the inside out and retakes). **So the machinery for a
real commit is there and `Codeunit.Run` is not using it** -- this is a call-site defect, not a
missing mechanism, which is why it is cheap and why it is dangerous.

## The choice

- **`Run` splits into the two contexts the generator already distinguishes**: `bool Run(...)`
  answers and COMMITS at the end; `void RunOrRaise(...)` propagates. The generator emits the second
  wherever AL discards the result, exactly as it will for `Find` and `Get`.
- **The commit is `Boundaries::Commit`**, the same call `Commit()` makes, so there is one commit path
  and not two.
- **A `Codeunit.Run` inside an open transaction refuses**, naming the codeunit, until a case shows BC
  doing something else. A silent commit of someone else's transaction is worse than a refusal.
- **`CommitBehavior` is read from the attribute** into the same `constexpr` metadata every other
  attribute lands in (board:0067), and it gates `Commit()` inside the scope and nothing else.

## Gate

`if CU.Run() then` over a codeunit that inserts a row: the row survives a rollback of the enclosing
transaction. `CU.Run();` over a codeunit that raises: the error reaches the caller. `if CU.Run()
then` over the same: `false`, and the row is gone. A `Commit()` inside a `[CommitBehavior(Error)]`
scope raises; the implicit commit at the end of `Run` does not.

**Negative control**: put the commit back to a savepoint release and require the first case to go
red. A gate that only checks the return value passes over both defects, which is the state today.

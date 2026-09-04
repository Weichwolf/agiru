Type:     task
Status:   open
Parent:   0039
Area:     gen, rt
Source:   developer/attributes/devenv-transactionmodel-attribute.md
Verdict:  deklariert
Class:    activation

# A `[TransactionModel]` decides what the runner does with the case's writes

`[TransactionModel(Model: TransactionModel)]` on a `[Test]` procedure. Three values:
**`AutoRollback`** -- the default -- undoes everything the case wrote; **`AutoCommit`** keeps it;
**`None`** leaves the transaction to the case itself.

That choice is the whole isolation story of the suite: 2 291 cases run against one database, and
what makes case 2 independent of case 1 is that case 1 rolled back.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**4 293 `[TransactionModel` declarations.** The default is `AutoRollback`, so the annotated cases are
the ones asking for something else -- which makes the population the count of EXCEPTIONS, and every
one of them a case that would leak state if the runner ignored it.

## The IST-state -- half implemented, and the half that exists is the mapping

`src/gen/CodeunitWriter.cpp:65` -- `TransactionModelOf(procedure)` walks the raw attribute list,
finds one whose lowered text contains `transactionmodel`, and returns
`TransactionModel::AutoCommit`, `TransactionModel::None` or `TransactionModel::AutoRollback`.
So the value reaches the generated `kTestMethods` entry.

**What does not exist is the runner acting on it.** `src/rt/TestRunner.cpp` has the field and no
boundary to apply it to; board:0039 owns that boundary and this item owns the value reaching it.
Verdict `deklariert` for exactly that split.

**And the parse is a substring match**, so an attribute whose text merely CONTAINS `transactionmodel`
would be read as one -- unlikely, and worth replacing with `HasAttribute` plus an argument parse
when this is worked.

## The choice

`TestRunner` opens a boundary per `[Test]` procedure and closes it according to the value:
`AutoRollback` discards, `AutoCommit` keeps, `None` opens nothing and lets the case's own `Commit`
and errors decide.

**`None` is the one that constrains the design**: with no boundary the runner cannot undo a case,
so isolation between cases becomes the CASE's responsibility -- which is what the value means and
why board:0039 cannot make the boundary unconditional.

## Ordering

After 0223 identifies the cases and after board:0039's boundary exists. Before any case that writes,
which is nearly all of them.

## Gate, and its negative control

Three cases, one per value, each inserting a row: after the run the `AutoRollback` row is gone, the
`AutoCommit` row is there, and the `None` row is whatever the case's own `Commit` left.

**The negative control is the `AutoCommit` case** -- a runner that rolls back unconditionally passes
the first and third and fails only this one, and a suite that never uses `AutoCommit` would never
notice.

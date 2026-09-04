Type:     task
Status:   open
Parent:   0039
Area:     rt, gen
Source:   developer/properties/devenv-testisolation-property.md, developer/properties/devenv-requiredtestisolation-property.md, developer/properties/devenv-testtype-property.md
Verdict:  fehlt
Class:    activation

# `TestIsolation` rolls back even a committed change

**Three pages, one item**: the isolation a test RUNNER provides, the isolation a test codeunit
REQUIRES, and the type that groups them. The two newer pages exist to constrain the first and each
names the others.

> **TestIsolation** (Codeunit, runtime 1.0): which changes to roll back after the tests in the **test
> runner** codeunit execute. `Disabled` (**default**) -- no rollback, tests are not isolated;
> `Codeunit` -- roll back after each test codeunit; `Function` -- after each test method.
>
> **"NOTE: If you specify that you want to roll back database changes, then ALL database changes are
> rolled back, INCLUDING CHANGES THAT WERE EXPLICITLY COMMITTED to the database during the test by
> using the `Commit` method."**
>
> **RequiredTestIsolation** (runtime 16.0): the isolation a test codeunit requires. `None`,
> `Disabled`, `Codeunit`, `Function`. **"If the selected TestRunner doesn't satisfy the property,
> then the tests MIGHT FAIL."**
>
> **TestType** (runtime 16.0): `UnitTest` (**default**), `IntegrationTest`, `Uncategorized`,
> `AITest`.

**The rollback-through-`Commit` clause is the item.** CLAUDE.md's first invariant says a `Commit()`
makes what came before it durable and no later rollback may undo that. Test isolation is a documented
exception to exactly that, and it is not optional: the UT milestone runs 2 291 tests that write, and
without it every test sees the previous one's rows.

So the runtime needs a mode in which a `Commit` is not durable, and it must be reachable **only** from
the test runner -- because the same code path in a posting would be the invariant broken.

board:0062 already measured the neighbouring property: `TestPermissions = Disabled` **66 of 80** UT
codeunits.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`TestType =` **564** · `RequiredTestIsolation =` **64** · `TestIsolation =` **7**.

**Seven `TestIsolation` declarations**, because the property is on the RUNNER and there are few
runners; 564 `TestType` because it is on every test codeunit. And `Disabled` is the default, so a
runner that declares nothing isolates nothing -- which is what `agiru run-tests` would do today.

## The IST-state

`src/gen/CodeunitWriter.cpp:60` reads `Subtype == "test"` and `:65` reads `TransactionModel` from an
attribute; neither isolation property is read. `src/rt/TestRunner.cpp` exists (board:0039).

## The choice

A savepoint per test codeunit or per test method, taken by the RUNNER, with the session's `Commit`
made non-durable inside it. **Non-durable means the savepoint is not released**, not that `Commit` is
skipped -- the AL code must still see its own committed rows, which is the whole difficulty.

`RequiredTestIsolation` is checked before the run: a codeunit requiring `Function` under a `Disabled`
runner is refused rather than silently failing later, which is more than BC's "might fail".

`TestType` is metadata for grouping and has no runtime behaviour.

## Ordering

Inside board:0039, before the 2 291 can be trusted: without isolation a green run may be an ordering
accident.

## Gate, and its negative control

Two test methods that insert the same primary key both pass under `TestIsolation = Function`; the
second fails under `Disabled`.

**The negative control is a test that calls `Commit`** -- its rows must ALSO be gone after the
rollback, which is the documentation's own clause and the one an implementation built on ordinary
transaction semantics gets wrong.

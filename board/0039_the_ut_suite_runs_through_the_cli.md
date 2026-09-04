Type: root
State: open
Area: cli, rt

# The UT suite runs through the CLI, the way BC runs it through a cmdlet

`make test` is the C++ gate over `src/`: 30 cases, seconds, and it proves the runtime is right about
what it already does. The 2 392 come from somewhere else, and that somewhere does not exist yet.
`src/cli/Main.cpp` is 55 lines that print reference paths -- in German, which the repository's own
rule forbids.

## Reference

**BC HAS NO TEST FRAMEWORK BESIDE THE PLATFORM.** `Run-TestsInBcContainer` calls
`Invoke-NavCodeunit`, and the work is done by the test-runner codeunits: `Test Runner` walks the
`[Test]` procedures of a codeunit with `Subtype = Test`, runs each one in its own isolation, catches
what it raises and writes the verdict into the test-suite tables. The cmdlet is a door in front of
that and nothing more. Those codeunits are already transpiled -- `apps/test_runner`, 37 objects.

**So the CLI is that door and NOT a second framework.** `agiru run-tests --suite <name>` opens the
session and the database, selects the codeunits the suite names, and hands them to the runner.

**Predecessor**: openerp stands up a test database per runner and rolls back per test, and its
backlog carries the reverts -- the savepoint stack whose `Commit` must SWAP rather than release is
the one that cost most. Grep there before deriving the isolation from scratch.

## What it needs that does not exist

- **Three more test properties the runner reads, and none is parsed** (found 2026-09-04,
  board:0071). `TestIsolation` itself is understood -- `RunnerDatabase` clones a template per run
  and `TestIsolationGate` holds six checks -- but its neighbours are not:

  | property | what it decides |
  |---|---|
  | `RequiredTestIsolation` | the isolation a test codeunit demands to be run UNDER; "if the selected TestRunner doesn't satisfy it, the tests might fail" |
  | `TestType` | `UnitTest` (default), `IntegrationTest`, `Uncategorized`, `AITest` -- how a run is grouped and reported |
  | `TestHttpRequestPolicy` | what an outgoing HTTP request does inside a test (board:0054's `HttpClientHandler`) |

  And `devenv-testisolation-property.md` states the rule that decides where the isolation LIVES:
  **"all database changes are rolled back, including changes that were explicitly committed to the
  database during the test by using the `Commit` method."** A runner therefore UNDOES a `Commit`,
  which CLAUDE.md's first invariant says may never happen -- so the isolation has to sit OUTSIDE the
  AL transaction model. The template clone already does that; it is worth writing down before
  someone implements it as a savepoint and finds the two rules irreconcilable.
  `TestIsolation`'s default is **`Disabled`** -- no rollback at all -- which is not what a runner
  wants and is what an unread property gives you.

- ~~`Codeunit.Run` as a nested savepoint boundary that `asserterror` reads (board:0021).~~ **Done**;
  board:0021 closed and board:0040 records the check: `Boundaries::Commit` releases every open
  savepoint from the inside out and retakes them, `Codeunit<Derived>::Run` opens a boundary and
  catches `Error`, and `TransactionGate` holds ten checks over it.
- `ClearLastError`, `GetLastErrorText`, `GetLastErrorCallStack`.
- **`[TransactionModel(...)]`, which decides whether a test's writes survive it.** 1 230
  declarations in the read roots and the attribute is parsed nowhere (board:0067).
  `attributes/devenv-transactionmodel-attribute.md` gives three values and they are not
  interchangeable:

  | value | the runner does |
  |---|---|
  | `AutoRollback` | opens a write transaction and rolls it back; **a `Commit` call is an ERROR** |
  | `AutoCommit` | opens a write transaction and commits it -- the model for code that commits |
  | `None` | starts NO transaction and **writes FAIL**; `transactionmodel-option.md`: "the transaction model mirrors the model used by the 'real' client. **Every call from the TestPage to the 'server' has its own transaction**" |

  The third row is the one that binds this item to board:0030: under `None` a `TestPage` interaction
  is a transaction boundary, so the page harness and the transaction model are one mechanism and not
  two.

  With `AutoCommit` and `AutoRollback` the triggers the test invokes INHERIT its open transaction;
  with `None` each interaction is separate. board:0040 counts 55 `[Test]` procedures calling
  `Commit`, so the value on those methods is what decides whether they raise or pass, and a runner
  that applies one model to everything gets a share of the suite wrong in each direction at once.
- The `Assert` codeunit and the handler-function registry.
- A database with the CRONUS data behind it (board:0004).

Each of those is a RUNTIME gap and not a CLI gap, which is why this item is a door and a short one.

## THE RUNNER'S OWN RULES, from `devenv-testrunner-codeunits.md` and
## `devenv-test-codeunits-and-test-methods.md` (read 2026-09-04, board:0071)

Five, and each is a decision a runner gets wrong differently:

- **A test codeunit runs its `OnRun` trigger FIRST, then each `[Test]` method.** `OnRun` is the
  codeunit's setup and it is not a test.
- **A test codeunit does not stop on failure.** "When a normal codeunit is run, if one of its
  methods fails, then the codeunit is terminated. When a test codeunit is run, even if the outcome
  of one test method is FAILURE, the next test methods are still running." That is the difference
  between the runner and `Codeunit.Run` (board:0077), and it is what makes a count of 2 291
  meaningful at all.
- **`OnBeforeTestRun` and `OnAfterTestRun` ALWAYS run in their own transactions** -- "regardless of
  the value of the `TestIsolation` property, the value of the `TransactionModel` property, or the
  outcome of a test method". So the runner's own bookkeeping cannot be rolled back with the test,
  which is exactly what a result log needs.
- **`OnAfterTestRun`, if implemented, suppresses the automatic results message.** Implementing it is
  how a runner takes over reporting -- which is what `agiru run-tests` is.
- **A method in a test codeunit is one of three kinds**: `[Test]`, a handler attribute, or
  `[Normal]` -- and `Normal` is a real attribute (`attributes.md`), not the absence of one.

`OnBeforeTestRun` also carries the `TestPermissions` value of the method about to run
(board:0062), which is how the property reaches anything at all.

## What will be true

- [ ] `agiru run-tests --suite <name>` runs one transpiled `[Test]` procedure and reports its
      verdict.
- [ ] It reports a count over a named population, and the population is measured rather than passed
      in.
- [ ] The CLI speaks English.
- [ ] **Negative control**: a test that raises is reported as failed rather than as passed, proven
      by a case that raises on purpose.

## A TEST CODEUNIT DOES NOT STOP AT ITS FIRST FAILURE, read 2026-09-04 (board:0071)

`properties/devenv-subtype-codeunit-property.md` -- one of the 14 property pages outside the sweep's
first denominator -- states the rule that makes a suite a suite:

> When a test codeunit runs, **it executes the `OnRun` trigger, and then executes each test method**
> in the codeunit. **Unlike a normal codeunit, where a failing method terminates the codeunit, a
> test codeunit continues to run its remaining test methods even if one test method fails.**

Two obligations, and the second is the one a runner gets wrong by writing the obvious loop:

- `OnRun` runs FIRST and once, before any `[Test]` procedure. It is the codeunit's own setup and it
  is not a test.
- **Each `[Test]` is isolated from the next by the RUNNER**, so an error escaping one procedure is
  caught, recorded and the next procedure still runs. A runner that lets the error propagate reports
  one failure and 30 not-run, which looks like a much worse regression than it is -- and is exactly
  the shape that makes a red build unreadable.

That isolation is the same boundary this item already carries for the transaction: the runner
UNDOES a `Commit`, so it cannot be a savepoint. One mechanism, two obligations.

The page also gives the five subtypes -- `Normal` (default), `Test`, `TestRunner`, `Upgrade`,
`Install` -- confirming that `Test` and `TestRunner` are properties of the OBJECT and not of the app,
which is what CLAUDE.md's collection rule stands on, and that `Upgrade` and `Install` are board:0070.

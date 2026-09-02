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

- `Codeunit.Run` as a nested savepoint boundary that `asserterror` reads (board:0021).
- `ClearLastError`, `GetLastErrorText`, `GetLastErrorCallStack`.
- The `Assert` codeunit and the handler-function registry.
- A database with the CRONUS data behind it (board:0004).

Each of those is a RUNTIME gap and not a CLI gap, which is why this item is a door and a short one.

## What will be true

- [ ] `agiru run-tests --suite <name>` runs one transpiled `[Test]` procedure and reports its
      verdict.
- [ ] It reports a count over a named population, and the population is measured rather than passed
      in.
- [ ] The CLI speaks English.
- [ ] **Negative control**: a test that raises is reported as failed rather than as passed, proven
      by a case that raises on purpose.

Type:     root
Status:   active
Area:     cli, rt, db
Source:   the isolation A/B of 2026-09-08: 600 -> 315 with a fresh database per codeunit
Class:    activation, measured

# The UT runner starts every codeunit on CRONUS, and nothing leaks between two

**285 OF 600 PASSES WERE LEAKS.** The harness ran the 78 UT codeunits one process after another
against ONE database, and a `Commit()` in a library -- `LibraryERM.CreateGLAccount` and its kin
commit -- left its rows for every codeunit after it. Run with `--fresh` (the runner drops and
recreates its database from the template before each codeunit) the count is 315 of 2 296, and
the `duplicate key` failures go from 37 to 3 -- the rows that collided were the same rows that
carried the 285.

**AND THE TEMPLATE IS EMPTY.** `agiru_master` is 7.5 MB: 680 tables, no rows. BC's own run is
`Run-TestsInBcContainer` against a CRONUS container; the BaseApp's test libraries assume the demo
company -- `The General Ledger Setup does not exist` is the first thing a fresh codeunit says.
The `cronus` database (448 MB, the 28.4 demo dataset loaded one to one, board:0004) holds it, and
`scripts/seed_demo.py` already copies it column by column into the runner's schema.

## The reference

- **`devenv-testing-pages.md` / the test toolkit:** `TestIsolation` on a test codeunit defaults to
  `Codeunit` -- every change a codeunit makes, committed or not, is rolled back when the codeunit
  ends. Between two codeunits nothing survives; within one, a `Commit()` holds.
- **The predecessor** ran on a CRONUS copy and reset it per codeunit; its 2 260 counted from there.

## The choice

**PER CODEUNIT, A FRESH CLONE OF A SEEDED TEMPLATE.** The runner already drops and clones on
`--fresh`; what changes is the template: `agiru_seeded` is `agiru_test_0` after one provisioning
run (the 1 602 tables the binary carries) and one `seed_demo.py`, marked `IS_TEMPLATE`. The
harness passes `--database .../agiru_seeded --fresh`. A clone of a 300 MB template costs seconds
per codeunit, which is what BC's own container reset costs too. `agiru run-tests` without
`--codeunit` will do the same inside one process (drop and clone per codeunit) so a single
invocation reports the honest number.

## What proves it

The milestone measured with `--fresh` on the seeded template, against 315 empty and 600 leaked;
the negative control is a codeunit that inserts a `User Setup` for `SYSTEM` -- it must pass
twice in a row.

## Standing (2026-09-08): the seeded template exists

`seed_demo.py` had two holes of its own, both measured on the first run: its fold of transpiled
names to SQL Server's knew only the dot, so `G/L Account`, `G/L Entry` and every other slashed
table were "only 28.4 has"; and a NULL in a 28.4 blob or text refused the whole table under the
transpiled schema's NOT NULL (`Sales Header."Work Description"`, 18 tables). With every unsafe
character folded and a NULL landing as the column's blank, the second run carries the general
ledger (295 accounts, 2 820 entries) and the documents; one table still refuses, `Employee
Relative`, on a column name with a quote in it -- the shell quoting of the copy pipe, not the data.

`agiru_seeded` is that database as a template; the harness runs `--database .../agiru_seeded
--fresh` per codeunit.


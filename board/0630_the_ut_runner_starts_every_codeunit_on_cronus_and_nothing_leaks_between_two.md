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

## Standing 2026-09-09: the measurement after one instance per run and the two-spelling forms

Chain 22 measured 603 of 2 249 over 75 codeunits against 674 of 2 296 over 78 before it, and the
number is read as three things and not one: 416 of the new reds are ONE site --
`ContBusRel.SetFilter("Link to Table", '<>''''')`, the blank Option member spelled as an empty
string, bound as `''` into an integer column -- which the `API Setup UT` tests reached for the
first time because their `Manual` subscriber is bound once now; three codeunits (`Phys. Invt. COD
UT`, `Phys. Invt. Order Line TAB UT`, `Phys. Invt. Order Subform UT`) die with SIGSEGV in
`StateHandle::operator=` while `AdoptRecord` copies a marked record into a page, a path the
statement form of `Codeunit.Run` now reaches; and the two codeunits that print no total were
never registered (`Graph Collect Mgt Item UT`, `O365 Integration Record UT` are not in the
slice). The blank member binds as its ordinal now; the crash is under an address-sanitizer build
in `build-asan/`, configured by hand from the Makefile's own invocation with `-fsanitize=address`.

The sanitizer named the crash: `RunPageByNumber`'s taker accepted an `Instance<Record>` handle as
a record because `TableTraits<Instance<T>>` is specialised, and took the handle's own address.
Every by-number run and `AdoptRecord` dereference a handle first now (board:0030's page runner).

## Standing 2026-09-09: two builds lost to a door edit under the build

Build 24 died on `Catalogue.h` and build 26 on `JsonObject.h`, each edited in `include/` while the
chain's `ninja` was compiling against the precompiled header, each reported as "file has been
modified since the precompiled header was built" and followed by a page of artefact errors in
files byte-identical to the committed tree. CLAUDE.md names this trap; the guard that holds is
mechanical -- `ps` shows no `ninja` before any edit under `include/` -- and every door edit
during a build is prepared in the scratchpad and applied only when the chain has printed its
`make exit`.

## Standing 2026-09-09: two more shells killed themselves, and a run by type would not link

The `pkill -f "chai[n]6\.sh"` guard holds only while NOTHING ELSE on the same command line spells
`chain6.sh`; a `sed` on the script and its `nohup` in the same command did, and the shell died
with 144 twice before running the edit it was meant to run. The name is spelled through a glob
(`ls $S/chai*6.sh`) or a split string (`"chai""n6"`) now, and the kill and the start never share a
command. The other finding: a synthesised action that ran its page BY TYPE
(`X_Page{}.Run(rec)`) made every page carrying one link against X's source, and most X are not in
the slice; the run goes BY NUMBER through the page catalogue (`Page<>::Run(5897, rec)`), which is
what the AL does too and refuses honestly at run time when the binary carries no such page.

## Standing 2026-09-09: a probe during a measurement costs a codeunit

`--fresh` clones the seeded template per codeunit, and PostgreSQL refuses a `CREATE DATABASE ...
TEMPLATE` while another session holds the template: a probe run started beside the measurement
made the measurement's next clone fail and `ERM Purch. Cr. Memo Aggr. UT` printed no total. A
measurement OWNS the template while it runs, the way a build owns the tree; a probe waits for the
milestone line, or clones from its own copy of the template.

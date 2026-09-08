Type:     root
Status:   open
Area:     rt, db, cli
Source:   the first run over the WHOLE population, 2026-09-08
Class:    activation

# The runner works against the demo data, and not against an empty schema

**THE WHOLE POPULATION RAN FOR THE FIRST TIME: 360 of 19 934 passed, 3 codeunits died.** That is
the measurement this item exists to move, and the ranked failures say where the lever is.

**~7 000 OF THE 19 543 FAILURES ARE ONE SENTENCE**, and it is not about the runtime:

| failures | message |
|---|---|
| 2 165 | `The Sales & Receivables Setup does not exist. Identification fields and values:` |
| 2 034 | `The Purchases & Payables Setup does not exist.` |
| 1 288 | `The Inventory Setup does not exist.` |
| 989 | `The Service Mgt. Setup does not exist.` |
| 603 | `The General Ledger Setup does not exist.` |

**THE RUNTIME IS ANSWERING CORRECTLY.** `Get` on a setup table finds nothing because the runner's
database holds nothing: `agiru_test_0` carries the 1 602 tables of the schema in `public`, all
EMPTY, and `Sales & Receivables Setup` has 0 rows (measured 2026-09-08). BC's own test run does not
work like that -- `Run-TestsInBcContainer` runs against a CRONUS container, and the BaseApp's test
libraries assume the demo company is there.

## What is already built, and what is not

**THE DEMO DATA IS ALREADY IN POSTGRESQL.** `make cronus` transferred it: the `cronus` database
carries 1 864 tables under the schema `"CRONUS International Ltd"`, 412 714 rows, with
`Sales & Receivables Setup` holding its one row (board:0004).

**WHAT IS MISSING IS THAT THE RUNNER'S TEMPLATE DOES NOT CARRY IT.** `agiru_master` is the schema
and nothing else, and every runner database is a clone of it -- so every test starts against an
empty company.

**1 019 OF THE RUNNER'S 1 602 TABLES EXIST IN CRONUS BY NAME** (measured 2026-09-08 by comparing
`information_schema.tables` on both). The other 583 are tables 28.4 does not have, which is the
version gap CLAUDE.md carries openly: the demo database is 28.4 and the source is 30.0.

## The two routes, and why the second one is not free either

1. **SEED THE MASTER FROM CRONUS.** Copy the rows of the 1 019 shared tables into `agiru_master`'s
   `public` schema, once, at provisioning time. Every cloned runner database then starts where a BC
   test expects to start. It is faithful -- it is the same data BC's own container runs against --
   and the transfer is written. What it needs is the COLUMN mapping per table, which is where
   board:0004's unmapped columns live: a column the 30.0 schema has and 28.4 does not is left at
   its default, and the reverse is a column with nowhere to go.
2. **LET THE TEST LIBRARIES BUILD IT.** BC's own `Initialize` in each test codeunit calls
   `LibraryERMCountryData` and friends, which INSERT the setup rows. That is what the AL says and
   it needs no data at all -- but those library codeunits are largely unlinked or refusing here, so
   the route runs through the same backlog the rest of this loop is working down.

**THEY ARE NOT ALTERNATIVES.** BC does both: the container has the demo data AND the libraries
create what a test needs on top. Route 1 is what makes route 2's failures legible, because a test
that fails on `Initialize` today cannot be told apart from a test that fails on an empty database.

## What proves it

`agiru run-tests --isolate` reports a pass count that rises, and the five "does not exist" messages
above fall out of the ranked failures. The negative control is the count itself: with the master
unseeded the same five stand at the top, which is where they are today.

## What made the measurement possible

**A CODEUNIT PER PROCESS.** One test walked off the end of memory and took the whole run with it --
855 codeunits reported as nothing, because a segmentation fault is not an exception and nothing in
the process can catch it. `agiru run-tests --isolate` re-enters the same binary with `--codeunit`
per codeunit, so the isolation is the operating system's; the runner's database is made once and
kept, so every child finds it rather than cloning it. 3 codeunits die and the other 852 report.
That is board:0612's standing half, and it closes with this item's measurement.

## The MILESTONE's own number, which is not the whole population's (2026-09-08)

The goal is the 78 UT codeunits and their 2 291 `[Test]` procedures, not the 855 codeunits the
binary carries. Measured apart:

| | |
|---|---|
| UT codeunits the binary carries | 62 |
| their `[Test]` procedures | 1 708 |
| passing | **201** |

The gap to 78 and 2 291 is codeunits whose generated source is not yet in `test/slice`, which is
the compile-fix loop's own backlog and not the runtime's.

**AND THE SETUP DATA IS ~600 OF THE 1 507 FAILURES HERE TOO** -- Purchases & Payables 140, Sales &
Receivables 138, General Ledger 123, Inventory 89, VAT Report 70, VAT Reg. No. Srv Config 40. The
same lever, one third the size of the whole population's, and still the second largest after `xRec`.

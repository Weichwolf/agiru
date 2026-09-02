Type: arc
State: open
Area: db
Tags: provision, owner

# The CRONUS dataset stands in PostgreSQL and demonstrably comes from 28.4

`make provision` fetches the artefact, restores it into SQL Server and brings up PostgreSQL. The
last step is missing: the data is not across yet, because the target schema is what the transpiler
has yet to emit.

## Measured, 2026-09-02, by `make schema`

| | |
|---|---:|
| relations in CRONUS | 2 129 |
| of them the platform's own `$ndo$...` storage, which no `.al` declares | 44 |
| matched against a generated table | **1 450** |
| with no generated counterpart | 635 |
| columns landing on a declared field | **30 654 of 30 818 -- 99.47 %** |

**The 164 columns that do not land are one cause and not a hundred.** `Capacity Ledger Entry` alone
carries 17 of them -- `Routing No_`, `Work Center No_`, `Setup Time`, `Run Time` -- and they are
fields a `tableextension` adds. board:0033 merges those at translation time, and this is the
measurement that says how much it is worth: about 0.5 % of the dataset's columns.

**The 635 unmatched tables are apps that are not in the read roots**, not gaps in the three that
are: `AIT Column Mapping`, `AIT Log Entry` and the rest are the test toolkit's own tables, and one
is a bare GUID -- an extension's storage.

**So the 28.4-against-30.0 gap is not what is standing in the way.** It was the open question and
the number answers it: the schema divergence between the demo database and the source is smaller
than the extension gap by an order of magnitude.

## Reference

**Measured 2026-09-01, without downloading** -- over a range request on the zip's central directory:

| | |
|---|---|
| CDN | `bcartifacts-exdbf9fwegejdqak.b02.azurefd.net`. The blob host answers nothing (`AuthorizationFailure`, network security perimeter); `bcartifacts.azureedge.net` no longer resolves |
| newest on-prem artefact | `28.4.53241.0/w1`, 372 706 292 B -- there is no 29.x and no 30.x |
| inside it | `database/Demo Database BC (28-0).bak`, 824 299 520 B |
| BCApps `main` | carries `30.0.0.0` and is the ONLY ref with a BaseApp at all |
| BCApps `releases/27.x` .. `28.x` | **zero** BaseApp `.al` files -- no `Sales Header`, no `Sales-Post` |

**NO PAIRING EXISTS, AND THE SOURCE IS THE REPOSITORY ON `main` BY DECISION.** Pinning the source
to the demo database's version is not available: no release branch carries the BaseApp at all. The
artefact does carry it (8 095 `.al` files at 28.4, readable over a range request for 44 MB instead
of the platform artefact's 1.37 GB) and that route was built and measured here before being
discarded -- it is in `git log` if it is ever needed. The transpiler reads `~/Git/BCApps` on `main`
directly, and the repository is never switched.

**It is temporary.** Version 30 ships in a few weeks and closes the gap on its own.

**How large the gap is today, measured rather than feared:** `Sales Header` carries the SAME 183
fields under 28.4 and 30.0 -- same numbers, same names, none present in only one. The divergence
sits in procedures (666 against 654). A field that does differ shows up in this item's own mapping
as an unmapped column, which this item already refuses to drop silently. **That makes this item the
place where the version gap becomes visible, and the list of unmapped columns its measurement.**

The container listing over Front Door is **cached by path and not by query**: two requests to
`/onprem?...` with different `prefix` return the same answer. Whoever needs the version list varies
the PATH (`//onprem/`), not the query. That cost a quarter of an hour here and is written down for
that reason.

**Platform documentation**: BC stores per company as `<Company>_$<Table>$<AppGuid>`, columns in BC's
SQL encoding of the AL field name (`No.` -> `No_`). Tables with `DataPerCompany=No` stand without a
company prefix.

**Predecessor**: `scripts/setup/cronus_bak_loader.py` solves exactly this mapping -- generically,
with no AL object names: table via `al_name_to_snake` with a collapse fallback, column via collapse
comparison, BC null date `1753-01-01` -> empty. **That file is the template and its comments are
days already paid for.** What it also shows: the second run went over a committed `pg_dump` rather
than SQL Server -- the container is a pass-through, not part of a test run.

**The choice:** transfer via `bcp` out and `COPY` in, not over a database driver. A driver would be
a dependency for something that runs once per release, and `bcp` is in the image that is there
anyway.

## What will be true

- [ ] Every table of the transpiled schema that has rows in CRONUS has them in PostgreSQL too, at
      the same row count.
- [ ] The mapping names **no** AL object -- it is a naming rule, not a table.
- [ ] What could not be mapped stands there as a list and is not passed over in silence: a silent
      omission is the defect that comes back looking like a runtime defect.
- [ ] `agiru_master` is read-only afterwards and serves as a template; a run clones it via
      `CREATE DATABASE ... TEMPLATE`.
- [ ] Proof: row counts per table on both sides, and the sum of the amounts of one entry table equal
      digit for digit -- the arithmetic proof `Decimal` still owes, now that the type stands.
- [ ] **Negative control**: drop one column from the mapping and require the comparison to go red. A
      comparison that only counts rows does not notice a missing column.

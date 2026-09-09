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

## THE DATA IS ACROSS. Measured 2026-09-07, on the running instance

`podman start agiru-pg` and the instance answers. What is in it:

| database | size | what |
|---|---:|---|
| `cronus` | **311 MB** | the demo dataset, copied out of SQL Server |
| `agiru_test_0` | 19 MB | a test run's own database, 341 tables in `public` |
| `agiru_master` | 8.2 MB | the template; it refuses connections, which is what a template does |
| `agiru_gate` | 8.1 MB | the gate cases' database, 3 tables |
| `agiru` | 7.5 MB | empty |

**A COMPANY IS A SCHEMA AND NOT A PREFIX, and that is the thing to remember about this instance.**
`cronus` holds

| schema | tables |
|---|---:|
| `CRONUS International Ltd` | **1 864** |
| `system` | 220 |
| `platform` | 44 |
| `AIT Eval Monthly Copilot Cred` | 1 |

and **`public` is empty**, which is how a session that queries `information_schema.tables WHERE
table_schema = 'public'` concludes the database is empty and is wrong. That happened here, and it is
recorded rather than corrected in silence: the company's own schema is where a company's data lives,
`system` is what `DataPerCompany = false` puts aside, and `platform` is the virtual tables.

**412 544 live rows**, the widest being the platform's own text storage (`$ndo$textmap` 139 173,
`$ndo$textobjectmap` 139 173, `$ndo$textlookup` 52 030) and then `Application Object Metadata`
14 192 and `Calendar Entry` 9 378.

`max_locks_per_transaction` is 1 024 on the running instance, so the note in CLAUDE.md about
`postgresql.auto.conf` not surviving `podman rm` has held.

**What this closes and what it does not.** The first three boxes of the list above are answered by
the transfer having happened; the ROW-COUNT PROOF and the negative control are not, because nothing
has compared the two sides since. The item stays open for the proof and not for the transfer.

## 2026-09-09: the platform tables the UT suite reaches, and where their rows come from

| table | rows | state |
|---|---|---|
| `All Profile` 2000000178 | one per translated `profile` object, written by `ProvisionInstalled` | done; 246 cases stopped on it |
| `AllObj` / `AllObjWithCaption` | one per installed table, codeunit, page, query, report -- the catalogues have them | open, 29 cases |
| `Feature Key` 2000000211 | the platform's feature registry; no AL source names the keys | open, 20 cases |
| `Privacy Notice` | stored in the `system` schema of the demo database, with two FlowFields over `Privacy Notice Approval` | open, 27 cases |

The demo database stores `system."Profile"` and `system."Tenant Profile"` EMPTY: on BC the
profiles a user sees are the installed apps' `profile` objects, unioned with the tenant's own, and
`All Profile` is that union. So the rows are the catalogue's and not the backup's.

`Company` in the seeded template holds no row, so the first `TableRelation` check against it
(`Application Area Setup."Company Name"`, `Country/Region UT.T120`) refused the test company. The
provisioning step now writes the session's company into `Company` when it is missing, which is what
a BC install does when a company is created; the row is the platform's and not the backup's.

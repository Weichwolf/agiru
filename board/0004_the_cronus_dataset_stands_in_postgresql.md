Type: arc
State: open
Area: db
Tags: provision, owner

# The CRONUS dataset stands in PostgreSQL and demonstrably comes from 28.4

`make provision` fetches the artefact, restores it into SQL Server and brings up PostgreSQL. The
last step is missing: the data is not across yet, because the target schema is what the transpiler
has yet to emit.

## Reference

**Measured 2026-09-01, without downloading** -- over a range request on the zip's central directory:

| | |
|---|---|
| CDN | `bcartifacts-exdbf9fwegejdqak.b02.azurefd.net`. The blob host answers nothing (`AuthorizationFailure`, network security perimeter); `bcartifacts.azureedge.net` no longer resolves |
| newest on-prem artefact | `28.4.53241.0/w1`, 372 706 292 B -- there is no 29.x and no 30.x |
| inside it | `database/Demo Database BC (28-0).bak`, 824 299 520 B |
| BCApps `main` | carries `30.0.0.0` -- **there is no demo database for that** |
| BCApps `releases/28.4` | carries `28.4.0.0` -- that is the pair |

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

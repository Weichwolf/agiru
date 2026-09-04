Type:     task
Status:   open
Parent:   0034
Area:     gen, rt, db
Source:   developer/properties/devenv-tabletype-property.md
Verdict:  fehlt
Class:    activation

# `TableType` says whether a table is a table at all

Seven values, and only one of them is a row in this database:

| value | what it is | since |
|---|---|---|
| `Normal` | a table in the BC database (default) | 1.0 |
| `CRM` | an integration table against Dataverse | 1.0 |
| `CDS` | the same, newer name | 5.0 |
| `ExternalSQL` | a table or view in **another** SQL Server database | 1.0 |
| `Exchange` | not supported in BC online | 1.0 |
| `MicrosoftGraph` | not supported in BC online | 1.0 |
| `Temporary` | an **in-memory** table | 6.0 |

> Tables marked as **CDS** or **ExternalSQL** are considered **external tables that are not managed
> by Business Central. These tables use a different SQL Server connection** than the normal tables.
>
> Marking a table as **Temporary** is the same as setting `SourceTableTemporary` on all pages that
> use the table. **Temporary tables are not synchronized with the SQL database.**

**So this one property decides whether the schema writer emits a `CREATE TABLE` at all**, and
`src/rt/Storage.cpp:94` currently emits one for every table it is handed.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`TableType =`: **386 declarations** -- `Temporary` **298**, `CRM` **83**, `Exchange` **4**,
`MicrosoftGraph` **1**. `CDS` and `ExternalSQL` do not appear; `Normal` is the default and needs no
writing.

**298 tables are in-memory and would get a real table today.** That is the finding: not a missing
feature but 298 `CREATE TABLE`s for tables BC never creates.

## The IST-state

Not among the nine properties the generator consumes (board:0067). `src/rt/Storage.cpp:94` creates a
relation for every table; `src/rt/Table.cpp` knows no temporary mode.

## The choice

A `TableType` enumerator on `TableDef`, and three different fates:

- **`Temporary`** is board:0032's territory and the largest population: no column, no cursor, an
  in-memory row set with the same `Record` surface. The property is one input to that, not the item.
- **`CRM`, `CDS`, `ExternalSQL`** name a SECOND connection to a database agiru does not have. Refused,
  naming the property and the table -- 83 declarations, all `CRM`, and refusing them is honest where
  emitting a local table for a Dataverse entity would be silently wrong data.
- **`Exchange`, `MicrosoftGraph`** are documented as unsupported by BC itself. Refused, with the
  documentation's own sentence as the reason.

**Not the alternative** -- creating a local table for a `CRM` type. It would accept every read and
return rows nobody wrote, which is the worst shape a defect can take here.

## Ordering

Before the schema writer grows anything else: it decides which tables the writer sees at all.

## Gate, and its negative control

A `Temporary` table produces no `CREATE TABLE`; a `CRM` table fails to transpile.

**The negative control is the row count** -- a `Temporary` table that silently got a real relation
still answers `Insert` and `Get` correctly, so only checking the SCHEMA, not the behaviour, catches
it.

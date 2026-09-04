Type:     task
Status:   open
Parent:   0070
Area:     rt, db
Source:   developer/devenv-data-transfer.md
Verdict:  fehlt
Class:    activation

# `DataTransfer` is a set operation, and only upgrade code may use it

> `DataTransfer` "supports the **BULK TRANSFERRING of data between SQL based tables**. Instead of
> operating on a row-by-row model, like the record API does, **DataTransfer PRODUCES SQL CODE THAT
> OPERATES ON SETS.** This behavior improves the performance when moving data during **upgrade and
> install**."
>
> ```AL
> dt.SetTables(Database::FromTable, Database::ToTable);
> dt.AddFieldValue(2, to.FieldNo("SmallCodeField"));
> dt.CopyRows();
> ```
> -- against the row-by-row `if from.FindSet() then repeat ... to.Insert() until from.Next() = 0`.

**This is the one place AL exposes a set operation**, and it exists because the row loop is too slow
for an upgrade over board:0045's row counts. So it is not a convenience wrapper: `CopyRows` is an
`INSERT ... SELECT` and `CopyFields` an `UPDATE ... FROM`, and translating it back into a loop would
defeat the only reason it exists.

## Five restrictions and a scope check

> **"The DataTransfer object CAN ONLY BE USED IN UPGRADE CODE and it'll THROW A RUNTIME ERROR if used
> outside of upgrade codeunits."**
>
> "Using it in INSTALL codeunits, **it's checked that the install code is running inside the SCOPE of
> installing an extension**, meaning triggered from `OnInstallAppPerDatabase` and
> `OnInstallAppPerCompany`."
>
> Cannot be used on: **non-SQL tables · system tables · virtual tables · AUDITED tables as the
> destination · OBSOLETED tables as the destination.**

**A runtime scope check, not a compile-time one** -- and board:0500's install and upgrade drivers are
where the scope is set. So this item needs a session flag those drivers raise, which is the same flag
board:0514 needs to switch isolated events off during install and upgrade. **One flag, two consumers.**

**Two of the five restrictions are about the DESTINATION only** -- audited and obsoleted tables --
which means the check is per role and not per table.

## The builder is four steps and the third is a JOIN

> 1. `SetTables` -- source and destination
> 2. `AddFieldValue` (field-to-field) or `AddConstantValue` (constant into destination)
> 3. **`AddJoin`** -- **"define the RELATIONSHIP between the source and destination tables ... In most
>    cases, this method is REQUIRED for copying fields."**
> 4. `AddSourceFilter` -- constraints on what to transfer

**`AddJoin` makes this the second place AL declares a real SQL join**, after board:0461's query data
items. And `CopyFields` without a join would be a cross product, which is why the documentation says it
is required in most cases.

**Two terminal operations**: `CopyRows` (whole rows, for an obsoleted TABLE) and `CopyFields` (named
fields, for an obsoleted FIELD or a default value).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`DataTransfer` is a data type used by method call; board:0028 owns the census. **Stated rather than
guessed** -- and the count is the number that says how much of the BaseApp's upgrade path needs this
before board:0070's drivers can run at all.

## The IST-state

board:0070 records the app install/upgrade state; board:0500 files the two drivers.
`include/type/DataTransfer.h` would be the door's per-type file (board:0051) -- **whether it exists is
this item's first check** and is not measured here.

## The choice

A builder that accumulates `{ source table, destination table, field pairs, constants, joins, filters }`
and emits ONE statement -- `INSERT ... SELECT` for `CopyRows`, `UPDATE ... FROM` for `CopyFields`.

**Not a loop.** The whole point is the set operation, and CLAUDE.md's benchmark rule says the
comparison is the same operation in plain SQL from `psql`.

The scope check reads the session flag board:0500's drivers raise; the five table restrictions are
checked against the metadata board:0364 and board:0069 already carry.

## Ordering

Behind board:0500's drivers, which set the scope. Behind board:0364's `TableType` and board:0069's
obsolete state, which the restrictions read.

## Gate, and its negative control

`CopyRows` between two tables produces one `INSERT ... SELECT` and the destination's row count matches
the source's; the same call outside an upgrade codeunit raises.

**The negative control is the statement count** -- a loop-based implementation produces the right rows
and N statements, and only counting the statements or comparing against `psql` shows it. That is the
whole reason the type exists.

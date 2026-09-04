Type:     task
Status:   open
Parent:   0045
Area:     gen, db
Source:   developer/properties/devenv-compressiontype-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `CompressionType` is carried, and PostgreSQL answers it differently

> **Version**: runtime 3.0. Applies to: **Table.**
>
> `Unspecified` -- use the compression type specified externally on the table, for example in SQL
> Server. `None` -- no compression. `Row` -- compress on a row level. `Page` -- compress on a page
> level, including row, prefix and dictionary compression.
>
> The `TableType` property must be `Normal`. **This property cannot be used on table extension
> objects.**
>
> With `None`, `Page` and `Row`, the table synchronization process **will make changes to the table
> in SQL Server, overwriting the current compression setting**. `Unspecified` lets you control data
> compression directly on SQL Server.

**SQL Server's row and page compression have no PostgreSQL counterpart.** PostgreSQL compresses at a
different layer entirely -- TOAST, per oversized value, with `lz4` or `pglz` -- which is not a table
setting and does not compress a row of small columns at all. So `Row` and `Page` cannot be translated;
they can only be dropped or refused.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`CompressionType =`: **1 declaration**, and it is `Page`.

One table in the entire BaseApp. Whichever it is, it is a table somebody measured.

## The IST-state

Not among the nine properties the generator consumes (board:0067). `src/rt/Storage.cpp:94` emits a
plain `CREATE TABLE` with no storage parameters.

## The choice

**Carry it into `TableDef` and act on nothing, with the divergence named** -- the handling board:0012
established for the missing dirty read and board:0348 takes for the clustered index. `Unspecified` and
`None` are already what PostgreSQL does; `Row` and `Page` are a SQL Server storage feature with no
equivalent, so mapping either onto a `TOAST` setting would be answering a different question.

**Not refusing it**, even at one declaration: the property does not change what the table CONTAINS,
only what it costs on disk, so ignoring it is correct behaviour rather than a silent gap. That
distinguishes it from `LinkedObject` (board:0366), which is also unpopulated and IS refused, because
that one changes the transaction.

The two preconditions are `static_assert`s: `TableType = Normal`, and not on a `tableextension`.

## Ordering

With board:0067's census. No dependency.

## Gate, and its negative control

The one table declaring `Page` transpiles and gets a plain `CREATE TABLE`; a `tableextension`
declaring the property fails to transpile.

**The negative control is the `tableextension`** -- an implementation that carries the property
without the precondition accepts a declaration AL rejects, and the one legal declaration cannot show
it.

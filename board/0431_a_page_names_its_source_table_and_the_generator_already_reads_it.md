Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-sourcetable-property.md, developer/properties/devenv-sourcetabletemporary-property.md
Verdict:  teilweise
Class:    activation

# A page names its source table, and the source may be temporary

**Two pages, one item**: `SourceTable` names the table and `SourceTableTemporary` says the page's copy
of it is in memory. The second is meaningless without the first and changes what the first means.

> **SourceTable**: Sets the ID of the table from which this page will display records. Applies to:
> **Page, Request Page.** "There are some page types that do not support having a source table."
>
> **SourceTableTemporary** (default **false**): whether the source table is a temporary table.

**And `TableType = Temporary` is documented as the same thing from the table's side**: board:0364
quotes it -- "Marking a table as Temporary is the same as setting `SourceTableTemporary` on all pages
that use the table." So one page property and one table property produce the same state, and a
temporary record is board:0032's, not board:0030's.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`SourceTable =` **6 295** · `SourceTableTemporary =` **680** (all necessarily `true`).

6 295 against `PageType`'s 6 891 -- so roughly 600 pages have no source table, which is the
documentation's "some page types do not support having one": the dialogs, the role centres and the
`UserControlHost`s.

## The IST-state, and it is why this is `teilweise`

`src/gen/PageWriter.cpp` consumes `SourceTable` -- it is one of the nine properties the generator
knows (board:0067), and the only page property among them. `SourceTableTemporary` is not read.

So the page knows its table and nothing else about itself.

## The choice

The `SourceTable` half stands. `SourceTableTemporary` is one bit that selects board:0032's in-memory
record instead of a cursor -- and it must select the SAME mechanism `TableType = Temporary` does, or
there are two temporary records with different behaviour.

**Which page types support a source table is a `static_assert`** once board:0429 puts the type in the
metadata: a `SourceTable` on a type that does not support one is a declaration BC rejects.

## Ordering

Behind board:0032's temporary record. With board:0429, which supplies the type for the assertion.

## Gate, and its negative control

A page declaring `SourceTableTemporary = true` writes nothing to the database and its rows survive
until the page closes.

**The negative control is the database** -- a temporary source that silently used a real table
behaves identically from the page and leaves rows behind, which only a query outside the page finds.

Type:     task
Status:   open
Parent:   0034
Area:     gen
Source:   developer/properties/devenv-externalname-property.md, developer/properties/devenv-externalschema-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# An external table carries the name the other database uses

**Two pages, one item**: `ExternalName` is the table's or field's name in the foreign database and
`ExternalSchema` is the schema it lives in. Both exist only for the same two `TableType` values and
neither means anything without the other or without board:0364.

> **ExternalName** (Table, Table field): Specifies the name of the original table in the external
> database. **This property appears when you specify CDS or ExternalSQL in the TableType property.**
>
> **ExternalSchema** (Table): Specifies the name of the database schema of the external database.
> **This property appears when you specify ExternalSQL in the TableType property.**

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ExternalName =` **3 900** · `ExternalSchema =` **2**.

**3 900 is a surprise and it is the item's real content.** board:0364 measures **zero** tables of type
`CDS` or `ExternalSQL` -- 83 are `CRM`, which the page does not list as needing this property. So
either the `CRM` proxy tables carry `ExternalName` on their FIELDS (which the property's own "Table
field" applicability allows and which the AL Table Proxy Generator would produce), or the property is
used somewhere the page does not describe.

**That is looked up before the item is built, not guessed.** 3 900 declarations against 0 tables of
the type the page names is a contradiction between the documentation and the source, and CLAUDE.md's
rule for that case is that the source declares where the documentation describes.

## The IST-state

Not among the nine properties the generator consumes (board:0067). The schema writer at
`src/rt/Storage.cpp:94` quotes `table.name` and `field.name` -- the AL names -- for every column.

## The choice

Follows board:0364's. If the `CRM` and `CDS` types are refused, these two properties are carried into
the metadata unused rather than refused, because a field-level `ExternalName` on a table that IS
translated must not change the local column name -- the AL name is the column name here
(`src/rt/Storage.cpp:96`) and the external name belongs to a connection that does not exist.

**The trap is exactly that**: an implementation that reads `ExternalName` and uses it as the column
name would rename 3 900 columns to names from a foreign schema.

## Ordering

Behind board:0364, which decides whether those tables exist at all. The 3 900-declaration puzzle is
resolved first, because it may move this item into a different theme entirely.

## Gate, and its negative control

A field declaring `ExternalName` keeps its AL name as its column name.

**The negative control is the column name** -- this item's whole risk is a rename, and only comparing
the emitted DDL against the AL declaration sees it.

Type:     task
Status:   open
Parent:   0364
Area:     gen
Source:   developer/properties/devenv-externaltype-property.md, developer/properties/devenv-externalaccess-property.md, developer/properties/devenv-provider-property.md, developer/properties/devenv-publickeytoken-property.md, developer/properties/devenv-iscontroladdin-property.md, developer/properties/devenv-enableexternalassemblies-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# The external-field properties belong to a table type nobody declares

**Six pages, one item**: the remaining properties describing a field or type that lives OUTSIDE the
Business Central database -- an external SQL table, a Dataverse entity, or a .NET assembly.

> **ExternalType** (Table field): the type of the original field in the external database. **"Used
> when you specify CDS, MicrosoftGraph or ExternalSQL in the `TableType` property."**
>
> **ExternalAccess** (Table field): `Full`, `Insert`, `Modify`, `Read`. **"Appears when you specify
> CDS in the `TableType` property."**
>
> **Provider**, **PublicKeyToken**, **IsControlAddIn**, **EnableExternalAssemblies**: the .NET interop
> declarations -- a DotNet type's assembly, its strong-name token, whether it is a control add-in, and
> whether external assemblies are enabled at all.

**Two groups and both are outside this system.** board:0364 measures **zero** tables of `TableType =
CDS` or `ExternalSQL`, so the first two properties describe a table kind the BaseApp never declares --
and yet `ExternalType` measures **3 789** and `ExternalAccess` **1 752**.

**Those counts together are the finding**: `ExternalName` 3 900, `ExternalType` 3 789,
`ExternalAccess` 1 752, `OptionOrdinalValues` 310 -- **9 751 declarations** of properties the
documentation ties to a `TableType` that measures zero. The explanation is almost certainly the `CRM`
proxy tables (83 of them, board:0364), and **board:0365 owns resolving it**; this item waits on that
answer rather than guessing a fourth time.

The .NET group is board:0035's territory: the DotNet surface is derived from its use, and these four
describe assemblies agiru does not load.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ExternalType =` **3 789** · `ExternalAccess =` **1 752** · `Provider =` **345** ·
`PublicKeyToken =` **45** · `IsControlAddIn =` **1**. `EnableExternalAssemblies` is measured with this
item and was not taken with the theme -- stated rather than rounded.

## The IST-state

Not among the nine properties the generator consumes (board:0067). board:0035 records the DotNet
surface; board:0364 the table types.

## The choice

Follows board:0364 and board:0365. If `CRM` tables are refused, all six are refused with them; if they
are translated, `ExternalType` and `ExternalAccess` are carried and never used to name a local column
-- which is board:0365's trap and the reason these two must not be built independently of it.

The .NET four are refused outright: agiru has no assembly loader, and a `PublicKeyToken` for an
assembly that will never load is a declaration with no meaning here.

## Ordering

Behind board:0365, which resolves the contradiction. With board:0035 for the .NET half.

## Gate, and its negative control

A field declaring `ExternalType` on a `CRM` table fails to transpile with the same message
board:0364's table type produces.

**The negative control is an ordinary field** -- it must be unaffected, and an implementation that
refuses on the property name alone stops 3 789 fields, which this item's own population makes the
likely mistake.

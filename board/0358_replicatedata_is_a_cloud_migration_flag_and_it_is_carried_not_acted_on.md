Type:     task
Status:   open
Parent:   0067
Area:     gen
Source:   developer/properties/devenv-replicatedata-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `ReplicateData` is a cloud-migration flag, and it is carried rather than acted on

> **Version**: runtime 2.0. Applies to: Table.
>
> **True** if the table data is replicated to the cloud service, otherwise **false**. **The default
> is true.**
>
> The property is used when **migrating data from a Business Central on-premises environment to an
> online environment**. It specifies whether the data in the on-premises table is replicated to a
> table in the cloud service as part of cloud migration.

There is no cloud service here and no migration into one, so the property gates a process agiru does
not have. **That is not the same as meaningless**: it is the BaseApp's own statement about which
tables hold data worth carrying between installations, and 1 064 tables answer it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ReplicateData =`: **1 064 declarations**, all necessarily `false` since `true` is the default. So
**1 064 tables of 1 609 declare their data not worth replicating** -- session state, caches,
buffers, the tables that exist to hold a report's intermediate rows.

That ratio is the finding. Two thirds of the schema is scratch, and that is a fact board:0004's
CRONUS load and board:0087's insert buffering would both want before they size anything.

## The IST-state

Not among the nine properties the generator consumes (board:0067).

## The choice

**Carry it into the table metadata and act on nothing.** One bit on `TableDef`, emitted because it is
free and because the first consumer that needs "is this table scratch" -- a backup, a fixture load, a
determinism digest that should not hash a cache -- would otherwise have to invent the list.

**Not refusing it**, unlike the zero-population properties: 1 064 declarations make a refusal a
translation error on two thirds of the schema.

**Not acting on it either.** There is no replication, and a bit that decided something here would be
deciding it on a documented statement about a different system.

## Ordering

With board:0067's census. No consumer, so no dependency.

## Gate, and its negative control

`TableDef` for a table declaring `ReplicateData = false` carries the flag; one declaring nothing
carries the default.

**The negative control is the default** -- a `bool` initialised to `false` inverts 545 tables and
nothing reads it, so nothing complains.

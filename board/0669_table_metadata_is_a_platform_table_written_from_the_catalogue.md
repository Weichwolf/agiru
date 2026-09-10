# `Table Metadata` is a platform table written from the catalogue

**Finding (2026-09-10).** `Record "Table Metadata"` (2000000136) was an absent stub, so
`TableMetadata.Get(TableID)` refused as an unbuilt .NET member: 39 `Get`, 22 `TableType` and 16
`ObsoleteState` reads in the BaseApp (`Workflow Event`, record links, data migration), 27 UT
cases.

**Reference.** `devenv-virtual-tables.md`; the predecessor's `virtual_metadata.py` carries the
field numbers (ID 1, Name 2, Caption 3, ObsoleteState 4, ObsoleteReason 5, TableType 6,
DataPerCompany 7, LookupPageID 8, DrillDownPageID 9) and notes that no BaseApp site names one by
number.

**Choice.** `include/platform/TableMetadata.h`, registered like `AllObj` and written from
`InstalledTables()` when the runner's database is provisioned: type, per-company flag, lookup
and drill-down page from the `TableDef`, `ObsoleteState = No` for every table this build
carries (an obsolete table is not translated). Activation of a codeunit path that refused;
measured by the milestone.

Type:     task
Status:   open
Parent:   0067
Area:     gen
Source:   developer/properties/devenv-properties.md, developer/properties/devenv-table-property-overview.md, developer/properties/devenv-page-property-overview.md, developer/properties/devenv-report-property-overview.md, developer/properties/devenv-codeunit-properties.md, developer/properties/devenv-enum-properties.md, developer/properties/devenv-key-properties.md, developer/properties/devenv-query-properties.md, developer/properties/devenv-xmlport-properties.md, developer/properties/devenv-view-properties.md, developer/properties/devenv-control-addin-properties.md, developer/properties/devenv-profile-properties.md, developer/properties/devenv-report-properties.md, developer/properties/devenv-demolicense-properties.md
Verdict:  fehlt
Class:    silent-wrong-data

# The property census is checked against the documentation's own index

**Fourteen pages, one item.** They are indexes rather than specifications -- and normally an index
page gets a ledger row and no WI. These get one because of what the tables contain.

`devenv-table-property-overview.md`, `devenv-page-property-overview.md` and
`devenv-report-property-overview.md` are not link lists: each is a table of

| **Property name** | **Extensible** | **Applies to** |

-- the property, whether a table/page/report EXTENSION may set it, and the full list of element kinds
it applies to. The nine smaller `*-properties.md` pages are the same table per object kind.

**That is board:0067's census, written by Microsoft.** board:0067 is "every declared property is
translated or counted" and this sweep has measured the other half: the generator consumes **nine**
property names of 349. What has been missing is an authoritative list of which property may appear on
which element -- and these fourteen pages are it, in a form a script can read.

**And the `Extensible` column is a rule nothing else in this sweep states.** Whether a
`tableextension` may set `AllowInCustomizations` (True) but not `Clustered` (board:0348 quotes the
prohibition from the property's own page) is exactly the kind of per-property, per-object-kind
constraint that is otherwise scattered across 349 pages.

## Population

**349 property pages, 14 index pages, 9 properties with a consumer.** The indexes are the denominator
board:0067's counter has been missing.

## The IST-state

board:0067 exists as a root and has no mechanical list to count against; this sweep's ledger has one
built by hand, page by page, which is what these fourteen pages already contain.

## The choice

Parse the three overview tables and the nine per-kind tables into a machine-readable list --
`{ property, object kind, element kind, extensible }` -- and make board:0067's counter compare the
generator's consumed set against it. **A property in the index that no generator branch names is a
counted hole; a property the generator reads that is not in the index is a defect.**

**Not a hand-maintained list.** This sweep's own ledger is one, it took 349 readings, and it will go
stale the next time Microsoft adds a property. The tables are in the documentation tree the transpiler
already reads for other purposes.

**`devenv-demolicense-properties.md` is the one index with no counterpart here** -- a demo licence is
a deployment artefact, not an AL declaration -- and it is listed for completeness rather than parsed.

## Ordering

With board:0067, and after this sweep: the hand list is the negative control for the parsed one.

## Gate, and its negative control

The parsed index contains at least the 349 property pages' subjects, and board:0067's counter reports
the nine consumed against it.

**The negative control is this sweep's own ledger** -- a property that appears in the ledger and not
in the parsed index means the parser missed a table, and **a count of 0 over N pages is an abort, not
a pass.**

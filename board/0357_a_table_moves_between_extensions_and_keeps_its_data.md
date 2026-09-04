Type:     task
Status:   open
Parent:   0033
Area:     gen, db
Source:   developer/properties/devenv-movedfrom-property.md, developer/properties/devenv-movedto-property.md
Verdict:  fehlt
Class:    activation

# A table moves between extensions and keeps its data

**Two pages, one item, because they are the two ends of one move**: `MovedTo` on the source names
where the table went, `MovedFrom` on the destination names where it came from, and neither is
meaningful alone.

> **MovedFrom** (runtime 12.0, Table and Table field): Specifies the origin extension ID when a table
> is moved to a new extension. **If the source table exists in the database during the sync
> operation, the table will be moved to this extension along with all its data.** Otherwise, a new
> table will be created in the database.
>
> **MovedTo** (runtime 12.0, Table and Table field): Specifies the destination extension Id when a
> table is moved to another extension.

**And they complete `ObsoleteState`'s vocabulary.** `devenv-obsoletestate-property.md`'s generated
table lists only `No` and `Pending`, and its Remarks correct it: "The full list of options is
`Moved`, `No`, `Pending`, `PendingMove`, and `Removed`. **Some of these settings are used when moving
tables and fields between extensions.**" board:0069 covers `Pending` and `Removed`; `Moved` and
`PendingMove` belong here, with the properties they exist for.

**The data-keeping clause is the whole risk.** A move that creates a new table instead of renaming
the old one loses every row, and the page says the choice is made at sync time by whether the source
table is present. That is a migration decision, not a translation one -- and this tree has no
migration yet, which is why the item's first half is a refusal rather than a mechanism.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`MovedFrom =` **133** · `MovedTo =` **143** · `ObsoleteState = Moved` **143** · `= PendingMove` **8**.

`MovedTo` and `ObsoleteState = Moved` match exactly at 143, which confirms the pairing: a table that
declares where it went also declares that it went.

## The IST-state

None of the four is among the nine properties the generator consumes (board:0067), and there is no
schema migration to run a move through.

## The choice

**A table declaring `MovedTo` is not translated into the app that declares it**, because its
definition now belongs to the destination app -- that is what the property says, and board:0033's app
boundary is a translation-time decision, so this one is too. A `MovedFrom` on the destination is then
the note that says why the table appears there.

**`ObsoleteState = Moved` and `= PendingMove` are refused for now**, loudly, naming these 151
declarations: the states describe a database migration this tree does not perform, and accepting them
silently would emit a table twice or not at all.

## Ordering

Behind board:0033, which decides what an app contains. Behind a schema migration for the data half,
which does not exist.

## Gate, and its negative control

A table declaring `MovedTo` produces no class in the source app and one in the destination app; the
field-table entry appears exactly once across the two.

**The negative control is the count across both apps** -- an implementation that emits it in neither
passes a "not in the source app" check and loses the table.

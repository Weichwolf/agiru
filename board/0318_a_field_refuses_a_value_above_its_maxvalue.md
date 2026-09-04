Type:     task
Status:   open
Parent:   0068
Area:     rt, gen
Source:   developer/properties/devenv-maxvalue-property.md
Verdict:  fehlt
Class:    activation

# A field refuses a value above its `MaxValue`, at the same boundary as `MinValue`

`MaxValue` is `MinValue`'s partner with the same four types and the same enforcement point: Integer,
Decimal, Date (`December 31, 9999`) and Time (`23:59:59`).

It is a separate item because the two are declared separately -- a field may carry one and not the
other -- and because the failure texts differ, which board:0055 will need.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`MaxValue =`: **1 448 declarations**, against 3 398 for `MinValue`. Fields are far more often floored
than capped, which is what a quantity or a line number needs.

## The IST-state

Not in `FieldDef` (`include/meta/TableDef.h:67`), like every sibling in this group.

## The choice

The same optional bound on `FieldDef` and the same UI-boundary check as 0317. The two share one
metadata field pair and one check, so whichever is built first should build both.

**The Date ceiling is the row to watch.** `December 31, 9999` is not PostgreSQL's maximum and not
SQL Server's; it is AL's, and board:0016's closing dates live just past the end of a year -- so the
bound has to be compared against the same `agiru::Date` the closing-date rule uses, not against a
column's range.

## Ordering

With 0317.

## Gate, and its negative control

Typing a value above the bound through a `TestPage` raises; assigning it in AL does not.

**The negative control is a closing date on a field capped at `December 31, 9999`** -- it must be
accepted, because a closing date is not a later date (board:0016), and a naive comparison rejects it.

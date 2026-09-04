Type:     task
Status:   open
Parent:   0032
Area:     gen, rt
Source:   developer/properties/devenv-usetemporary-property.md, developer/properties/devenv-usetemporary-report-property.md, developer/properties/devenv-usetemporary-xmlport-property.md
Verdict:  fehlt
Class:    activation

# A temporary source is declared the same way on three object kinds

**Three pages, one item**: an overview page and one per object kind -- report data items and XMLport
table elements -- with identical semantics. The same shape as `Scope` (board:0361) and
`SourceTableView` (board:0432), and here, as with `SourceTableView`, the two uses really are one
property.

> Specifies whether a report data item / XMLport table element **uses a temporary table**. **The
> default is false.**
>
> **"You can import records into a temporary table when the incoming data must be TRANSFORMED before
> it's inserted into a physical table. The temporary table keeps the records IN MEMORY and doesn't
> write them to the Business Central database."**

**This is the fourth way to declare the same thing**, and that is the finding worth carrying out of
this item:

| declaration | on | WI |
|---|---|---|
| `TableType = Temporary` | the table -- every use of it | 0364 |
| `SourceTableTemporary` | a page | 0431 |
| `UseTemporary` | a report data item | this |
| `UseTemporary` | an XMLport table element | this |

Four declarations, one mechanism. **board:0032's temporary record must be reachable from all four or
there will be four temporary records**, and the AL-visible behaviour -- `Rec.IsTemporary`, no
`SystemId` stamping, no cursor, `DeleteAll` in memory -- has to be identical whichever way it was
declared.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`UseTemporary =`: **336 declarations**, report data items and XMLport table elements together; not
separable by `grep`, so the split is counted by file extension when the item is pulled. **Stated
rather than rounded.**

With board:0364's 298 `TableType = Temporary` and board:0431's 680 `SourceTableTemporary`, that is
**1 314 declarations of temporariness** across four properties -- which is what sizes board:0032.

## The IST-state

Reports and XMLports have no generator (board:0063, board:0065, board:0034). board:0032 records the
temporary record's state.

## The choice

One bit on the data item and on the table element, both selecting board:0032's in-memory record --
the same one board:0364 and board:0431 select.

**Not a fourth implementation and not three.** The property is the declaration; the mechanism is one.

## Ordering

Behind board:0032. Inside board:0063 and board:0065 for the declarations.

## Gate, and its negative control

A report data item declaring `UseTemporary` writes nothing to the database, and `Rec.IsTemporary`
returns true inside its triggers.

**The negative control is the same assertion through each of the four declarations** -- one mechanism
means one behaviour, and a gate that only exercises one route cannot show that the other three reach
it.

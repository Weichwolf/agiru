Type:     task
Status:   open
Parent:   0065
Area:     rt, gen
Source:   developer/triggers-auto/xmlporttableelement/ (7 pages)
Verdict:  fehlt
Class:    activation

# An XmlPort table element's seven triggers are two different paths, and the direction decides which

The seven pages divide by DIRECTION, which is what `Direction` (board:0067) declares on the XmlPort:

| trigger | runs on | when |
|---|---|---|
| `OnPreXmlItem` | **export** | "after the table is initialized and before you start exporting data ... only applies to XMLPort elements that have a source type of Table" |
| `OnAfterGetRecord` | **export** | after a row is read and before it is written to the document |
| `OnAfterInitRecord` | **import** | after a record is loaded |
| `OnBeforeInsertRecord` | **import** | after a record has been loaded and before it is inserted |
| `OnAfterInsertRecord` | **import** | after the insert |
| `OnBeforeModifyRecord` / `OnAfterModifyRecord` | **import**, when `AutoUpdate` or `AutoReplace` is on | around the update of an existing row |

**They are one task** because they are seven points in one element loop, and because the import path
cannot be built without knowing which of them the direction selects.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

The seven together: **1 184 declarations**, of which `OnAfterInitRecord` and `OnBeforeInsertRecord`
are the bulk -- the import path is where XmlPorts do work.

## The IST-state

XmlPort has no generator (board:0034, board:0065).

## The choice

One loop per table element with the seven call points, selected by `Direction`. **The four
properties board:0067 calls semantics rather than layout decide what the loop does**: `AutoSave`
writes the imported record, `AutoUpdate` updates an existing row with the same primary key,
`AutoReplace` replaces it, and `FieldValidate` runs `OnValidate` on the source field.

So the modify triggers exist only when `AutoUpdate` or `AutoReplace` is declared, and a driver that
calls them unconditionally runs them on a row it is about to insert.

## Ordering

Blocked on board:0065, and it is the largest piece of it.

## Gate, and its negative control

An import over two records where one already exists, with `AutoUpdate` on: the new one goes through
`OnBeforeInsertRecord`, the existing one through `OnBeforeModifyRecord`.

**The negative control is the existing record** -- a driver that inserts unconditionally raises a
duplicate key, and one that runs both trigger sets on every row fires four triggers where AL fires
two.

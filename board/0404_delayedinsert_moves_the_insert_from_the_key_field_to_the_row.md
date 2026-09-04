Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/properties/devenv-delayedinsert-property.md
Verdict:  fehlt
Class:    activation

# `DelayedInsert` moves the insert from the key field to the end of the row

> Sets a value that specifies whether a user must **leave a record** before it is inserted into the
> database. **By default, new records are inserted when the user leaves the control that shows the
> primary key in the table.**
>
> **True** if the record is inserted when the user leaves the record; **false** if the record is
> inserted when the user leaves the control that shows the primary key. **The default is false.**
>
> Applies to: **Page.**

**The default is the surprising half.** Without this property BC inserts the row as soon as the user
tabs out of the primary-key control -- so a half-filled line is already in the database and the
remaining fields arrive as `Modify`s. With it, nothing is written until the user leaves the row.

That is not a UI preference; it decides **how many statements a line costs and what a concurrent
reader sees**. Under board:0012's transaction rules a row inserted at the first field is visible to
the session's own later reads and holds its locks for the rest of the row.

**And it decides which triggers run in which order.** `OnInsertRecord` fires at the insert point, so
moving the insert moves the trigger relative to every `OnValidate` in the row.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`DelayedInsert =`: **1 048 declarations**, all necessarily `true` since `false` is the default.

**A thousand pages ask for the non-default behaviour**, which for document line subforms is every one
of them: a sales line cannot be inserted knowing only its line number.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; board:0030's page has no insert path at all,
so neither timing exists yet.

## The choice

One bit on the page descriptor, read by board:0030's new-record path, which either inserts on leaving
the primary-key control or buffers the row until the record changes.

**The buffered row is a record in memory and not a database row**, so it must not be visible to a read
in the same session -- which is the opposite of the default behaviour and is the whole point of the
property.

## Ordering

Behind board:0030's page insert path and board:0354's `AutoSplitKey`, which computes the key the
insert uses.

## Gate, and its negative control

On a page with `DelayedInsert = true`, a `Find` after filling the key control finds nothing; leaving
the row inserts it and the `OnInsertRecord` trigger fires once.

**The negative control is the same page WITHOUT the property** -- the row must be findable after the
key control, and an implementation that buffers always passes the first gate and changes the timing
for every page that did not ask for it.

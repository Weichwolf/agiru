Type:     task
Status:   open
Parent:   0032
Area:     rt, gen
Source:   developer/devenv-temporary-tables.md
Verdict:  fehlt
Class:    activation

# A temporary record is memory, and it needs no permission

board:0449 counted **1 314 declarations of temporariness across four properties** and said all four
must reach one mechanism. **This page is that mechanism's specification**, and it adds two rules
neither property page states.

## The three implementations are the same thing

> - `TableType = Temporary` on the table object (board:0364)
> - a **temporary record VARIABLE**: `TempInvoicePostBuffer: Record "Invoice Post. Buffer" temporary;`
> - `SourceTableTemporary` on a page (board:0431)
>
> "This implementation has **THE SAME EFFECT** as using a temporary record variable or setting
> `SourceTableTemporary`."
>
> **"Whichever way you choose, you MUST CREATE THE TABLE OBJECT that defines the fields, like any other
> table object."**

**So a temporary record is a normal generated class with a different storage backend**, and the fourth
declaration -- `UseTemporary` on report data items and XMLport table elements (board:0449) -- makes
four.

**And the VARIABLE form is a fourth spelling this sweep had not seen**: `temporary` as a suffix on a
variable declaration, not a property. That is parser work, and it is the most common form in the
BaseApp by inspection of the example.

## Two rules that decide the design

> **"Temporary tables DON'T REQUIRE the user to have permissions on the underlying table. Because
> temporary table data is held only in memory and never read from or written to the database, THE
> PERMISSION SYSTEM DOESN'T APPLY.** A user can create, read, modify, and delete records in a
> temporary table even if they have no permissions defined for that table."
>
> **"This behavior applies REGARDLESS OF HOW the temporary table is implemented."**

**So board:0062's permission check must be SKIPPED for a temporary record**, and skipped at the record
and not at the table -- because the same table is permission-checked through a normal variable and not
through a temporary one, in the same session, possibly in the same procedure.

> **"Temporary tables RETAIN SYSTEM FIELDS, like SystemId and data audit fields."**

**That contradicts what board:0511 read on the system-fields page**, which says: "If a record is copied
into a temporary table, the audit field values are copied as well. **The values aren't changed by the
server when calling a modify or insert method.**"

**Both are true and they say different things**: the fields EXIST on a temporary record and are not
STAMPED by insert or modify. So a temporary record carries whatever audit values it was given and the
platform does not update them. **Recorded as the resolution of an apparent contradiction rather than a
contradiction**, because the two sentences are compatible and a reader meeting only one of them would
guess wrong.

## What a temporary table is not

> "A temporary table data isn't stored in the database. It's only held in memory **until the table is
> closed**."
>
> **"The WRITE TRANSACTION PRINCIPLE that applies to a database table DOESN'T APPLY to a temporary
> table."**

board:0514 reaches the same conclusion from the isolated-events side: only `TableType: Normal` changes
roll back. **A temporary record is outside the transaction entirely** -- no rollback, no commit, no
lock, no isolation state (board:0490).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0364: `TableType = Temporary` **298**. board:0431: `SourceTableTemporary` **680**. board:0449:
`UseTemporary` **336**. **The `temporary` variable suffix is not a property and this sweep's pattern
does not count it** -- it is the fourth and probably largest form, and taking its count is this item's
first task. Stated rather than guessed.

## The IST-state

board:0032 records the state. `src/rt/Storage.cpp:94` creates a relation for every table
(board:0364's 298). `include/runtime/Table.h` has no temporary mode; `src/rt/Cursor.cpp` is the read
path.

## The choice

One flag on the record instance -- not on the table -- selecting an in-memory row set instead of a
cursor. The generated class is unchanged; `Insert`, `Modify`, `Delete`, `Find`, `SetRange` and the rest
dispatch on the flag.

**On the instance, because the same table is both** in one session.

**The permission check reads the same flag** and returns granted. board:0062's check therefore takes
the record, not the table id.

**The system fields exist and are not stamped**: `RuntimeInsert`'s `StampInserted` is skipped for a
temporary record, which board:0511 also requires.

## Ordering

board:0032's core. Before board:0062's permission check, which must know about it, and before
board:0364, board:0431 and board:0449, which are its four declarations.

## Gate, and its negative control

A temporary record inserts, finds and deletes without touching the database; a user with no permission
on the table can do all three; the record's `SystemCreatedAt` is unchanged by the insert.

**The negative control is the permission** -- a user with no rights on `Item` must be able to use a
`Record Item temporary`, and an implementation that checks permissions by table id refuses it. That
failure looks exactly like a correct permission check.

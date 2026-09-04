Type: leaf
State: open
Area: rt, db
Tags: navision, semantics, performance

# A BLOB is not read with its record, and CALCFIELDS is what reads it

`blob-data-type.md` carries this in a commented-out block that renders nowhere, which is exactly
why it is easy to miss:

> To optimize performance, when you access a record that has a BLOB field, the data in the BLOB is
> not always read into memory. You must call the CALCFIELDS method to read the BLOB into memory and
> calculate it.

`type/Blob.h` today is a `std::vector<std::uint8_t>` that a SELECT would fill like any other
column. That is wrong twice over.

**It is wrong on behaviour.** BaseApp code calls `CALCFIELDS` before touching a BLOB and AL test
code checks that a BLOB is empty before it does. A runtime that fills the field eagerly makes those
checks pass for the wrong reason and hides a defect until something depends on the lazy read.

**It is wrong on the target.** A BLOB is up to 2 GB. Reading one per row on a `FINDSET` over a
table with a picture column is the difference between a query and an out-of-memory kill on the Pi
(board:0006). The same holds for `Media` and `MediaSet`, which are worse: they do not live in the
row at all but in a tenant media table, reached by a GUID.

## What it needs

A field CLASS beside the field type. AL has `FieldClass = Normal | FlowField | FlowFilter`, and
Normal-but-deferred is what a BLOB is. `FieldDef` carries no class today, so the SELECT that
`Rows.h` builds cannot leave a column out and `CALCFIELDS` has nothing to ask for. The same
`FieldDef.class` is what FlowFields will need, so this is not a BLOB feature -- it is the field
class, and BLOB is the first user of it.

## The benchmark

The column list of the SELECT the runtime issues for a table with a BLOB field, quoted from the
statement rather than described: the BLOB column is absent until `CALCFIELDS` names it. And the
resident set of a `FINDSET` over a thousand rows with a megabyte BLOB each, measured, before and
after.

## Closed when

A BLOB field is empty after a GET and holds its bytes after CALCFIELDS, in a gate against
PostgreSQL, and the SELECT is shown not to have named the column.

## A BLOB DECLARES WHAT IT HOLDS, read 2026-09-04 (board:0071)

`properties/devenv-subtype-blob-property.md`: a `BLOB` field carries a `SubType` -- `UserDefined`
(the default), `Bitmap`, `Memo` or `Json`. It is a declaration and therefore `constexpr` beside the
field like every other property (board:0067), and it decides what a client does with the bytes:
`Bitmap` is an image a page renders, `Json` is text a `JsonObject` reads back, `Memo` is text.

It does not change how the BLOB is STORED or when it is read, which is this item's subject -- so it
is recorded here as a field-table column to carry rather than as behaviour to build.

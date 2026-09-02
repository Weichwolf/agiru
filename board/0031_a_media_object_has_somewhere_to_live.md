Type: arc
State: open
Area: rt, db

# A media object has somewhere to live

`Media` and `MediaSet` are field types now, and they hold the IDENTIFIER of a media object rather
than its bytes: the column is a `uuid`. What the identifier points AT does not exist -- there is no
tenant media table -- so everything that moves bytes refuses and names it.

Measured over BCApps on 2026-09-02: **90 `Media` field declarations, 23 `MediaSet`, across 100
tables, and NOT ONE variable or parameter of either type**. The page says the same thing --
"can be used as a table field data type, but cannot be used as a variable or parameter" -- so the
source and the documentation agree and the type needs no assignment story.

## What refuses today, and what does not

| answers | refuses |
|---|---|
| `Media.MediaId()`, `Media.HasValue()` | `ImportFile`, `ImportStream`, `ExportFile`, `ExportStream`, `FindOrphans` |
| `MediaSet.MediaId()`, `MediaSet.Count()` on an EMPTY field | `Count()` on a full one, `Item`, `Insert`, `Remove`, `ImportFile` |

`Count()` is split on purpose and the gate says why: an empty field holds no media and that answer
needs no table, while a field that names a set must NOT answer zero -- a wrong count that looks like
an empty gallery is the silent-wrong-data class.

`HasValue()` answers half of its documented question. The page asks whether the field "has been
initialized with a media object AND that the specified media object exists in the database"; the
second half needs this item. It cannot be reached yet either way, since nothing can put an
identifier into the field except a row read and every import refuses.

## What the references say

`Tenant Media` (2000000181) and `Tenant Media Set` (2000000182) are PLATFORM tables -- they are in
the 206 tables the transpiler reports as "declared outside this source root", so they arrive with
that item and not with this one. Their shape: a media entry carries its id, its content as a BLOB,
its mime type and a description; a set is rows pairing a set id with a media id and an index.

`~/Git/openerp` has no answer here worth taking: grep finds no media store, which is consistent
with a UT subset that never imports a picture.

## What is true when this closes

- A media object can be imported from a stream and exported to one, and the bytes survive the round
  trip.
- `HasValue()` answers the whole documented question.
- `MediaSet.Count()` and `Item(Index)` walk a real set.
- A `Media` field whose identifier names nothing is a LOUD failure on read, not a blank picture.

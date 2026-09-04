Type:     task
Status:   open
Parent:   0031
Area:     rt, gen, db
Source:   developer/devenv-working-with-media-on-records.md, developer/devenv-lists-as-tiles.md
Verdict:  fehlt
Class:    activation

# Media lives in two system tables, and a `MediaSet` is an ordered list

board:0031 is "a media object has somewhere to live". **This page says where**, by table id, and adds
an indexing rule that is behaviour rather than storage.

## Two system tables, named

> "Imported media is stored as an object in the system table **2000000184 Tenant Media** of the tenant
> database. **Each media object is assigned a unique identifier (ID).**"
>
> "If a media object is added to a **MediaSet** field, the media object is assigned to a media set in
> the system table **2000000183 Tenant Media Set**. The media set is assigned a unique identifier,
> which is then referenced from the field."

**So the field holds an ID and the bytes live elsewhere** -- board:0031's question answered with two
table numbers. Both are in board:0032's platform-table range and board:0032 counts 87 such tables.

**And the field's SQL type follows**: `src/rt/Storage.cpp:88` already maps `FieldType::Media` and
`MediaSet` to `uuid`, which is exactly right -- an id, not bytes.

## Why `Media` beats `Blob`, and it is a caching argument

> "With a **BLOB** data type, **EACH TIME the media is rendered in the client, it's RETRIEVED FROM THE
> SQL DATABASE SERVER**, which requires extra bandwidth and affects performance. With Media and
> MediaSet, **the client uses MEDIA ID TO CACHE the media data.**"

**The id is the cache key**, so a media object is immutable once stored and the renderer emits a URL
containing the id. That is the design, and it falls straight out of an htmx renderer: a
`<img src="/media/{id}">` the browser caches.

board:0017 is "a BLOB is not read with its record" -- **the same argument, applied to the other type**.

## A `MediaSet` is ordered, one-based, and reindexed on removal

> **"A media set is an ORDERED LIST, determined by the order in which the media objects were added.
> THIS ORDER CAN'T BE CHANGED.** Each media object is assigned an index number, **STARTING AT 1**. If a
> media object is REMOVED, **THE LIST IS REINDEXED.**"

**One-based, insertion-ordered, and reindexed on delete** -- so index 2 refers to a different object
after index 1 is removed. That is not a stable identifier and AL code holding an index across a
removal is holding the wrong object. **Reproducing it means the index is a position, never a key**, and
an implementation that kept sparse indices would be more stable and wrong.

> **"If a `MediaSet` field is used in a REPORT object, then ONLY THE FIRST associated media file is
> displayed."**

A board:0063 rule, from an unexpected page.

## Word macros are stripped on import

> "when importing Microsoft Word files (.docx), **MACRO PACKAGES (VBA CODE) WILL AUTOMATICALLY BE
> REMOVED** from the file when stored in the database."

**A platform-level content transformation on write**, and a security one. It is not optional and it is
not visible in the AL. Whether agiru performs it is a decision this item names: **a `.docx` stored
byte-for-byte here differs from the same file stored in BC**, and the difference is executable code.

## MIME types are the classification

> "The Media and MediaSet datatypes **support ALL RECOGNIZED MIME TYPES** ... `type/subtype`."

So the stored object carries a MIME type, and board:0485's `AllowedFileExtensions` is the upload-side
filter over the same vocabulary.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Media` and `MediaSet` are field TYPES, not property declarations; this sweep's pattern does not count
them. **The count of `Media` and `MediaSet` fields across 1 609 tables belongs to this item** and is
its first task -- stated rather than guessed.

board:0505 records that `Truncate` is unsupported on **tables with media fields**, which is a seventh
reason that method needs this one.

## The IST-state

`src/rt/Storage.cpp:88` -- `FieldType::Media` and `FieldType::MediaSet` map to `uuid`. **So the column
is already an id.** board:0031 records that there is nowhere for the bytes to live: the two system
tables are among board:0032's 87 and are not generated.

## The choice

The two platform tables as generated tables (board:0032), the field holding a `Guid`, and a media
store keyed by that id. The renderer emits a URL carrying the id; the browser caches it.

**A `MediaSet` index is computed from position at read time**, never stored -- which makes the
reindex-on-removal behaviour automatic rather than something to maintain.

**The Word macro stripping is refused rather than implemented for now**, and named: it needs a `.docx`
parser, it is a security transformation, and doing it wrong is worse than not claiming it. A `.docx`
import is accepted and stored unchanged, with this deviation recorded.

## Ordering

Behind board:0032's platform tables. With board:0017's BLOB reading, which is the same
not-with-the-record rule.

## Gate, and its negative control

A media object stored on a record is retrievable by id; a `MediaSet` with three objects reports indices
1, 2, 3, and after removing the first reports 1, 2 for the remaining two.

**The negative control is the reindex** -- an implementation with stable indices reports 2, 3, which is
more useful and is not what BC does; and every gate that never removes an object passes.

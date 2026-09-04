Type:     task
Status:   open
Parent:   0017
Area:     gen, rt
Source:   developer/properties/devenv-compressed-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# A BLOB declares whether it is compressed, and the default is that it is

> Sets a value that specifies whether a BLOB is compressed. **True** if the BLOB is compressed,
> otherwise false. **The default is true.**
>
> It must be applied on a field of the **BLOB Data Type**.

**The default is the finding.** Every BLOB in BC is compressed unless it says otherwise, so a runtime
that stored them raw would be storing something a BC client cannot read, and reading a BC-written one
would return compressed bytes to AL code that expects the content.

That matters for board:0004's CRONUS load more than for anything else: the demo database's BLOB
columns hold BC's compressed form, and whatever reads them has to know it.

**What the page does not say is WHICH compression**, and that is the item's open question. BC's BLOB
compression is a documented-by-behaviour format, not a named one on this page; the answer is in the
demo data, and the CRONUS load is where it becomes visible.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Compressed =`: **6 declarations**, all necessarily `false` since `true` is the default.

**Six fields opt OUT.** Every other BLOB in 1 609 tables is compressed by default and says nothing,
which is why the population understates the item completely: what has to be built is the DEFAULT, and
what is declared is the exception.

That is a distribution worth naming on its own -- for most properties in this sweep the declaration
count is the work; here the six declarations are the easy part.

## The IST-state

Not among the nine properties the generator consumes (board:0067). `src/rt/Storage.cpp:87` maps
`FieldType::Blob` to `bytea` and stores what it is given, uncompressed, always.

## The choice

One bit on `FieldDef`, defaulting to `true`, and the compression happens where the BLOB crosses
between `Record` and the column -- which board:0017 already owns, because a BLOB is not read with its
record.

**The format is decided from the CRONUS data and not chosen.** Interoperability with a BC-written
database is the whole reason the property exists here; a different algorithm that round-trips
correctly within agiru would pass every gate and read nothing BC wrote.

## Ordering

Behind board:0017. Before board:0004's BLOB columns can be read at all.

## Gate, and its negative control

A BLOB written and read back returns the same bytes; a BLOB from the CRONUS database decompresses to
readable content.

**The negative control is the CRONUS read** -- a round trip inside agiru passes with no compression at
all, and only foreign data proves the format.

Type: leaf
State: open
Area: gen, build
Tags: gate, measured

# Every limit the documentation states is a `static_assert` beside the object that could break it

`devenv-object-specifications-limitations.md` is a table of hard numbers, and **every one of them is
decidable when the tree is compiled**. CLAUDE.md asks for exactly this and names the reason:

> **Anything decidable at translation time is a `static_assert`, never a test case.** Field counts,
> sort order, layout, enum exhaustiveness. The transpiler EMITS them beside every object, so a
> mis-generated table is a translation error rather than a lookup that quietly finds nothing.

The generator already emits four per table -- the field sort order, the `offsetof` agreement, the
standard-layout check and the field count (`src/gen/TableWriter.cpp:528`) -- so the mechanism is
there and the DOCUMENTED limits are not in it.

## The limits, from the page

| what | limit |
|---|---:|
| fields in a record | **500** |
| record size | **8 060 bytes** |
| keys per table | **40** |
| distinct fields per key | **16** |
| key size | 900 bytes |
| `SumIndexFields` per key | 20 |
| characters in a Text or Code field | **2 048** |
| BLOB size | 2 GB |
| characters in an object, field or table NAME | 30 |
| characters in a caption | 1 024 |
| object and field ID range | 1 .. 999 999 999 |

`code-data-type.md` adds the one the table does not: **a Code VARIABLE is at most 1 024 characters
while a Code FIELD is 2 048**, so the bound differs by where the value lives.

## Why it is worth the two lines per object

- **1 609 tables and 3 272 keys** go through the generator, and `Sales Line` alone declares 17 keys
  and 183 fields. A table that crossed a limit would be accepted here and refused by BC.
- **The record-size limit is the one that bites without warning.** 8 060 bytes is SQL Server's page
  payload; a table of wide `Text` fields can pass every other check and fail on the row. PostgreSQL
  has no such limit -- it TOASTs -- so agiru would accept a table BC cannot store, which is a
  divergence that only shows up when the schema is carried back.
- **They cost nothing.** A `static_assert` over a `constexpr` field table is checked once per
  translation unit and emits no code.

## The choice

- **The transpiler emits one `static_assert` per documented limit that applies to the object it is
  writing**, beside the four it already emits, with the limit's NAME in the message so a failure
  reads as "this table declares 501 fields, the documented maximum is 500".
- **The limits are `constexpr` constants in one place** -- `meta/` -- with the page cited beside
  them, not literals repeated per writer. CLAUDE.md's rule that every number carries its origin
  applies to a limit as much as to a measurement.
- **A limit PostgreSQL does not share is still asserted**, and the header says which side it comes
  from: the point is that a table agiru accepts is one BC would accept, because the source is the
  same AL either way.

## Gate

The asserts are the gate. A generated table that declares one field too many, one key too many, or
a `Code[2049]`, fails to compile with the limit named.

**Negative control**: raise one limit by one and require the case that was failing to pass -- an
assert whose bound is unreachable is decoration, and the record-size one is the bound most likely to
be written wrong.

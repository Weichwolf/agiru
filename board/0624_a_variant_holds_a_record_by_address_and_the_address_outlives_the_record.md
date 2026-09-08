Type:     root
Status:   open
Area:     rt
Source:   241 UT failures on `Format(Variant)` and what implementing it cost, 2026-09-08
Class:    activation

# A Variant holds a record BY ADDRESS, and the address outlives the record

**241 UT PROCEDURES FAIL ON ONE REFUSAL**, and implementing it made things worse rather than better,
which is the whole content of this item:

```
Format: a Variant holding a record renders its primary key, which needs a key
```

`Format(SomeRecord)` renders `<Caption>: <key values>` in BC, and everything needed is there: the
Variant carries the record's ADDRESS and its table NUMBER, the catalogue finds the `TableDef` by
number, and the declaration carries the primary key. Written that way it removes all 241.

## What it cost, measured

| | |
|---|---|
| before | 326 of 1 708 over 62 codeunits |
| after | **279 of 1 593 over 61** |

**THE DENOMINATOR MOVED, WHICH IS THE ABORT.** `Price Source UT` -- 115 procedures, 54 of them
passing, the largest single contributor on the board -- stopped reporting and left with
`std::bad_array_new_length`.

**AND THE CAPTION ALONE IS ENOUGH TO CRASH IT.** Rendering only `entry->table->caption` and no field
at all still throws, so it is not the field read and not the offset: **the `RecordInVariant` itself
is stale**, and `held.table` is whatever is at that address now. `FindTable` then answers with a
`TableEntry` that is not one, and a `std::string_view` built from it has a length nobody wrote.

## The design that is being reported

`Variant.h` says it outright, as a warning on the record constructor: "a Variant built from a
TEMPORARY would dangle, which is the same defect as in AL". That is true of AL and true here -- but
AL's platform holds a record variant by a handle its own runtime owns, and this one holds a raw
`const void *` with no way to ask whether it is still a record.

**SO THE 241 ARE NOT A MISSING RENDERING. THEY ARE A LIFETIME**, and the item is which of these the
runtime takes:

1. **The Variant OWNS a copy of the record** when it is built from one. That is what makes
   `Format` safe and what makes `RecordRef.GetTable(Variant)` safe, and it costs a record copy at
   every `Variant := Rec` -- 55 402 `SetRange` call sites are the wrong population to read for
   this, the right one is how often a record reaches an `Any` parameter.
2. **The Variant holds a HANDLE the runtime owns**, the way `Instance<T>` does, so a dead record is
   a refusal rather than a read.
3. **The record REGISTERS itself** while it is alive, and the Variant holds a generation beside the
   address -- which is what the cursor guard (board:0617) also wants, and would settle both.

## What proves it

`Price Source UT` reports 115 again with `Format(SomeRecord)` rendering `Price Source: ...`, and the
UT count rises by what the 241 were hiding. The negative control is the one that found this: a
Variant built from a record that has gone must REFUSE, not read.

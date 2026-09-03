Type: root
State: open
Area: rt

# A record finds, filters and counts, and the 120 049 call sites that need it work

`SetRange`, `FindSet`, `Next`, `Count`, `IsEmpty`, `DeleteAll`, `ModifyAll` and their neighbours are
variadic refusals. Measured over the generated tree on 2026-09-03:

| call | sites | call | sites |
|---|---:|---|---:|
| `.SetRange(` | 55 402 | `.SetCurrentKey(` | 2 245 |
| `.FindFirst(` | 14 078 | `.Count(` | 1 560 |
| `.Next(` | 10 557 | `.ModifyAll(` | 883 |
| `.SetFilter(` | 9 887 | `.CopyFilters(` | 287 |
| `.FindSet(` | 6 555 | `.Mark(` | 202 |
| `.DeleteAll(` | 4 811 | `.MarkedOnly(` | 156 |
| `.Find(` | 4 257 | `.GetView(` | 146 |
| `.Reset(` | 3 418 | `.SetView(` | 132 |
| `.IsEmpty(` | 2 684 | `.Ascending(` | 124 |
| `.FindLast(` | 2 565 | `.GetFilters(` | 100 |

**120 049 altogether, and every one of them throws.** No UT test can reach an assertion past its own
setup, because a setup is `SetRange` and `FindSet`.

## Reference

The filter LANGUAGE is already built and gated: `src/rt/Filter.cpp` parses `..`, `|`, `&`, `<>`, `*`,
`?`, `@` and quoting into DNF, compares numerically where the field is numeric, and `FilterGate`
holds 62 checks over it (board:0018). Nothing calls it from a record. `src/rt/Storage.cpp` has
single-row `Insert`/`Get`/`Modify`/`Delete` by primary key and no set operation at all.

**THE DOCUMENTED RULES THAT ARE NOT GUESSABLE:**

| | |
|---|---|
| `FindSet` | requests ALL matching rows in one query; `FindSet(true)` reads them `UPDLOCK` |
| `Find` | pages instead, and RESPECTS THE FILTERS -- `record-find-method.md` says so in as many words |
| `DeleteAll()` | does NOT run `OnDelete`; `DeleteAll(true)` does. Table-extension `OnBeforeDelete`/`OnAfterDelete` run either way |
| `ModifyAll` | **never** runs `OnValidate`, and runs `OnModify` only with `RunTrigger`. A row already holding the value is written anyway |
| `SetFilter(F,'')` | is NOT a filter on the empty value -- it is no filter |
| `SetRange` | REPLACES the field's filter rather than merging |
| `CopyFilters` | REPLACES the target's filters, and ignores the filter group on both sides |
| `Reset` | clears filters, marks, `MarkedOnly`, the load-field set, the isolation level AND the current key |
| `FilterGroup(-1)` | is the one group whose fields are OR'd together; every other group ANDs |
| `SetCurrentKey` | matches an active key by exact match or PREFIX; no match still sorts, without an index |

## What the predecessor paid for

| item | finding |
|---|---|
| WI-1173, WI-1206 | **a bare `FindSet`/`FindFirst`/`FindLast` that raises on failure is NET NEGATIVE, twice** -- -34, then GAINED 1 LOST 49. An empty result is the NORMAL case for these three in BaseApp idiom, not an error |
| WI-1136 | a bare `Find` that raises IS right, but only after `Copy(xRec)`, the call-argument value context and the case-selector context were fixed first -- four rounds |
| WI-1229 | `Find('=')` must NOT route through `Get()`: `Get` ignores filters by design and `Find('=')` must not |
| WI-1063 | `HasFilter` was group-blind. One filter list tagged by group, never a second store |
| WI-870 | `CopyFilters` from an UNFILTERED source must CLEAR the target |
| openerp `is_empty` | `IsEmpty` as `Count() != 0` cost a 90-second per-test timeout. It is its own `LIMIT 1` |

## The choice

**ONE POINTER on the record, null until something filters** -- board:0018 already decided this and it
is what keeps the generated class standard-layout. It holds what belongs to the record VARIABLE and
not to the row: the filters (one list, each entry tagged with its group), the current key, the sort
direction, the marks, `MarkedOnly`, the result set a `FindSet` produced and the cursor into it. A
record that never filters costs eight bytes and no allocation.

**The WHERE fragment binds as `"col" OP $n` and never casts.** `Connection.cpp` passes
`paramTypes = nullptr`, so PostgreSQL infers each parameter's type from the column it is compared
against -- which is the whole of the predecessor's 370-line `_coerce_filter_operand` apparatus,
obtained by not doing something.

**The value-context form lands first.** `if Rec.FindSet() then` for all four; the bare-discard-raises
rule only for `Find`, which is the shape WI-1136 proved. The other three stay silently false until
there is a green UT baseline to A/B against -- repeating a twice-rejected activation without one
measures nothing.

## Gate

`RecordNavigationGate` over a fixture table: each filter operator produces the row set AL would;
`FindSet` honours `SetCurrentKey` and `Ascending(false)`; `Next()` is 0 at the end; `FindFirst`
agrees with `Find('-')`; `Count` and `IsEmpty` see the same filter as `FindSet`; `DeleteAll` removes
only the filtered rows and the negative control asserts the rest survive; `ModifyAll` fires no
`OnValidate`; `CopyFilters` from an unfiltered source clears; and `Reset` shows the whole table again.

The measurement is `agiru run-tests` over the 2 291, before and after.

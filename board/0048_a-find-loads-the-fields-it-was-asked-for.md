# 0048 -- A find loads the fields it was asked for, decided at translation time

`Navigate.cpp` selects `Columns(table)` -- every column, always. `Purchase Line` has 217 fields,
`Gen. Journal Line` 215, `Item` 206, `CRM Organization` 240. A `FindSet` over a hundred million
rows reads all of them when the AL asked for three.

**The reference.** `devenv-partial-records.md` and its FAQ. AL says it with
`SetLoadFields(f1, f2, ...)`, which is 1 136 call sites under `Layers/W1` (plus 4 `AddLoadFields`).

**What the platform does, and where it loses.** Reading a field that was not loaded triggers a
JIT LOAD -- an implicit `Get` for the missing field. The page names three ways it fails outright
(the row was modified, deleted or renamed in between) and one way it is merely slow:

> When passing a record by value ... a new copy of the record is created. The original record and
> its copy don't share filters, fields selected for load, and so on. So accessing an unloaded field
> will trigger a JIT load. But it won't update the enumerator, which means future iterations will
> also require JIT load.

And a JIT load can raise **"Inconsistent read of field(s)"** when the row changed between the two
reads -- a correctness failure produced by an optimisation. The page also advises against partial
records on any record that inserts, deletes, renames or transfers fields, because all of those need
every field anyway.

So the feature is fast when the developer got the list right and slow-or-wrong when they did not,
and BC cannot know which because it decides at RUN time.

**The choice, and it is the one C++ makes available.** THE LOAD SET IS KNOWABLE AT TRANSLATION
TIME. A generated procedure names the fields it touches, in its own body, and the transpiler reads
that body. So:

1. `SetLoadFields` is honoured -- it is AL and it is 1 136 call sites, and a `Rec` narrowed by hand
   is the developer's statement of intent.
2. Where AL says nothing, the transpiler COMPUTES the set from the bodies reachable from the loop
   and emits it as `constexpr` -- the same move as every other piece of metadata here.
3. A field outside the set is a COMPILER ERROR and not a JIT load. There is no second round trip
   to be inconsistent with, so `Inconsistent read of field(s)` cannot arise, and the by-value copy
   losing its load set cannot arise either -- the set is a property of the TYPE, not of the value.
4. Where the set cannot be computed -- a `RecordRef`, a `Variant`, a field reached by number --
   the whole record is loaded, loudly and by a named rule rather than by omission.

**What it costs to get wrong.** Point 2 is the one that can be wrong in the dangerous direction: a
set that is too NARROW must be impossible, not merely unlikely, or a field silently reads its
default. So the emitted set is a `static_assert`-checked superset of what the body names, and the
gate is a case that reads a field left out of the set and does not compile.

**Order.** After board:0047, because a FlowField is not a column and must not be in any load set,
and after board:0044's navigation is gated -- which it now is.

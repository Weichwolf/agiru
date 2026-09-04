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
| `DeleteAll()` | does NOT run `OnDelete`; `DeleteAll(true)` does. The table's `OnBeforeDeleteEvent`/`OnAfterDeleteEvent` run either way -- they are PLATFORM trigger events and not the table's trigger (board:0057), which is why a `tableextension` still sees the delete |
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

## What thousands of sessions change about it

**THE SNAPSHOT IS PER SESSION AND THE IMAGE IS SHARED, so the snapshot is the number that scales
wrong.** `FindSet` requests all matching rows in one query -- that is what the page says and what
separates it from `Find` -- and holding them costs memory per RECORD VARIABLE, in every session at
once. One session reading a large journal is nothing; a thousand doing it is the per-session budget
board:0006 measures, spent in one place.

Three things follow, and none is optional at a thousand sessions:

- **IT IS A CURSOR AND NOT A SNAPSHOT.** SQL Server gives BC a server-side cursor and PostgreSQL has
  the same thing: `DECLARE <name> CURSOR FOR SELECT ... ORDER BY ...` and `FETCH FORWARD n`. A
  session then costs the fetch buffer rather than the result set, the position lives in the server,
  and `Next` past the buffer fetches the next block instead of re-querying. A cursor lives inside a
  transaction, which is where a session already is -- board:0012 pins the connection for exactly
  that. `WITH HOLD` is what carries one across a commit, and it costs a materialisation, so it is
  taken only where AL's own behaviour needs it. The predecessor never closed this (openerp WI-889,
  still open there) because Python had no equivalent to reach for.
- **The filters, not the rows, are what `Copy` carries.** A copied record variable must not double
  the snapshot; it takes the filters and re-reads. That falls out of `StateHandle` copying the state,
  and the snapshot being cleared on copy rather than duplicated.
- **A session's connection stays pinned for its transaction** (board:0012), and `FindSet(true)` reads
  `UPDLOCK`. With one user a missing lock is invisible; with a thousand it is the difference between
  a posting run and a deadlock.

## Gate

`RecordNavigationGate` over a fixture table: each filter operator produces the row set AL would;
`FindSet` honours `SetCurrentKey` and `Ascending(false)`; `Next()` is 0 at the end; `FindFirst`
agrees with `Find('-')`; `Count` and `IsEmpty` see the same filter as `FindSet`; `DeleteAll` removes
only the filtered rows and the negative control asserts the rest survive; `ModifyAll` fires no
`OnValidate`; `CopyFilters` from an unfiltered source clears; and `Reset` shows the whole table again.

The measurement is `agiru run-tests` over the 2 291, before and after.

## `ModifyAll` AND `DeleteAll` FALL BACK TO ROW-BY-ROW, AND FOUR OF THE FIVE REASONS ARE KNOWN AT TRANSLATION TIME

`administration/optimize-sql-al-Database-methods-and-performance-on-server.md` (read 2026-09-04 --
`dev-itpro/administration/` was outside board:0071's first denominator) states the rule exactly:

> Using **ModifyAll** and **DeleteAll** can improve performance by limiting the amount of SQL calls
> needed. However, **ModifyAll** and **DeleteAll** revert to individual calls if any of the following
> conditions exist:
>
> - There's trigger code on the table.
> - There are event subscribers to `OnBeforeModify`, `OnAfterModify`, `OnGlobalModify`,
>   `OnBeforeDelete`, `OnAfterDelete`, `OnGlobalDelete`, and `OnDatabaseModify`.
> - Security filtering is active.
> - The table contains `Media` or `MediaSet` data type fields.
> - There are fields that are added through companion tables.

**This is not a performance note, it is the SEMANTICS**: whether the triggers and events fire for
each row is the same question as whether one SQL statement runs. `Record.DeleteAll(true)` runs them
and `DeleteAll()` does not, which board:0044 already carries -- but the list above says the platform
ALSO drops to row-by-row when a subscriber exists, whether or not the caller asked for triggers.

**Four of the five are decidable when the tree is compiled**, and that is this tree's habit:

| condition | when it is known |
|---|---|
| the table declares `OnModify` / `OnDelete` | translation time -- it is in the `.al` |
| a subscriber exists for one of the seven events | translation time -- board:0057's subscriber table is `constexpr` |
| the table has a `Media` or `MediaSet` field | translation time -- the field table (board:0031) |
| the table has companion-table fields | translation time -- board:0033 merges extensions |
| security filtering is active | RUN time -- board:0062, per session |

So the generator can emit, beside each table, a `constexpr bool kBulkDeleteIsSafe` and the runtime
needs one further check. **A `static_assert` cannot state it** -- it is a property of the whole
program rather than of one object -- but a `constexpr` beside the table is the same idea one step
weaker, and it turns a decision the runtime would otherwise make by walking metadata into a load of
one byte.

## THE OTHER RULES ON THE SAME PAGE

- **A record is CLONED by `Copy`, by `RecordRef.GetTable`, and by being passed WITHOUT `var`** -- and
  "cloning a record before a **Modify** or **Delete** operation issues an extra SQL statement, since
  the SQL `SELECT` query is restarted every time the table is cloned". So AL's parameter mode has a
  measurable cost at the database, and the documented fix is to open the loop `FindSet(true)` and
  modify the SAME variable. That is a shape the generator must not accidentally break by copying
  where AL passed by reference -- CLAUDE.md's first tabulated trap, seen from the performance side.
- **`LockTable` costs no SQL statement of its own**: "It causes any subsequent reading from any
  tables to be done with an update lock." Confirms board:0012's tri-state exactly.
- **Every `Insert`, `Modify` and `Delete` is its own SQL statement**, and a table with SIFT indexes
  pays more per write (board:0019).

## `DeleteAll(true)` PASSES A COPY OF THE VARIABLE, AND THERE IS NO PERFORMANCE REASON FOR IT

`devenv-insert-modify-modifyall-delete-and-deleteall-methods.md` (read 2026-09-04, board:0071):

> When you use `DeleteAll(true)`, **a copy of the AL variable with its initial values is created**.
> This means that when you use `DeleteAll(true)` to run the `OnDelete` trigger, all the changes that
> were made to the variables in the method or codeunit that's making the call, **can't be seen in the
> `OnDelete` trigger**. If you want to see the changes that you made to the variables, you must use
> `Delete(true)` in a loop. **There's no difference in performance between using `DeleteAll(true)` and
> using `Delete(true)` in a loop.**

**That last sentence removes the only reason an implementer would have to deviate.** `DeleteAll(true)`
and `Delete(true)` in a loop cost the same, and they differ ONLY in what the trigger can see -- so a
runtime that implements the first as the second is wrong in a way that costs nothing to be right
about. The BaseApp's `OnDelete` triggers read record globals; handing them the caller's mutated
variable instead of a fresh copy changes what they delete.

This is the same distinction the previous section is about, one level in: `DeleteAll` WITHOUT
triggers is one SQL statement when the five conditions allow it; `DeleteAll(true)` is a loop over a
COPY. Two different mechanisms behind one name.

The same page settles three smaller things:

- **`Delete` takes filters into consideration** -- so `Rec.Delete()` after a `SetRange` is not simply
  "delete the row with this primary key".
- **`ModifyAll` returns nothing and does not raise on an empty set**, which makes it the one write
  in the family with no value context.
- **`Insert` may be given a `SystemId`, and "after the `SystemId` is set on a record, it can't be
  changed"** -- an immutability rule board:0013 owes, and the reason
  `record-insert-boolean-boolean-method.md` is the page CLAUDE.md names as the overload that matters.

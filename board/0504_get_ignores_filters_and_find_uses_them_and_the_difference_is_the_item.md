Type:     task
Status:   open
Parent:   0056
Area:     rt, db
Source:   developer/devenv-get-find-and-next-methods.md
Verdict:  teilweise
Class:    silent-wrong-data

# `Get` ignores filters, `Find` uses them, and `Next` returns a count

board:0056 is "the `Find` family reads one row" and this page is the concept behind it. It states four
things the individual method pages do not put side by side.

> **"`Get` searches for a record WITHOUT CHANGING ANY CURRENT FILTERS. `Get` ALWAYS SEARCHES THROUGH
> ALL THE RECORDS in a table."**
>
> The important differences between `Get` and `Find`:
> - **"`Find` USES the current filters."**
> - `Find` can look for records where the key value is **equal to, greater than, or smaller than** the
>   search string.
> - `Find` can find the first or the last record, **depending on the sort order defined by the current
>   key**.
>
> **"`Get` produces a RUNTIME ERROR if it fails AND THE RETURN VALUE ISN'T CHECKED BY THE CODE."**
>
> `Next` -- **"When there are no more records, `Next` returns a 0."**

**Three of these are already the tree's, and one is not.**

**`Get` ignoring filters is the fact to hold.** A `Get` on a filtered record variable finds a row the
same variable's `Find` would not -- so `Get` must not be implemented as "apply the filters and add
the primary key". That is the natural implementation and it is wrong, silently, only for filtered
variables.

**"`Get` produces a runtime error if the return value isn't checked" is VALUE CONTEXT**, which
CLAUDE.md lists as an inherited failure mode: "AL decides at consumption-versus-discard whether a
failure throws or yields `false`", with the guard "the contexts are named: assignment, `if`/`while`,
`exit`, argument, `case` selector". This page is the platform's own statement of it for `Get`, and it
applies to the whole family.

**`Next` returns a COUNT, not a Boolean.** `Steps := Record.Next([Steps])`, and `until Rec.Next = 0`
is the idiom. A `Next` returning `bool` would compile against `= 0` in C++ and be wrong for
`Next(5)`.

**And `GetBySystemId` is a second key path**: it retrieves by `SystemId`, which board:0013 owns, and
it takes a `Guid` in braces.

## The performance note, which is board:0048's

> "Consider using the **partial records** methods to improve performance, especially when looping
> through several records **or when TABLE EXTENSIONS are defined on the table.**"

Table extensions widen the row; board:0033 merges them at translation time, so in agiru a merged table
is one table and the extension half of that advice does not apply -- but the looping half does, and it
is board:0048's "a find loads the fields it was asked for".

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

These are method calls; board:0028 owns the builtin census and CLAUDE.md already records the
neighbouring number (`SetRange` 55 402 call sites, `GetView` 132). **The per-method counts belong
there and are not taken with this sweep's property pattern** -- stated rather than guessed.

## The IST-state

board:0056 records the family. `include/runtime/Table.h` carries the `Find` family; whether `Get`
bypasses the record's filters is exactly this item's question and is **not measured here** -- the
item's first task is to read `src/rt/Record.cpp`'s `Get` and say.

That is why the verdict is `teilweise` rather than `fehlt`: the family exists and one specific
property of it is unverified.

## The choice

`Get` builds its `WHERE` from the primary key alone and never from `Selection`/`Where`
(`src/rt/Selection.h`, `src/rt/Where.h`). `Next` returns `Integer`, and the `Steps` overload moves
that many rows and returns how many it actually moved.

The value-context rule is board:0056's and is not re-decided here; this page is a second citation for
it.

## Ordering

Inside board:0056. The `Get`-ignores-filters check is a read of existing code and comes first,
because it is either already right or a silent defect.

## Gate, and its negative control

`Rec.SetRange(Name, 'X'); Rec.Get('4711')` finds customer 4711 even though its name is not `X`;
`Rec.Find('-')` under the same filter does not.

**The negative control is the filtered `Get`** -- an implementation that ANDs the filters into the
primary-key lookup returns false, which looks like "no such record" and is indistinguishable from a
correct miss without the second assertion.

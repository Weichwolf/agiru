Type:     task
Status:   open
Parent:   0018
Area:     al, rt, db
Source:   developer/devenv-entering-criteria-in-filters.md
Verdict:  fehlt
Class:    activation

# The filter language is thirteen operators and a date shorthand

board:0018 is "the filter is a language and it gets a parser". **This page is that language**, in one
table, and it is the grammar the parser is written against.

## The operators

| symbol | meaning | example | matches |
|---|---|---|---|
| *(none)* / `=` | equal | `377` | 377 |
| `..` | interval | `1100..2100` | 1100 through 2100 |
| | open on the left | `..2500` | up to and including 2500 |
| | open on the right | `P8..` | accounting period 8 and thereafter |
| `\|` | either/or | `1200\|1300` | 1200 or 1300 |
| `&` | and | `<2000&>1000` | between them |
| `<>` | not equal | `<>0` | all but 0 |
| `>` `>=` `<` `<=` | comparison | `>=1200` | |
| `*` | **an indefinite number of unknown characters** | `*Co*` · `*Co` · `Co*` | contains · ends with · begins with |
| `?` | **one** unknown character | `Hans?n` | Hansen, Hanson |
| `( )` | **calculate before rest** | `30\|(>=10&<=20)` | |
| `@` | **ignore case** | `@location` | LOCATION, location, Location |

> **"The `&` sign can't be used by itself with numbers because no record can have two numbers."**

**Thirteen operators, and four of them have no SQL counterpart as written.** `*` and `?` are `LIKE`
with a different escape vocabulary; `@` is a per-TERM case-insensitivity switch, not a per-column
collation; `..` on an open end has no upper or lower bound to emit. Each is a translation decision,
and `@` is the sharpest: **case-insensitivity is declared inside the filter value**, so the same column
is compared case-sensitively in one filter and insensitively in the next. A collation cannot express
that; `LOWER(col) = LOWER(?)` can, and it defeats the index.

**That is a real cost and it belongs in the item**: board:0045 says every declared key is an index, and
an `@` term on an indexed column throws the index away unless a functional index exists. The frequency
decides whether that matters, and the frequency is measurable.

## The date and datetime shorthand, which is a second grammar

| written | means |
|---|---|
| `22` | a **datetime range**: 22-current month-current year `0:00:00` .. `22:59:59` |
| `22 10` | an exact datetime: 22-01-01 10:00:00 |
| `..12 31 00` | dates up to and including 12 31 00 |
| `..23` | from the beginning of time until 23-current month-current year 23:59:59 |
| `23..` | from 23-current month-current year 0:00:00 until the end of time |
| `22..23` | 22 at 0:00:00 until 23 at 23:59:59 |

**A bare number in a datetime filter is a RANGE over a day**, and the missing parts come from the
current month and year -- so the same filter string means different rows in January and February.
board:0082's `DateFormula` is a different grammar; this one is inside the filter.

**One contradiction, recorded rather than resolved**: the first row ends the day at **`22:59:59`**
while the `..23` and `22..23` rows end it at **`23:59:59`**. Three rows of one table, two answers. The
`23:59:59` rows are self-consistent and the `22:59:59` looks like the day number leaking into the
hour, but **this item does not decide it** -- a running BC or the AL source does.

## The closing warning, which is board:0016's

> "It's possible to specify an interval that doesn't exist, and **the system can't check this for
> you**. In order to enter meaningful filters, **you must know the SORTING RULES for the field you're
> filtering on.**"

A range is defined by the field's sort order, so a Code field's range depends on board:0080's
collation and a Date field's on board:0016's closing dates -- a closing date sorts after its own date
and before the next, so `010124..311224` includes `C311224` or not depending on that rule.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

CLAUDE.md records `SetRange` at **55 402** call sites. The per-operator frequency inside filter
STRINGS is the number that decides the parser's shape and is **not taken here** -- it is a scan of
`SetFilter` literals, not a property count. Stated rather than guessed.

## The IST-state

board:0018 records it. `src/rt/Filter.cpp`, `src/rt/Where.cpp` and `src/rt/Selection.cpp` exist;
**which of the thirteen operators they accept is not measured here** and is the item's first task.

## The choice

A recursive-descent parser in `src/al` producing a filter TREE -- board:0508 argues the same from
`GetRangeMin`'s side -- with `|` and `&` as the two binary levels, parentheses overriding, and the
prefix comparisons as leaves. The SQL is generated from the tree.

**`*` and `?` become `LIKE` with a generated escape**, `@` becomes a case-insensitive comparison on
that term alone, and the date shorthand is resolved against the SESSION's current date -- which makes
a filter string's meaning session-dependent and is worth a `\warning` in the door.

**Not a translation to SQL text.** The tree is needed by `GetRangeMin` (board:0508),
`PopulateAllFields` (board:0474), `GetView`/`SetView` (board:0432) and the request page's visible
filters (board:0451).

## Ordering

board:0018's core, and one of the earliest things in the tree: board:0331, board:0340, board:0430,
board:0432, board:0451, board:0453 and board:0475 all consume it.

## Gate, and its negative control

`30|(>=10&<=20)` matches 30 and 10..20 and not 25; `@location` matches `LOCATION`; `Hans?n` matches
`Hansen` and not `Hanssen`; `*Co` matches `Tesco` and not `Cost`.

**The negative control is `Hanssen` and `Cost`** -- `?` matching more than one character, or `*Co`
anchoring at the wrong end, both produce a superset that looks like a working filter. A gate with only
positive matches passes either way.

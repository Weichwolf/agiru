Type:     task
Status:   open
Parent:   0033
Area:     gen
Source:   developer/properties/devenv-extensible-property.md
Verdict:  fehlt
Class:    activation

# `Extensible` decides whether an extension may reach an object

> **Version**: runtime 4.0. Applies to: **Table, Page, Report, Enum Type**.
>
> **True** if the table, page, report, or enum can be extended; otherwise false. **The default is
> true on tables, pages, and reports, whereas it is false on enums.**
>
> If the value is true, the object can be extended using a `tableextension`, `pageextension`,
> `reportextension`, or `enumextension` respectively.

**The default flips by object kind**, and that is the sentence to get right: a table with no
`Extensible` is extensible, an enum with no `Extensible` is not. One default per kind, decided by the
generator, and a single `bool` initialised one way is wrong for one of the two.

**The enum half is board:0084's** -- it measures 1 061 `true` against 1 226 `false` on enums and ties
the property to `AssignmentCompatibility`. This item is the other three kinds and the shared
mechanism.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Extensible =`: **2 285 declarations** -- 1 225 `false`, 1 060 `true`.

board:0084 counts 1 061 / 1 226 over enums specifically, so **the property is declared almost
exclusively on enums**: at most a few dozen of the 2 285 are on a table, a page or a report. The
non-enum half of this property is nearly unused, and that is the number that sizes the item.

## The IST-state

Not among the nine properties the generator consumes (board:0067). Extensions are merged at
translation time (board:0033), and nothing checks whether the target permitted it.

## The choice

**A `static_assert`, and nothing at run time.** Whether an extension may extend an object is decided
entirely from two declarations, both of which the transpiler holds; so a `tableextension` over a
table declaring `Extensible = false` is a translation error naming both objects.

That is this tree's stated rule -- "anything decidable at translation time is a `static_assert`, never
a test case" -- and this property is a clean instance of it: no runtime state, no session, no data.

**The per-kind default is a `constexpr` in the generator and not a literal at each call site.**

## Ordering

With board:0033's merge, which is where the pair of declarations meets. board:0084 owns the enum half
and should not be duplicated here.

## Gate, and its negative control

A `tableextension` over `Extensible = false` fails to transpile; over a table declaring nothing it
succeeds.

**The negative control is an `enumextension` over an enum declaring nothing** -- it must FAIL, because
the enum default is `false`, and an implementation with one shared default passes the table case and
gets this one backwards.

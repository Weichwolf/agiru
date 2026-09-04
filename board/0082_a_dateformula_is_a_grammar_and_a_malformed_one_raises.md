Type: root
State: open
Area: net

# A DateFormula is a GRAMMAR, and a malformed one raises instead of returning the reference date

`system-calcdate-dateformula-date-method.md` states the production rules and one consequence:

```
<Subexpression> = [<Sign>] <Term>
<Sign>          = + | -
<Term>          = <Number><Unit> | <Unit><Number> | <Prefix><Unit>
<Number>        = Positive integer
<Unit>          = D | WD | W | M | Q | Y
<Prefix>        = C
```

> These production rules show that date expressions consist of **zero, one, two, or three
> subexpressions**. ... **A run-time error occurs if the syntax of DateExpression is incorrect.**

`src/net/DateFormula.cpp` implements most of it and gets the hard part right. What it does not do is
REFUSE, and what it silently mis-parses is one of the three term forms.

## What is right, and it is checked against the documentation's own worked examples

`FromText` + `CalcDate` reproduce all three examples on the page, from `RefDate := 19960521D`:

| expression | documented | agiru's path |
|---|---|---|
| `<CQ+1M-10D>` | 07/20/96 | `Boundary(Q, first=false)` -> 06/30, `AddMonths(+1)` -> 07/30, `-10D` -> **07/20** |
| `<-WD2>` | 05/14/96 -- "the last weekday no. 2" | `Weekday(d, 2, backwards)` with `back == 0` forced to a full week -> **05/14** |
| `<CM+30D>` | 06/30/96 | `Boundary(M, first=false)` -> 05/31, `+30D` -> **06/30** |

The `C` prefix carries the subtlety a reader would get wrong: `CM` is the LAST day of the current
month and `-CM` the first, and `Boundary(d, unit, first)` is called with `first = negative`
(`src/net/DateFormula.cpp:164`), which is exactly that rule and not an accident. **Those three lines
are the gate corpus this item asks for** -- they come from the specification, so they cannot be
back-filled from the output.

## What is wrong, and each is separately reachable

**A MALFORMED EXPRESSION RETURNS THE REFERENCE DATE.** `FromText`'s loop ends every unrecognised
character with a bare `++at` (`src/net/DateFormula.cpp:153`), so `CalcDate('<XYZ>', d)` yields `d`.
The page says it is a run-time error. A due date computed from a corrupted `Payment Terms` formula
is then the document date, which posts and reconciles and is wrong -- **silent-wrong-data of the
most expensive kind**, because the answer looks like a date.

**`<Unit><Number>` IS IMPLEMENTED FOR TWO UNITS OF SIX.** The parser has branches for `WD<n>`
(weekday) and `W<n>` (week); `D`, `M`, `Q` and `Y` fall through to the `<Number><Unit>` branch with
`count = 1` when no digits preceded them, and the digits that FOLLOW are then skipped one at a time
by the same bare `++at`:

| written | means (`ui-enter-date-ranges.md`) | agiru today |
|---|---|---|
| `D10` | the next 10th day of a month | **`+1D`**, and the `10` is dropped |
| `M10` | the next 10th month of a year | **`+1M`** |
| `Q2` | the next 2nd quarter | **`+1Q`** |
| `Y3` | the next 3rd year | **`+1Y`** |

**AND `W<n>` IGNORES ITS SIGN.** `Kind::Week` computes week `n` of the reference date's own year from
the ISO 4-January rule and never reads `term.negative`, so `-W23` and `W23` give the same date.
`Weekday` reads the sign and `Week` does not, which is the shape of a case written once and not
finished.

**Three documented BOUNDS are unchecked**: at most three subexpressions (this page), at most 20
characters, and a number no larger than 9999 (`ui-enter-date-ranges.md`). They belong beside
board:0081's limits, and for a formula read from a FIELD they are a parse-time refusal rather than a
`static_assert`.

## The choice

`FromText` becomes a parser that consumes its input or refuses it. It is some thirty lines and no
new type.

- The loop reads one `<Subexpression>` per turn and **fails on any character it did not consume**,
  replacing three bare `++at` sites with one `throw Error(...)` naming the formula and the offset.
  That is the documented run-time error, and it is the whole of the first defect.
- The `<Unit><Number>` branch generalises: after a unit letter, if digits follow, the term is
  `Kind::Ordinal` with that unit -- "the next n-th D/W/M/Q/Y" -- and `WD` stays the two-letter unit
  it already is. `Kind::Weekday` becomes the `WD` case of `Ordinal` rather than its own kind, so the
  forward/backward rule (a shift of zero means a whole unit, not a no-op) is written ONCE and
  `W<n>` inherits the sign it currently drops.
- The three bounds are checked in the same pass, since the parser is already counting terms.

**What this does NOT take on:** the formula is language-dependent unless it is written in `<>`, and a
user in French stores `1S+1J`. `FromText` strips the delimiters and then reads English letters only.
agiru is single-language today, so that is a NAMED deviation rather than a defect -- and it is the
reason `FromText` must keep the original text (it does, in `text_`) rather than normalise it, because
`Format(DateFormula)` owes the user their own language back (board:0066).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`, all `.al`

| | |
|---|---|
| `CalcDate(` call sites | **16 410**, in 2 347 files |
| `DateFormula` declared as a type | 2 269 |

Payment terms, reminder levels, recurring journals, lead times and safety lead times are all this
type, so a wrong answer here is a wrong DUE DATE on a document that posts.

## Gate, and its negative control

The three worked examples above, plus one per defect: `D10`, `M10`, `Q2` and `-W23` against a hand-
computed date, and `CalcDate('<XYZ>')` required to RAISE. **The negative control is the last one** --
delete the refusal and the case must go red. A gate that checks only the three good expressions
passes today, which is why this item names the bad ones first.

Classification: **silent-wrong-data** for every row of the table; the refusal is an **activation** (a
formula that previously computed something now stops), so it carries an A/B over the suite once the
tests run.

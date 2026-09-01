Type: leaf
State: open
Area: net, rt
Tags: navision, semantics

# A date formula is parsed, and the parts the documentation does not pin down are asked rather than guessed

`<CQ+1M-10D>`, `<-WD2>`, `<CM+30D>`, `<D15>`, `<2W>`. NAV's date formula is a small language with a
production the documentation states outright:

    <Prefix><Unit><Sign><Number><Unit><Sign><Number><Unit>

and 17 BaseApp tables declare a field of this type. `Evaluate` is the only way to assign one, and
`Format` is the only way to compare one against text -- the page says both.

## What the documentation settles, with worked examples

`system-calcdate-string-date-method.md` gives three, all from Tuesday 21 May 1996, with the results
printed:

| formula | result | what it forces |
|---|---|---|
| `<CQ+1M-10D>` | 1996-07-20 | CQ is the LAST day of the current quarter: 06-30, +1M = 07-30, -10D = 07-20 |
| `<CM+30D>` | 1996-06-30 | CM is the LAST day of the current month: 05-31, +30D = 06-30 |
| `<-WD2>` | 1996-05-14 | WD2 is Tuesday, and `-WD2` from a Tuesday goes to the PREVIOUS one rather than staying put |

Plus: "In Business Central, weeks begin on Monday and end on Sunday", and the angle brackets mean
the expression is not translated -- without them the formula is read in the user's language, where
`1W+1D` is `1S+1J` in French.

## What it does NOT settle, and must not be guessed

- **`-CM`.** NAV's rule as remembered is that the C prefix goes to the END of the period with a
  plus and to the BEGINNING with a minus, so `<-CM>` is the first of the month. The three examples
  above contain no negative C term, so nothing here proves it.
- **`CY` and `CW`**, by analogy last day of year and Sunday -- analogy, not a citation.
- **`D15`**, "on the 15th of each month": the next 15th, or this month's even if past?
- **`WD2` with no sign**: the next Tuesday, or today when today is Tuesday?
- The **language-dependent** reading, which needs a language on the session that does not exist.

Each of these is a case where openerp is worth grepping first -- it implemented this and its
backlog records what it cost -- and where the answer is then confirmed against BaseApp usage before
it is written down.

## The benchmark

The three documented examples as a gate, and then the distinct DateFormula literals in the BaseApp
as a corpus: every one parses, and the count is a baseline that may only rise.

## Predecessor -- which answers every open question above, with its evidence

`openerp/runtime/builtins/_datetime.py` implements the whole grammar and records both its citations
and its refuted attempts. Taking these as HINTS to confirm, not as verdicts:

| question | its answer | its evidence |
|---|---|---|
| `-CM` | the FIRST day of the period; no sign or `+` gives the last | `Date2DMY('<-CM>')=1` in an ERMKPIWebService test |
| `CY` / `CW` / `CQ` | Jan 1 or Dec 31; Monday or Sunday; quarter bounds | same sign rule throughout |
| `WDn` signed | next or previous occurrence, EXCLUSIVE of the reference date, so `D` itself moves `D±7` | the `<-WD2>` example this item already quotes |
| `Dn` / unsigned `WDn` | a selector, where the sign is meaningless | -- |

**And a form neither this item nor the platform page mentions: `<Wn>`.** It is the Monday of ISO
week n of the base year, first-class grammar, distinct from `<nW>` (n weeks) and `<WDn>` (weekday).
Its absence was silent-wrong rather than an error:

> Der frühere Regex las `W1` als W(oche)+Quantity-0 -> Ziffer verworfen, Basisdatum unverandert
> (silent-wrong).

**Three more that only an implementation finds:**

- `<nM>` keeps the day of month and CLAMPS to the target month's length rather than adding 28 or 30:
  `Dec-31 <1M>` is `Jan-31`.
- `<nY>` clamps the same way, so `Feb-29 <1Y>` is `Feb-28`.
- `<nQ>` is `3n` months and shares the month logic.

**Three refuted attempts are recorded there and must not be repeated**: dropping the sign on a
C-period (which made `<-CM>` the last day), ignoring `WD` entirely (no shift at all), and letting
`n`-quarter fall through the pattern.

The order the terms are matched in is itself load-bearing: `WD` first, so that it does not split
into W(eek) + D(ay).

## Closed when

The three worked examples pass, every open question above is answered with a citation or a measured
BaseApp usage rather than an analogy, and the corpus parses whole.

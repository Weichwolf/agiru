Type: root
State: open
Area: rt, net
Tags: navision, semantics

# `Format` is a LANGUAGE, `Evaluate` reads it back, and every expected error text stands on both

board:0028 deferred this in one sentence -- "`_format.py` is the one that is not a function but a
LANGUAGE ... It gets its own item when it is reached". It is reached: `Format` is the third-most
called builtin in the milestone and the second-largest primitive behind every error text a test
compares.

## Measured 2026-09-04

Over the 2 305 `[Test]` procedures of the 80 UT codeunits (board:0058), counting the procedures that
call each, with comments and strings removed:

| | procedures | share | codeunits |
|---|---:|---:|---:|
| `StrSubstNo(` | **184** | 8.0 % | 35 |
| `Format(` | **108** | 4.7 % | 20 |
| `CalcDate` / `DateFormula` | 54 | 2.3 % | 11 |

Over the whole of `Layers/W1`: **`StrSubstNo(` 14 800 sites, `Format(` 11 992.**

**And board:0055 makes them load-bearing rather than cosmetic**: 130 of the 276 `ExpectedError`
texts in the milestone are found verbatim as a `Label` in the BaseApp, "provided `StrSubstNo` and
`Error(Label, ...)` substitute identically". A number formatted one separator off makes a message
unequal and the failure looks like a semantic defect. `Format` is not a display concern here; it is
half the assertion surface.

## What the platform documents, and it is a grammar rather than a function

`devenv-format-property.md` is the specification and it has two halves.

**The STANDARD formats, per type, as a table this item can be gated against directly:**

| type | formats | example, Format 0 |
|---|---|---|
| Decimal | 0-4 and 9 | Europe `-76.543,21`, US `-76,543.21` |
| Integer | 0-2 | `-567` |
| Date | 0-9 | `<Day,2><Filler Character, >. <Month Text,3> <Year4>` is 7 -> `5. Apr 2021` |
| Time | 0-2 | `043555.553T` |

The Europe/US rows differ for the SAME format number, which is what makes a locale part of the
signature and not a setting somebody forgot.

**The FORMAT STRING, which is the language:** `<Sign>`, `<Integer>`, `<Integer Thousand>`,
`<Decimals>`, `<Comma,.>`, `<Filler Character,0>`, `<Precision,2:3>`, `<Standard Format,5>`,
`<Day,2>`, `<Month Text,3>`, `<Year4>`, `<Hours24,2>`, `<Second dec.>`, `<1000Character>`,
`<Overflow>`. The angle brackets are required, attributes compose, and `<Precision,2:3>` binds the
number of decimals to the field's `DecimalPlaces` property -- so the formatter needs the FIELD and
not only the value.

`Format(Value, Length, FormatNumberOrString)` has two documented overloads
(`system-format-joker-integer-integer-method.md` and `-integer-string-`), and `Evaluate` is the
inverse: it parses a text back into a typed value, by the same rules.

## AND THE FIELD DECIDES ITS OWN FORMAT, which is the half a `Format(Value)` signature cannot carry

Measured over the read roots, 2026-09-04:

| property | declarations | what it decides |
|---|---:|---|
| `AutoFormatType` | **9 690** | which formatting rule the field takes -- amount, unit amount, currency-qualified |
| `AutoFormatExpression` | **4 803** | the argument to it, usually a currency code, often an expression over the record |
| `DecimalPlaces` | 3 413 | the precision `<Precision,2:3>` otherwise names inline |
| `BlankZero` / `BlankNumbers` | 1 976 | whether a zero renders as an empty string |

None of the four is read by the generator (board:0067). **A `Decimal` field with
`AutoFormatType = 1` and `AutoFormatExpression = "Currency Code"` renders as `1.234,56` in one
company and `USD 1,234.56` in another**, and BC resolves that through the **`Auto Format` codeunit**
and its **`OnResolveAutoFormat`** and **`OnAfterResolveAutoFormat`** events -- so the resolution is
AL code
this tree already transpiles, and what the platform owes is the property reaching it.

The predecessor's WI-1036 is the evidence that this is reachable and load-bearing: its probe was
`auto_format.resolve_auto_format(1,'')` returning `''` where BC returns
`<C,GBP><Precision,2:2><Standard Format,0>` -- an AutoFormat that resolves to nothing formats every
amount without its currency, and nothing raises.

## What is here now

- `board:0007` holds the decimal half and says the right thing about where it lives: the value type
  keeps `ToInvariantString`, and `Format` is a RUNTIME service taking a locale and the field's
  `DecimalPlaces`, "because a value that depends on a session is not a value".
- `System.Format(Any, Integer, Integer)` stands in `include/Builtins.h` and is one of the functions
  the door's generator DROPS when regenerated -- board:0046, where `Assert.cpp` calling
  `Format(Left, 0, 2)` is the immediate casualty.
- `StrSubstNo` exists as a declaration; whether its `%1` substitution agrees with `Format`'s output
  is untested, and that agreement is what board:0055's 130 texts depend on.

## What the predecessor made of it

`~/Git/openerp/openerp/runtime/builtins/_format.py`: **14 functions and a specifier parser**, the
single largest builtin module of the twelve, on a run that reached 97 % of the same subset. That is
the size estimate, and it is a measurement rather than a guess.

## The choice

- **One parser for the format string, in `src/rt` beside the filter parser** (board:0018), producing
  a small `constexpr`-friendly instruction list. It is the same shape twice: AL has two little
  languages and neither maps onto a printf.
- **The standard formats are a TABLE per type, not code per case** -- the documentation's own tables
  transcribed as `.rodata`, so a format number is an index and adding a locale is data.
- **The locale comes from the session and the precision from the field.** Neither is a constant, and
  `<Precision,2:3>` is why the formatter's signature carries the field rather than only the value.
- **`Evaluate` shares the tables.** A formatter and a parser that disagree produce a value that
  round-trips wrong, which in an accounting system is a wrong amount rather than a wrong string.
- **`StrSubstNo` calls `Format` for every substituted value** -- one path, so the two cannot drift.

## Gate

The documentation's own tables, both regions, every standard format, the same number -- as cases
(board:0007 already asks for the decimal ones). A format string composed from three attributes.
`<Precision,2:3>` against a field declaring `DecimalPlaces = 2`. `Evaluate` reading back what
`Format` wrote, for Decimal, Date, Time and DateFormula. `StrSubstNo('%1', 1234.5)` equal to
`Format(1234.5)`.

**Negative control**: swap the thousands separator for the decimal separator and require the region
cases to go red -- an invariant formatter passes every case written in one locale, which is the
state the tree is in.

## THE OTHER DIRECTION HAS A SUBLANGUAGE TOO, read 2026-09-04 (board:0071)

`ui-enter-data.md` documents what a user may TYPE into a numeric field, and it is not a number:

| typed | recorded | |
|---|---|---|
| `19+19` | 38 | "you can enter the formula instead of the sum quantity" |
| `41-9` | 32 | |
| `12*4` | 48 | |
| `12/4` | 3 | |
| `20.5-` | -20.5 | a TRAILING sign |
| **`10-20+`** | **10** | **"If the last character of the expression is a `+` or a `-`, the entire expression will be recorded with that sign"** -- so the arithmetic gives -10 and the trailing sign then makes it positive |

The last row is the one a reader would never guess and a test would never think to write: the
trailing sign is not an operator, it is a SIGN APPLIED TO THE RESULT. Any implementation that
tokenised left to right gets -10 and looks correct.

**The route is the page's field-input conversion, not `Evaluate`.** `Evaluate(Decimal, '19+19')`
returns `false` -- the arithmetic belongs to what the client does before the value reaches
`Validate`. So it is board:0030's surface, and a `TestPage.SetValue('19+19')` must produce 38 for a
test to behave the way a user does. It is recorded HERE because it is the same class of thing this
item is about: **a value's text form is a language in both directions**, and the two must round-trip.

`ui-enter-data.md` also fixes WHEN validation happens, which board:0030 needs and no page in
`triggers-auto/` states: **"After you specify a value, Business Central will only check that it's
valid after you click outside the field or set focus to another element."** So `OnValidate` fires on
FOCUS LOSS and not per keystroke, and one `TestPage.SetValue` is exactly one such cycle.


## THE EVENT NAMES, CORRECTED 2026-09-04 (board:0071)

This item named the resolution point as codeunit 45 `AutoFormatManagement` / `OnAfterAutoFormatTranslate`.
`devenv-format-field-data.md` names the codeunit **`Auto Format`** and TWO events --
`OnResolveAutoFormat` and `OnAfterResolveAutoFormat` -- and the AL source confirms both, in the
System Application rather than the BaseApp:

- `System Application/App/Auto Format/src/AutoFormatImpl.Codeunit.al:38`
  `AutoFormat.OnResolveAutoFormat(AutoFormatType, AutoFormatExpr, Result, Resolved);`
- `:58` `AutoFormat.OnAfterResolveAutoFormat(...)`
- and a subscriber in `System Application/Test/Auto Format/src/AutoFormatTest.Codeunit.al:139`,
  which is a GATE that already exists in AL and will run under `agiru run-tests`.

**Two events and not one, and the first carries a `var Resolved: Boolean`** -- the handled flag
board:0057's dispatch has to pass by reference. So the AutoFormat route is the same mechanism as
every other event and needs nothing of its own; what it needs is that a `var` parameter written by a
subscriber reaches the publisher, which is board:0057's own subject.

**And the same page fixes the PRECEDENCE**: `AutoFormatType` and `AutoFormatExpression` may be set on
the table field, or on the page or report field, and "**if you specify the properties on the table
field and the page or report field, the settings on the page or report field take precedence**".
That is the third three-level resolution this sweep has found, after the report language and the
report layout (board:0063) -- and all three resolve the same way round: the most specific wins.

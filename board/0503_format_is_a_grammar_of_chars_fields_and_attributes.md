Type:     task
Status:   open
Parent:   0066
Area:     net, al
Source:   developer/devenv-format-property.md
Verdict:  fehlt
Class:    activation

# `Format` is a grammar of chars, fields and attributes

board:0066 is "Format is a language and Evaluate reads it". **This page is that language's grammar**,
and it is a page a reader could mistake for a properties page -- it sits in the root, not in
`properties/`, and it describes the `Format` METHOD rather than the XMLport property board:0442 filed.

## The grammar, verbatim

```
FormatProperty := [ <Char> | <Field> | <Attribute> ]
<Char>         := character with ASCII value [32..255]
<Field>        := '<' <FieldName> [',' <FieldLen>] '>'   [, <Attribute>]
<FieldLen>     := length of field -- 0 or no entry means the length is DYNAMIC
<Attribute>    := '<' <AttributeName> ',' <Char> '>'
<AttributeName>:= Standard Format | 1000Character | Comma | Overflow
                | Filler Character | Precision
```

> **"You must enter the `<` and `>` angle brackets."**

**Six attributes, and two of them have POSITIONAL rules:**

> "The **`1000Character`** attribute must be **AFTER the `Integer` or `Integer Thousand` field name
> and BEFORE the `Decimals` field name.**"
>
> "The **`Comma`** attribute must be **AFTER the `Decimals` field name.**"

So the format string is order-sensitive in a way a naive tokeniser would not check, and a misplaced
attribute is a declaration BC rejects.

`Overflow` is the character used **"when the formatted result exceeds the field length"**, which is
what makes `<FieldLen>` more than a hint -- a fixed-length field truncates and marks it.

## The field names, per data type

| type | fields |
|---|---|
| Decimal | Sign, Integer, Decimals, **Integer Thousand** |
| Date | Day, Month, **Month Text**, Quarter, Year, **Year4**, Week, Week Year, Week Year4, Weekday, **Weekday Text**, **Closing** |
| Time | Hours24, Hours12, Minutes, Seconds, Thousands, **AM/PM**, **Second dec** |
| DateTime | the Date set plus the Time set |
| Integer, BigInteger | Sign, Integer, Integer Thousand |
| Boolean | **Text, Number** |
| Option | **Text, Number** |
| Code, Text | Text |
| Char | **Char/Number, Char, Number** |

**`Closing` is a Date field name**, which is board:0016's closing date appearing in the format grammar
-- standard date format 2 renders it as a trailing `D`: `050421D`. So a closing date is not only a
sort-order rule, it has a rendering, and board:0016 needs it.

**`Text` and `Number` on Boolean and Option** is board:0053's caption-versus-ordinal question with a
format selector: `<Number>` renders an Option's ordinal, `<Text>` its caption.

## The standard formats are CULTURE-DEPENDENT, and format 2 is the escape

The page gives Europe and US tables for Decimal, Date and more. Reading them together:

| format | Europe | US | |
|---|---|---|---|
| 0 | `-76.543,21` | `-76,543.21` | thousands, culture separators |
| 1 | `-76543,21` | `-76543.21` | no thousands, culture separator |
| **2** | `-76543.21` | `-76543.21` | **`<Comma,.>` -- forced point, culture-free** |
| 3 | `76.543,21-` | `76,543.21-` | trailing sign |
| 4 | `76543,21-` | `76543.21-` | |
| **9** | `-76543.21` | `-76543.21` | **XML format** |

**Format 2 and format 9 are the two culture-free decimal renderings**, and the tables show how 2 gets
there -- it is format 1 with an explicit `<Comma,.>`. board:0491 saw the same pair from the
`AutoFormat` side; this page shows the mechanism.

**And date format 2 is `<Day,2><Month,2><Year><Closing>D`** -- the AL code-constant form, which is
what `Evaluate` must read back. So format 2 across types is the round-trip format, and that is the one
board:0066's `Evaluate` half depends on.

> "The settings under Regional and Language Options ... determine how some separators are displayed.
> In the client you can specify a **Region** under Settings."

**A per-session region**, which board:0438 already flagged: a locale that reached a stored value or a
sort would break determinism. Here it reaches only the rendering.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Format` is a method, not a property; board:0028 owns the builtin census. **Stated rather than
guessed** -- and the count matters, because board:0066's engine is sized by it.

## The IST-state

board:0066 records it; board:0007 owns `Decimal` formatting with one format;
`src/rt/Builtins.cpp` refuses the `Format` family (board:0035's counted refusals).

## The choice

**A parser over the grammar above, in `src/net`, producing a `constexpr` format program where the
string is a literal** -- and a run-time parse only where AL builds the string. The ten standard
formats per type are `constexpr` field-and-attribute sequences, exactly as the tables spell them, so
"standard format 3" is not a special case but a named sequence.

**The positional rules for `1000Character` and `Comma` are checked in the parser**, not at render
time.

**`Precision` is board:0325's `DecimalPlaces` fallback**: "if you don't specify a precision, the page
uses the precision specified in the `DecimalPlaces` property of the corresponding field" -- so the
format engine reads field metadata, which board:0491 also needs.

## Ordering

board:0066's core. Behind board:0007's `Decimal` and ahead of board:0491, which selects among these
formats, and board:0442, whose XMLport `Format` property is a different thing with the same name.

## Gate, and its negative control

`<Weekday Text>, <Month Text> <Day>` renders 2021-04-05 as `Monday, April 5`;
`<Precision,2:3><Standard Format,0>` renders with two to three decimals; standard format 2 renders the
same string under a European and a US region.

**The negative control is standard format 0 under two regions** -- it must render DIFFERENTLY, and an
implementation that ignores the region passes the format-2 assertion and gets every other format
wrong in exactly one place.

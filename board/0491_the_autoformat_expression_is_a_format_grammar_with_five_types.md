Type:     task
Status:   open
Parent:   0066
Area:     net, rt, gen
Source:   developer/devenv-format-field-data.md
Verdict:  fehlt
Class:    activation

# The auto-format expression is a format grammar, and the type selects which

board:0437 filed `AutoFormatType` and `AutoFormatExpression` from the property pages and recorded a
gap: the property page names six values and explains three. **This is the page that explains them**,
and it is more than a lookup table -- the expression is a grammar.

## The five types the page documents

| type | expression | what it does |
|---|---|---|
| **0** | ignored; **`DecimalPlaces` applies** | standard format 0 with that many decimals |
| **1** | a **currency code**; blank `''` is LCY and the default | format as an **amount** |
| **2** | a currency code | format as a **unit amount** -- unit prices |
| **10** | `'[SubType][,<currencycode or expression>[,<PrefixedText>]]'` | subtype `1` or `2` reuses the amount/unit precision **and adds the currency symbol**; any other number or none takes a custom format |
| **11** | a standard format string | **"the format string is applied EXACTLY as specified"** |
| **N/A** | `'<FormatString>#,##0.00;(#,##0.00);Zero'` | the **.NET numeric format string**, supported **from version 26.0** |

> **"With the following setup, the `DecimalPlaces` property is IGNORED"** -- for types 1, 2, 10 and 11.

**So `DecimalPlaces` (board:0325) applies to exactly one of the six types.** A field declaring
`AutoFormatType = 1` and `DecimalPlaces = 5` shows the CURRENCY's precision, not five places, and an
implementation that applied both would be wrong on every amount in the system.

## The expression grammar

```
<precision,minimum:maximum>     minimum and maximum decimal places
<standard format,N>             one of the six standard formats below
%                               at the END -- "the decimal value is ASSUMED TO BE THE RATIO
                                and will be MULTIPLIED BY 100"; 0.98 renders as 98%
```

Literal text may precede and follow: `'$<precision, 2:2><standard format, 0>'` gives `$76,453.21`.

**The six standard formats**, which are what `<standard format,N>` selects:

| N | shape | Europe | US |
|---|---|---|---|
| 0 | sign, thousands, separator, decimals | `-76.543,21` | `-76,543.21` |
| 1 | sign, integer, separator, decimals -- **no thousands** | `-76543,21` | `-76543.21` |
| 2 | as 1, but **the point is always a point** | `-76543.21` | `-76543.21` |
| 3 | thousands, decimals, **trailing sign** | `76.543,21-` | `76,543.21-` |
| 4 | integer, decimals, trailing sign, no thousands | `76543,21-` | `76543.21-` |
| 9 | **XML format** | `-76543.21` | `-76543.21` |

**Format 2 and format 9 are culture-independent** -- both render the same in both columns -- and 9 is
named XML, which is what board:0442's `FormatEvaluate = Xml` produces. So the culture-free decimal
this tree needs for a wire format already has a number in AL.

## The contradiction, recorded rather than resolved

`devenv-autoformattype-property.md` says the value is **"`0`, `1`, `2`, `3`, `10` or `11`"**. This
page documents 0, 1, 2, 10, 11 and the version-26 `.NET` case -- **and no `3`**. Either the property
page lists a value that does nothing, or this page omits one. The AL source decides, and board:0437's
population of 40 808 makes the sample large enough to see whether any field declares `3`.

## The precedence, which board:0437 states from the other side

> "If you specify the properties on the table field and the page or report field, **the settings on
> the page or report field take precedence.**"

The ordinary direction, unlike board:0374's `DataCaptionFields`.

## The IST-state

board:0437: neither property is among the nine the generator consumes (board:0067). board:0007 owns
`Decimal` formatting and has one format; board:0066 has no expression parser.

## The choice

**The expression is parsed by the generator into `constexpr` data** where it is a literal -- which is
most of the 23 126 -- and the standard formats are six `constexpr` renderers, not a format string
interpreted per value. The precision pair is two integers.

**The `%` multiplication happens on `agiru::Decimal` and never on a `double`** -- multiplying by 100
is exact in decimal and a rounding step in binary, and CLAUDE.md's second invariant is that no binary
float carries an amount.

**Types 1, 2 and the subtype forms still call the AL resolver** (board:0437): the currency code is
looked up by codeunit 45, and this item supplies the FORMAT the resolver's answer is rendered with.

## Ordering

Behind board:0007's `Decimal` rendering and board:0437's resolver event. This item is the grammar;
board:0437 is the declaration and the event.

## Gate, and its negative control

`'$<precision, 2:2><standard format, 0>'` renders `76543.21` as `$76,543.21`;
`'<precision, 1:1><standard format,0>%'` renders `0.98` as `98%`; `AutoFormatType = 0` with
`DecimalPlaces = 0` renders `-76543.21` as `76,543`.

**The negative control is `DecimalPlaces` under type 1** -- it must be IGNORED, and an implementation
that honours both passes every gate where the currency's precision happens to equal the declared one.

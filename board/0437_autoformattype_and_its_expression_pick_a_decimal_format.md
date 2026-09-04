Type:     task
Status:   open
Parent:   0066
Area:     gen, rt, net
Source:   developer/properties/devenv-autoformattype-property.md, developer/properties/devenv-autoformatexpression-property.md
Verdict:  fehlt
Class:    activation

# `AutoFormatType` and its expression pick a decimal's format

**Two pages, one item**: each is written in terms of the other and the documentation's own syntax
block shows them together. Neither formats anything alone.

> Applies to: **Table field, Page Field, Report Column.**
>
> `AutoFormatType` is `0`, `1`, `2`, `3`, `10` or `11`. **These properties are ONLY used to format
> DECIMAL data types**, such as amounts that can be stated in a foreign currency, or ratios.
>
> | data | type | expression | meaning |
> |---|---|---|---|
> | other | `0` | blank | default formatting |
> | **amount** | `1` | returns a **currency code** | line amounts, document totals, accounting values |
> | **unit amount** | `2` | returns a currency code | unit prices |
>
> `AutoFormatExpression = 'USD'; AutoFormatType = 1;` **results in a value like `7,564.00`.**
>
> **The AL expression is evaluated when the expression performs updates.**

**The page documents three of the six values.** `3`, `10` and `11` are named in the property-value
line and explained only in `devenv-format-field-data.md`, which is a root page and is read separately.
That gap is recorded rather than filled by guessing.

**And the resolution is AL, not platform** -- board:0066 already measured it: the `Auto Format`
codeunit's `OnResolveAutoFormat` and `OnAfterResolveAutoFormat`, confirmed in
`System Application/App/Auto Format/src/AutoFormatImpl.Codeunit.al:38` and `:58`. So this is
board:0384's `CaptionClass` shape exactly: the runtime carries the type and the expression's value and
CALLS the BaseApp, and hardcoding what type `1` means in `src/` would break the invariant that the
runtime knows no AL object.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AutoFormatType =` **40 808** · `AutoFormatExpression =` **23 126**.

**`AutoFormatType` at 40 808 is the sixth-largest population in the sweep** -- above `TableRelation`'s
40 221. Every Decimal field in an ERP declares how it is formatted, which is what an accounting system
looks like: 40 808 amounts whose rendering is a currency's decision.

The 17 682 declaring a type and no expression are the `AutoFormatType = 0` cases, where the
documentation says the expression must be blank.

## The IST-state

Neither is among the nine properties the generator consumes (board:0067). board:0007 owns Decimal
formatting and board:0066 the format engine; neither has a field-property input.

## The choice

An `std::uint8_t` type and a `string_view` expression on `FieldDef` and on the control, and the format
path raises the resolver event with both. **The expression is AL** -- `'<Currency Code>'` in the syntax
block is a placeholder for a field reference -- so where it is not a literal it is a generated
predicate, like board:0407's `ShowMandatory`.

**Not a switch on the type value in `src/`.** Six values whose meaning lives in a BaseApp codeunit.

## Ordering

Behind board:0066's format engine and board:0057's event dispatch. The metadata half goes with the
other `FieldDef` properties.

## Gate, and its negative control

A Decimal field with type `1` and expression `'USD'` renders `7,564.00`, produced by the transpiled
`Auto Format` codeunit.

**The negative control is a build with no AL app** -- `src/` must then be unable to produce that
string, which is what proves no type value was hardcoded.

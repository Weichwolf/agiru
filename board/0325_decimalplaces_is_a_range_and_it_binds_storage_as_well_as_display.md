Type:     task
Status:   open
Parent:   0068
Area:     net, gen, db
Source:   developer/properties/devenv-decimalplaces-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `DecimalPlaces` is a range, and it binds storage as well as display

> Sets **display and storage** requirements for the Decimal Data Type.

The value is a MINIMUM and a MAXIMUM, and the syntax carries four shapes:

| written | means |
|---|---|
| `1` | exactly 1 |
| `1:4` | at least 1, at most 4 |
| `2:` | at least 2, no ceiling |
| `:2` | no floor, at most 2 |

> The default storage requirements for Decimal are **two decimal places for amounts**. Use this
> property to specify storage requirements that are different than the default.
>
> The maximum number of decimal places that can be specified is **18**. If you set a maximum greater
> than 18, the digits following the 18th decimal place will be ignored.

**The 18 is a documented limit and therefore a `static_assert`** (board:0081), and it is not
`agiru::Decimal`'s own limit -- .NET `decimal` carries a scale up to 28, so a field may declare less
than the type can hold and never more.

**And "storage" is a schema decision.** The column's scale follows the declared maximum, which makes
this a `numeric(p,s)` question and not only a rendering one. Getting it wrong is cheap today and a
migration over 1 609 tables later.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`DecimalPlaces =`: **10 577 declarations** -- the largest population of any property in this sweep so
far, and three times `MinValue`'s.

## The IST-state

Not in `FieldDef` (`include/meta/TableDef.h:67`). The Decimal field's SQL column carries whatever
board:0004's schema writer chooses, with no per-field scale.

## The choice

A min/max pair on `FieldDef`, parsed by the generator from the four written shapes -- `constexpr`,
so `1:4` is two numbers in `.rodata` and never a string split at run time. The absent half is
`std::nullopt` and not a sentinel, because `:2` and `0:2` are different declarations.

Two consumers, and they are separate work: the column scale (`db`) and the rendered text (`net`,
board:0066). The metadata is one item; that is this one.

## Ordering

Early. 10 577 declarations and a schema consequence mean the metadata should exist before the schema
writer is settled, not after.

## Gate, and its negative control

`DecimalPlaces = 2:5` renders `1.5` as `1.50` and `1.234567` as `1.23457`; `DecimalPlaces = 19`
fails to compile.

**The negative control is the 19** -- a reader that accepts it and truncates at run time turns a
documented limit into a silent rounding.

Type: root
State: active
Area: rt
Tags: semantics

# A validated Decimal keeps the places its field declares

**Finding (2026-09-10, chain 93).** `SCM Whse. UOM Rnding. UT` fails 9 cases with "The quantity per
unit of measure 6.2857142857142857142857142876 for item GU00000010 does not align with the quantity
rounding precision 0.1428571428571428": the test validates `1 / RandIntInRange(2, 10)` into `Item
Unit of Measure."Qty. Rounding Precision"`, whose field declares `DecimalPlaces = 0 : 5`, then
multiplies and expects `QtyPerUoM mod Precision = 0` -- which only holds when the precision was
STORED with five places (0.14286 x 44 mod 0.14286 = 0) and never with twenty-eight.

**Reference.** `properties/devenv-decimalplaces-property.md`: "Sets display and **storage**
requirements for the Decimal data type ... This setting is **evaluated on text boxes and fields
during validation**." The predecessor read the property, carried it, and used it in one place --
a TestPage's value comparison -- and left the storage half as an open question (openerp
WI-1320: "assignment or validation?"). The documentation's "during validation" is the answer.

**Choice.** `Record.Validate(Field, Value)` rounds a Decimal value to the declared MAXIMUM before
`OnValidate` runs (`detail::DeclaredPlaces`, `src/rt/Table.cpp`); a direct assignment does not,
and a field without the property is not rounded. The gate is `EntryRangeGate`
(`AValidatedDecimalKeepsTheDeclaredPlaces`): `0 : 5`, `2:5`, `2`, `:3`, `2:` and none.

**Not done.** `AutoFormatType` alone (Amount = 2 places, UnitAmount = 5 -- the predecessor's
derivation) is not applied; the count of fields that declare only `AutoFormatType` is the
population to measure before it is. The column itself stays `numeric(38,20)`.

**Measurement.** Chain 95 A/B against chain 93 (1 518); an activation, so a loss names the cases.

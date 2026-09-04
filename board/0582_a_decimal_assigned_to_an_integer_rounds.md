Type:     bug
Status:   open
Area:     rt, gen
Source:   ~/Git/BCApps/src/Layers/W1/Tests/TestLibraries/LibraryRandom.Codeunit.al:32-36
Verdict:  fehlt
Class:    silent-wrong-data

# A `Decimal` assigned to an `Integer` rounds, and the assignment compiles

`Library - Random` writes `Pow: Integer; Pow := Power(10, Precision)` and
`exit(RandInt(Range * Power(10, Decimals)) / Power(10, Decimals))` -- a `Decimal` into an `Integer`
variable and into an `Integer` parameter, with no `Round`. BC compiles and runs it, so AL converts
`Decimal` to `Integer` implicitly. The platform documentation does not say so; the source declares.

## Measured 2026-09-04

An implicit `Decimal::operator std::int32_t()` was tried and TAKEN BACK the same hour: with
`Decimal(std::int64_t)` already implicit, `Decimal / int` and `Decimal * int` became AMBIGUOUS
(`LibraryUtility.cpp:175`, `:179`) -- two user-defined conversions in opposite directions.

## The choice

Not a conversion operator. Two routes remain and the first is the honest one:

- **The generator inserts the conversion where the DECLARED target is `Integer`**: an assignment or
  an argument whose declared type is `Integer` and whose expression is a `Decimal` gets
  `::agiru::Round(...)` and an explicit narrowing. That needs the expression's type, which the
  generator knows for a variable and a builtin return (`Power` returns `Decimal`) and not for an
  arbitrary expression -- so the first cut covers those two shapes and REFUSES the rest.
- **`Decimal` gains `operator/(const Decimal &, std::int64_t)` and the three others**, which makes the
  conversion operator unambiguous. It is the cheaper edit and the worse design: every arithmetic
  overload set doubles, and the ambiguity was the type system saying two implicit directions are one
  too many.

## What the rounding is

`Round(number)` with no precision -- to the nearest whole number, halves away from zero -- which is
`Decimal.Round` and what `platform/Numeric.cpp` already implements. Measured against C/SIDE's
behaviour before it is wired: the predecessor's board has the case.

## Gate

`Pow := Power(10, 2)` into an Integer yields 100; `X := 2.5` into an Integer yields 3 and `-2.5`
yields -3. The negative control is `2.4` yielding 2 -- truncation passes the first two and fails it.

# AL `mod` over decimals is a decimal remainder

**Finding (2026-09-09).** `ItemUoM."Qty. per Unit of Measure" mod BaseRoundingPrecision` in table
5404 is emitted as `%`, `Decimal` had no `operator%`, and the call compiled through the door's
`operator std::int32_t()`: `0.00001` became `0` and the process died of SIGFPE. `SCM Whse. UOM
Rnding. UT` was LOST whole (exit 136, 19 cases out of the denominator, chain 65).

**Reference.** `devenv-al-simple-statements.md` / operators: `mod` takes Integer and Decimal
operands and yields the remainder; .NET `decimal` `%` is the truncated-quotient remainder with
the dividend's sign, and `Decimal` here is that type.

**Choice.** `Decimal::operator%=` (`|a| - trunc(|a|/|b|)*|b|`, signed like `a`, refusing a zero
divisor the way `/` does) with the mixed integral overloads the other operators carry so the
Integer conversion cannot compete. Gate cases in `DecimalGate`. Silent-wrong-data turned crash.

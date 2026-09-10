Type: root
State: open
Area: rt
Tags: invariant

# A Decimal filter compares as a Decimal and not as a double

**Finding (2026-09-10).** `src/rt/Filter.cpp` (`Compare`, the `FieldType::Decimal` branch) turns
both sides of a filter comparison into `double` with `std::stod` before ordering them. Above
2^53 the two sides can differ in the Decimal and agree in the double, so a range filter on a
large amount admits or refuses a row wrongly -- silently. CLAUDE.md: NO BINARY FLOATING-POINT
TYPE CARRIES AN AMOUNT.

**Choice.** Compare through `Decimal::FromInvariantString` and `Decimal`'s own ordering; Integer
and BigInteger through `std::int64_t`; an Option or Enum by its ordinal. The gate is a filter on
`10000000000000001` against `10000000000000000` that a double calls equal.

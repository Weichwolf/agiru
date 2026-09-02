Type: leaf
State: open
Area: net
Tags: semantics, measured

# A Duration is its own type, or the alias keeps costing what it just cost

`Duration` and `BigInteger` are both `using X = std::int64_t` in the door. That was a deliberate
choice with a stated reason -- generated AL code does arithmetic on both constantly, and a wrapper
would either forward every operator or change how that code reads -- and it held until `Variant`.

## What it cost

A Variant holds one AL type and answers which. `variant-data-type.md` lists `IsDuration()` and
`IsBigInteger()` as two separate questions, because in AL they are two types: a Duration is a count
of MILLISECONDS, comes out of subtracting two DateTimes, and renders as `2 days 3 hours`. In C++ the
alias makes them one type, `std::variant` refuses the duplicate outright, and the Variant now has
`IsBigInteger()` and no `IsDuration()`.

Answering one of the two wrongly would be worse than answering neither, so the type is left out.
That is a hole in a documented surface, and it will be a hole in every place that reaches a value
through a Variant.

## What the documentation says

`duration-data-type.md`: a Duration "denotes the difference between two DateTime or Time values" and
"is measured in milliseconds". `system-format-joker-integer-string-method.md` gives it a rendering
of its own. Neither is true of a BigInteger, and neither is reachable while the two are one type.

## The choice

- **Duration becomes a class**, carrying an `std::int64_t` and the operators AL uses on one:
  `+`, `-` against another Duration, `*` and `/` against a number, comparison, and the DateTime
  subtraction that produces it. Fifteen lines, and the alias's argument -- that arithmetic stays
  readable -- survives, because those are exactly the operators it would carry.
- **Or the alias stays** and the Variant is documented as unable to hold a Duration, permanently.

The first is almost certainly right; what stops it being done in the same breath is that `Duration`
is already spelled across the door and the generated tree, and the change wants its own measurement
of what it touches rather than being folded into a type it was found by.

## The benchmark

The count of Duration-typed fields, variables and parameters in the BaseApp, and whether `Format`
of one differs from `Format` of a BigInteger -- which is the observable the change buys.

## Closed when

`IsDuration()` and `IsBigInteger()` are two answers over the same Variant, and a Duration renders
as the documentation says a Duration renders.

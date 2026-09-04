Type: bug
State: open
Area: net

# A number that leaves its documented range raises, and does not wrap

`biginteger-data-type.md` states the range and then states what happens outside it, and the second
sentence is the one with no implementation:

> Stores very large whole numbers that range from -9,223,372,036,854,775,807 to
> 9,223,372,036,854,775,807. ... If you try to indirectly assign a value that is smaller than
> -9,223,372,036,854,775,808, or larger than 9,223,372,036,854,775,807, then you get a **run-time
> error**.

`include/type/BigInteger.h` carries the range faithfully -- `kMinimum = -9223372036854775807LL`,
symmetric, which is NOT `INT64_MIN` and is exactly what the page says. And the type is
`using BigInteger = std::int64_t`, so the arithmetic that reaches the bound **wraps silently**
(signed overflow is undefined behaviour, which is worse than wrapping: `-Werror` does not see it and
neither does anything else).

**In an accounting system a wrapped sum is the failure mode with no symptom.** CLAUDE.md's first
invariant is that a posting is all or nothing; a quantity that silently becomes negative at the
boundary is neither.

## What the sweep has verified so far

| type | documented bound | in the door | raises? |
|---|---|---|---|
| BigInteger | symmetric 64-bit, run-time error outside | `BigIntegerRange`, `int64_t` | **no** |
| Byte | 0..255 | `std::uint8_t` | **no** -- and the page's char/number duality is absent too |
| Integer, Decimal, Char | read as the sweep reaches them (board:0071) | | |

`agiru::Decimal` is the exception in the making: it is a real class rather than an alias, so it has
somewhere to put the check, and board:0008 already touches its arithmetic.

## AND BC CATCHES IT TWICE, WHILE AGIRU CATCHES IT NEITHER TIME

`diagnostics/diagnostic-al662.md` and `-al663.md` are compiler warnings:

> Implicit conversion from BigInteger '{0}' to {1} '{2}' in property expression **may overflow at
> runtime**. Consider changing the target field type to BigInteger.
>
> Implicit conversion from BigInteger '{0}' to Enum '{1}' in property expression may overflow at
> runtime **and can lead to unexpected enum value behavior**.

So the platform warns when the source is WRITTEN and raises when the value is ASSIGNED. This tree
has the ranges as constants and neither check, which means a value that BC refuses twice is written
silently here.

The compile-time half is free in C++ for the cases the transpiler can see -- a literal outside the
range is a `static_assert` beside the field, not a test case (CLAUDE.md) -- and it is the same
mechanism board:0068 needs for `MinValue`/`MaxValue`.

## The choice

- **A checked assignment belongs to the FIELD, not to every arithmetic operator.** AL's error is on
  assignment, and a table field already runs through `Validate`; that is where the bound is tested,
  beside `MinValue`/`MaxValue` (board:0068), and it costs nothing on a local variable.
- **The alias types cannot carry it and that is the trade.** `using BigInteger = std::int64_t` buys
  the whole of C++'s arithmetic for free; wrapping it in a class to catch a bound the BaseApp reaches
  almost never would cost every arithmetic expression in 7 885 translation units. **So the bound is
  checked where a value CROSSES A BOUNDARY** -- assignment to a field, a `Validate`, a read from the
  database, a `Evaluate` -- and not in the operators.
- **Where it is not checked, the header says so.** A range constant that nothing enforces reads as a
  guarantee, and that is the state this item found.

## Gate

A field of each numeric type refuses a value outside its documented range with BC's wording, and
accepts the boundary value itself. `Evaluate` of a literal past the bound refuses rather than
wrapping.

**Negative control**: remove the check and require the boundary case to go red -- and assert the
BOUNDARY value passes, so a check that refuses everything cannot pass the gate.

## `Decimal` HAS FOUR LIMITS AND ONLY THE WIDEST IS IMPLEMENTED, read 2026-09-04 (board:0071)

`methods-auto/decimal/decimal-data-type.md` tabulates them, and they are four different numbers for
four different ROLES:

| limit | value | reached by |
|---|---|---|
| maximum **format** value | **±999,999,999,999,999.99** | `Format`, UI and XmlPort input, a literal in source |
| maximum **field** value | **±999,999,999,999,999.99** | a record's field, even unpersisted |
| maximum **persisted** value | reads a wider previously stored value, but **cannot store one**, "since field variables cannot be assigned values outside the formatting range" | the database |
| maximum **calculating** value | **±79,228,162,514,264,337,593,543,950,335** (2^96 - 1), scale up to **28** | an intermediate that is not assigned to a field, stored, or formatted |

**`agiru::Decimal` implements the fourth exactly** -- `U128 units_`, `kMaxUnits`, scale to 28 -- and
`include/type/Decimal.h:27` already quotes the page's own "maximum calculating value" wording. That
row is *implementiert* and the door names its source.

**The other three do not exist.** Assigning 10^18 to a Decimal FIELD succeeds here and raises in BC,
and the difference is invisible until a posting writes a number no report can format. This is the
same shape as the Integer and Option rows above: the calculation type is wider than the field, and
the narrowing happens on ASSIGNMENT TO THE FIELD -- which is where this item already puts the check.

**The schema agrees with BC and that is worth recording rather than re-deriving.**
`fieldtype-option.md` gives the column as `DECIMAL(38,20)` -- 18 integer digits and 20 fractional,
"the size ... is 17 bytes" -- and `src/rt/Storage.cpp:76` emits `numeric(38,20)`. The two match, so
the persisted limit is enforced by the column and the FIELD limit is the one the runtime owes.

The same page also settles a wording question this tree could have got wrong from
`fieldtype-option.md` alone: that page says a field Decimal is "held in memory with 18 significant
digits ... Binary Coded Decimal", which describes the FIELD and the COLUMN, not the AL variable. The
variable is the CLR `Decimal`, which is what CLAUDE.md's invariant is written about. **Both pages are
right about different roles**, and reading only one of them would have narrowed `agiru::Decimal` to
18 digits.

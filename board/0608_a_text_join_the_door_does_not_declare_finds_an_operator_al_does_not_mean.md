Type:     finding
Status:   open
Area:     net, rt
Source:   the Guid conversion that `guid-data-type.md` documents, 2026-09-07
Class:    silent-wrong-data

# A text join the door does not declare finds an operator AL does not mean

**`TableCaption() + ' ' + FieldCaption(No)` WROTE A NULL GUID, and nothing said so.**

```
Family {00000000-0000-0000-0000-000000000000}
```

is what `ProdOrderWarehouseMgt` put in `CaptionClass` where AL puts `Family No.` -- measured
2026-09-07 by running the join against the door as it stood.

## Why

`Record.TableCaption()` returns a `std::string` and `Record.FieldCaption()` a `std::string_view`.
The standard library joins `string + string` and `string + const char *` and does NOT join
`string + string_view`. The door did not join it either -- so unqualified lookup inside
`namespace agiru::...`, where every generated body lives, went on to the next candidate and found

```cpp
std::string operator+(const ::agiru::Text<0> &left, const Guid &right);
```

which is viable at one user-defined conversion per side: `std::string` -> `Text<0>` on the left,
`std::string_view` -> `Guid` on the right. `Guid::FromText` returns the NULL GUID for text that does
not spell one, so the right side became `{00000000-…}` and the join succeeded.

**IT COMPILED, IT RAN, AND IT RETURNED A STRING.** No warning, no throw, no failing gate -- the
signature of the whole class of defect this tree left Python for.

## What is done

- **The join AL means is declared**: `operator+` over two text-like class types neither of which is
  a `Text[N]` or `Code[N]`, in `include/type/StringValue.h`. The operator AL means has to exist, or
  overload resolution finds one AL does not mean.
- **The Guid operators take no conversion.** `operator+(Text<0>, Guid)` and `operator+(Guid, Text)`
  are constrained with `std::same_as<G, Guid>`, so nothing reaches them by converting. This is what
  turned the defect into 33 compile errors, which is how it was found at all.
- **`Text == Guid` is spelled out**, comparing AS GUIDS rather than as strings, which is what
  `guid-data-type.md` describes and what `AADApplicationCard` depends on -- it strips the brackets
  and lower-cases before comparing.

## The population, and what it says about the rest of the door

33 call sites in the slice joined a `std::string` with a `std::string_view`; every one of them was
wrong in the same way, and every one is a caption. The wider lesson is a rule rather than a fix:

**A DOOR OPERATOR THAT TAKES A TYPE WITH IMPLICIT CONVERSIONS IS A CATCH-ALL.** `Guid` is
constructible from text because AL assigns text to a Guid; that same constructor makes every
Guid operator a candidate for every pair of text-shaped operands the door has not covered. The
guard is the one applied here: an operator written for one type says `std::same_as` for that type,
and the operators AL actually means are declared rather than left to be found.

**What is not yet swept:** the same shape can exist for `Decimal`, `DateTime`, `Date`,
`DateFormula` and `RecordId` -- every door type with an implicit constructor from text or from a
number, and an operator taking it. Each is a `static_assert` or a gate case away from being ruled
out, and none has been checked.

## What proves it

`test/gate/GuidGate.cpp` pins the assignment and the comparison the documentation states;
`test/gate/TextBuiltinGate.cpp` pins that a caption joined to a caption is the two captions. The
negative control is the finding itself: with the constraint removed, the join compiles again and
the gate goes red with a GUID in the message.

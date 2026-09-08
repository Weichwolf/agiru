Type:     task
Status:   open
Area:     rt, gen
Source:   the two unlinked sources at the top of the UT ranking, 2026-09-08
Class:    activation

# A builtin returns the AL type, and a Variant reaches a `var` parameter

**THE TOP TWO ITEMS OF THE UT RANKING ARE NOT MISSING DECLARATIONS. THEY ARE TWO SOURCES THAT DO
NOT COMPILE**, and between them they hold 511 of the 1 427 remaining failures (measured 2026-09-08):

| failures | the unlinked procedure | what stops its source |
|---|---|---|
| 267 | `TypeHelper.GetUserTimezoneOffset` | a `Variant` handed to a `var Date` parameter |
| 244 | `GraphMgtGeneralTools.IsApiEnabled` | `Format(Guid).TrimStart('{')` |

## `Format` RETURNS `Text` IN AL AND `std::string` HERE

```al
exit(Format(Id).TrimStart('{').TrimEnd('}'));
```

`TrimStart` is a method of AL's `Text`, and `Format` is documented to return one. Here `Format`
answers a `std::string`, which has no such member -- so a body that chains a text method onto a
builtin's result does not compile.

**IT IS THE NAME-EQUALITY INVARIANT WITH A TYPE INSTEAD OF A NAME.** CLAUDE.md's rule is that a
reader who knows AL must be able to write the next file; a builtin whose declared return type is
not AL's breaks that for every method of the type it should have returned.

**THE CHANGE IS `Format` -> `::agiru::Text<0>`, and its blast radius is the whole door**: `Format`
is called from `StrSubstNo`, from every error message, and from the generated tree. `Text<0>`
reaches `std::string_view` implicitly, so a site that reads the result keeps working; a site that
ASSIGNS it to a `std::string` does not. The population is a `grep` and the measurement is one full
build -- and the same question stands for every other builtin the documentation types as `Text`.

## A VARIANT HANDED TO A `var` TYPED PARAMETER

```al
TryEvaluateDate(String, Format, CultureName, Variable)   // Variable is a Variant, the parameter a var Date
```

AL unwraps the `Any` into the typed `var` and writes the result BACK into the Variant. C++ cannot:
`Variant::operator T()` returns a value and a value does not bind to `Date &`.

**THE THREE SHAPES, and none is free:**

1. **The generator emits a temporary and a write-back** around the call: `Date tmp = V; f(tmp);
   V = tmp;`. It is what AL does, it needs no door change, and it needs the generator to know the
   parameter is `var` and typed -- which it does, because it wrote the signature.
2. **A proxy the Variant hands out**, holding a reference back. It makes `Variant` a class with
   lifetime rules and puts the write-back in the door rather than at the call site.
3. **The parameter takes a `Variant &`**, which is what the callee's own AL declares it is NOT.

**1 IS THE ONE TO MEASURE.** It is local to the call site, it matches AL's own semantics exactly,
and a wrong write-back is visible in the generated text rather than hidden in a conversion.

## What proves it

Both sources compile and enter `test/slice`; `agiru run-tests` stops reporting
`the AL procedure ... is declared and its source is not in the slice` for those two, and the UT
count rises by what those 511 failures were hiding.

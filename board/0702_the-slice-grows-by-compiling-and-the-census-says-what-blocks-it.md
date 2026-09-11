# 0702 The slice grows by compiling, and the census says what blocks it

**The measurement.** 482 generated sources were absent from `test/slice` on 2026-09-11. Compiling
every one of them with `-fsyntax-only` and grouping the FIRST diagnostic is one sweep (six workers,
about four minutes) and it ranks the generator's remaining gaps by how many units each keeps out:

| first error | units |
|---|---|
| `no member named 'X' in 'X'` | 105 |
| `non-const lvalue reference to type 'X' cannot bind to a value of unrelated type` | 47 |
| `no matching member function for call to 'X'` | 39 |
| `reference to non-static member function must be called` | 54 -> **31** |
| `use of overloaded operator 'X' is ambiguous` | 27 |
| `no matching function for call to 'X'` | 27 |

**Two of that fourth class are fixed here, and they are one AL rule:** a parameterless method is
written WITHOUT parentheses in AL ("This method can be invoked using property access syntax", which
the documentation states on the method's own page). The generator appends `()` only when it can see
the receiver, and it could not see two shapes:

- **an ARRAY ELEMENT** -- `At(SalesHeader, Index).RecordId` -- because the receiver is an index
  expression and not a name. It asks the ARRAY's declaration now, which is the same declaration.
- **a bare FIELD of the dataitem's record** -- `"Work Description".HasValue`, which resolves to
  `Rec.WorkDescription` -- because the receiver named no variable at all. A receiver that is a
  field of `Rec` now answers the door the same way a variable does.

**24 units joined the slice** (13 975 -> 13 999), and the suite measured 1 788 -> 1 788: the cases
those units carry are blocked further along. **That is the expected shape of this work** -- a unit
in the slice is a precondition for its cases, not a cause of them -- and it is why the census is
worth keeping: it is the only ranking that does not depend on which test happens to reach a file
first.

**What the census says to do next, in order:** the `std::string` half of the door (board:0695 --
`IndexOf`, `TrimEnd`, `Split` called on what a door method returned, 11 units) is the largest
single named cause inside class one, and it is the name-equality invariant failing: AL `Text` is
`::agiru::Text<0>` and never `std::string`. 169 door declarations return `std::string` today.

**Second census, 2026-09-11 evening, after board:0703's round (the array view, the property
assignment through a chain, the un-hidden base overloads, `Rec.` before a page's own table
procedures, `Option := Variant`, `SetFilter` with a value, `Text` out of `GetFilter`/`ToText`/
`AsText`):** 458 absent, **114 compile and joined the slice** (13 999 -> 14 113). What is left:

| first error | units |
|---|---|
| `no member named 'X' in 'X'` | 105 -> 96 |
| `reference to non-static member function must be called` | 31 -> 25 |
| `no matching function for call to 'X'` | 27 -> 25 |
| `use of undeclared identifier 'X'` | 26 -> 23 |
| `no type named 'X' in 'X'` (all `Page<>::SetBackgroundTaskResult`) | 21 |
| `no matching member function for call to 'X'` | 39 -> 19 |
| `non-const lvalue reference ... unrelated type` | 47 -> 12 |

Suite: 1 788 -> 1 800.

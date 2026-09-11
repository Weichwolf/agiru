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

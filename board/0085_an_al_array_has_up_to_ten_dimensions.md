Type: root
State: open
Area: gen, net

# An AL array has up to TEN dimensions, and `A[i,j]` does not compile today

`developer/methods/devenv-array-methods.md` -- one of seven AL LANGUAGE pages that were never in
the sweep's denominator (board:0071) -- specifies the type:

> The maximum number of dimensions is **10** and the total number of elements in all dimensions is
> **1,000,000**. ... For a dimension of length N, indices can range from **1 to N** inclusive.

```al
arrayOfInteger: array [10] of Integer;
arrayOfCodeunits: array [10] of Codeunit 10;
Array2: array[3,4] of Integer;
```

## What agiru does, and where it stops

The parser reads every dimension -- `ReadDimensions` pushes each integer into `declared.dimensions`
(`src/al/Parser.cpp:485`) -- and the generator nests them correctly: `TypeOf` wraps from the inside
out, so `array[3,4] of Integer` becomes `AlArray<AlArray<Integer, 4>, 3>`
(`src/gen/CodeunitWriter.cpp:204`). The declaration side is right.

**The USE side has one index.** `A[i,j]` is emitted as `At(A, i, j)` (`src/gen/BodyWriter.cpp:673`,
which already writes every index it parsed) and the door declares

```cpp
template <typename Container, typename Index>
[[nodiscard]] decltype(auto) At(Container &container, Index index);
```

-- exactly two parameters (`include/type/AlArray.h:86`). **So every read or write of a
multi-dimensional array fails to compile**, at 358 declaration sites (measured 2026-09-04, `grep -E
"array *\[[0-9]+ *, *[0-9, ]+\]"` over `~/Git/BCApps/src`, against 32 239 single-dimensional ones).

That is the good direction for a defect -- `make apps` stops rather than a wrong number appearing --
but it is a HOLE with a count, which board:0034 requires to be recorded as one.

## `ArrayLen` ANSWERS THE WRONG QUESTION ON A NESTED ARRAY

`system-arraylen-method.md`:

> Returns the **total** number of elements in an array **or** the number of elements in a specific
> dimension. ... `ArrayLen(Array: Array of [Any] [, Dimension: Integer])`

and its own example spells out the answers for `Array2: array[3,4] of Integer`: `ArrayLen(Array2,1)`
is 3, `ArrayLen(Array2,2)` is 4, and **`ArrayLen(Array2)` is 12**.

agiru's `ArrayLen(const AlArray<T, N> &)` returns `N`, which for the nested type is the OUTER
dimension -- **3 where AL says 12** -- and the `Dimension` overload does not exist at all. The
single-dimensional case is right, which is why this would survive any gate written from the
one-dimensional 32 239.

**And the door's own note points at a declaration that is not there.** `AlArray.h:65` says the
constrained overload "shadows the door's refusing `ArrayLen(Any)`"; `grep` finds no `ArrayLen` in
`include/Builtins.h` (measured 2026-09-04). So `ArrayLen` on a genuine `Variant` -- which is what
the page's signature is -- resolves to nothing.

## THE TWO ARRAY BUILTINS HAVE THE WRONG SHAPE, not merely an empty body

```cpp
::agiru::Integer CompressArray(const ::agiru::Variant &StringArray);
void CopyArray(const ::agiru::Variant &NewArray, const ::agiru::Variant &Array,
               ::agiru::Integer Position, ::agiru::Integer Length = {});
```

Both MUTATE what they are given -- `CompressArray` "moves all non-empty strings to the beginning of
the array" in place, `CopyArray` writes into `NewArray` -- and both take it by `const` reference.
**That is CLAUDE.md's first tabulated trap, "an out parameter never written", declared rather than
written**: the guard the table names is that "`var` is a reference and the compiler checks it --
closed in C++, provided the generator never copies", and here the door itself dropped the reference.
Filling in the body later would produce a builtin that runs, returns a count, and changes nothing.

The pages also carry three refusals that are behaviour rather than omission:

- **`CompressArray` is not supported on multidimensional arrays** -- "In earlier versions,
  CompressArray works for arrays of arrays." A refusal, not a silent pass.
- **`CompressArray` takes Text or Code and NOT BigText.**
- **`CopyArray` copies only from one-dimensional arrays, and not from arrays of a complex type.**
  Its `Length` is bounded: `1 <= Length <= MAXLEN(Array) - Position + 1`.

And `ArrayLen` "with an input parameter that is not an array" is a run-time error.

## AN ARRAY OF TEMPORARY RECORDS SHARES ONE TABLE

The same page, and it is the finding no signature could carry:

> each element of the array contains a temporary Item record **referencing the same temporary
> table**, meaning that an insert into `itemRecArrayTemp[0]` is also reflected in
> `itemRecArrayTemp[1]`. This is the same behavior as using `Copy(RecordRef [, Boolean])` with the
> **ShareTable** parameter set to `true`.

`AlArray<T, N>` holds `std::array<T, N> held_{}` -- N independent `Temporary<T>` objects, each with
its own rows. So an AL routine that fills `A[1]` and reads `A[2]` expecting the same rows gets an
empty record and no error. **silent-wrong-data**, and it is the same object identity question
board:0078 asks about `List` and `Dictionary`: AL's collection semantics are by reference and C++'s
containers are by value.

## The choice

- **`At` becomes variadic and recurses**: `At(c, i, rest...)` returns `At(c[i], rest...)`, with the
  one-index case as the base. Four lines, and it is the only change the 358 sites need, because the
  declaration side is already nested correctly.
- **`ArrayLen` gains the two documented answers.** `Depth<T>` and `Total<T>` are `constexpr`
  recursions over the nesting, so `ArrayLen(A)` is `Total` and `ArrayLen(A, n)` walks `n` levels in.
  Both are compile-time constants; neither costs a run-time branch. The `Dimension` argument is
  checked against `Depth` with a `static_assert` where it is a literal -- which it is at every call
  site in the corpus -- and a refusal where it is not.
- **`CompressArray` and `CopyArray` take non-const references** and are constrained to `AlArray`
  rather than to `Variant`, so the multidimensional and complex-type refusals become
  `static_assert`s rather than run-time checks. That is the whole of the "wrong shape" finding: the
  page's restrictions are all on TYPES, and a type is what the door was not using.
- **The two documented BOUNDS are `static_assert`s in `AlArray` itself**: `N >= 1`, the nesting
  depth at most 10, and the product of the dimensions at most 1 000 000 (board:0081).
- **The shared temporary table is deferred to board:0078's answer** and named here rather than
  guessed, because whatever makes `Temporary<T>` shareable makes this row free, and anything else
  makes it a special case.

## Gate, and its negative control

`Array2: array[3,4] of Integer` filled through `A[i,j]` and read back, plus the page's own three
`ArrayLen` answers -- 3, 4 and **12** -- which come from the specification and cannot be
back-filled. `CompressArray` over `"Redmond", "Copenhagen", "", "Fargo", "Paris"` must give the
page's own result with the empty string LAST and the order otherwise preserved.

**The negative control is `ArrayLen(Array2)`**: it returns 3 today and a gate that only checks
`ArrayLen(Array2,1)` passes over the defect. A second control gives `CompressArray` a
multidimensional array and requires the build to fail.

Classification: **activation** for the indexing (nothing compiles today, so nothing regresses) and
**silent-wrong-data** for `ArrayLen` and for the shared temporary table.

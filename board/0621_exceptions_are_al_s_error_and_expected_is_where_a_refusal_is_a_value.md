Type:     root
Status:   open
Area:     rt, gen
Source:   the question whether `-fno-exceptions` is reachable, 2026-09-08
Class:    measurement

# Exceptions ARE AL's `Error`, and `expected` is where a refusal is a VALUE

**THE QUESTION WAS WHETHER THIS TREE NEEDS EXCEPTIONS AT ALL, and the population answers it.**
Measured 2026-09-08 over the transpiled tree and `~/Git/BCApps/src`:

| | |
|---|---|
| `Error()` raised from a generated body | **7 126** |
| `asserterror` statements in AL, each of which CATCHES one and continues | **15 840** |
| AL files declaring a `[TryFunction]`, which catches and returns `false` | **452** |
| call sites in generated bodies that would each need to propagate a result | **620 206** |
| `throw` in the door and the runtime | 519 |
| `catch` in `src/` | 16 |

**AN AL `Error` UNWINDS, AND THAT IS THE LANGUAGE RATHER THAN AN IMPLEMENTATION CHOICE.** It aborts
the call chain and rolls back to the enclosing boundary -- through `Insert`, the table's `OnInsert`,
a subscriber, a nested codeunit, a validate trigger -- and none of those frames mentions the error.
`asserterror` and `[TryFunction]` are the two places AL stops it, and both are `catch`.

**AND THE NAME-EQUALITY INVARIANT IS THE SECOND HALF.** `Rec.Insert()` is a STATEMENT in AL. With
`-fno-exceptions` every one of those 620 206 call sites becomes a propagation site -- `TRY(...)`, an
`if` on an `expected`, or a macro -- and the generated file stops reading like the `.al` file it
came from. CLAUDE.md's reason for that invariant is the reader: "a reader who knows AL and has never
seen agiru must open one file and know how to write the next."

**SO EXCEPTIONS STAY, and they are not the exception path -- they are the AL path.**

## What the question is RIGHT about, and it is a real gap

`std::expected` appears **0 times** in this tree, and CLAUDE.md names it outright:
"`std::expected` where a refusal carries its reason". `[[nodiscard]]` is at 762 and `static_assert`
at 16 721, so two of the three are pulling their weight and one is not there at all.

**WHERE IT BELONGS IS WHERE A FAILURE IS A VALUE THE CALLER DECIDES ABOUT, not where AL unwinds:**

- **the AL parser** (`src/al`) -- a file that does not parse is a diagnostic with a position, and
  the caller decides whether to stop;
- **the database layer** (`src/db`) -- a statement that fails carries the server's message, and the
  transaction layer above it decides between a refusal and an AL `Error`;
- **the readers that AL itself gives a boolean to**: `Evaluate`, `Guid::FromText`,
  `DateFormula::FromText`, the filter parser. Those return `false` today and throw away the REASON,
  which is exactly what `expected` is for -- and `Evaluate`'s own AL contract already says the
  caller decides.

Each of those is inside `src/` and reaches no generated file, so none of them touches the invariant.

## What proves it

`std::expected` rises from 0 with each of the three groups, and the door's `throw` count does not:
a refusal that AL asks about by value stops being a `throw` that something has to catch.

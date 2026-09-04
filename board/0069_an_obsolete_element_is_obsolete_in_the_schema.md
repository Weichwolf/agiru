Type: bug
State: open
Area: gen, db

# A field marked `Removed` cannot be referenced, and `Pending` is not an error

**THE TITLE OF THIS ITEM WAS WRONG AND IS CORRECTED.** It first read "a field marked `Removed` is
not a column" and proposed that the generator emit nothing for it -- no member, no field-table
entry, **no column**. `devenv-obsolete-objects.md`, read 2026-09-04 (board:0071), says the opposite
in as many words:

> An important thing to note here is that **you don't comment out the code for the obsolete
> objects. Instead, you should keep the code in place so that you don't break any dependencies**,
> but instead you mark it as obsolete.

and `devenv-obsoletestate-property.md` explains why tables and fields are the exception that may
carry `Removed` at all: every OTHER element is simply deleted once it has been `Pending` long
enough, and a table or a field **cannot be deleted because it holds DATA**.

So dropping the column would have been a data-loss bug proposed as a fix. What `Removed` changes is
the COMPILER, not the schema: "the AL compiler will return warnings for references to elements
marked as **Pending** and **errors** for references to elements marked as **Removed**."

The generator reads seven properties (board:0067) and `ObsoleteState` is not one of them. So a field
BC has removed still gets a member, a field-table entry and a COLUMN.

Measured over `Layers/W1`, 2026-09-04:

| `ObsoleteState` | declarations |
|---|---:|
| `Pending` | 659 (+4 spelled `pending` or without the space) |
| **`Removed`** | **172** |
| `Moved` | 8 |
| `TableMetadata` and one-off spellings | 15 |

`ObsoleteTag` 1 004 and `ObsoleteReason` 977 stand beside them, and `[Obsolete(...)]` marks 629
procedures.

## What the platform documents

`properties/devenv-obsoletestate-property.md`. Two sentences decide this:

- **"For all elements, except for Tables and Table fields, setting `ObsoleteState = Removed` will
  throw Compiler Error AL0169"** -- so `Removed` is legal in exactly the two places it appears here,
  and it means the element is gone rather than deprecated.
- **"the AL compiler ... will return warnings for references to elements marked as Pending and
  errors for references to elements marked as Removed."** A reference to a Removed field does not
  compile in AL. Here it does, silently, because the property never left the token list.

The full option set is `Moved`, `No`, `Pending`, `PendingMove`, `Removed`; `Moved` and `PendingMove`
belong to moving fields between extensions, which is board:0033's subject.

## Why it is a defect and not tidiness

- **A reference to a `Removed` field COMPILES here and does not in AL.** That is the whole of it:
  the AL compiler makes it an error, `-Werror` has nothing to make an error of, and the reference
  reads a column BC has stopped maintaining. Whatever is in that column is whatever was last written
  to it, which is the plausible-wrong-value class rather than a crash.
- **`Pending` must NOT be an error**, here or anywhere: 659 fields carry it and the BaseApp still
  uses them. Treating the two states alike in either direction is wrong -- one is a warning, the
  other is a refusal.
- **The count is worth having on its own.** 659 `Pending` and 172 `Removed` under `Layers/W1` is how
  much of the BaseApp stands on deprecated surface, and it is a number nothing in the tree prints.

## The choice

- **`ObsoleteState` reaches `FieldDef` and the table declaration** as one `constexpr` enum, like
  every other property board:0067 will account for.
- **`Removed` KEEPS the column and the field-table entry, and REFUSES the reference.** The data
  stays -- that is the whole reason `Removed` exists for a field rather than deletion -- and a body
  that names one is a translation ERROR carrying the object, the field and the `ObsoleteTag`. That
  is the AL compiler's own behaviour, reproduced rather than invented.
- **The generated MEMBER goes, and the schema does not.** In C++ the two are separable: the field
  table and `CreateTable` read `FieldDef`, and the class member is what a body names. Emitting the
  descriptor without the member makes the reference a compile error for free and leaves the column
  where it is -- which is the AL rule expressed in the language rather than checked beside it.
- **`Pending` emits everything and is counted.** The count is the number that says how much of the
  BaseApp is standing on deprecated surface, which is worth knowing and is not worth refusing.
- **A `Removed` TABLE keeps its schema too**, for the same reason, and a body that names it fails to
  translate.

## Gate

A table with one `Pending` and one `Removed` field: `CreateTable` writes BOTH columns and the
`static_assert`ed field count counts both; the generated class has a member for the `Pending` one
and not for the `Removed` one; a body referring to the `Removed` field fails the translation naming
the field and its tag; a body referring to the `Pending` one translates and is counted.

**Negative control**: drop the `Removed` field's column and require a round trip over an existing
row to go red. That is the control this item needs most, because its first form proposed exactly
that drop.

## WHICH BASEAPP GETS TRANSLATED IS DECIDED BY THE `CLEAN` SYMBOLS, and the decision is recorded here

`developer/directives/devenv-directives-in-al.md` (read 2026-09-04 -- one of the five directive
pages that were never in board:0071's denominator) specifies the AL preprocessor: `#if`, `#else`,
`#elif`, `#endif`, `#define`, `#undef`, with `and` / `or` / `not`, symbols defined either at the top
of a file or in `app.json`'s `preprocessorSymbols`, and no built-in symbols.

Measured 2026-09-04 over `~/Git/BCApps/src`:

| directive | occurrences |
|---|---:|
| `#if` | **10 126**, of which **9 781 are `#if not <symbol>`** |
| `#pragma warning` | 27 138 |
| `#region` | 807 |
| `#pragma implicitwith` | 14 |
| **`#elif`, `#define`, `#undef`** | **0 each** |

and the symbols are five: `CLEAN27` (197), `CLEAN28` (82), `CLEAN29` (59), `CLEAN26` (5),
`CLEANSCHEMA28` (2). **They guard obsolete code**, which is what makes them this item's business:
`#if not CLEAN27 ... #endif` is code that survives until the cleanup for version 27 removes it.

**Where they are defined settles what agiru must do.** `~/Git/BCApps/.github/AL-Go-Settings.json`
declares them under `conditionalSettings` for the `Clean` BUILD MODE ONLY:

```json
{ "buildModes": ["Clean"],
  "settings": { "preprocessorSymbols": ["CLEAN25", ..., "CLEAN30"] } }
```

So the DEFAULT build -- the one that ships -- leaves every one undefined, and the obsolete code is
KEPT. `src/al/Lexer.cpp:ApplyDirectives` evaluates every symbol as false and therefore keeps exactly
the same code. **That is correct, and it is correct by evidence rather than by accident** -- but
nothing in the tree said so, and "every symbol is false" is a CHOICE that a reader would otherwise
have to reverse-engineer from a lexer. It is written down here because CLAUDE.md puts a decision in
the board.

Two gaps in the same lexer, both now measured rather than assumed:

- **`#elif`, `#define` and `#undef` are unimplemented and the corpus contains none of them.** A
  recorded non-gap: `#elif`'s tokens would otherwise fall under the enclosing `#if`'s verdict rather
  than their own, which is a wrong answer and not a refusal -- so if one ever appears, it must
  refuse rather than be ignored.
- **A malformed `#if` condition yields false instead of raising** (`ReadPrimary` advances one
  character and returns false when it cannot read a word). Same class as board:0082: a parser that
  cannot read its input answers anyway.

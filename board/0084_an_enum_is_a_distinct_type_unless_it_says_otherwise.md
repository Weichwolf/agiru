Type: root
State: open
Area: gen

# An enum is a DISTINCT type, and only `AssignmentCompatibility` lets another one be assigned to it

`properties/devenv-assignmentcompatibility-property.md`:

> Sets whether an Enum can be assigned to from another Enum type. This is intended for backwards
> compatibility when splitting existing Options into multiple Enums. ... The default value of this
> property is `false`. ... **Because the assignment is done by ordinal value without validation,
> there is no guarantee that the target will have a corresponding value.**

So AL has two kinds of enum: one that is a closed type, and one that any other enum's ORDINAL may be
poured into. The distinction is a declaration, it is known when the transpiler runs, and **the C++
type system expresses it exactly** -- which is this tree's stated reason for leaving Python.

## The population, measured 2026-09-04 over `~/Git/BCApps/src`

| | |
|---|---:|
| `.Enum.al` objects | 1 436 |
| `.EnumExt.al` objects | 256 |
| `AssignmentCompatibility = true` | **573** |
| `Extensible = true` / `= false` | 1 061 / 1 226 |

**573 of 1 436 enums are assignable-from.** That is not an edge case to be discovered later; it is
40 % of them, and the other 60 % are a type error the compiler can catch for free.

## What agiru does today

`src/gen/EnumWriter.cpp` emits `enum class <Name> : std::int32_t` with its values and an
`EnumTraits` table, and reads NEITHER property -- `grep` over `src/gen/` and `src/al/` finds no
occurrence of `AssignmentCompatibility` or `Extensible` (measured 2026-09-04). Two consequences,
opposite in direction:

- **A distinct `enum class` already refuses the assignment**, so the 863 closed enums are correct by
  accident -- the C++ default happens to be AL's default.
- **The 573 compatible ones fail to translate.** `Rec."Document Type" := OtherEnum::Invoice` is a
  compile error, and the AL it came from is legal. That is a translation HOLE with a count, and it
  surfaces as `make apps` stopping rather than as a wrong answer, which is the good direction.

## The choice

**The target enum gets one converting constructor, and only when it declares the property.**

- `AssignmentCompatibility = true` emits, beside the `enum class`, a small wrapper the generated
  code assigns THROUGH -- or, simpler and in this tree's idiom, a free
  `constexpr <Name> AssignFrom(std::int32_t ordinal)` plus an `operator=` on the field wrapper that
  accepts any `EnumTraits`-known type. **No validation, by specification**: the ordinal is copied
  and a value the target does not declare is a value the target now holds. That is the documented
  behaviour and it is also why the property exists.
- **An enum WITHOUT the property emits nothing**, so the assignment stays a compile error naming the
  two types. **That is the `static_assert` half of this item**: the check is the absence of a
  conversion, enforced by the language, at zero cost -- exactly what CLAUDE.md means by "every
  construct the type system can carry, it carries".
- `Extensible` is recorded in `EnumTraits` rather than in the type, because it decides whether an
  `enumextension` may add values (board:0033 merges those at translation time) and not whether an
  assignment compiles. An `enumextension` over a non-extensible enum is a refusal the generator owes.

## Why this is not board:0076's subject

board:0076 asserts that a SYSTEM option's ordinals match its documentation page. This one is about
whether two GENERATED enums are the same type. They meet only in that both are decided at
translation time.

## Gate, and its negative control

One enum declaring `AssignmentCompatibility = true` and one without, and an AL fragment assigning
another enum into each. The first must translate and produce the source ordinal verbatim -- including
an ordinal the target does not declare, which the specification requires to be kept.

**The negative control is the second: it must FAIL TO COMPILE.** A gate that only checks the
compatible case passes over a generator that emits the conversion for everything, which is the exact
mistake this item exists to prevent -- and it is the more likely mistake, because emitting one
constructor unconditionally is less code than reading the property.

Classification: **activation** -- 573 enums that do not translate today begin to; nothing regresses,
because nothing compiles.

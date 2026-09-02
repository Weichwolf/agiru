Type: bug
State: open
Area: gen

# A generated name never collides with its class

Two shapes the BaseApp writes make C++ refuse a class the generator emits, and they are ONE defect
seen twice: a member whose identifier equals the identifier of the class it stands in.

| shape | AL | what is emitted | files |
|---|---|---|---|
| a procedure named after its codeunit | `codeunit "Create Reserv. Entry"` with `procedure CreateReservEntry(...)` | `void CreateReservEntry(...)` inside `class CreateReservEntry` -- "constructor cannot have a return type" | 15 |
| a field named after the field-number struct | table `Error Message` with field `Field Number` | `static constexpr FieldNo FieldNumber` inside `struct FieldNumber` -- "member has the same name as its class" | 3 |

C++ forbids both outright, so a deviation is FORCED. What it must not be is a rename that lands on
whatever the generator happens to reach first.

## What the references say

Nothing: this is not an AL question. AL is happy with either -- its objects and their members live in
separate namespaces -- and `openerp` never met it, because Python allows a method with its class's
name and used a dict for field numbers.

So the decision is made against CLAUDE.md's own rule: **a reader who knows AL and has never seen
agiru must be able to open one file and know how to write the next one**, and where idiomatic C++
cannot produce the AL shape, the deviation is VISIBLE and uniform rather than clever.

## What is already known, and it decides half of it

**A codeunit's class name appears at DECLARATIONS ONLY, never at call sites.** AL writes
`CreateReservEntry: Codeunit "Create Reserv. Entry"` and then `CreateReservEntry.CreateReservEntry(...)`
-- and the receiver there is the VARIABLE's name, which AL chose. So renaming the class costs the
reader one declaration; renaming the procedure costs them every call. Whatever is done, it is the
CLASS that moves and not the member.

The field-number struct is the mirror image: `FieldNumber` is a RUNTIME name, not an AL one, and
the member is the AL field. Again the enclosing name is the one that may move.

## What is not decided

Whether the rename applies to every generated object or only to the colliding one. A conditional
rule is exactly the trap the reader-priors argument warns about -- an exception is a trap for
anyone generalising from one example to the next -- and a suffix on all 3 914 codeunits to serve 15
is the other kind of bad. Neither is obviously right, and 18 files is not enough pressure to guess.

## What is true when this closes

- The 18 files translate.
- The rule is stated in one sentence a reader can apply to a file they have not seen.
- A collision the rule does not cover is a translation ERROR naming the object and the member, not
  a C++ diagnostic in a generated file.

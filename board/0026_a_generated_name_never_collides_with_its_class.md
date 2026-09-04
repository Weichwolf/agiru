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

## `this` IS AN AL KEYWORD SINCE 2024 WAVE 2, AND THE TRANSPILER DOES NOT KNOW IT

`devenv-al-this-keyword.md` (read 2026-09-04, board:0071 -- a root concept page that had only a
group verdict):

> The `this` keyword can be used in codeunits in AL as a self-reference, and it allows passing the
> current object as an argument to methods. ... **The newest version of the System Application has
> been updated to use the `this` keyword** for referencing methods and globals within the same
> object.

Measured 2026-09-04 over `~/Git/BCApps/src`: **2 600 uses of `this.<Member>`**, and they are exactly
where the page says -- the System Application and the newer apps, in shapes like
`this.GeneratePDF := GeneratePDFValue;` (`SalesCrMemoPEPPOL30NO.XmlPort.al:2439`) and
`this.MockDate := MockDate;` (`XRechnungStructValidations.Codeunit.al:104`).

**`grep -rn '"this"' src/al/ src/gen/` returns nothing.** So `this` is lexed as an ordinary
identifier and `.` is emitted as `.` (`src/gen/BodyWriter.cpp:57`), which produces `this.Member` in
C++ -- where `this` is a POINTER and `.` does not apply. **2 600 translation units fail to compile**,
loudly, which is the good direction but is a hole with a count.

**Why it belongs to THIS item.** The subject here is that a generated name never collides with what
the class already means, and `this` is the sharpest case: it is a C++ keyword whose meaning in the
generated class is ALMOST what AL means by it -- self-reference -- but through a pointer rather than
a reference. The two spellings differ by exactly the arrow.

**The choice**: the name resolver rewrites the identifier `this` to `(*this)`, which makes
`this.Member` become `(*this).Member` and keeps every other emission untouched. Not `this->`, because
the `.` comes from the operator table and rewriting the OPERATOR would change every member access in
the tree; rewriting the operand changes one identifier. The shape is also what the second use of the
keyword needs -- "passing the current object as an argument" -- since `f(this)` in AL passes the
object and `f(*this)` is its C++ spelling.

**Gate**: a codeunit that writes `this.Field := X` and one that passes `this` to a procedure taking
its own type. **The negative control** is the second: a resolver that only handles `this.` in a
member access compiles the first and not the second.

Classification: **activation** -- 2 600 files do not translate today, so nothing regresses.

Type: root
State: open
Area: gen, rt

# The generated SOURCES compile, not only the generated headers

`make gap` and `make tree` measure HEADERS. A header is declarations: signatures, members, field
tables. The 5 835 generated `.cpp` files are the bodies -- the AL statements and expressions the
whole thing exists to run -- and nothing has ever compiled them as a set.

## Reference

**The body emitter is the half that is not built yet.** `src/gen/BodyWriter.cpp` is 445 lines
against openerp's 4 075 in `generator/body_emitter/`, and openerp reached 97.0 % of the UT subset
with it. The statement grammar is complete -- all twelve `StmtKind`s translate -- and the
EXPRESSIONS are where the gap is: a builtin call, a member access on a handle, an option scope, a
`Rec.TestField(Field)` whose argument is a field of the receiver rather than a name in scope.

**What is already known to be wrong**, measured 2026-09-02 on one file rather than the population,
which is exactly why this item exists:

- `TempSourceTrackingSpecification.TestField(SourceType)` emits a bare name that is not in scope.
  The argument is a FIELD OF THE RECEIVER, so it has to be spelled against it.
- A call on a table's own procedure resolves through `TableNames`, which learned procedures only
  when the tables started carrying code.

## What the first pass through them found, 2026-09-03

`make gap SOURCE=1` walks the bodies the way `make gap` walks the headers: one translation unit,
stopping at the first diagnostic, rather than a full ninja rebuild for every repair. What it found
in the first few files was five DEFECT CLASSES and not five defects:

- **The casing collapse, which CLAUDE.md lists as a measured failure mode and which was here all
  along.** `Resolve` matched a name case-insensitively -- correctly -- and then returned the CALL
  SITE's spelling. `AgentConsumptionOverview` declares `AgentUserSecurityId` and its body writes
  `AgentUserSecurityID`; the two became different C++ symbols and the second named nothing. The
  declaration's spelling wins now.

- **A field argument is not a name in scope.** `X.SetRange("Agent User Security ID", V)` names a
  field OF X, and AL resolves it there -- so the C++ has to spell it against the receiver.
  Emitting the bare identifier made it undeclared in every body that filters, which is most of
  them: **3 530 of 9 600 files changed** when it was repaired. The method decides how many leading
  arguments are fields, and the table comes from the documentation's own signatures.

- **`Page.Run(Page::"X", Rec)` names the kind twice.** AL writes the dispatcher and the identity;
  what it means is "run that object", and the generated form says it once: `pages::X::Run(Rec)`.

- **A quoted name is a name.** The absent-type gatherer accepted only bare identifiers, so every
  member whose AL name has a space in it was missing from the stub -- which is most of them,
  because BC names fields the way a caption reads. The absent surface grew 3 778 -> 4 575 members.

- **The door refused conversions AL makes silently.** `exit(0)` in a Decimal procedure, a builtin
  returning `std::string` reaching a `Text` parameter, `Refused` assigned to a `Code`. Each was one
  `explicit` too many or one ambiguity, and each is now a constrained conversion with the reason
  written down.

**WHAT IS LEFT IS ONE GAP AND IT IS STRUCTURAL: the body writer does not know a variable's TYPE.**
`Agent.Substate::Archived` -- a field's enumeration reached through a record variable -- needs the
variable's table, then the field, then its enum. `Names` today maps a name to a SPELLING. Making it
map a name to a TYPE is the next piece, and it is what the remaining body defects hang off.

## How

- `make apps` builds them, stopping at the first error -- it exists and has never been run to
  completion. The first number is how many of 5 835 compile, and it is a baseline that may only
  rise.
- The loop is the one `make gap` already gives for headers: a root, a repair in `src/gen`, a
  re-transpile. What is missing is the census over sources, which is `make tree` with a different
  file list.
- **Order: the headers first.** A source that includes a header that does not compile says nothing
  about the body, and the header census is nearly green.

## What will be true

- [ ] `make tree` measures sources beside headers and records both counts.
- [ ] The compiling-source count is a baseline in `test/tree-baseline` that may only rise.
- [ ] `make apps` completes.

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

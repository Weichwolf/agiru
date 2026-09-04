Type:     task
Status:   open
Parent:   0067
Area:     gen
Source:   developer/properties/devenv-description-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# A `Description` is internal, and the transpiler drops it deliberately

> Sets the description. **This description is for internal use and does not appear to end-users.**
>
> Applies to: Codeunit, Table field, Table key, Page and each of its elements, Query and its columns
> and data items and filters, Report and its data items.

**The one sentence settles it**: the value never reaches a user, so nothing at run time needs it and
carrying it into `.rodata` would cost bytes for a comment.

**And it is a comment, which this tree deletes.** CLAUDE.md: `src/` carries no comments and `make`
runs `test/strip-comments.py` before it builds. A generated file carries no comments either -- only a
two-line provenance header. So emitting `Description` as a C++ comment beside the member would put
back exactly what the tree removes, and emitting it as data would store a comment in `.rodata`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Description =`: **3 995 declarations.**

## The IST-state

Not among the nine properties the generator consumes (board:0067); the AST holds it like every other.

## The choice

**Accept and drop, with this item as the citation.** board:0067's census lists it as
KNOWN-AND-DROPPED, so the decision is provable from the census rather than being the absence of a
decision.

**Not refusing it** -- 3 995 declarations, and the property is legal AL that harms nothing.

**Not carrying it either.** The alternative arguments are both weak: a diagnostic could quote it, but
board:0055's diagnostics quote AL error labels and not developer notes; a documentation generator
could use it, and there is none.

## Ordering

With board:0067's census. No runtime work.

## Gate, and its negative control

A table field declaring `Description` transpiles, and the string appears nowhere in the generated
file.

**The negative control is the generated file** -- searching it for the description text must find
nothing. An implementation that emits it as a comment passes a "transpiles" gate and reintroduces
comments into the generated tree.

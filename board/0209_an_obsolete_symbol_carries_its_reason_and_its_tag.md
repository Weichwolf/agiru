Type:     task
Status:   open
Parent:   0069
Area:     gen
Source:   developer/attributes/devenv-obsolete-attribute.md
Verdict:  fehlt
Class:    activation

# An `[Obsolete]` symbol carries its reason and its tag into the generated tree

`[Obsolete([Reason: Text] [, Tag: Text])]` applies to a **method, a DeclareMethod, a variable and an
event** -- four symbol kinds, where the object-level `ObsoleteState` / `ObsoleteReason` /
`ObsoleteTag` properties (board:0069) apply to objects and fields.

| argument | |
|---|---|
| `Reason` | why it is deprecated |
| `Tag` | "free-form text to support tracking of where and when the object was marked as obsolete, for example, branch, build, or date" |

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**1 985 `[Obsolete` declarations.**

## The IST-state

The attribute parses into the raw list and is dropped. An obsolete method is emitted exactly like
any other, so nothing in the generated tree records that 1 985 symbols are on their way out.

## The choice

**`[[deprecated("<Reason>")]]`**, with the reason text passed through verbatim. It is the C++
attribute that means what AL means, two front ends implement it, and a caller gets a warning naming
the reason -- which under `-Werror` is an ERROR, so the generated tree cannot call an obsolete symbol
without somebody deciding to.

**That last consequence is the reason to be careful.** The BaseApp calls its own obsolete methods
during the deprecation window; emitting `[[deprecated]]` naively would make `make apps` red on 1 985
symbols' worth of internal calls. So the attribute is emitted **only where `ObsoleteState` is
`Pending`**, and a `Removed` symbol is board:0069's business -- the two items divide there and this
one must not act alone.

The `Tag` becomes a comment in the provenance header rather than part of the deprecation text: it is
tracking metadata for BC's own release process and means nothing to a reader of the C++ tree.

## Ordering

After board:0069 settles what `ObsoleteState` does to a field and a table, because the same
three-value state decides whether this attribute is emitted at all.

## Gate, and its negative control

A method marked `[Obsolete('Use X instead')]` with `ObsoleteState = Pending`: a call to it must
produce a diagnostic carrying that text. **The negative control is a method with no attribute** --
calling it must produce nothing, which a generator that emits `[[deprecated]]` unconditionally gets
wrong on every method in the tree.

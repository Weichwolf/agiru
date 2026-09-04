Type:     task
Status:   open
Parent:   0069
Area:     gen
Source:   developer/properties/devenv-obsoletetag-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# An `ObsoleteTag` is carried and never interpreted

> Specifies a **free-form text** to support tracking of where and when the object was marked as
> obsolete, for example, **branch, build, or date** of obsoleting the object.

**"Free-form" is the specification.** The page names three conventional contents and requires none of
them, so anything that parsed the tag -- as a version, as a date, as a release -- would be inventing
a rule AL does not have.

It is `ObsoleteReason`'s sibling (board:0355) and differs in exactly one way: the reason is for the
READER of the diagnostic, the tag is for whoever cleans up later.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ObsoleteTag =`: **4 922 declarations**, against 4 926 `ObsoleteState` and 4 538 `ObsoleteReason`.

**Almost every obsolete element carries a tag and 7.9 % carry no reason.** Microsoft's own practice
tags consistently and explains inconsistently, which is worth knowing before deciding which of the
two matters more.

## The IST-state

Not among the nine properties the generator consumes (board:0067).

## The choice

Carry it into board:0069's diagnostic beside the reason, and **parse nothing**. If the tag ever needs
to be compared -- "everything tagged before 24.0 may now be deleted" -- that is a tool over the AL
source and not a rule in the transpiler, because the transpiler would have to guess a format the
documentation refuses to fix.

## Ordering

With board:0355 and board:0069.

## Gate, and its negative control

A reference to a `Pending` element emits a diagnostic containing the tag verbatim, including one
whose tag is not a version, a date or anything else recognisable.

**The negative control is the unrecognisable tag** -- an implementation that tries to interpret it
either drops it or fails, and only a tag like `refactor/xyz` shows which.

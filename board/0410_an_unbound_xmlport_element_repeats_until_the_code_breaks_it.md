Type:     task
Status:   open
Parent:   0065
Area:     rt, gen
Source:   developer/properties/devenv-unbound-property.md
Verdict:  fehlt
Class:    activation

# An unbound XMLport element repeats until the code breaks it

> Sets whether the element **can be repeated an unknown number of times** at runtime before the
> import or export moves on to the next element. **The default is false.**
>
> Applies to: Xml Port Text Element, Xml Port Field Element, Xml Port Table Element, Xml Port Field
> Attribute, Xml Port Text Attribute.
>
> **WARNING: If this property is set to true, then your code for handling exports must specify when
> to break to the next element by using the `BreakUnbound` method.**
>
> **During an import, the XMLport will automatically move on to the next record when the code reaches
> a record separator.**

**Export and import differ, and only one of them terminates by itself.** On export the loop is
infinite until AL calls `BreakUnbound()`; on import the record separator ends it. So the property
introduces a loop whose exit condition is in AL code -- which is exactly the shape that needs a
timeout guard, because a missing `BreakUnbound` is an export that never finishes.

`BreakUnbound` is a method on the XMLport and belongs to board:0028's builtin census; this property
is what makes it reachable.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Unbound =`: **6 declarations.**

Six, in a BaseApp with 22 `DefaultFieldsValidation` and 115 `LinkTable` declarations -- so it is a
real but rare shape, used where a flat file has a variable number of columns.

## The IST-state

XMLports have no generator (board:0065, board:0034).

## The choice

One bit on the element descriptor, and the writer emits a loop rather than a single read -- with
`BreakUnbound` setting the flag the loop tests.

**The guard is not optional.** CLAUDE.md names timeout guards in loops as a standing rule, and this
is a loop whose exit is in translated AL that may be wrong; six call sites do not make an unbounded
export acceptable.

## Ordering

Inside board:0065's XMLport writer, with the element kinds.

## Gate, and its negative control

An export whose AL calls `BreakUnbound` after three elements emits three; an import stops at the
record separator without any AL call.

**The negative control is an export whose AL never calls `BreakUnbound`** -- it must terminate with a
diagnostic rather than run forever, and only a deliberately broken XMLport proves the guard exists.

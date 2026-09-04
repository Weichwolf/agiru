Type:     task
Status:   open
Parent:   0027
Area:     gen, rt
Source:   developer/properties/devenv-implementation-property.md, developer/properties/devenv-defaultimplementation-property.md, developer/properties/devenv-unknownvalueimplementation-property.md
Verdict:  teilweise
Class:    activation

# An enum value names the interface implementation it stands for

**Three pages, one item**: they are one three-level fallback and the pages share a single code example.

> **Implementation** (Enum Value, runtime 5.0): the **explicit** interface implementer for an enum
> value.
>
> **DefaultImplementation** (Enum Type, runtime 5.0): the implementer **if there is no explicit one
> set for the value** -- "to catch the case where some extension uses an enum value that does not
> implement the interface."
>
> **UnknownValueImplementation** (Enum Type, runtime 7.0): the implementer **for ordinal values that
> are NOT INCLUDED in the defined list of enum values.**
>
> `ifoo := e; ifoo.Foo();` -- assigning an enum to an interface variable yields the implementation.

**Three levels and the third is the one that matters here**: an ordinal that is not a declared member.
An extensible enum can carry a value from an app this build does not know, and `UnknownValueImplementation`
is what runs then. board:0027 already records a three-level interface fallback and this is its
declaration.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Implementation =` **571** · `DefaultImplementation =` **40** · `UnknownValueImplementation =` **9**.

## The IST-state, and it is why this is `teilweise`

`src/gen/EnumWriter.cpp` consumes `Implementation` on an enum value -- it is one of the nine
properties the generator knows (board:0067) -- and looks the named codeunit up in
`objects.codeunits`. So the FIRST level exists.

The other two do not: neither `DefaultImplementation` nor `UnknownValueImplementation` is read, so an
enum value with no explicit implementer and an ordinal outside the declared list both resolve to
nothing.

## The choice

Two more `constexpr` entries on the enum's metadata, and the resolution is a three-step fallback
folded by the GENERATOR wherever the value is a compile-time constant -- which is most assignments --
and a run-time lookup only where the ordinal is dynamic.

**The unknown-ordinal case cannot be folded** and is the one that needs a table: an ordinal outside the
declared set has no `EnumValueDef`, so the lookup falls through to the type's own entry.

## Ordering

Behind board:0027's interface representation. The first level already works, so this is an extension
of a working path rather than a new one.

## Gate, and its negative control

An enum value with an explicit implementation resolves to it; one without resolves to the type's
default; an ordinal outside the declared list resolves to the unknown-value implementation.

**The negative control is the third case** -- it is unreachable from AL that only uses declared
members, so the gate has to construct an out-of-range ordinal, and an implementation that indexes the
value table crashes or silently picks a neighbour.

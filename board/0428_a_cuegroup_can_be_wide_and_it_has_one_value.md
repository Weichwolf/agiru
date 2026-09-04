Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-cuegrouplayout-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# A cue group can be wide, and the property has exactly one value

> Specifies if the layout is wide. Applies to: **Page Group.**
>
> | value | |
> |---|---|
> | **Wide** | Sets the `cuegroup` control to the wide layout |
>
> `CuegroupLayout = wide;`

**A property with one documented value and no documented default.** The table lists `Wide` alone, so
the only thing it can say is "wide"; absent means the normal layout, which the page never names. That
is a small documentation gap and it is recorded rather than filled in: whatever the normal layout is
called, AL cannot declare it.

The spelling on the page is also inconsistent -- the title says `CuegroupLayout`, other pages write
`CueGroupLayout` -- which is harmless in AL and is the same trap that made board:0329's
`ExtendedDataType` measure 4 instead of 2 745. The measurement below is case-insensitive for that
reason.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`CueGroupLayout =`: **10 declarations.**

Ten, against 4 128 `Importance = Promoted` and 46 008 `Image` -- so role centres are full of cues and
almost none of them is wide.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

One bit on the group descriptor -- `bool`, not an enumerator, because the property has one value and
a one-member enum would suggest more are coming. The renderer emits the wide cue layout.

## Ordering

With board:0030's cue-group rendering, which is board:0047's FlowFields on the other side.

## Gate, and its negative control

A `cuegroup` declaring the property renders in the wide layout; one declaring nothing renders in the
normal one.

**The negative control is the undeclared group** -- there are 10 declarations and every other cue
group in the BaseApp is the negative case, so an implementation that made every cue group wide passes
the positive gate and changes every role centre.

Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-gesture-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# A touch gesture runs an action on a row

> Specifies a gesture that runs the action on a device with a **touch interface, such as the phone
> client**. Applies to: **Page Action, Page File Upload Action.**
>
> `None` -- no gesture. `LeftSwipe` -- **swipe in from the RIGHT edge**. `RightSwipe` -- **swipe in
> from the LEFT edge**. `ContextMenu` -- the action has a context menu.
>
> You typically use the property on **list type pages** for running an action on items in a repeater
> control.

**The two swipe names are inverted relative to the edge they name**, and the documentation says so
plainly: `LeftSwipe` is a swipe from the RIGHT edge. A reader implementing from the name alone gets
both backwards. That is worth recording whether or not the property is built, because it is the kind
of statement a later reader will not re-check.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Gesture =`: **32 declarations.**

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

A four-valued enumerator on the action descriptor, **carried and not acted on for now.** agiru's
target is a browser (CLAUDE.md's htmx renderer), and a touch gesture on a desktop browser has no
input to bind to -- but a browser on a tablet does, so this is not the same as board:0373's SQL
Server compression with no PostgreSQL counterpart. The mechanism could exist; nothing consumes it
yet.

**Carried rather than refused** because 32 declarations are legal AL that harm nothing, and carried
rather than dropped because the enumerator costs one byte and the alternative is re-reading 32 pages
of AL later.

`ContextMenu` is the one value that is not touch-specific and could be honoured now; it is named here
and left to board:0030's action rendering.

## Ordering

With board:0030's action metadata. No consumer, so no dependency.

## Gate, and its negative control

An action declaring `Gesture = LeftSwipe` carries that value in its descriptor.

**The negative control is the direction** -- a gate that also asserts the descriptor's value maps to
"from the right edge" is the only thing that keeps the inversion from being lost.

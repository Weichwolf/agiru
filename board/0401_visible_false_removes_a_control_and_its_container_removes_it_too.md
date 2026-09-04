Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-visible-property.md
Verdict:  fehlt
Class:    activation

# `Visible = false` removes a control, and its container removes it too

> Sets whether to display the page or control. **The default is true.**
>
> **Because this property also applies to containers, such as pages and subpages, if the `Visible`
> property for the container is set to false, then controls on the container are also not displayed,
> even if the `Visible` property is set to true.**
>
> Applies to: Page Label, Page Field, Page Group, Page Part, Page System Part, Page Chart Part, Page
> Action, Page Action Group, **Page Action Ref**, Page Custom Action, Page File Upload Action, Page
> View, Page Analysis View, Page User Control.

**The same container rule as board:0400's `Editable`** -- an AND down the tree, not an override -- and
the two should be built together because a renderer computes both in the same walk.

**Where they differ is what happens to the value.** A non-editable control is still rendered and still
holds a value; an invisible one is not in the fragment at all. That is the same "removed, not
disabled" shape board:0377's `AccessByPermission` has, and for an htmx renderer it is the correct one:
an element merely hidden with CSS is still reachable.

`Visible` is also assignable from AL -- `CurrPage.Control.VISIBLE := false` -- so the declared value is
an initial state and not a constant, which rules out folding the container tree at translation time.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Visible =`: **48 225 declarations.**

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

One bit per control, initialised from the declaration, ANDed down the container tree at render time,
and assignable from AL. The renderer omits the element rather than hiding it.

## Ordering

With board:0400 -- one tree walk, two bits.

## Gate, and its negative control

A control inside an invisible group is absent from the fragment even though it declares
`Visible = true`.

**The negative control is the fragment, not the rendering** -- a control hidden with `display: none`
looks identical in a screenshot and is still in the DOM, so the gate has to assert the element is not
present.

Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-importance-property.md
Verdict:  fehlt
Class:    activation

# `Importance` decides what a FastTab shows before "Show more"

> **Version**: runtime 3.2. Applies to: **Page Label, Page Field.**
>
> `Standard` -- displays the field on the page by default. `Promoted` -- displays the field on the
> page **and also in the header of the FastTab when the FastTab is collapsed**. `Additional` --
> **hides the field by default**; on a FastTab, to show it, a user chooses **Show more**.
>
> Users can change the setting for their workspace by using personalization.

Three states and they are not a scale: `Promoted` adds a second rendering position (the collapsed
header), `Additional` removes the default one. So a renderer needs two questions per control -- is it
in the default set, and is it in the collapsed header -- and a single ordering would answer neither.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Importance =`: **16 781 declarations** -- `Additional` **12 490**, `Promoted` **4 128**, `Standard`
**163**.

**`Standard` is the default and 163 declare it anyway**, so the real distribution is 12 490 hidden and
4 128 in the collapsed header, against the tens of thousands that say nothing. Three quarters of every
declaration is a field BC hides until asked.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

A three-valued enumerator on the control descriptor. The renderer emits the default set and the
"Show more" set as two groups in one fragment, so expanding is a client-side toggle and not a round
trip -- 12 490 controls' worth of hidden fields are already known when the page is rendered.

**Not a second request per FastTab.** htmx would allow it; the data is already in hand and the round
trip would be paid on every expansion.

Personalisation overrides it, as it does board:0406's `QuickEntry` -- the declaration is the default.

## Ordering

With board:0030's control metadata and its FastTab grouping.

## Gate, and its negative control

A field declaring `Additional` is not in the default view and appears after "Show more"; one declaring
`Promoted` appears in the collapsed FastTab header.

**The negative control is the collapsed header** -- an implementation that treats `Promoted` as
`Standard` renders it in the right place when expanded and shows an empty header when collapsed,
which no expanded-page gate can see.

Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-infooterbar-property.md
Verdict:  fehlt
Class:    activation

# An action can sit in the footer bar

> Sets whether an action should be viewed as an **exit action in the lower part of the page**. **The
> default is false.** Applies to: **Page Action, Page File Upload Action.**

The footer bar is where a wizard's Back / Next / Finish live and where a dialog's OK and Cancel go. So
the property moves an action out of the action bar and into a second, differently-rendered region --
one more placement alongside board:0362's repeater scope and board:0411's promoted position.

**Four placements from three properties**: action bar, row menu (`Scope = Repeater`), collapsed FastTab
header (`Importance = Promoted`), footer bar. A renderer that emits actions in one pass has to ask all
three.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`InFooterBar =`: **492 declarations**, all necessarily `true` since `false` is the default.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

One bit on the action descriptor. The renderer emits a footer region and places the action there.

**The placement decisions belong together and not to each property separately**: the generator
resolves scope, importance and footer into ONE placement enumerator per action, so the renderer reads
one value and there is no combination left to get wrong at render time.

## Ordering

With board:0030's action metadata, board:0362 and board:0411.

## Gate, and its negative control

An action declaring `InFooterBar = true` renders in the footer region and not in the action bar.

**The negative control is the action bar** -- an implementation that adds a footer copy without
removing the original renders the action twice, and a gate that only looks at the footer passes.

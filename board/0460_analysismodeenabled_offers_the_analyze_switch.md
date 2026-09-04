Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-analysismodeenabled-property.md, developer/properties/devenv-clearviews-property.md
Verdict:  fehlt
Class:    activation

# `AnalysisModeEnabled` offers the Analyze switch, and a customization can clear the views

**Two pages, one item**: both are about the page's view pane -- the mode that turns a list into a
pivot, and the customization that removes the saved views. They are small, they share one renderer
region, and each alone is a paragraph.

> **AnalysisModeEnabled** (runtime 12.0, Page): whether analysis mode is allowed. **"When enabled, the
> Analyze switch is available at the top of the page." The default is true.** "Data analysis mode
> enables users to analyze data directly from the page, without having to run a report or switch to
> another application like Excel."
>
> **ClearViews** (runtime 14.0, Page Customization): **clears all views from the page's view pane.**

**`ClearViews` is `ClearActions`' twin** (board:0427) and carries the same finding: a page
customization SUBTRACTS, so board:0033's merge needs a subtractive step, and the ORDER decides the
outcome -- clear before the customization's own views and after the base page's.

**And `AnalysisModeEnabled` defaults to true**, so the population understates it as usual: every list
page in the BaseApp offers the Analyze switch, and 58 turn it off.

That default is a real feature, not a bit: an interactive pivot over the page's rows. It is named here
at its true size rather than filed as a switch.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AnalysisModeEnabled =` **58** (all necessarily `false`) · `ClearViews =` **8**.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; there is no view pane and no analysis mode.

## The choice

One bit on the page descriptor and a subtractive step in board:0033's customization merge.

**The analysis mode itself is deferred and said to be deferred**: it is a client-side pivot over rows
the page already streams, and it belongs after board:0030's list rendering works. The bit costs
nothing now and the feature is a separate piece of work.

## Ordering

The bit with board:0030's page descriptor; `ClearViews` with board:0427 in board:0033's merge; the
analysis mode itself last.

## Gate, and its negative control

A page declaring `AnalysisModeEnabled = false` offers no Analyze switch; a customization declaring
`ClearViews` over a page with two views leaves the views it declares itself.

**The negative control for `ClearViews` is the ORDER** -- clearing after the customization's own views
leaves zero, which also looks plausible, exactly as in board:0427.

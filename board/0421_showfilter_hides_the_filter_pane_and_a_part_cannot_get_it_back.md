Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-showfilter-property.md
Verdict:  fehlt
Class:    activation

# `ShowFilter` hides the filter pane, and inside a part the user cannot get it back

> Sets whether the filter pane is shown on a page by default. **The default is true.** Applies to:
> Page, Request Page, Page Part, Page System Part, Page Chart Part.
>
> **Using customization, a user can override the setting** to show or hide the filter -- **except
> when the page is displayed in a PART of another page**, such as a FastTab or FactBox. **When the
> page is displayed in a page part, the property permanently hides or shows the filter pane.**

**The same declaration is a default in one context and an absolute in another**, and the context is
not the page's own: a page is a part or not depending on who embedded it. So the property cannot be
resolved at translation time -- the same page object is both.

That makes it a render-time decision over the page's ROLE, which is a thing board:0030's renderer has
to carry anyway for `SubPageLink` and `SubPageView`, and this is the first item that needs it named.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ShowFilter =`: **1 017 declarations**, all necessarily `false` since `true` is the default.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; there is no page-part rendering and no
personalisation.

## The choice

One bit on the page descriptor plus the renderer's knowledge of whether it is rendering the page
standalone or as a part. Standalone, the bit is the initial state and personalisation may override it;
as a part, it is final.

**Personalisation has no board item** and is named here as the second half of the rule rather than
assumed away.

## Ordering

Behind board:0030's page-part rendering.

## Gate, and its negative control

A page declaring `ShowFilter = false` opens without the filter pane and a user can bring it back;
the same page embedded as a FactBox has no way to bring it back.

**The negative control is the embedded case** -- an implementation that treats the bit as a default
everywhere lets a user restore a pane BC keeps hidden, and only rendering the same page both ways
shows it.

## The metadata half landed 2026-09-07 (board:0553)

The property this item is about now reaches `constexpr` metadata -- `ControlDef`/`PageDef` in
`include/meta/PageDef.h`, `FieldDef`/`TableDef` in `meta/TableDef.h`, `CodeunitDef` in
`meta/CodeunitDef.h` -- emitted per object into the `.cpp` and reached through the object's traits.
What is open here is what READS it, not what carries it: the property census over the whole BaseApp
went from 46 203 declarations dropped in silence to 813 in the same round.

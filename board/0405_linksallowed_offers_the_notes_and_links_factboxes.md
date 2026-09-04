Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-linksallowed-property.md
Verdict:  fehlt
Class:    activation

# `LinksAllowed` offers the Notes and Links FactBoxes

> Sets whether links are allowed. **The default is true.** Applies to: **Page, Request Page.**
>
> If `LinksAllowed` is set to true, then you can **add links or notes to a page**. The links can be
> links to web sites, files stored on the local computer or on a remote computer, or links to pages.
>
> **On a page, the links and notes are displayed in FactBoxes.** If `LinksAllowed` is set to true,
> then the **Actions** menu has a **Notes** item and a **Links** item.

**This is a feature and not a flag**, which is what makes it worth an item rather than a bit: the
property being `true` by default means every page in BC offers record-attached notes and links, and
that requires a platform table to store them against the record's `SystemId`.

**The storage is BC's own and board:0032 owns which tables the platform provides.** The property
decides whether a page offers the FactBox; the table decides whether there is anywhere to put a note.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`LinksAllowed =`: **927 declarations**, all necessarily `false` since `true` is the default.

**927 pages switch the feature OFF**, and every other page in the BaseApp offers it. So the
population understates the work exactly as board:0372's `Compressed` does: what has to be built is
the default.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone. board:0032 lists the platform tables agiru
provides; whether a record-link table is among them is unmeasured, **and that is stated rather than
assumed** -- it decides whether this item is a bit or a feature.

## The choice

One bit on the page descriptor, defaulting to `true`, plus the two FactBoxes and the platform table
behind them. **The bit is trivial and the feature is not**, so this item is honest about being two
pieces of work and the bit alone is not a solution.

## Ordering

Behind board:0032's platform tables and board:0030's FactBox rendering. The bit itself is free and
goes with the page descriptor.

## Gate, and its negative control

A page declaring `LinksAllowed = false` offers no Notes or Links action; a page declaring nothing
offers both and a note attached to a record is found again after reopening the page.

**The negative control is the default page** -- an implementation that carries the bit and builds no
FactBox passes a "false hides it" gate and offers the feature on zero of the remaining pages.

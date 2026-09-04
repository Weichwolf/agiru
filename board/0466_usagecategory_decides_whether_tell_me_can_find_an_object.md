Type:     task
Status:   open
Parent:   0083
Area:     gen, rt
Source:   developer/properties/devenv-usagecategory-property.md
Verdict:  fehlt
Class:    activation

# `UsageCategory` decides whether Tell Me can find an object at all

> **Version**: runtime 12.0. Applies to: **Page, Report, Query.**
>
> `None` · `Lists` · `Tasks` · `ReportsAndAnalysis` · `Documents` · `History` · `Administration`
>
> **"If `UsageCategory` is set to `None`, or if you DON'T SPECIFY `UsageCategory`: the page or report
> won't show up when you use the search functionality. Users won't be able to bookmark a link to the
> page or report object from the user interface."**
>
> "The `UsageCategory` is also used to categorize pages and reports shown in the **role explorer**."

**This is board:0083's gate, not a label.** Tell Me searches the object catalogue -- and an object
without this property is not IN that catalogue. So the search's population is not "every page", it is
"every page declaring a usage category", and board:0083 sizes itself from this number.

**And the default is exclusion**, which is the opposite of most defaults in this sweep: an undeclared
page is invisible to search, so the 3 378 declarations ARE the searchable set rather than the
exceptions to it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`UsageCategory =`: **3 378 declarations.**

Against board:0429's 6 891 pages plus 668 reports plus 346 queries -- roughly **7 900 objects, of
which 3 378 are findable**. Less than half the BaseApp's UI objects appear in Tell Me, by
declaration.

That is the number board:0083 needs and it is the item's main content: a search over the whole
catalogue would return twice what BC returns.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; board:0083 records that the catalogue has no
search.

## The choice

A seven-valued enumerator on the page, report and query descriptors, and **board:0083's searchable set
is built from it at translation time** -- a `constexpr` array of the objects that declare something
other than `None`, sorted, in `.rodata`. No per-session catalogue, no lazily sorted global (CLAUDE.md
names that as a data race the catalogue already had once).

## Ordering

Ahead of board:0083's search, which needs the set. With board:0389's search terms and board:0382's
captions, which are the other two inputs.

## Gate, and its negative control

Tell Me finds a page declaring `UsageCategory = Lists` and does not find one declaring nothing.

**The negative control is the undeclared page** -- an implementation that searches the whole catalogue
finds it, which looks like a better search and returns 4 500 objects BC hides.

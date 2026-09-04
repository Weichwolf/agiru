Type:     task
Status:   open
Parent:   0331
Area:     gen, rt
Source:   developer/properties/devenv-navigationpageid-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `NavigationPageId` names where a `TableRelation` navigates

> **Version**: runtime 6.3. Applies to: **Page Field.**
>
> Specifies **which page the `TableRelation` should navigate to.**

**One sentence and no remarks.** It is the third page-id property on a field, and the three are easy
to confuse:

| property | opens | when | WI |
|---|---|---|---|
| `LookupPageId` | a list to PICK from | the user opens the dropdown | 0334 |
| `DrillDownPageId` | the rows BEHIND a value | the user drills into a FlowField | 0335 |
| `NavigationPageId` | the related RECORD | the user follows the relation | this |

The distinction is what the user gets back: a lookup writes a value into the field, a drill-down shows
what made it up, a navigation leaves for the other record's own page. Three properties, three
interactions, and the documentation distinguishes them in one line each.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`NavigationPageId =`: **0 declarations.**

Against `LookupPageId` 2 294 and `DrillDownPageId` 1 822. **Nobody declares it**, which means BC
navigates by the relation's target page in every case -- the property exists to override a default
that is evidently always right.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; board:0331's relation does not exist, so there
is nothing to navigate along.

## The choice

**Refuse it**, joining the sweep's other zero-population properties (board:0327, board:0333,
board:0346, board:0347, board:0361, board:0366, board:0394). The default -- navigate to the relation
target's own card -- is what board:0331 and board:0334 give for free, and a per-field override with no
call site is a code path that can only be wrong.

**What the refusal costs is one line**, and what it buys is the notification if a page ever declares
one, at which point the default was not right and somebody has to know.

## Ordering

With board:0067's census. Behind board:0331 for the navigation itself, which is not this item.

## Gate, and its negative control

A page field declaring `NavigationPageId` fails to transpile.

**The negative control is the whole BaseApp transpiling with the refusal in place** -- which proves
the zero rather than trusting it. This item's population is the reason for its decision, so an unproven
zero would make the decision unfounded.

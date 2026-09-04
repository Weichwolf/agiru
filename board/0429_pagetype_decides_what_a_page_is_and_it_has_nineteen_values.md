Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-pagetype-property.md
Verdict:  fehlt
Class:    activation

# `PageType` decides what a page is, and it has nineteen values

> Sets the type of page to create. Applies to: **Page.**

Nineteen values, and they are not variations of one renderer:

| value | what it is | since |
|---|---|---|
| `Card` | master data, one record | 1.0 |
| `List` | entity overview, inline editing | 1.0 |
| `Document` | a header with lines | 1.0 |
| `Worksheet` | line-based entry -- journals | 1.0 |
| `ListPlus` | statistics and details | 1.0 |
| `CardPart` / `ListPart` | embedded in another page -- a FactBox | 1.0 |
| `RoleCenter` | the start page for a profile | 1.0 |
| `HeadlinePart` | insights embedded in a role centre | 1.0 |
| `ConfirmationDialog` | a warning | 1.0 |
| `StandardDialog` | a routine dialog | 1.0 |
| `NavigatePage` | a multi-step wizard | 1.0 |
| `API` | **"used to generate web service endpoints and CANNOT be shown in the user interface"** | 1.0 |
| `ReportPreview` / `ReportProcessingOnly` / `XmlPort` | platform-internal page kinds | 1.0 |
| `PromptDialog` | a Copilot interaction | 12.1 |
| `ConfigurationDialog` | configuring a process | 14.0 |
| `UserControlHost` | hosts a single user control | 15.0 |

**This is the property board:0030's whole renderer branches on**, and several items in this sweep
already depend on it: board:0368's `DataAccessIntent` applies only to `API`; board:0369's
`ChangeTrackingAllowed` only to `API`; board:0388's teaching tip is suppressed on five of these
types; board:0330's `MaskType` is not allowed on `ConfigurationDialog`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`PageType =`: **6 891 declarations**, and the distribution decides the build order:

| value | count | | value | count |
|---|---:|---|---|---:|
| `List` | **2 740** | | `RoleCenter` | 211 |
| `Card` | 923 | | `StandardDialog` | 190 |
| `ListPart` | 817 | | `NavigatePage` | 159 |
| `Document` | 429 | | `UserControlHost` | 157 |
| `Worksheet` | 412 | | `ListPlus` | 152 |
| `API` | 374 | | `ConfirmationDialog` | 25 |
| `CardPart` | 271 | | `HeadlinePart` | 16 |

**`List`, `Card`, `ListPart` and `Document` are 4 909 of 6 891 -- 71 %.** `PromptDialog`,
`ConfigurationDialog`, `ReportPreview`, `ReportProcessingOnly` and `XmlPort` do not appear at all.

That is the milestone's ordering: four renderers cover seven pages in ten, and the five unused values
are refused rather than built.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone -- so a page's own TYPE is not read, and every
page would render identically.

## The choice

A `constexpr` enumerator on the page descriptor, and the renderer dispatches on it. **Not one
renderer with branches** -- a `Card` and a `List` differ in their record cursor, not only in their
markup, and folding them costs the distinction board:0056's `Find` family depends on.

The five unused values are refused, with their zero measured.

**Every per-type rule already recorded in this sweep becomes a `static_assert` here**, because the
type is a declaration: `DataAccessIntent` outside `API`, `MaskType` on `ConfigurationDialog`, the
teaching tip on the five suppressed types.

## Ordering

**First in board:0030.** Nothing else in the page metadata is meaningful until the renderer knows what
kind of page it is rendering.

## Gate, and its negative control

A `List` page renders a repeater over its source table; a `Card` page renders one record; an `API`
page is not reachable through the UI at all.

**The negative control is the `API` page** -- "cannot be shown in the user interface" is a rule, and a
renderer that treats it as a list renders 374 pages BC does not show.

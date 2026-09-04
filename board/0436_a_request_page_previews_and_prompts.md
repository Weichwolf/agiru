Type:     task
Status:   open
Parent:   0063
Area:     gen, rt
Source:   developer/properties/devenv-previewmode-property.md, developer/properties/devenv-promptmode-property.md, developer/properties/devenv-userequestpage-property.md, developer/properties/devenv-processingonly-property.md
Verdict:  fehlt
Class:    activation

# A report decides whether it asks, previews and produces anything

**Four pages, one item**: they are the four switches on a report's interaction -- whether a request
page appears, in which mode, how the result is previewed, and whether there is a result at all. Each
is a `bool` or a small enum on the report, and none is separable from the others in the request-page
flow.

- **`UseRequestPage`** -- whether the report shows one at all.
- **`PromptMode`** -- how it is shown.
- **`PreviewMode`** -- how the output is previewed.
- **`ProcessingOnly`** -- the report produces **no layout output**: it runs its data items for effect
  and prints nothing.

**`ProcessingOnly` is the one with teeth.** A processing-only report is a batch job wearing a report's
clothes -- it iterates data items, posts, and returns. So the report generator must not require a
layout for 767 objects, and board:0063's rendering path is skipped entirely rather than producing an
empty document.

**And it interacts with board:0413.** `SaveValues` together with `AllowScheduling` decides whether the
request page supports multiple previews; `PreviewMode` decides what a preview IS. Four properties
across two items, and board:0413 already records the pairing from its side.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ProcessingOnly =` **767** · `PreviewMode =` **223** · `UseRequestPage =` **140** ·
`PromptMode =` **2**.

**767 of CLAUDE.md's 668 in-scope reports are processing-only** -- the count exceeds the report total
because the property is also on other objects, which is itself worth checking when the item is pulled.
Either way, the batch-job shape is the COMMON one and the rendered report is not.

That reverses the obvious build order: board:0063's first deliverable is a report that runs its data
items and produces nothing, not one that renders.

## The IST-state

Reports have no generator (board:0063, board:0034). `PromptMode` at 2 declarations is close enough to
zero to be checked before anything is built for it.

## The choice

Four fields on the report descriptor, resolved by the generator. `ProcessingOnly` selects a path that
never touches board:0063's XSL-FO renderer, so a processing-only report has no layout dependency at
all.

## Ordering

Inside board:0063. **`ProcessingOnly` first**, on population: it is the report shape that needs no
renderer.

## Gate, and its negative control

A `ProcessingOnly` report runs its data items and produces no document; a normal report produces one.

**The negative control is the empty document** -- an implementation that renders an empty PDF for a
processing-only report satisfies "it ran" and produces a file BC never creates.

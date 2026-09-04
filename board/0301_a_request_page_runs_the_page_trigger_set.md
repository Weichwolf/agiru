Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/triggers-auto/requestpage/ (12 pages), developer/triggers-auto/requestpageextension/ (9 pages)
Verdict:  fehlt
Class:    activation

# A request page runs the PAGE trigger set, and the report's run hangs off its close

The 21 pages of `requestpage/` and `requestpageextension/` describe the SAME twelve triggers as
`page/` -- `OnInit`, `OnOpenPage`, `OnQueryClosePage`, `OnClosePage`, `OnFindRecord`, `OnNextRecord`,
`OnAfterGetRecord`, `OnAfterGetCurrRecord`, `OnNewRecord`, `OnInsertRecord`, `OnModifyRecord`,
`OnDeleteRecord` -- minus the two background-task ones, which a request page does not have.

**They are one task and not twenty-one**, because the semantics are board:0278 to board:0289's: once
the page runtime fires page triggers, it fires them for a request page too. What this item owes is
the ATTACHMENT -- making a request page a page the runtime can drive -- and the two things about it
that are not true of an ordinary page:

- **Its `Rec` is not a table row.** A request page's source is the report's request-page controls
  and its dataitem filters, so `OnFindRecord` and `OnNextRecord` have nothing to read. That is why
  `requestpageextension/` has 9 of the 12 and not all: `OnInit`, `OnFindRecord` and `OnNextRecord`
  are absent from the extension set.
- **Its close decides whether the report runs.** board:0063's sequence is `OnInitReport`, the
  request page, then `OnPreReport` -- and a cancelled request page ends the run without an error.
  So `OnQueryClosePage`'s Boolean and the `CloseAction` it receives are what the report driver reads,
  not just what the page does.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`requestpage` blocks in report objects: **1 342**. The per-trigger counts are inside board:0278 to
board:0289's numbers, which count `page` and `requestpage` declarations together -- and separating
them is work this item does when it is pulled, not before.

## The IST-state

No page runtime and no report runtime. `include/runtime/test/TestRequestPage.h` exists as a door
header; board:0213's `RequestPageHandler` is the other half and is also unbuilt.

## The choice

The report driver constructs the request page as a page over its own control set and drives it
through the same trigger machinery board:0278-0289 build. **One implementation, two callers** -- and
the alternative, a second trigger path for request pages, is how the two would drift.

## Ordering

Blocked on board:0030's page runtime and board:0063's report driver. It is the seam between them.

## Gate, and its negative control

A report whose request page's `OnOpenPage` sets a filter and whose `OnQueryClosePage` returns
`false`: the report does NOT run.

**The negative control is the report running anyway** -- a driver that treats the request page as
decoration reads its filters and ignores its refusal, which is the failure that makes a cancelled
report post.

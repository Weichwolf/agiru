Type:     task
Status:   open
Parent:   0054
Area:     rt, gen
Source:   developer/attributes/devenv-requestpagehandler-attribute.md
Verdict:  fehlt
Class:    activation

# A `[RequestPageHandler]` fills a report's request page before the report runs

```al
[RequestPageHandler]
procedure RequestPageHandler(var RequestPage: TestRequestPage <id>)
```

A request page runs BEFORE the report's dataset is processed (board:0063's trigger order:
`OnInitReport`, then the request page, then `OnPreReport`). The handler sets options and filters and
then invokes `OK` or `Cancel` -- and cancelling is an ordinary outcome that ends the run without an
error.

**A `[ReportHandler]` REPLACES the whole run including the request page**, so a
`RequestPageHandler` is NOT called when one is present (board:0212). That precedence is the rule an
implementation gets wrong by dispatching both.

The dispatch key is (kind, report id) from the parameter type; the two declaration rules apply.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**5 205 `[RequestPageHandler` declarations** -- the second-largest handler population. Over the
milestone's 78 UT codeunits: 24 declarations in 10 codeunits.

## The IST-state

The attribute parses and is dropped. `TestRequestPage` has a door header under
`include/runtime/test/`; no report runs.

## The choice

A table entry with kind `RequestPage` and the report id. The runtime consults it at the request-page
stage of board:0063's sequence, hands a `TestRequestPage` over the report's request-page controls,
and treats the handler's `Cancel` as the documented early exit.

**The request page's saved state is consulted FIRST** (board:0063: "Last used options and filters"
is always available), then the handler overwrites what it sets. A handler that sets nothing leaves
the previous run's filters in place, which is what makes a second run in the same test differ from
the first -- and is why the two mechanisms have to be ordered rather than merged.

## Ordering

Blocked on board:0063 (a report that runs) rather than on the page runtime. 280 of 676 reports are
`ProcessingOnly` and need no layout, so this handler is reachable well before a renderer exists.

## Gate, and its negative control

A `ProcessingOnly` report with a filter on its dataitem, a handler that sets that filter, and an
assertion on the rows processed. A second run with no handler must reuse the first run's filters.

**The negative control is the precedence pair**: register a `[ReportHandler]` for the same report
and require the `RequestPageHandler` NOT to be called.

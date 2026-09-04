Type:     task
Status:   open
Parent:   0054
Area:     rt, gen
Source:   developer/attributes/devenv-reporthandler-attribute.md
Verdict:  fehlt
Class:    activation

# A `[ReportHandler]` replaces the whole report run, request page included

```al
[ReportHandler]
procedure ReportHandler(var Report: Report <id>)
```

The handler stands in for the ENTIRE run: no request page, no dataitem processing, no layout. That
is the rule that orders it against board:0213 -- **a `RequestPageHandler` is not called when a
`ReportHandler` is present for the same report.**

The dispatch key is (kind, report id) from the parameter type; the attribute is legal only inside a
`Subtype = Test` codeunit and the method must be global.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**121 `[ReportHandler` declarations.** Over the milestone's 78 UT codeunits: 4 declarations in 1
codeunit -- the smallest report-side handler, and the one that needs the least of board:0063.

## The IST-state

The attribute parses and is dropped; no report runs.

## The choice

A table entry with kind `Report`. The report runner consults it FIRST, before
`OnInitReport` -- because the handler replaces the run rather than joining it -- and returns
immediately after the handler returns.

**Why first and not at the request page.** Putting the check at the request-page stage would run
`OnInitReport` before the handler, and a report whose `OnInitReport` has side effects would produce
them under a handler that was meant to suppress the whole run.

## Ordering

Needs 0199's table and a report object that TRANSLATES; it does not need the dataset to be
processed, because it replaces the processing. With 4 milestone declarations it is the cheapest
report-side handler and can precede board:0063's renderer entirely.

## Gate, and its negative control

A report whose `OnPreReport` raises, run with a `[ReportHandler]` registered: the test must pass,
because the handler replaced the run.

**The negative control is registering both a `[ReportHandler]` and a `[RequestPageHandler]`** and
requiring the second NOT to be called -- a runtime that dispatches both passes the first case and
breaks the precedence.

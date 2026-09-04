Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/properties/devenv-executiontimeout-property.md
Verdict:  fehlt
Class:    activation

# A report stops at a declared timeout

> Sets the maximum time the report will run **after which it is automatically terminated**. **A string
> in the format `hh:mm:ss`.**
>
> **"At runtime, this property will OVERRIDE the limit set by the Default Max Rendering Timeout
> (`ReportDefaultTimeout`) setting for the server instance. The server instance also includes the Max
> Rendering Timeout (HARD LIMIT) (`ReportTimeout`) setting, which this property WON'T override."**

**The same three-way precedence as board:0455's row limits**, on time instead of rows: a per-report
value overrides one server setting and is itself capped by another. Two limits with different
authority, and an implementation that keeps one number gets the hard limit wrong.

**And it is a real guard, not a setting.** CLAUDE.md names timeout guards in loops as a standing rule;
this is the AL-declared form of one, and board:0455's `ShowPrintStatus` cancellation is the
user-driven form. **Both terminate the same loop, so they are one mechanism with two triggers** and
should not be built twice.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ExecutionTimeout =`: **1 declaration.**

One, in 668 reports. So the property is nearly unused and the SERVER DEFAULT is what actually stops a
runaway report -- which does not exist here at all, and that is the finding: **agiru currently has no
report timeout of any kind**, declared or configured.

## The IST-state

Reports have no generator (board:0063, board:0034); no server-instance configuration exists, so
neither the default nor the hard limit has a source.

## The choice

A duration on the report descriptor, parsed from `hh:mm:ss` by the generator, combined with the two
server values into one effective deadline the data-item loop checks -- the same check board:0455's
cancellation uses.

**The two server settings need somewhere to live**, and that is a configuration question with no
board item; it is named here rather than assumed.

## Ordering

Inside board:0063, with board:0455 -- one loop check, two triggers.

## Gate, and its negative control

A report declaring `00:00:05` over a data item that would run longer terminates at five seconds with a
diagnostic.

**The negative control is a report declaring nothing** -- it must still terminate at the server's
default, and an implementation that only honours the declared value leaves 667 reports unbounded,
which is exactly today's state.

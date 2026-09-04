Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/triggers-auto/reportextensiondatasetmodify/ (6 pages)
Verdict:  fehlt
Class:    activation

# A report extension's dataset-modify triggers BRACKET the base report's three

The six pages of `reportextensiondatasetmodify/` are three pairs, and each brackets one of the
base report's dataitem triggers:

| extension trigger | brackets |
|---|---|
| `OnBeforePreDataItem` / `OnAfterPreDataItem` | `OnPreDataItem` (0306) |
| `OnBeforeAfterGetRecord` / `OnAfterAfterGetRecord` | `OnAfterGetRecord` (0307) |
| `OnBeforePostDataItem` / `OnAfterPostDataItem` | `OnPostDataItem` (0308) |

**They are one task and not six**: the same call sites, the same merge rule, and an implementation
that wires one pair wires all three. The doubled name `OnAfterAfterGetRecord` is the documentation's
own, and it is what an extension writes.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

The six triggers together: **231 declarations**, against 6 971 for the base dataitem triggers they
bracket -- so about one dataitem in thirty is extended.

## The IST-state

No report generator, and `reportextension` is not among the object kinds with a writer (board:0034).

## The choice

Each of the three base call sites gains two lines under `if constexpr (requires ...)`, with the
per-extension member naming board:0234 describes -- the same mechanism the tableextension brackets
use, on a different object.

**`OnBeforeAfterGetRecord` runs per ROW**, so it inherits 0307's cost profile: at 3 613 base
declarations and one row per record, a bracket that is not `constexpr`-guarded costs every report
two calls per row.

## Ordering

Blocked on board:0063 and on board:0033's merge carrying more than one trigger of a name.

## Gate, and its negative control

An extension whose `OnBeforeAfterGetRecord` counts rows and whose `OnAfterAfterGetRecord` counts
again: both counts equal the base's row count, and the base trigger ran between them.

**The negative control is the order** -- a driver that runs both brackets before the base trigger
passes the counts and breaks every extension that reads what the base computed.

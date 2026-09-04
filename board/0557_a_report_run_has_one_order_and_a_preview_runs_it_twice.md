Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/devenv-report-triggers.md, developer/devenv-request-pages-for-reports.md
Verdict:  teilweise
Class:    activation

# A report run has one order, and a preview runs it twice

board:0302, board:0303, board:0306 and board:0308 each own ONE report trigger. **This is the order they
stand in**, and the documentation prints it as a listing rather than describing it -- so it is a
specification a gate can compare against line for line.

## The order, quoted

`ROOT` is the primary instance, `CHILD` the preview instance, `--PAGE` a trigger on the request page:

```
ROOT OnAfterSubstituteReport
ROOT OnInitReport
ROOT OnInit                        --PAGE
ROOT OnAfterHasCustomLayout
ROOT OnAfterGetPrinterName
ROOT OnOpenPage                    --PAGE
ROOT OnAfterGetCurrRecord          --PAGE
	CHILD OnInitReport             --PAGE
	CHILD OnInit                   --PAGE
	CHILD OnOpenPage               --PAGE
	CHILD OnClosePage              --PAGE
	CHILD OnPreReport
	CHILD OnAfterGetPaperTrayForReport
	CHILD MasterRecord - OnPreDataItem
		CHILD MasterRecord - OnAfterGetRecord 1
			CHILD MasterRecordLines - OnPreDataItem
				CHILD MasterRecordLines - OnAfterGetRecord 101
			CHILD MasterRecordLines - OnPostDataItem
		CHILD MasterRecord - OnPostDataItem
	CHILD OnAfterHasCustomLayout
	CHILD OnPostReport
	CHILD OnMergeDocumentReport
ROOT OnQueryClosePage              --PAGE
ROOT OnClosePage                   --PAGE
ROOT OnPreReport
ROOT OnAfterGetPaperTrayForReport
ROOT MasterRecord - OnPreDataItem
	ROOT MasterRecord - OnAfterGetRecord 1
		ROOT MasterRecordLines - OnPreDataItem
			ROOT MasterRecordLines - OnAfterGetRecord 101
		ROOT MasterRecordLines - OnPostDataItem
ROOT MasterRecord - OnPostDataItem
ROOT OnPostReport
ROOT OnMergeDocumentReport
```

**Five things follow, and four of them are not derivable from any single trigger page.**

1. **`OnInitReport` RUNS TWICE before the report is executed**, and the page says so outright: "before
   the report is even executed, the `OnInitReport` trigger has already run twice." An implementation
   that initialises once is wrong in a way no single-trigger test can see.
2. **The whole data-item walk runs TWICE** -- once in the child to build the preview, once in the root
   to produce the document. So every `OnAfterGetRecord` fires twice per record, and a trigger with a
   side effect (a counter, an insert into a temporary table) doubles it.
3. **The nesting IS the control flow, depth first.** "If there's an indented data item, its records
   are also processed. After the last record in the indented data item, the control returns ... to the
   next record of the data item on the next highest level." board:0549 established the dataset is a
   tree; this says the tree is walked, not flattened.
4. **`OnQueryClosePage` and `OnClosePage` fire on the ROOT AFTER the child has finished its entire
   run** -- so the request page is still open while a complete report executes underneath it.
5. **board:0546's `OnAfterSubstituteReport` is FIRST**, before `OnInitReport`. That item already
   records that it fires BEFORE the substitution despite its name; this fixes its position.

## Two flows, and the head-less one is missing triggers

| | visual request page | head-less |
|---|---|---|
| where | the primary instance the user sees | the child, a scheduled run, an XML parameter list, or `UseRequestPage = false` |
| `OnAfterGetCurrRecord` | fires | **does not** |
| `OnQueryClosePage` | fires | **does not** |
| lookup, validate, other page triggers | fire | **do not** |

> "Only VISIBLE user interface elements are transferred to the child instances through the request
> page XML data."

**So a hidden request-page control's value does not reach the run that produces the document.** That is
the mechanism behind the page's own list of things to avoid -- setting globals in page triggers,
changing globals in `OnQueryClose`, depending on instance methods called before invocation. Each is a
value that lives in the ROOT and is never transferred.

## The mode, and the default the page gets wrong

Multiple-preview versus preview & close is decided by two properties:

| | `AllowScheduling = true` | `AllowScheduling = false` |
|---|---|---|
| `SaveValues = true` | **multiple-preview** | preview & close |
| `SaveValues = false` | preview & close | preview & close |

**`devenv-report-triggers.md` says "By default, reports use the multiple-preview mode." Its own truth
table plus the property defaults say otherwise.** `devenv-allowscheduling-property.md`: "the default is
**true**." `devenv-savevalues-property.md`: **"the default is false."** `false` AND `true` lands in the
preview & close cell. **The default is preview & close**, and the contradiction is recorded rather than
resolved -- the property pages state a default each and the concept page states a conclusion, and a
conclusion drawn from two defaults is the thing that can be wrong.

It matters because the mode decides whether the child instance exists at all, and therefore whether
every trigger above runs once or twice.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`report <id>` **2 135** objects -- CLAUDE.md's 668 is the share `scope.json` admits, and the two
numbers are different questions. Properties by the standard `(^|[{;])\s*<Name>\s*=` pattern:

| | count | values |
|---|---:|---|
| `DataItemTableView =` | 7 710 | |
| `RequestFilterFields =` | 1 944 | |
| `SaveValues =` | 1 690 | **`true` 1 667**, `false` 23 |
| `DataItemLinkReference =` | 896 | |
| `ProcessingOnly =` | 767 | `true` 761, `false` 6 |
| `PrintOnlyIfDetail =` | 489 | |
| `RequestFilterHeading =` | 425 | |
| `UseRequestPage =` | 140 | **`false` 112**, `true` 28 |
| `ContextSensitiveHelpPage =` | 53 | |
| **`AllowScheduling =`** | **25** | **`false` 24, `true` 1** |

Triggers by `^\s*trigger\s+<Name>\s*\(`:

| trigger | count |
|---|---:|
| `OnAfterGetRecord` | 8 137 |
| `OnPreDataItem` | 5 482 |
| `OnPreReport` | 1 453 |
| `OnPostDataItem` | 1 284 |
| `OnInitReport` | 551 |
| `OnPostReport` | 536 |

**The mode census is decided by `SaveValues`, not by `AllowScheduling`.** 1 667 objects set
`SaveValues = true` and only 24 turn scheduling off, so **roughly 1 643 reports are in
multiple-preview mode and run everything twice**, while the ~445 objects that never declare
`SaveValues` take the `false` default and run once. The word "roughly" is doing work: the two
properties are counted independently and the 24 overlap is not resolved per object, so the figure is
an upper bound on the first group. **It is given as one rather than rounded into a fact.**

**`OnAfterGetRecord` at 8 137 is not separable from the page trigger of the same name**, which fires
on every list page; the pattern sees a `trigger` line and not its enclosing object. The report share
is the smaller part of it and no split is offered.

**`requestpage` sections count 2 267 against 2 135 reports**, because XMLports and report extensions
declare them too. Also not separable.

## What the request page is

- **A request page is shown by default**; `UseRequestPage = false` runs the report immediately and
  **"end users can't cancel the report"** -- 112 objects.
- **`RequestFilterFields` puts columns on the data item's FastTab.** 1 944 declarations.
- **A data item with NO `RequestFilterFields` and a `DataItemTableView` set gets NO FastTab at all.**
  So the tab's existence is derived from two properties rather than declared.
- **A source table with calculated fields adds a "Filter totals by:" section automatically** -- so the
  request page reads the table's FlowField dimensions, which board:0018's field classes decide.
- **The buttons depend on the layout type**, and that is a table, not a rule:

| button | RDLC | Word | Excel | ProcessingOnly |
|---|---|---|---|---|
| OK | | | | x |
| Cancel | x | x | x | x |
| Preview | x | x | | |
| Print | x | x | | |
| SendTo | x | x | x | |
| Download | | | x | |

## The IST-state

- **board:0302, 0303, 0306, 0308 exist and each names one trigger**; none names the sequence, and none
  mentions the child instance. This item is the constraint they are each a point on.
- **There is no report generator and no report node in the AST** -- board:0063's hole, the same shape
  as board:0556 found for queries.
- **`src/rt/` has no report run at all**, so there is no place the order is wrong; it is absent. That
  is why the verdict is `teilweise` rather than `fehlt`: four of the trigger points are already
  specified in the board, and what is missing is the order over them.

## The choice

**One `RunReport` function, and the order is its control flow rather than data.**

```cpp
class Report {
  void Run();                      // ROOT: substitute, init, request page, then Execute
  void Execute();                  // OnPreReport, walk, OnPostReport, merge
  void WalkDataItem(int index);    // recursive; the tree IS the loop nest
};
```

**Why control flow and not a table of trigger points:** the order has a NESTED shape -- a data item's
children run inside its record loop -- and a flat list of stages cannot express "return to the next
record of the next highest level". A recursive walk over board:0549's data-item tree is the same
statement as the documentation's own indentation.

**The child instance is a SECOND `Report` object over the same class**, constructed from the request
page's transferred values and run to completion before the root continues. Not a thread, not a fork:
the listing puts the entire child run strictly between two root triggers, so it is a nested call.
**And that is why the doubling must be visible rather than optimised away** -- a preview that reused
the root's data would not fire the child's triggers, and 1 643 reports are written against those
triggers firing.

**Only visible controls transfer.** The transfer is a `constexpr`-known subset of the request page's
controls -- board:0553's tree carries `Visible` -- so it is a filter over the layout and not a second
serialisation format.

**`static_assert` where it fits:** the mode is a `constexpr` function of two `constexpr` properties, so
`Mode(SaveValues, AllowScheduling)` is computed at translation time and the child instance's existence
is a compile-time fact per report. That removes a run-time branch from 2 135 objects.

## Ordering

**After board:0063's report object and board:0549's dataset** -- there is nothing to walk before the
tree exists. **Before board:0546's substitution and board:0547's layouts**, both of which are points in
this order.

**`OnAfterGetRecord` and `OnPreDataItem` first**, at 8 137 and 5 482; `OnInitReport` and `OnPostReport`
at ~540 each are last of the six. The child instance comes with the preview and not before it: the
document path runs the ROOT half of the listing and is already useful.

## Gate, and its negative control

A report with a master data item and one indented lines data item, run in multiple-preview mode, with
every trigger appending its name to a log:

1. the log EQUALS the listing above, line for line
2. `OnInitReport` appears **twice**
3. each record's `OnAfterGetRecord` appears twice -- once `CHILD`, once `ROOT`
4. the lines data item's triggers appear INSIDE the master's record loop, once per master record
5. with `SaveValues = false` the log has no `CHILD` lines at all and `OnInitReport` appears once

**The negative control is case 5, and it is the only one that catches a hardcoded child.** An
implementation that always runs the child passes 1 through 4 and produces every document twice --
which for a report that posts, numbers or writes is not a rendering defect but a duplicate.

**Second control, for case 4:** flatten the walk so the lines run after all master records. Case 4 goes
red; cases 1, 2, 3 and 5 stay green if the log is compared as a SET rather than a sequence. So the
comparison is ordered, and that is stated because a set comparison is the easy mistake here.

## Class

`activation`. No report runs today, so there is nothing to regress -- but every trigger in the listing
is AL code that has never executed, and 8 137 `OnAfterGetRecord` bodies starting to run is the largest
single activation in this board. The A/B is the whole UT suite and the classification is why: the
failure mode is not a wrong report, it is a posting routine reached through a report trigger that had
never been called.

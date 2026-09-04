Type:     task
Status:   open
Parent:   0063
Area:     rt, gen
Source:   developer/devenv-substituting-reports.md
Verdict:  fehlt
Class:    activation

# A report is substituted by an event before it runs

> "you can OVERRIDE the base report with your own customized version by subscribing to the
> **`OnAfterSubstituteReport`** event published by **Codeunit 44 -- ReportManagement**."
>
> ```AL
> [EventSubscriber(ObjectType::Codeunit, Codeunit::ReportManagement, 'OnAfterSubstituteReport', '', false, false)]
> local procedure OnSubstituteReport(ReportId: Integer; var NewReportId: Integer)
> ```
>
> **"NOTE: The event is called `OnAfterSubstituteReport` to match the pattern followed by other events
> in the `ReportManagement` codeunit, but THE SUBSCRIBER WILL BE INVOKED BEFORE THE SUBSTITUTION TAKES
> PLACE."**

**A documented naming lie, stated as one.** The event is named `OnAfter` and fires BEFORE -- because
the codeunit's other events are named that way. board:0512 lists `ReportManagement`'s events among the
"global" ones, so this is one of them, and its name does not describe when it fires.

**That matters for one reason**: an implementation that raised it after choosing the report would be
following the name and would make the substitution impossible.

## Four raise points, and they are named

> The event is raised when:
> 1. **the user activates a page action** whose `RunObject` is the report (board:0433);
> 2. **the report is invoked from Tell Me** (board:0466);
> 3. the report is called by the **static** `Report.Run` or `Report.RunModal`.

**So substitution happens at the INVOCATION, not at the object** -- three different call paths, each of
which must raise. A report called any other way is not substituted, which is why the list is exhaustive
rather than illustrative.

**And `var NewReportId: Integer` is board:0516's `var` pattern again**: the subscriber writes the
replacement id through a reference, and a copied `var` means every substitution is silently ignored and
the original report runs. **Nothing raises.** That is the third item in this sweep whose failure mode is
exactly that (board:0516, board:0543, this).

## The mechanism is legacy and the modern one is `reportextension`

> "In versions prior to 2021 release wave 1, extensibility isn't supported for report objects ...
> **From 2021 release wave 1, it's possible to extend reports** ... **Using a report extension, you
> might not need to use the report substitution feature.**"

board:0545 files `reportextension` as one of the five extension kinds. **Both mechanisms are live**, as
board:0452 found for the two layout generations -- so a report generator must support substitution AND
extension, and the BaseApp uses both.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0196: `[EventSubscriber]` **11 142**. **The count of subscribers to `OnAfterSubstituteReport`
specifically is a subscriber declaration, not a property** -- stated rather than guessed, and it is the
number that says whether substitution is reachable in phase 1.

## The IST-state

board:0063 records that reports have no generator; board:0057 that no event dispatch exists. So neither
half of this exists.

## The choice

The three invocation paths raise the event before resolving the report id, with `NewReportId` passed by
reference and initialised to the original -- **so a subscriber that does nothing leaves it unchanged**,
which is what the `var` semantics give for free and what a copy would break.

**No AL object name in `src/`**: codeunit 44 is found through the generated catalogue like every other
publisher (board:0512).

## Ordering

Behind board:0512's dispatch and board:0433's `RunObject`. Inside board:0063.

## Gate, and its negative control

A subscriber that maps report A to report B causes an action with `RunObject = Report A` to run B; a
`Report.Run(A)` does the same; with no subscriber, A runs.

**The negative control is the no-subscriber case** -- `NewReportId` must come back as A, and an
implementation that initialises it to zero or leaves it uninitialised runs nothing or runs report 0,
which every gate with a subscriber passes.

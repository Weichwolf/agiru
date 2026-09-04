Type:     task
Status:   open
Parent:   0039
Area:     rt, gen
Source:   developer/triggers-auto/codeunit/devenv-onaftertestrun-codeunit-trigger.md
Verdict:  fehlt
Class:    activation

# A test runner's `OnAfterTestRun` records the result, and runs after the case's boundary closed

`OnAfterTestRun` runs after each test codeunit, receives whether it succeeded and the permission
value, and is where an AL-side runner writes its result log.

**The ordering against the transaction boundary is the item.** board:0225's `TransactionModel`
decides what happens to the case's writes; this trigger must run AFTER that decision, or a runner
that logs its result into a table has the log rolled back with the case.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnAfterTestRun` declarations: **29**, alongside 0268's 31.

## The IST-state

Nothing calls it (`grep -rn "OnAfterTestRun" src/ include/`, 2026-09-04).

## The choice

`TestRunner` calls it after closing the case's boundary, with the documented parameters and the
success flag.

**And the runner's own writes need a boundary of their own.** board:0039 records that a test runner
UNDOES a `Commit`, so the result log cannot rely on one -- it is written outside the case's
boundary, which is what "after" has to mean here rather than merely "later in the function".

## Ordering

With 0268, and after board:0225 decides the boundary's fate.

## Gate, and its negative control

A case marked `AutoRollback` that inserts a row, and a runner whose `OnAfterTestRun` logs the
result: after the run the row is gone and the LOG ENTRY IS THERE.

**The negative control is the log entry** -- a trigger called inside the case's boundary loses it
along with the row, and a suite that only asserts the row is gone reports success.

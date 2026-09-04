Type:     task
Status:   open
Parent:   0090
Area:     rt, gen
Source:   developer/devenv-page-background-tasks.md
Verdict:  fehlt
Class:    activation

# A page background task is a read-only child session

board:0090 is "a session starts a child session and it is read-only". **This page is the page-side
form of that**, and it lists thirteen characteristics -- several of which are constraints a runtime
must enforce rather than features it may offer.

> A page background task is a **child session** that runs a codeunit in the background. When the
> process completes, the child session ends and **the parent session is notified with the results.**
>
> - **"Does READ-ONLY operations; it CAN'T WRITE TO OR LOCK the database."**
> - Runs on the **same server instance** as the parent session.
> - **"The parameters passed to and returned from a page background task are in the form of a
>   `dictionary<string, string>`."**
> - **"Calls `OpenCompany` and executes IN ITS OWN TRANSACTION."**
> - **"The callback triggers CAN'T EXECUTE UI OPERATIONS, except notifications and control
>   updates."**
> - **"If the calling page or session closes, OR THE CURRENT RECORD IS CHANGED, the background task
>   is CANCELED."**
> - **"It has a default and MAXIMUM TIMEOUT, which cancels the task automatically."**
> - **"Runs ISOLATED from the parent session. Apart from the completion and error triggers, IT CAN'T
>   CALL BACK."**
> - **"There's a LIMIT on the number of background tasks per session ... requests are QUEUED."**
> - **"EXECUTED SYNCHRONOUSLY FROM WEB SERVICES."**
> - Doesn't insert session event records; **not counted as part of the license calculation.**

**Four of these are the item.** The read-only rule is enforceable and must be enforced -- a background
task that writes is a task that holds a lock on a page's session, which is the thing the feature
exists to avoid. The cancellation on record change is what makes a cue correct: moving to another
customer must abandon the in-flight count rather than deliver it against the new record. The
dictionary-of-strings interface is the entire parameter surface. And the synchronous execution from
web services means the same codeunit runs two ways.

**And it is board:0047's other consumer**: a cue is a FlowField and the documentation names cues and
FactBoxes as the typical places for background tasks. So the expensive aggregate a role centre shows
is computed in a child session, not in the page's.

## The API, which is five methods and two triggers

| | |
|---|---|
| `EnqueueBackgroundTask` | queues it; completion fires `OnPageBackgroundTaskCompleted`, failure `OnPageBackgroundTaskError` |
| `GetBackgroundParameters` | reads the input dictionary, inside the task |
| `SetBackgroundTaskResult` | sets the result dictionary, inside the task |
| `CancelBackgroundTask` | **"ATTEMPT to cancel"** |
| `TestPage.RunPageBackgroundTask` | **runs it in the CURRENT session** -- the test seam |

**`TestPage.RunPageBackgroundTask` is why this is reachable in phase 2 at all**: a TestPage drives the
task synchronously, so the UI proof CLAUDE.md describes can exercise it without a real child session.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0295 and its neighbours measured the two triggers with the page-trigger family. The five methods
are builtins and belong to board:0028's census, not to this sweep's property pattern. **Stated rather
than guessed.**

## The IST-state

board:0090 records the child-session state: `StartSession` and its neighbours refuse the door.
`src/gen/PageWriter.cpp` consumes `SourceTable` alone, so neither trigger is emitted.

## The choice

A child session that borrows a connection for its own transaction (board:0012) and is **refused write
access at the runtime boundary, not by convention** -- `RuntimeInsert`, `RuntimeModify`,
`RuntimeDelete` and `LockTable` raise inside it. That is one flag on the session and four checks, and
it is the cheapest possible way to hold the strongest of the thirteen rules.

The parameter and result dictionaries are `Dictionary of [Text, Text]` (board:0078's reference-type
collections), which is what AL declares.

**The cancellation on record change belongs to board:0030's page**, not to the session: the page knows
its current record changed and cancels; the session only knows it was cancelled.

## Ordering

Behind board:0090's child session and board:0030's page lifecycle. `TestPage.RunPageBackgroundTask`
first, because it needs no child session and unblocks the tests.

## Gate, and its negative control

A background task that computes a cue's value delivers it to `OnPageBackgroundTaskCompleted`; the
same task attempting an `Insert` raises.

**The negative control is moving the current record while the task runs** -- the completion trigger
must NOT fire for the old record's result, and an implementation that delivers whatever comes back
shows the previous customer's count against the new customer.

Type: root
State: open
Area: rt

# A session starts a CHILD SESSION, and a page background task's is read-only by construction

Three items already name a missing background session as a dependency and none of them owns it:
board:0083 for "search company data", board:0063 for a scheduled report, board:0030 for
`TestPage.RunPageBackgroundTask`. This is that item.

## What the platform documents

`devenv-page-background-tasks.md` (read 2026-09-04, board:0071) specifies the page variant
completely, and its constraints are what make it implementable at all:

| | |
|---|---|
| **read-only** | "Does read-only operations; **it can't write to or lock the database**" |
| where | "Runs on the same Business Central Server instance as the parent session" |
| transaction | "Calls `OpenCompany` and **executes in its own transaction**" |
| parameters and results | a `Dictionary<Text, Text>`, both ways |
| what a callback may do | "can't execute UI operations, **except notifications and control updates**" |
| cancellation | "If the calling page or session closes **or the current record is changed**, the background task is canceled" |
| timeout | "has a **default and maximum timeout**, which cancels the task automatically" |
| isolation | "Apart from the completion and error triggers, **it can't call back to the parent session**" |
| back-pressure | "There's a **limit on the number of background tasks per session**. If there are more tasks than the threshold ... the requests are queued" |
| web services | **"Executed synchronously from web services"** |
| licensing and telemetry | not counted against the licence; "doesn't insert session event records; it relies on the parent session event records" |

The API is five methods and two triggers: `EnqueueBackgroundTask`, `GetBackgroundParameters`,
`SetBackgroundTaskResult`, `CancelBackgroundTask`, `RunPageBackgroundTask` (the TestPage one), with
`OnPageBackgroundTaskCompleted` and `OnPageBackgroundTaskError`.

**"Read-only, own transaction, dictionary in and dictionary out, no call back except two triggers"
is a specification a runtime can hold to.** It is not "a thread": it is a task with a closed
interface, and every one of those constraints removes a class of race rather than adding a
mechanism.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`, all `.al`

| | |
|---|---:|
| `CurrPage.EnqueueBackgroundTask` | 61 |
| `OnPageBackgroundTaskCompleted` / `...Error` triggers | 65 / 14 |
| `StartSession(` -- the general background session | **35** |
| `StopSession(` | 5 |
| `SessionId(` | 205 |
| `ActiveSession` (the platform table) | 296 |

**The general form is 35 call sites and the page form 61**, so neither is large -- but board:0083's
"search company data", board:0063's scheduled report and the whole Job Queue stand on the general
one, and `TestPage.RunPageBackgroundTask` stands on the page one.

## The choice, and the ORDER is the point

**The page variant comes FIRST and it is not a thread.** `RunPageBackgroundTask` is documented on
`TestPage` as running "the page background task codeunit **in the current session**" -- so the
milestone's need is satisfied by a SYNCHRONOUS call that honours the interface: build the parameter
dictionary, run the codeunit in a nested read-only transaction, collect the result dictionary, fire
`OnPageBackgroundTaskCompleted`. No session, no thread, no queue. **The documentation itself says
that is faithful**: "Executed synchronously from web services."

- `EnqueueBackgroundTask` from a real page then becomes the same call plus a queue, once there is a
  page runtime to be responsive for (board:0030).
- The read-only constraint is enforced rather than documented: the child runs with the transaction
  type `Browse` or `Snapshot` (board:0012), both of which the platform documents as read-only --
  "Modifications cannot occur within the transaction". So the guarantee comes from the isolation
  level that already has to exist, not from a check somebody has to remember.
- **`StartSession` is a different item's worth of work and is NOT taken on here.** It writes, it
  outlives its caller, and it needs the connection-per-transaction rule board:0012 owns. Naming the
  boundary is the point: the milestone needs the page form, and building the general one first would
  buy nothing the 2 291 need.

**What it costs**: one `Dictionary<Text, Text>` round trip and a nested transaction scope, both of
which exist. What it does NOT cost is a thread, which is why this is worth doing early.

## Gate, and its negative control

A codeunit that reads a count and returns it in the result dictionary, driven through
`RunPageBackgroundTask`, with `OnPageBackgroundTaskCompleted` observing the value. A second case
whose codeunit WRITES must fail, because the task is read-only.

**The negative control is the second**: run the child in the parent's own transaction type and it
succeeds, which is exactly the defect -- a background task that can write is a background task that
can deadlock against the page that started it.

Classification: **activation** -- nothing runs today; the 61 + 35 call sites do not translate.

## THERE ARE THREE KINDS OF BACKGROUND WORK, NOT ONE

`devenv-task-scheduler.md` and `devenv-job-queue.md` (read 2026-09-04, board:0071) name the other
two, and the layering is strict:

| kind | what it is | who owns it |
|---|---|---|
| **page background task** | a read-only child session bound to an open page | the platform -- this item |
| **`StartSession`** | a background session the caller starts directly | the platform, deferred |
| **task scheduler** | `TaskScheduler.CreateTask` / `SetTaskReady` / `TaskExists` / `CancelTask`, recorded in system table **2000000175 `Scheduled Task`** (board:0032) | the platform, deferred |
| **job queue** | "an abstraction that uses the **task scheduler** from the platform to enable end users to view, create, or modify jobs" | the BaseApp, transpiled |

**So the Job Queue is not platform work at all** -- it is AL over the task scheduler, and every item
that named "the job queue" as a dependency (board:0063's scheduled report, board:0083's data search)
actually depends on `TaskScheduler`.

The task scheduler's flow carries two rules worth recording before anyone builds it:

- **A failure ROLLS BACK the transaction and then retries**, in the same session, "until the task
  succeeds or until the maximum number of retries is exceeded". So a scheduled task is
  at-least-once, not exactly-once, and any AL it runs must be written for that -- which is BC's own
  design and not something to improve on.
- **A non-retriable failure with a FAILURE CODEUNIT terminates the session and starts a new one** to
  run the failure codeunit down the same path. Two sessions, in a declared order.

Neither is built here. They are named so the boundary between this item and board:0070's install
scope is drawn once.

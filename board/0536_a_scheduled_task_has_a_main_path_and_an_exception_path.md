Type:     task
Status:   open
Parent:   0090
Area:     rt, db
Source:   developer/devenv-task-scheduler.md, developer/devenv-job-queue.md, developer/devenv-async-overview.md
Verdict:  fehlt
Class:    activation

# A scheduled task has a main path and an exception path

**Three pages, one item**: the task scheduler, the job queue that sits on it, and the async overview
that names the three background mechanisms. board:0090 is "a session starts a child session and it is
read-only" -- and this is the OTHER kind of background session, which is not read-only.

> "a task is a codeunit or report that is **scheduled to run at a specific date and time**. Tasks run
> in a **BACKGROUND SESSION** ... **the task scheduler is used BY THE JOB QUEUE to process job queue
> entries.**"
>
> "When a task is created, the task is recorded in table **2000000175 Scheduled Task**."

**So the job queue is built on the task scheduler, and the task scheduler is built on a platform
table** -- three layers, and only the bottom one is the runtime's. board:0032 counts the platform
tables and 2000000175 is one of them.

## The retry model, and it is the item

> **Main path**: open the company, validate the task row, run the main codeunit. **On success the row
> is REMOVED** from 2000000175.
>
> **Exception path**: **"1. The TRANSACTION IS ROLLED BACK."** Then:
>
> - **retriable** -> **"the main codeunit is RERUN following the main path. This retry flow continues
>   IN THE SAME SESSION until the task succeeds or until the MAXIMUM NUMBER OF RETRIES is exceeded."**
> - **not retriable, no failure codeunit** -> the task fails.
> - **not retriable, with a failure codeunit** -> **"the current session is TERMINATED. A NEW SESSION
>   is started, and the FAILURE CODEUNIT runs in this session, following the main path."** If it fails
>   too, the exception path retries IT.

**Two codeunits, two sessions, two retry loops.** A failure codeunit is not an error handler in the
same session -- the session is destroyed and a new one runs it, which means it cannot see any state
the failed run left in memory.

**And the rollback is the first thing the exception path does**, so a retried task starts from the
committed state and CLAUDE.md's first invariant holds across the retry: a task that half-posted and
failed has posted nothing.

**"Retriable" is a classification of the exception** -- so the runtime must categorise errors into
retriable and not, which board:0055's error type does not currently carry. That is this item's main
piece of new metadata: an `ErrorInfo` (board:0518) gains a retriable flag, and the default for an AL
`Error()` has to be decided. **BC's answer is not on this page** and is recorded as an open question
rather than guessed.

## Four methods and a state

> `CreateTask(codeunit, failure codeunit, [ready], [company], [not-before], [record id], [timeout])` ·
> `SetTaskReady(task id, [not-before])` -- **"A task CAN'T RUN until it's Ready"** ·
> `TaskExists(task id)` · `CancelTask(task id)`

**A task is created not-ready by default and armed separately**, which is a two-phase commit over the
task row: create it inside the transaction, arm it after. That is what stops a task from running
against a transaction that later rolled back.

**`CreateTask` takes a `RecordId`**, so a task carries the record it is about -- which is how a job
queue entry finds its own row.

## What the page says about limits

> "there are limits on **how many child or background sessions, or scheduled tasks can run at the same
> time**" -- configurable on-premises.

board:0498 records the same for page background tasks. **Three background mechanisms, one shared
budget**, and agiru has none of the three.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`TaskScheduler` methods are method calls; board:0028 owns the census. **Stated rather than guessed** --
and the count of `CreateTask` call sites says whether the job queue is reachable in phase 1 or 3.

## The IST-state

board:0090 records the child-session state: `StartSession` and its neighbours refuse the door.
2000000175 is among board:0032's 87 ungenerated platform tables.

## The choice

The platform table, a scheduler loop, and a session per attempt -- **a real session, because the
failure codeunit's page says the previous one is terminated**, and reusing it would let state leak
between the two.

**The retriable classification lands on `ErrorInfo`** (board:0518), which already carries five
addressing fields; one more flag.

**Not a thread pool with reused workers.** CLAUDE.md's shape is many sessions not contending, per-session
arenas, `thread_local` for what belongs to one session -- and a task IS a session, so it gets one.

## Ordering

Behind board:0032's platform tables, board:0090's child sessions and board:0518's `ErrorInfo`. The job
queue is above this and is a BaseApp codeunit, not runtime work.

## Gate, and its negative control

A task created and set ready runs its codeunit in a background session and its row disappears; a task
whose codeunit raises a non-retriable error with a failure codeunit runs the failure codeunit **in a
different session**.

**The negative control is the session identity** -- a failure codeunit that can see the failed run's
in-memory state is running in the same session, which is what the documentation says must not happen,
and every gate that only checks the failure codeunit ran will pass.

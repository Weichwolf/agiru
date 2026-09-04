Type:     task
Status:   open
Parent:   0090
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-onpagebackgroundtaskcompleted-page-trigger.md, developer/triggers-auto/pageextension/devenv-onpagebackgroundtaskcompleted-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnPageBackgroundTaskCompleted` receives the child session's result dictionary

```al
trigger OnPageBackgroundTaskCompleted(TaskId: Integer; Results: Dictionary of [Text, Text])
```

It runs when a task started by `CurrPage.EnqueueBackgroundTask` finishes, and receives the
`Dictionary of [Text, Text]` the task set with `SetBackgroundTaskResult` (board:0090). `TaskId`
identifies which task, because a page may have several in flight.

**The callback may not do UI** -- board:0090 records the constraint from
`devenv-page-background-tasks.md`: "The callback triggers can't execute UI operations, except
notifications and control updates."

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnPageBackgroundTaskCompleted(` on a page or pageextension: **61 declarations**, matching
the 61 `CurrPage.EnqueueBackgroundTask` call sites board:0090 counts -- so essentially every task
that is started has a completion handler.

## The IST-state

No page runtime and no background task.

## The choice

board:0090 makes the milestone's path SYNCHRONOUS -- `TestPage.RunPageBackgroundTask` runs the
codeunit in the current session -- so this trigger is called immediately after the codeunit returns,
with the dictionary it filled. No thread, no queue, no waiting.

**The `TaskId` must still be real**, because a page with two tasks branches on it, and a synchronous
implementation that always passed 0 would collapse that branch.

## Ordering

Blocked on board:0090 and board:0030.

## Gate, and its negative control

A page that enqueues two tasks with different ids: each completion receives its own id and its own
dictionary.

**The negative control is the second task** -- a runtime that passes a constant id gives both
completions the same branch and passes any single-task test.

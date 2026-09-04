Type:     task
Status:   open
Parent:   0090
Area:     rt, gen
Source:   developer/triggers-auto/page/devenv-onpagebackgroundtaskerror-page-trigger.md, developer/triggers-auto/pageextension/devenv-onpagebackgroundtaskerror-pageextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page's `OnPageBackgroundTaskError` decides whether the task is retried or the error is shown

```al
trigger OnPageBackgroundTaskError(TaskId: Integer; ErrorCode: Text; ErrorText: Text;
                                  ErrorCallStack: Text; var IsHandled: Boolean)
```

It runs when a background task raises, and its `var IsHandled` follows the same convention
board:0057 records for the 180 075 other uses: setting it `true` means the page dealt with the
error and the platform must not surface it.

The three error fields are board:0055's: the code, the text and the call stack -- the same triple
`GetLastErrorCode`, `GetLastErrorText` and `GetLastErrorCallStack` return.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnPageBackgroundTaskError(` on a page or pageextension: **14 declarations**, against 61
completion handlers -- so most pages that start a task do not handle its failure, and the platform's
default surfacing is what they rely on.

## The IST-state

No page runtime and no background task. `GetLastErrorCode` and `GetLastErrorCallStack` are door
refusals, so even the parameters have no producer yet.

## The choice

Called when the task's codeunit raises, with the three error values and `IsHandled` initialised
`false`. **Unhandled means the platform surfaces the error**, which for a page means a notification
rather than a dialog -- the same "no UI except notifications" constraint 0290 carries.

## Ordering

Blocked on board:0090, board:0030 and board:0055 for the error triple.

## Gate, and its negative control

A task that raises with a handler setting `IsHandled := true`: nothing is surfaced. The same task
with no handler: the error reaches the page.

**The negative control is the second case** -- a runtime that swallows an unhandled background error
makes 47 pages' worth of failures invisible, which is the shape CLAUDE.md counts as a silent place.
